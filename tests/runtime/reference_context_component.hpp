//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : reference_context_component.hpp
// Description : Test-only reference Component that holds a ComponentContext and runs scripted plans.
//
// Component   : Kritva Core
// Module      : Component Context Test Support
// Layer       : Core Foundation
//
// Requirements: CORE-CTX-005
// API         : CORE-TEST-REFERENCE-CONTEXT-COMPONENT
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#pragma once

// TEST SUPPORT ONLY (never compiled into or installed with the production library).
//
// A conforming Component that holds a ComponentContext, built from its own identity in
// its constructor (the injection pattern of the contract), and whose lifecycle operations
// first run a SCRIPTED plan of steps through that context, then behave as the plain
// ReferenceComponent. It implements only public contracts and adds observation:
//   - plans: per operation, an ordered list of steps (require a service, use a service,
//     check requirements, query a capability);
//   - the first failing step ends the plan; its error is returned from the operation exactly
//     as a real integrator would return it: a Core availability error comes from the context
//     (already attributed to this component) and a platform service error is attributed with
//     context.attribute(); the component's lifecycle state machine moves exactly as a failed
//     operation of the plain ReferenceComponent does (the failing operation leaves FAULT);
//   - an ordered record of every executed step with its outcome;
//   - lifetime observation through a flag the test owns;
//   - the ReferenceComponent fault injection (fail_next_*) still works.
// Nothing here is a model of any real component.

#include <atomic>
#include <cstdint>
#include <utility>
#include <vector>

#include <kritva/core/core.hpp>

#include "../contract/reference_component.hpp"

namespace kritva::core::runtime::contract {

enum class ContextStep : std::uint8_t {
    REQUIRE_SCHEDULER, REQUIRE_CLOCK, REQUIRE_TIMER, REQUIRE_WATCHDOG,      // the context's typed queries
    CREATE_TASK, START_SCHEDULER, START_TIMER, START_WATCHDOG, KICK_WATCHDOG,
    STOP_WATCHDOG, STOP_TIMER, STOP_SCHEDULER,                              // use of a service through its own contract
    CHECK_REQUIREMENTS, EVALUATE_REQUIREMENTS,                              // the context's requirement binding
    QUERY_CAPABILITY, QUERY_SUPPORT                                         // the context's capability and support queries
};

/// One executed step and how it ended.
struct StepRecord {
    ContextStep step;
    bool ok;
    ErrorCode code;      // NONE when ok
    Id source;           // the source of the returned error (invalid when ok or not attributed)
};

struct ContextPlans {
    std::vector<ContextStep> configure, initialize, start, stop, shutdown;
};

class ReferenceContextComponent : public ReferenceComponent {
public:
    ReferenceContextComponent(ComponentInfo info, platform::PlatformContext platform, ContextPlans plans,
                              platform::PlatformRequirements needs = {}, bool* destroyed_flag = nullptr)
        : ReferenceComponent(std::move(info)), context_(*this, platform), plans_(std::move(plans)),
          needs_(std::move(needs)), destroyed_flag_(destroyed_flag) {}
    ~ReferenceContextComponent() override { if (destroyed_flag_ != nullptr) *destroyed_flag_ = true; }

    Result<void> configure(const Configuration& configuration) override {
        if (auto r = run(plans_.configure); !r) { fail_next_configure = r.error().code; (void)ReferenceComponent::configure(configuration); fail_next_configure = ErrorCode::NONE; return r; }
        return ReferenceComponent::configure(configuration);
    }
    Result<void> initialize() override {
        if (auto r = run(plans_.initialize); !r) { fail_next_initialize = r.error().code; (void)ReferenceComponent::initialize(); fail_next_initialize = ErrorCode::NONE; return r; }
        return ReferenceComponent::initialize();
    }
    Result<void> start() override {
        if (auto r = run(plans_.start); !r) { fail_next_start = r.error().code; (void)ReferenceComponent::start(); fail_next_start = ErrorCode::NONE; return r; }
        return ReferenceComponent::start();
    }
    Result<void> stop() override {
        if (auto r = run(plans_.stop); !r) { fail_next_stop = r.error().code; (void)ReferenceComponent::stop(); fail_next_stop = ErrorCode::NONE; return r; }
        return ReferenceComponent::stop();
    }
    Result<void> shutdown() override {
        if (auto r = run(plans_.shutdown); !r) { fail_next_shutdown = r.error().code; (void)ReferenceComponent::shutdown(); fail_next_shutdown = ErrorCode::NONE; return r; }
        return ReferenceComponent::shutdown();
    }

    [[nodiscard]] const ComponentContext& context() const noexcept { return context_; }
    [[nodiscard]] const std::vector<StepRecord>& executed() const noexcept { return executed_; }
    void clear_executed() { executed_.clear(); }
    [[nodiscard]] int task_runs() const noexcept { return runs_.load(); }

private:
    static void count(void* context) { if (context != nullptr) ++*static_cast<std::atomic<int>*>(context); }

    Result<void> run(const std::vector<ContextStep>& plan) {
        for (const ContextStep step : plan) {
            const Result<void> r = execute(step);
            executed_.push_back(StepRecord{step, r.has_value(), r ? ErrorCode::NONE : r.error().code, r ? Id{} : r.error().source});
            if (!r) return r;                                        // the first failing step ends the plan
        }
        return Result<void>::success();
    }

    // A Core availability error comes back from the context already attributed; a service's own Error is attributed here.
    template<class T> Result<void> from_query(const Result<T>& q) const {
        return q ? Result<void>::success() : Result<void>::failure(q.error());
    }
    Result<void> from_service(const Result<void>& r) const { return r ? r : Result<void>::failure(context_.attribute(r.error())); }
    template<class T> Result<void> from_service(const Result<T>& r) const { return r ? Result<void>::success() : Result<void>::failure(context_.attribute(r.error())); }

    Result<void> execute(ContextStep step) {
        switch (step) {
            case ContextStep::REQUIRE_SCHEDULER: return from_query(context_.require_scheduler());
            case ContextStep::REQUIRE_CLOCK:     return from_query(context_.require_clock());
            case ContextStep::REQUIRE_TIMER:     return from_query(context_.require_timer());
            case ContextStep::REQUIRE_WATCHDOG:  return from_query(context_.require_watchdog());
            case ContextStep::CREATE_TASK: {
                const auto s = context_.require_scheduler();
                if (!s) return Result<void>::failure(s.error());
                return from_service(s.value()->create_task(platform::TaskConfig{"ctx", 0, 0, Duration::from_milliseconds(1)}, &count, &runs_));
            }
            case ContextStep::START_SCHEDULER: {
                const auto s = context_.require_scheduler();
                if (!s) return Result<void>::failure(s.error());
                return from_service(s.value()->start());
            }
            case ContextStep::START_TIMER: {
                const auto t = context_.require_timer();
                if (!t) return Result<void>::failure(t.error());
                return from_service(t.value()->start(Duration::from_milliseconds(5), time::TimerMode::PERIODIC, Callback{&count, &runs_}));
            }
            case ContextStep::START_WATCHDOG: {
                const auto w = context_.require_watchdog();
                if (!w) return Result<void>::failure(w.error());
                return from_service(w.value()->start(Duration::from_milliseconds(1000)));
            }
            case ContextStep::KICK_WATCHDOG: {
                const auto w = context_.require_watchdog();
                if (!w) return Result<void>::failure(w.error());
                return from_service(w.value()->kick());
            }
            case ContextStep::STOP_WATCHDOG: {
                const auto w = context_.require_watchdog();
                if (!w) return Result<void>::failure(w.error());
                return from_service(w.value()->stop());
            }
            case ContextStep::STOP_TIMER: {
                const auto t = context_.require_timer();
                if (!t) return Result<void>::failure(t.error());
                return from_service(t.value()->stop());
            }
            case ContextStep::STOP_SCHEDULER: {
                const auto s = context_.require_scheduler();
                if (!s) return Result<void>::failure(s.error());
                return from_service(s.value()->stop());
            }
            case ContextStep::CHECK_REQUIREMENTS:    return context_.check_required(needs_);
            case ContextStep::EVALUATE_REQUIREMENTS: (void)context_.evaluate(needs_); return Result<void>::success();
            case ContextStep::QUERY_CAPABILITY:      (void)context_.has_capability(CapabilityId{100}); return Result<void>::success();
            case ContextStep::QUERY_SUPPORT:         (void)context_.supports(platform::PlatformService::TIMER); return Result<void>::success();
        }
        return Result<void>::success();
    }

    const ComponentContext context_;
    ContextPlans plans_;
    platform::PlatformRequirements needs_;
    bool* destroyed_flag_;
    std::vector<StepRecord> executed_;
    std::atomic<int> runs_{0};
};

} // namespace kritva::core::runtime::contract
