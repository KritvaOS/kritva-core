//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : reference_platform_test.cpp
// Description : Tests of the test-only reference platform and its deterministic controls.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-016
// API         : CORE-TEST-REFERENCE-PLATFORM-RUN
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#include <atomic>
#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>

#include <kritva/core/core.hpp>

#include "../platform/adapter_conformance.hpp"
#include "../platform/reference_platform.hpp"

using namespace kritva::core;
using namespace kritva::core::platform;
using namespace kritva::core::platform::testing;
using time::TimerMode;

namespace {

constexpr std::int64_t MS = 1'000'000;

struct Counter { std::atomic<int> value{0}; };
void count(void* context) { if (context != nullptr) ++static_cast<Counter*>(context)->value; }

CapabilitySet capabilities_100_101() {
    CapabilitySet set;
    set.add(Capability{CapabilityId{100}, "gpio", Version{1, 0, 0}});
    set.add(Capability{CapabilityId{101}, "can-bus", Version{2, 1, 0}});
    return set;
}

ReferencePlatform::Config config(ReferencePlatform::Provides provides = {}) {
    ReferencePlatform::Config c;
    c.provides = provides;
    c.capabilities = capabilities_100_101();
    return c;
}

// ---- the reference adapter -------------------------------------------------------------------------

void test_the_reference_platform_implements_only_public_contracts() {
    static_assert(std::is_base_of_v<IPlatformAdapter, ReferencePlatform>);
    static_assert(std::is_base_of_v<IScheduler, FakeScheduler>);
    static_assert(std::is_base_of_v<time::IClock, FakeClock>);
    static_assert(std::is_base_of_v<time::ITimer, FakeTimer>);
    static_assert(std::is_base_of_v<IWatchdog, FakeWatchdog>);
    static_assert(!std::is_copy_constructible_v<ReferencePlatform> && !std::is_move_constructible_v<ReferencePlatform>);
    ReferencePlatform platform(config());
    const IPlatformAdapter& adapter = platform;                          // everything below goes through the public contract
    assert(adapter.info().name == "reference-platform" && adapter.info().version == (Version{1, 0, 0}));
}

void test_service_availability_is_selectable_for_every_combination() {
    for (int mask = 0; mask < 16; ++mask) {
        const ReferencePlatform::Provides provides{(mask & 1) != 0, (mask & 2) != 0, (mask & 4) != 0, (mask & 8) != 0};
        ReferencePlatform platform(config(provides));
        const IPlatformAdapter& adapter = platform;
        assert(adapter.supports(PlatformService::SCHEDULER) == provides.scheduler && (adapter.scheduler() != nullptr) == provides.scheduler);
        assert(adapter.supports(PlatformService::CLOCK) == provides.clock && (adapter.clock() != nullptr) == provides.clock);
        assert(adapter.supports(PlatformService::TIMER) == provides.timer && (adapter.timer() != nullptr) == provides.timer);
        assert(adapter.supports(PlatformService::WATCHDOG) == provides.watchdog && (adapter.watchdog() != nullptr) == provides.watchdog);
        assert(adapter.scheduler() == adapter.scheduler() && adapter.timer() == adapter.timer());    // stable for the adapter's life
        if (provides.scheduler) assert(adapter.scheduler() == static_cast<IScheduler*>(&platform.fake_scheduler()));   // the fakes are the services
    }
}

void test_capabilities_and_identity_are_selectable_and_deterministic() {
    ReferencePlatform::Config c = config();
    c.info = PlatformInfo{"custom", Version{9, 8, 7}};
    c.capabilities = CapabilitySet{};
    c.capabilities.add(Capability{CapabilityId{500}, "x", Version{}});
    ReferencePlatform platform(c);
    assert(platform.info().name == "custom" && platform.info().version == (Version{9, 8, 7}));
    assert(PlatformContext(platform).has_capability(CapabilityId{500}) && !PlatformContext(platform).has_capability(CapabilityId{100}));
    const CapabilitySet a = platform.capabilities(), b = platform.capabilities();
    assert(a.size() == b.size() && a.all()[0].id == b.all()[0].id);
    ReferencePlatform none{ReferencePlatform::Config{}};
    assert(none.capabilities().empty());
}

void test_adapter_queries_are_counted_and_service_calls_are_not_adapter_queries() {
    ReferencePlatform platform(config());
    assert(platform.controls().adapter_queries() == 0);
    (void)platform.info(); (void)platform.scheduler(); (void)platform.clock(); (void)platform.timer(); (void)platform.watchdog(); (void)platform.capabilities();
    assert(platform.controls().adapter_queries() == 6);
    const std::size_t before = platform.controls().adapter_queries();
    assert(platform.fake_watchdog().start(Duration::from_milliseconds(10)));      // using a service is not a query to the adapter
    assert(platform.controls().adapter_queries() == before);
    (void)platform.supports(PlatformService::TIMER);                              // supports() queries the accessor once
    assert(platform.controls().adapter_queries() == before + 1);
}

// ---- fault injection ---------------------------------------------------------------------------------

void test_fault_injection_is_deterministic_atomic_and_logged_for_the_scheduler() {
    ReferencePlatform platform(config());
    Controls& controls = platform.controls();
    FakeScheduler& scheduler = platform.fake_scheduler();
    controls.fail_nth(Method::SCHEDULER_CREATE_TASK, 2, ErrorCode::RESOURCE_UNAVAILABLE);
    Counter counter;
    const auto first = scheduler.create_task(TaskConfig{}, &count, &counter);
    assert(first && first.value() == 1);
    const auto second = scheduler.create_task(TaskConfig{}, &count, &counter);              // the injected failure
    assert(!second && second.error().code == ErrorCode::RESOURCE_UNAVAILABLE && second.error().message == "injected failure");
    const auto third = scheduler.create_task(TaskConfig{}, &count, &counter);               // atomic: no id and no slot was consumed
    assert(third && third.value() == 2);
    assert(scheduler.task_count() == 2);
    assert(controls.calls(Method::SCHEDULER_CREATE_TASK) == 3);                            // injected calls are counted
    const auto& log = controls.log();
    assert(log.size() == 3 && log[0].ok && !log[1].ok && log[1].code == ErrorCode::RESOURCE_UNAVAILABLE && log[2].ok);

    controls.fail_all(Method::SCHEDULER_START, ErrorCode::TIMEOUT);
    assert(!scheduler.start() && !scheduler.running());                                    // atomic: not started
    assert(!scheduler.start() && controls.calls(Method::SCHEDULER_START) == 2);
    assert(controls.log_size(Method::SCHEDULER_START) == 2 && !controls.log()[controls.log().size() - 1].ok);   // injected failures are logged with their code
    assert(controls.log()[controls.log().size() - 1].code == ErrorCode::TIMEOUT && controls.log()[controls.log().size() - 2].method == Method::SCHEDULER_START);
    controls.clear_faults();
    assert(scheduler.start() && scheduler.running());                                      // faults are removable
    controls.fail_nth(Method::SCHEDULER_STOP, 1, ErrorCode::INTERNAL_ERROR);
    assert(!scheduler.stop() && scheduler.running());                                      // atomic: still running
    assert(scheduler.stop() && !scheduler.running());                                      // the next call is not injected
}

void test_fault_injection_for_timer_and_watchdog_is_atomic() {
    ReferencePlatform platform(config());
    Controls& controls = platform.controls();
    FakeTimer& timer = platform.fake_timer();
    FakeWatchdog& watchdog = platform.fake_watchdog();
    Counter counter;

    controls.fail_nth(Method::TIMER_START, 1, ErrorCode::RESOURCE_UNAVAILABLE);
    assert(!timer.start(Duration::from_milliseconds(10), TimerMode::ONE_SHOT, Callback{&count, &counter}) && !timer.running());
    assert(timer.start(Duration::from_milliseconds(10), TimerMode::ONE_SHOT, Callback{&count, &counter}) && timer.running());
    controls.fail_all(Method::TIMER_STOP, ErrorCode::INTERNAL_ERROR);
    assert(!timer.stop() && timer.running());                                              // a failed stop leaves it running
    platform.let_time_pass(Duration::from_milliseconds(10));
    assert(counter.value == 1);                                                            // and it keeps working
    controls.clear_faults();

    controls.fail_nth(Method::WATCHDOG_START, 1, ErrorCode::UNSUPPORTED);
    assert(!watchdog.start(Duration::from_milliseconds(50)) && !watchdog.running());
    assert(watchdog.start(Duration::from_milliseconds(50)) && watchdog.running());
    controls.fail_nth(Method::WATCHDOG_KICK, 1, ErrorCode::TIMEOUT);
    assert(!watchdog.kick());
    assert(watchdog.kick());                                                               // the window restarts only on a real kick
    controls.fail_nth(Method::WATCHDOG_STOP, 3, ErrorCode::INTERNAL_ERROR);               // counts since construction: stop #3
    assert(watchdog.stop() && !watchdog.running());                                        // stop #1
    assert(watchdog.stop());                                                               // stop #2
    assert(!watchdog.stop());                                                              // stop #3 is the injected one
    assert(controls.calls(Method::WATCHDOG_START) == 2 && controls.calls(Method::WATCHDOG_KICK) == 2 && controls.calls(Method::WATCHDOG_STOP) == 3);
}

void test_reset_counts_restarts_the_nth_numbering() {
    ReferencePlatform platform(config());
    Controls& controls = platform.controls();
    (void)platform.fake_watchdog().start(Duration::from_milliseconds(10));
    (void)platform.fake_watchdog().stop();
    controls.clear_faults();
    controls.reset_counts();
    assert(controls.calls(Method::WATCHDOG_START) == 0 && controls.log().size() == 2);     // the log is kept
    controls.fail_nth(Method::WATCHDOG_START, 1, ErrorCode::TIMEOUT);                     // numbering restarted
    assert(!platform.fake_watchdog().start(Duration::from_milliseconds(10)));
    controls.clear_log();
    assert(controls.log().empty());
}

// ---- deterministic services -------------------------------------------------------------------------

void test_the_fake_clock_is_controllable_and_keeps_its_domain() {
    ReferencePlatform platform(config());
    FakeClock& clock = platform.fake_clock();
    assert(clock.now() == Timestamp(0, ClockDomain::MONOTONIC));
    clock.advance(5 * MS);
    assert(clock.now() == Timestamp(5 * MS, ClockDomain::MONOTONIC));
    clock.advance(-3);                                                                    // time never goes backward by advance()
    assert(clock.now().nanoseconds() == 5 * MS);
    assert(clock.set(7 * MS) && clock.now().nanoseconds() == 7 * MS);
    assert(!clock.set(1 * MS) && clock.now().nanoseconds() == 7 * MS);                    // a MONOTONIC clock refuses to go backward
    platform.let_time_pass(Duration::from_milliseconds(3));
    assert(clock.now().nanoseconds() == 10 * MS);                                         // only explicit time advancement moves it

    ReferencePlatform::Config realtime = config();
    realtime.clock_domain = ClockDomain::REALTIME;
    ReferencePlatform wall(realtime);
    assert(wall.fake_clock().set(100) && wall.fake_clock().set(50));                      // a REALTIME clock may step backward
    assert(wall.fake_clock().now() == Timestamp(50, ClockDomain::REALTIME) && wall.fake_clock().now().domain() == ClockDomain::REALTIME);
}

void test_nothing_advances_by_itself() {
    ReferencePlatform platform(config());
    Counter runs;
    assert(platform.fake_scheduler().create_task(TaskConfig{"t", 0, 0, Duration::from_milliseconds(1)}, &count, &runs));
    assert(platform.fake_scheduler().start());
    assert(platform.fake_timer().start(Duration::from_milliseconds(5), TimerMode::PERIODIC, Callback{&count, &runs}));
    assert(platform.fake_watchdog().start(Duration::from_milliseconds(10)));
    assert(runs.value == 0 && platform.fake_clock().now().nanoseconds() == 0);             // started, yet nothing has run
    platform.let_time_pass(Duration::from_milliseconds(5));
    assert(runs.value == 5 + 1);
    platform.let_time_pass(Duration::from_milliseconds(30));                            // far longer than the 10 ms watchdog timeout
    assert(!platform.fake_watchdog().expired());                                          // let_time_pass never advances the watchdog                                                          // 5 task ticks + 1 timer firing, caused only by let_time_pass
    assert(!platform.fake_watchdog().expired());                                          // the watchdog is advanced only explicitly
    platform.fake_watchdog().advance(10 * MS);
    assert(platform.fake_watchdog().expired());
}

// ---- callbacks are observable ------------------------------------------------------------------------

void test_callbacks_and_task_entries_are_logged_in_order() {
    ReferencePlatform platform(config());
    Controls& controls = platform.controls();
    Counter runs;
    assert(platform.fake_scheduler().create_task(TaskConfig{"t", 0, 0, Duration::from_milliseconds(1)}, &count, &runs));
    assert(platform.fake_scheduler().start());
    assert(platform.fake_timer().start(Duration::from_milliseconds(2), TimerMode::ONE_SHOT, Callback{&count, &runs}));
    platform.let_time_pass(Duration::from_milliseconds(2));
    assert(controls.log_size(Method::TASK_ENTRY) == 2 && controls.log_size(Method::TIMER_CALLBACK) == 1);
    assert(runs.value == 3);                                                              // the user callbacks really ran
    // The log is ordered: creation, start, timer start, then entries and the callback.
    const auto& log = controls.log();
    assert(log[0].method == Method::SCHEDULER_CREATE_TASK && log[1].method == Method::SCHEDULER_START && log[2].method == Method::TIMER_START);
    assert(log[3].method == Method::TASK_ENTRY && log[4].method == Method::TASK_ENTRY && log[5].method == Method::TIMER_CALLBACK);
    // After stop() no entry or callback is logged.
    assert(platform.fake_scheduler().stop());
    const std::size_t entries = controls.log_size(Method::TASK_ENTRY);
    platform.let_time_pass(Duration::from_milliseconds(10));
    assert(controls.log_size(Method::TASK_ENTRY) == entries && controls.log_size(Method::TIMER_CALLBACK) == 1);
}

struct Reenter { FakeTimer* timer{nullptr}; ErrorCode stop_code{ErrorCode::NONE}; };
void reenter(void* raw) {
    auto* r = static_cast<Reenter*>(raw);
    const auto stopped = r->timer->stop();
    r->stop_code = stopped ? ErrorCode::NONE : stopped.error().code;
}

void test_re_entrant_use_from_a_callback_is_observable() {
    ReferencePlatform platform(config());
    Reenter reentrant{&platform.fake_timer()};
    assert(platform.fake_timer().start(Duration::from_milliseconds(2), TimerMode::ONE_SHOT, Callback{&reenter, &reentrant}));
    platform.let_time_pass(Duration::from_milliseconds(2));
    assert(reentrant.stop_code == ErrorCode::INVALID_STATE);                              // the R0.4 contract rule, visible to the test
    const auto& log = platform.controls().log();
    bool saw_failed_stop = false;
    for (const Record& r : log) if (r.method == Method::TIMER_STOP && !r.ok && r.code == ErrorCode::INVALID_STATE) saw_failed_stop = true;
    assert(saw_failed_stop);                                                              // and it is in the call log with its outcome
}

// ---- lifetime ------------------------------------------------------------------------------------------

void test_lifetime_is_observable_and_owned_by_the_integrator() {
    LifetimeProbe probe;
    ReferencePlatform::Config c = config();
    c.probe = &probe;
    auto platform = std::make_unique<ReferencePlatform>(c);
    {
        const PlatformContext context(*platform);
        const PlatformContext copy = context;
        (void)copy.require_timer();
        (void)copy.require_watchdog();
    }                                                                                      // contexts are gone
    assert(!probe.platform_destroyed && probe.services_destroyed == 0);                    // a view never destroys anything
    platform.reset();                                                                      // the integrator ends the platform
    assert(probe.platform_destroyed && probe.services_destroyed == 4);                    // the platform and its four services
}

// ---- the platform is a conforming one, and the suite notices injected faults ----------------------------

void test_the_reference_platform_passes_the_conformance_suite_for_every_combination() {
    for (int mask = 0; mask < 16; ++mask) {
        const ReferencePlatform::Provides provides{(mask & 1) != 0, (mask & 2) != 0, (mask & 4) != 0, (mask & 8) != 0};
        ReferencePlatform platform(config(provides));
        conformance::Report report;
        conformance::check_platform_adapter(platform, platform.environment(), report);
        assert(report.ok());
        assert(report.checks() > 0);
    }
}

void test_injected_faults_are_detected_by_the_conformance_suite() {
    {
        ReferencePlatform platform(config());
        platform.controls().fail_all(Method::TIMER_START, ErrorCode::INTERNAL_ERROR);   // a code the contract does not permit for a valid start
        conformance::Report report;
        conformance::check_platform_adapter(platform, platform.environment(), report);
        assert(!report.ok());
    }
    {
        ReferencePlatform platform(config());
        platform.controls().fail_nth(Method::WATCHDOG_START, 3, ErrorCode::UNSUPPORTED);   // the first VALID start (calls 1 and 2 are the invalid timeouts): a permitted capability limit, a skip
        conformance::Report report;
        conformance::check_platform_adapter(platform, platform.environment(), report);
        assert(report.ok() && !report.skips().empty());
    }
    {
        ReferencePlatform platform(config());
        platform.controls().fail_all(Method::WATCHDOG_START, ErrorCode::UNSUPPORTED);   // masks argument validation too: INVALID_ARGUMENT is mandatory for a zero timeout
        conformance::Report report;
        conformance::check_platform_adapter(platform, platform.environment(), report);
        assert(!report.ok());
    }
    {
        ReferencePlatform platform(config());
        platform.controls().fail_nth(Method::SCHEDULER_CREATE_TASK, 1, ErrorCode::INTERNAL_ERROR);
        conformance::Report report;
        conformance::check_platform_adapter(platform, platform.environment(), report);
        assert(!report.ok());
    }
}

void test_a_rejected_timer_start_never_disturbs_the_running_activation() {
    ReferencePlatform platform(config());
    Counter first, second;
    assert(platform.fake_timer().start(Duration::from_milliseconds(2), TimerMode::PERIODIC, Callback{&count, &first}));
    const auto rejected = platform.fake_timer().start(Duration::from_milliseconds(3), TimerMode::ONE_SHOT, Callback{&count, &second});
    assert(!rejected && rejected.error().code == ErrorCode::INVALID_STATE);                // already running
    platform.let_time_pass(Duration::from_milliseconds(4));
    assert(first.value == 2 && second.value == 0);                                         // the original activation and callback are intact
    assert(platform.fake_timer().running());
}

void test_replay_is_deterministic() {
    auto run = [] {
        ReferencePlatform platform(config());
        platform.controls().fail_nth(Method::TIMER_START, 2, ErrorCode::RESOURCE_UNAVAILABLE);
        Counter c;
        (void)platform.fake_timer().start(Duration::from_milliseconds(2), TimerMode::PERIODIC, Callback{&count, &c});
        platform.let_time_pass(Duration::from_milliseconds(7));
        (void)platform.fake_timer().stop();
        (void)platform.fake_timer().start(Duration::from_milliseconds(2), TimerMode::PERIODIC, Callback{&count, &c});
        (void)platform.fake_timer().start(Duration::from_milliseconds(2), TimerMode::PERIODIC, Callback{&count, &c});
        std::string trace;
        for (const Record& r : platform.controls().log()) trace += std::to_string(static_cast<int>(r.method)) + (r.ok ? "+" : "-") + std::to_string(static_cast<int>(r.code)) + " ";
        return trace + std::to_string(c.value);
    };
    assert(run() == run());                                                                // the same inputs always give the same log and results
}

} // namespace

int main() {
    test_the_reference_platform_implements_only_public_contracts();
    test_service_availability_is_selectable_for_every_combination();
    test_capabilities_and_identity_are_selectable_and_deterministic();
    test_adapter_queries_are_counted_and_service_calls_are_not_adapter_queries();
    test_fault_injection_is_deterministic_atomic_and_logged_for_the_scheduler();
    test_fault_injection_for_timer_and_watchdog_is_atomic();
    test_reset_counts_restarts_the_nth_numbering();
    test_the_fake_clock_is_controllable_and_keeps_its_domain();
    test_nothing_advances_by_itself();
    test_callbacks_and_task_entries_are_logged_in_order();
    test_re_entrant_use_from_a_callback_is_observable();
    test_lifetime_is_observable_and_owned_by_the_integrator();
    test_the_reference_platform_passes_the_conformance_suite_for_every_combination();
    test_injected_faults_are_detected_by_the_conformance_suite();
    test_a_rejected_timer_start_never_disturbs_the_running_activation();
    test_replay_is_deterministic();
    return 0;
}
