//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : platform_test.cpp
// Description : Kritva Core — Platform Contract Tests
//               These tests validate only the contracts exposed by the current Platform
//               headers. No Linux/RTOS/CPU-affinity/timing/scheduling behavior is assumed.
//
// Component   : Kritva Core
// Module      : PLATFORM
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-001 : IScheduler
//               CORE-PLAT-002 : platform::IClock adapter contract
//               CORE-PLAT-003 : IWatchdog
// API         : CORE-TEST-PLATFORM
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <string>
#include <vector>

#include "kritva/core/platform/clock.hpp"
#include "kritva/core/platform/scheduler.hpp"
#include "kritva/core/platform/watchdog.hpp"

#include "../contract/scheduler_contract.hpp"

using namespace kritva::core;
using namespace kritva::core::platform;

namespace {

// -----------------------------------------------------------------------------
// Test doubles
// -----------------------------------------------------------------------------

class FakeClock final : public IClock {
public:
    explicit FakeClock(Timestamp timestamp) noexcept
        : timestamp_(timestamp) {}

    [[nodiscard]] Timestamp now() const noexcept override {
        return timestamp_;
    }

private:
    Timestamp timestamp_{};
};

// Reference scheduler that conforms to the IScheduler contract (scheduler.hpp).
// It is a test double, not an implementation: `start()` runs each task's entry
// once, synchronously, so the tests can observe invocation.
class FakeScheduler final : public IScheduler {
public:
    // Adapter-specific policy and limits (see scheduler.hpp: priority range,
    // affinity, capacity and dynamic creation are adapter properties, not
    // Core properties).
    bool allow_dynamic_creation{false};  // policy (b): reject while RUNNING
    static constexpr std::uint32_t kMaxPriority = 255;
    static constexpr std::uint32_t kCpuMask = 0xF;  // logical CPUs 0..3
    std::size_t capacity{4};
    bool affinity_supported{true};
    std::size_t fail_start_at{static_cast<std::size_t>(-1)};  // task index

    Result<TaskId> create_task(
        const TaskConfig& config,
        void (*entry)(void*),
        void* context) override {

        ++create_count;

        if (running && !allow_dynamic_creation) return fail(ErrorCode::INVALID_STATE);
        if (entry == nullptr || config.name == nullptr ||
            config.period.nanoseconds() < 0 || config.priority > kMaxPriority) {
            return fail(ErrorCode::INVALID_ARGUMENT);
        }
        if (config.cpu_affinity != 0) {
            if (!affinity_supported) return fail(ErrorCode::UNSUPPORTED);
            if ((config.cpu_affinity & kCpuMask) == 0) return fail(ErrorCode::INVALID_ARGUMENT);
        }
        if (tasks.size() >= capacity) return fail(ErrorCode::RESOURCE_UNAVAILABLE);

        last_config = config;
        last_entry = entry;
        last_context = context;
        tasks.push_back({entry, context});
        if (running) entry(context);  // policy (a): joins the running set

        return Result<TaskId>::success(next_task_id++);
    }

    Result<void> start() override {
        ++start_count;
        if (running) return Result<void>::success();  // idempotent

        for (std::size_t i = 0; i < tasks.size(); ++i) {
            if (i == fail_start_at) {
                // All-or-nothing: roll back tasks already started.
                started_then_stopped += i;
                Error error;
                error.code = ErrorCode::RESOURCE_UNAVAILABLE;
                return Result<void>::failure(error);
            }
        }
        for (const auto& task : tasks) task.entry(task.context);
        running = true;
        return Result<void>::success();
    }

    Result<void> stop() override {
        ++stop_count;
        running = false;  // idempotent
        return Result<void>::success();
    }

    TaskId next_task_id{1};
    TaskConfig last_config{};
    void (*last_entry)(void*){nullptr};
    void* last_context{nullptr};

    std::uint32_t create_count{0};
    std::uint32_t start_count{0};
    std::uint32_t stop_count{0};
    std::size_t started_then_stopped{0};
    bool running{false};

private:
    struct Task { void (*entry)(void*); void* context; };
    std::vector<Task> tasks;

    static Result<TaskId> fail(ErrorCode code) {
        Error error;
        error.code = code;
        return Result<TaskId>::failure(error);
    }
};

class FakeWatchdog final : public IWatchdog {
public:
    Result<void> start(Duration timeout) override {
        ++start_count;
        last_timeout = timeout;
        running = true;
        return Result<void>::success();
    }

    Result<void> kick() override {
        ++kick_count;
        return Result<void>::success();
    }

    Result<void> stop() override {
        ++stop_count;
        running = false;
        return Result<void>::success();
    }

    Duration last_timeout{};
    std::uint32_t start_count{0};
    std::uint32_t kick_count{0};
    std::uint32_t stop_count{0};
    bool running{false};
};

void task_entry(void* context) {
    if (context != nullptr) {
        *static_cast<int*>(context) += 1;
    }
}

// -----------------------------------------------------------------------------
// CORE-PLAT-002 — Clock adapter
// -----------------------------------------------------------------------------

void test_platform_clock_is_core_clock_alias() {
    // platform::IClock is the same type as the canonical time::IClock, not a
    // second contract: pointers/references convert without adaptation and the
    // alias adds no members.
    static_assert(std::is_same_v<IClock, kritva::core::time::IClock>);
    static_assert(std::is_same_v<platform::IClock, kritva::core::time::IClock>);
    static_assert(sizeof(platform::IClock) == sizeof(kritva::core::time::IClock));

    const FakeClock fake(Timestamp{9, ClockDomain::REALTIME});
    const platform::IClock& via_alias = fake;
    const kritva::core::time::IClock& via_canonical = via_alias;
    assert(via_canonical.now() == fake.now());
    assert(via_canonical.now().domain() == ClockDomain::REALTIME);
}

void test_platform_clock_contract() {
    static_assert(std::is_abstract_v<IClock>);
    static_assert(std::is_polymorphic_v<IClock>);
    static_assert(std::has_virtual_destructor_v<IClock>);

    const Timestamp expected{
        123456789LL,
        ClockDomain::MONOTONIC
    };

    const FakeClock clock(expected);

    assert(clock.now() == expected);
    assert(clock.now().nanoseconds() == 123456789LL);
    assert(clock.now().domain() == ClockDomain::MONOTONIC);
}

// -----------------------------------------------------------------------------
// CORE-PLAT-001 — Scheduler
// -----------------------------------------------------------------------------

void test_scheduler_contract_shape() {
    static_assert(std::is_abstract_v<IScheduler>);
    static_assert(std::is_polymorphic_v<IScheduler>);
    static_assert(std::has_virtual_destructor_v<IScheduler>);

    static_assert(std::is_same_v<TaskId, std::uint64_t>);
}

void test_scheduler_task_config_defaults() {
    const TaskConfig config;

    assert(config.name != nullptr);
    assert(std::string(config.name) == "kritva");
    assert(config.priority == 0);
    assert(config.cpu_affinity == 0);
    assert(config.period == Duration{});
}

void test_scheduler_create_task() {
    FakeScheduler scheduler;
    int context_value = 0;

    TaskConfig config;
    config.name = "sense-task";
    config.priority = 10;
    config.cpu_affinity = 2;
    config.period = Duration::from_milliseconds(10);

    const Result<TaskId> result =
        scheduler.create_task(config, task_entry, &context_value);

    assert(result);
    assert(result.has_value());
    assert(result.value() == 1);
    assert(scheduler.create_count == 1);

    assert(scheduler.last_config.name == config.name);
    assert(scheduler.last_config.priority == config.priority);
    assert(scheduler.last_config.cpu_affinity == config.cpu_affinity);
    assert(scheduler.last_config.period == config.period);

    assert(scheduler.last_entry == task_entry);
    assert(scheduler.last_context == &context_value);

    // The scheduler contract does not state that create_task executes entry.
    assert(context_value == 0);
}

void test_scheduler_create_task_returns_distinct_ids() {
    FakeScheduler scheduler;

    TaskConfig config;

    const Result<TaskId> first =
        scheduler.create_task(config, task_entry, nullptr);
    const Result<TaskId> second =
        scheduler.create_task(config, task_entry, nullptr);

    assert(first);
    assert(second);
    assert(first.value() != second.value());
    assert(first.value() == 1);
    assert(second.value() == 2);
}

void test_scheduler_start_stop() {
    FakeScheduler scheduler;

    assert(scheduler.start());
    assert(scheduler.running);
    assert(scheduler.start_count == 1);

    assert(scheduler.stop());
    assert(!scheduler.running);
    assert(scheduler.stop_count == 1);
}

void test_scheduler_polymorphic_access() {
    FakeScheduler implementation;
    IScheduler& scheduler = implementation;

    TaskConfig config;

    const Result<TaskId> task =
        scheduler.create_task(config, task_entry, nullptr);

    assert(task);
    assert(task.value() == 1);
    assert(scheduler.start());
    assert(scheduler.stop());
}


// Documented defaults and the meaning of the zero values.
void test_scheduler_zero_value_semantics() {
    const TaskConfig config;
    // cpu_affinity == 0 means "no constraint", period == 0 means aperiodic,
    // priority == 0 is the least urgent level (scheduler.hpp).
    assert(config.cpu_affinity == 0);
    assert(config.period.nanoseconds() == 0);
    assert(config.priority == 0);
}

void test_scheduler_reference_conforms_to_contract() {
    FakeScheduler scheduler;
    contract::check_scheduler_contract(scheduler);
    assert(!scheduler.running);
}

void test_scheduler_create_does_not_start_or_run_entry() {
    FakeScheduler scheduler;
    int runs = 0;
    const TaskConfig config;

    assert(scheduler.create_task(config, task_entry, &runs));
    assert(runs == 0);
    assert(!scheduler.running);

    assert(scheduler.start());
    assert(runs == 1);
    assert(scheduler.start());   // idempotent: entry not started again
    assert(runs == 1);
    assert(scheduler.stop());
    assert(scheduler.stop());    // idempotent
}

void test_scheduler_affinity_semantics() {
    FakeScheduler scheduler;
    TaskConfig config;

    config.cpu_affinity = 0;      // no constraint
    assert(scheduler.create_task(config, task_entry, nullptr));
    config.cpu_affinity = 0x1;    // pin to CPU 0
    assert(scheduler.create_task(config, task_entry, nullptr));

    config.cpu_affinity = 0x10;   // malformed: selects only a non-existent CPU
    auto bad = scheduler.create_task(config, task_entry, nullptr);
    assert(!bad && bad.error().code == ErrorCode::INVALID_ARGUMENT);

    // Well-formed request on a platform without affinity: UNSUPPORTED (not
    // INVALID_ARGUMENT, which is reserved for a malformed mask).
    FakeScheduler no_affinity;
    no_affinity.affinity_supported = false;
    config.cpu_affinity = 0x1;
    auto unsupported = no_affinity.create_task(config, task_entry, nullptr);
    assert(!unsupported && unsupported.error().code == ErrorCode::UNSUPPORTED);
    config.cpu_affinity = 0;      // zero is always acceptable
    assert(no_affinity.create_task(config, task_entry, nullptr));
}

// Priority range handling is adapter policy; this double documents rejecting
// values it cannot represent. Core defines no maximum.
void test_scheduler_adapter_priority_policy() {
    FakeScheduler scheduler;
    TaskConfig config;
    config.priority = FakeScheduler::kMaxPriority;
    assert(scheduler.create_task(config, task_entry, nullptr));
    config.priority = FakeScheduler::kMaxPriority + 1;
    auto r = scheduler.create_task(config, task_entry, nullptr);
    assert(!r && r.error().code == ErrorCode::INVALID_ARGUMENT);
    config.priority = std::numeric_limits<std::uint32_t>::max();
    assert(!scheduler.create_task(config, task_entry, nullptr));
}

void test_scheduler_resource_exhaustion() {
    FakeScheduler scheduler;
    scheduler.capacity = 2;
    const TaskConfig config;

    const auto a = scheduler.create_task(config, task_entry, nullptr);
    const auto b = scheduler.create_task(config, task_entry, nullptr);
    assert(a && b);

    const auto full = scheduler.create_task(config, task_entry, nullptr);
    assert(!full);
    assert(full.error().code == ErrorCode::RESOURCE_UNAVAILABLE);

    // The failed call consumed no id and left the scheduler usable.
    assert(scheduler.start());
    assert(scheduler.stop());
}

void test_scheduler_start_failure_is_all_or_nothing() {
    FakeScheduler scheduler;
    scheduler.fail_start_at = 1;
    int runs = 0;
    const TaskConfig config;
    assert(scheduler.create_task(config, task_entry, &runs));
    assert(scheduler.create_task(config, task_entry, &runs));

    const auto r = scheduler.start();
    assert(!r);
    assert(r.error().code == ErrorCode::RESOURCE_UNAVAILABLE);
    assert(!scheduler.running);   // remains STOPPED
    assert(runs == 0);            // no entry left running
}

void test_scheduler_dynamic_creation_policies() {
    int runs = 0;
    const TaskConfig config;

    // Policy (b): rejected atomically with INVALID_STATE.
    FakeScheduler rejecting;
    assert(rejecting.create_task(config, task_entry, &runs));
    assert(rejecting.start());
    assert(runs == 1);
    const auto r = rejecting.create_task(config, task_entry, &runs);
    assert(!r && r.error().code == ErrorCode::INVALID_STATE);
    assert(rejecting.running);   // state unchanged
    assert(runs == 1);           // nothing created or run
    assert(rejecting.stop());
    const auto after = rejecting.create_task(config, task_entry, &runs);
    assert(after && after.value() == 2);  // failed call consumed no id

    // Policy (a): accepted; the task joins the running set.
    FakeScheduler dynamic;
    dynamic.allow_dynamic_creation = true;
    runs = 0;
    assert(dynamic.create_task(config, task_entry, &runs));
    assert(dynamic.start());
    assert(runs == 1);
    const auto added = dynamic.create_task(config, task_entry, &runs);
    assert(added && added.value() != 0);
    assert(runs == 2);
    assert(dynamic.running);

    // The shared conformance checks accept either policy.
    FakeScheduler dynamic_ref;
    dynamic_ref.allow_dynamic_creation = true;
    contract::check_scheduler_contract(dynamic_ref);
}

void test_scheduler_ids_nonzero_and_unique() {
    FakeScheduler scheduler;
    TaskConfig config;
    config.name = nullptr;  // rejected: must not consume an id
    assert(!scheduler.create_task(config, task_entry, nullptr));
    config.name = "t";
    const auto first = scheduler.create_task(config, task_entry, nullptr);
    assert(first && first.value() != 0);
    scheduler.stop();
    const auto second = scheduler.create_task(config, task_entry, nullptr);
    assert(second && second.value() != 0 && second.value() != first.value());
}

// -----------------------------------------------------------------------------
// CORE-PLAT-003 — Watchdog
// -----------------------------------------------------------------------------

void test_watchdog_contract_shape() {
    static_assert(std::is_abstract_v<IWatchdog>);
    static_assert(std::is_polymorphic_v<IWatchdog>);
    static_assert(std::has_virtual_destructor_v<IWatchdog>);
}

void test_watchdog_start() {
    FakeWatchdog watchdog;

    const Duration timeout = Duration::from_milliseconds(100);
    const Result<void> result = watchdog.start(timeout);

    assert(result);
    assert(result.has_value());
    assert(watchdog.start_count == 1);
    assert(watchdog.running);
    assert(watchdog.last_timeout == timeout);
}

void test_watchdog_kick() {
    FakeWatchdog watchdog;

    assert(watchdog.start(Duration::from_milliseconds(100)));
    assert(watchdog.running);

    const Result<void> result = watchdog.kick();

    assert(result);
    assert(result.has_value());
    assert(watchdog.kick_count == 1);
    assert(watchdog.running);
}

void test_watchdog_stop() {
    FakeWatchdog watchdog;

    assert(watchdog.start(Duration::from_milliseconds(100)));
    assert(watchdog.running);

    const Result<void> result = watchdog.stop();

    assert(result);
    assert(result.has_value());
    assert(watchdog.stop_count == 1);
    assert(!watchdog.running);
}

void test_watchdog_zero_timeout_transport() {
    // The current IWatchdog contract does not define timeout validation.
    // Therefore this only verifies that Duration(0) is accepted by the
    // interface and transported to the implementation.
    FakeWatchdog watchdog;

    const Duration zero = Duration::from_nanoseconds(0);
    const Result<void> result = watchdog.start(zero);

    assert(result);
    assert(watchdog.last_timeout == zero);
}

void test_watchdog_polymorphic_access() {
    FakeWatchdog implementation;
    IWatchdog& watchdog = implementation;

    assert(watchdog.start(Duration::from_milliseconds(100)));
    assert(watchdog.kick());
    assert(watchdog.stop());

    assert(implementation.start_count == 1);
    assert(implementation.kick_count == 1);
    assert(implementation.stop_count == 1);
}

} // namespace

int main() {
    // CORE-PLAT-002
    test_platform_clock_is_core_clock_alias();
    test_platform_clock_contract();

    // CORE-PLAT-001
    test_scheduler_contract_shape();
    test_scheduler_task_config_defaults();
    test_scheduler_create_task();
    test_scheduler_create_task_returns_distinct_ids();
    test_scheduler_start_stop();
    test_scheduler_polymorphic_access();
    test_scheduler_zero_value_semantics();
    test_scheduler_reference_conforms_to_contract();
    test_scheduler_create_does_not_start_or_run_entry();
    test_scheduler_affinity_semantics();
    test_scheduler_adapter_priority_policy();
    test_scheduler_dynamic_creation_policies();
    test_scheduler_resource_exhaustion();
    test_scheduler_start_failure_is_all_or_nothing();
    test_scheduler_ids_nonzero_and_unique();

    // CORE-PLAT-003
    test_watchdog_contract_shape();
    test_watchdog_start();
    test_watchdog_kick();
    test_watchdog_stop();
    test_watchdog_zero_timeout_transport();
    test_watchdog_polymorphic_access();

    return 0;
}
