//==============================================================================
// Kritva Core — Platform Contract Tests
// SPDX-License-Identifier: Apache-2.0
//
// Requirements:
//   CORE-PLAT-001 : IScheduler
//   CORE-PLAT-002 : platform::IClock adapter contract
//   CORE-PLAT-003 : IWatchdog
//
// These tests validate only the contracts exposed by the current Platform
// headers. No Linux/RTOS/CPU-affinity/timing/scheduling behavior is assumed.
//==============================================================================

#include <cassert>
#include <cstdint>
#include <type_traits>
#include <string>

#include "kritva/core/platform/clock.hpp"
#include "kritva/core/platform/scheduler.hpp"
#include "kritva/core/platform/watchdog.hpp"

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

class FakeScheduler final : public IScheduler {
public:
    Result<TaskId> create_task(
        const TaskConfig& config,
        void (*entry)(void*),
        void* context) override {

        ++create_count;
        last_config = config;
        last_entry = entry;
        last_context = context;

        return Result<TaskId>::success(next_task_id++);
    }

    Result<void> start() override {
        ++start_count;
        running = true;
        return Result<void>::success();
    }

    Result<void> stop() override {
        ++stop_count;
        running = false;
        return Result<void>::success();
    }

    TaskId next_task_id{1};
    TaskConfig last_config{};
    void (*last_entry)(void*){nullptr};
    void* last_context{nullptr};

    std::uint32_t create_count{0};
    std::uint32_t start_count{0};
    std::uint32_t stop_count{0};
    bool running{false};
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
    static_assert(std::is_same_v<IClock, kritva::core::time::IClock>);
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

    // CORE-PLAT-003
    test_watchdog_contract_shape();
    test_watchdog_start();
    test_watchdog_kick();
    test_watchdog_stop();
    test_watchdog_zero_timeout_transport();
    test_watchdog_polymorphic_access();

    return 0;
}
