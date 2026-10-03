//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : reference_scheduler.hpp
// Description : Reference IScheduler implementing the documented contract (test double).
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-005
// API         : CORE-TEST-SCHEDULER-REFERENCE
//
// Author      : KritvaOS Core Team
// Created     : 04-10-2026
//==============================================================================

#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <vector>

#include <kritva/core/platform/boundary.hpp>
#include <kritva/core/platform/scheduler.hpp>

namespace kritva::core::platform::contract {

/// A conforming IScheduler built only on Core types. It is a test double, not a
/// scheduler: tasks run inline when start() or tick() is called, which makes the
/// contract rules observable without threads or an operating system. Adapter-defined
/// behavior (capacity, dynamic creation, priority range, affinity) is configured
/// through Policy so tests can show both sides of each adapter policy.
class ReferenceScheduler : public IScheduler {
public:
    struct Policy {
        std::size_t capacity{4};
        bool dynamic_creation{false};            // adapter policy (a) when true, (b) when false
        std::uint32_t max_priority{255};          // values above are INVALID_ARGUMENT (adapter policy)
        std::uint32_t cpu_mask{0xF};              // logical CPUs 0..3 exist
        bool affinity_supported{true};
        std::size_t fail_start_at{static_cast<std::size_t>(-1)};   // task index whose activation fails
    };
    Policy policy;

    Result<TaskId> create_task(const TaskConfig& config, void (*entry)(void*), void* context) override {
        ++create_calls;
        if (entry == nullptr) return fail<TaskId>(ErrorCode::INVALID_ARGUMENT, "entry is null");
        if (config.name == nullptr) return fail<TaskId>(ErrorCode::INVALID_ARGUMENT, "name is null");
        if (config.period.nanoseconds() < 0) return fail<TaskId>(ErrorCode::INVALID_ARGUMENT, "period is negative");
        if (config.priority > policy.max_priority) return fail<TaskId>(ErrorCode::INVALID_ARGUMENT, "priority not representable");
        if (config.cpu_affinity != 0) {
            if (!policy.affinity_supported) return fail<TaskId>(ErrorCode::UNSUPPORTED, "affinity unsupported");
            if ((config.cpu_affinity & policy.cpu_mask) == 0) return fail<TaskId>(ErrorCode::INVALID_ARGUMENT, "mask selects no CPU");
        }
        if (running_ && !policy.dynamic_creation) return fail<TaskId>(ErrorCode::INVALID_STATE, "dynamic creation unsupported");
        if (tasks_.size() >= policy.capacity) return fail<TaskId>(ErrorCode::RESOURCE_UNAVAILABLE, "no task slots");

        // Every check passed: only now is state changed (atomicity) and an id consumed.
        const TaskId id = ++last_id_;
        tasks_.push_back(Task{id, entry, context, config.period.nanoseconds() > 0, false});
        if (running_) dispatch(tasks_.back());                        // dynamic: joins the running set
        return Result<TaskId>::success(id);
    }

    Result<void> start() override {
        ++start_calls;
        if (running_) return Result<void>::success();                 // idempotent
        for (std::size_t i = 0; i < tasks_.size(); ++i) {
            if (i == policy.fail_start_at) {
                return fail<void>(ErrorCode::RESOURCE_UNAVAILABLE, "task could not be activated");   // all-or-nothing
            }
        }
        running_ = true;
        for (Task& task : tasks_) {
            if (!task.periodic) dispatch(task);                       // aperiodic: once per start()
        }
        return Result<void>::success();
    }

    Result<void> stop() override {
        ++stop_calls;
        if (in_entry_) return fail<void>(ErrorCode::INVALID_STATE, "stop() from a task entry");
        running_ = false;                                             // idempotent
        return Result<void>::success();
    }

    /// Simulate one period elapsing: activate every periodic task that is not still executing.
    void tick() {
        if (!running_) return;
        for (Task& task : tasks_) {
            if (!task.periodic) continue;
            if (task.executing) { ++overruns_skipped; continue; }     // overrun policy: skip, never overlap
            dispatch(task);
        }
    }

    [[nodiscard]] bool running() const noexcept { return running_; }
    [[nodiscard]] std::size_t task_count() const noexcept { return tasks_.size(); }
    [[nodiscard]] std::uint64_t invocations(TaskId id) const {
        const auto it = invocations_.find(id);
        return it == invocations_.end() ? 0 : it->second;
    }

    int create_calls{0}, start_calls{0}, stop_calls{0};
    int overruns_skipped{0};

private:
    struct Task { TaskId id; void (*entry)(void*); void* context; bool periodic; bool executing; };

    template<class T> static Result<T> fail(ErrorCode code, const char* message) {
        return Result<T>::failure(make_error(code, message));
    }

    void dispatch(Task& task) {
        task.executing = true;
        in_entry_ = true;
        task.entry(task.context);
        in_entry_ = false;
        task.executing = false;
        ++invocations_[task.id];
    }

    std::vector<Task> tasks_;
    std::map<TaskId, std::uint64_t> invocations_;
    TaskId last_id_{0};
    bool running_{false};
    bool in_entry_{false};
};

} // namespace kritva::core::platform::contract
