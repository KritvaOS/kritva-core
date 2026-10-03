//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : platform_service_test.cpp
// Description : Explicit platform service consumption (PlatformContext::require_*) contract tests.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-014
// API         : CORE-TEST-PLATFORM-SERVICE
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#include <atomic>
#include <cassert>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>

#include <kritva/core/core.hpp>

#include "../contract/reference_adapter.hpp"

using namespace kritva::core;
using namespace kritva::core::platform;
using kritva::core::platform::contract::ReferenceAdapter;
using kritva::core::platform::contract::ReferenceScheduler;
using kritva::core::platform::contract::ReferenceWatchdog;
using kritva::core::time::contract::ReferenceTimer;
using kritva::core::time::TimerMode;

namespace {

constexpr std::int64_t MS = 1'000'000;

// Records every accessor call, so "requiring is a query and nothing else" is observable.
class SpyAdapter final : public ReferenceAdapter {
public:
    explicit SpyAdapter(Provides provides = {}) : ReferenceAdapter(PlatformInfo{"spy", Version{1, 0, 0}}, provides) {}
    [[nodiscard]] IScheduler* scheduler() const noexcept override { ++calls; return ReferenceAdapter::scheduler(); }
    [[nodiscard]] time::IClock* clock() const noexcept override { ++calls; return ReferenceAdapter::clock(); }
    [[nodiscard]] time::ITimer* timer() const noexcept override { ++calls; return ReferenceAdapter::timer(); }
    [[nodiscard]] IWatchdog* watchdog() const noexcept override { ++calls; return ReferenceAdapter::watchdog(); }
    [[nodiscard]] CapabilitySet capabilities() const override { ++calls; return ReferenceAdapter::capabilities(); }
    ReferenceScheduler& sched() const { return *static_cast<ReferenceScheduler*>(ReferenceAdapter::scheduler()); }
    ReferenceTimer& tim() const { return *static_cast<ReferenceTimer*>(ReferenceAdapter::timer()); }
    ReferenceWatchdog& dog() const { return *static_cast<ReferenceWatchdog*>(ReferenceAdapter::watchdog()); }
    mutable int calls{0};
};

struct Counter { std::atomic<int> value{0}; };
void count(void* context) { if (context != nullptr) ++static_cast<Counter*>(context)->value; }

void expect_unsupported(const Error& error, const char* service, const char* situation) {
    assert(error.code == ErrorCode::UNSUPPORTED);
    assert(error.severity == ErrorSeverity::ERROR);
    assert(!error.source.valid());                                           // Core is the source, not a component
    assert(error.message.find(service) != std::string::npos);
    assert(error.message.find(situation) != std::string::npos);
    for (const char* other : {"scheduler", "clock", "timer", "watchdog"}) {  // and it names only that service
        if (std::string(other) != service) assert(error.message.find(other) == std::string::npos);
    }
}

void test_shape() {
    static_assert(std::is_same_v<decltype(std::declval<const PlatformContext&>().require_scheduler()), Result<IScheduler*>>);
    static_assert(std::is_same_v<decltype(std::declval<const PlatformContext&>().require_clock()), Result<time::IClock*>>);
    static_assert(std::is_same_v<decltype(std::declval<const PlatformContext&>().require_timer()), Result<time::ITimer*>>);
    static_assert(std::is_same_v<decltype(std::declval<const PlatformContext&>().require_watchdog()), Result<IWatchdog*>>);
    static_assert(sizeof(PlatformContext) == sizeof(void*));                  // still one pointer: no state was added
    static_assert(std::is_trivially_destructible_v<PlatformContext>);
}

void test_scheduler_access() {
    SpyAdapter adapter;
    const PlatformContext context(adapter);
    const auto required = context.require_scheduler();
    assert(required.has_value() && required.value() != nullptr);
    assert(required.value() == context.scheduler());                          // the same adapter-owned object
    assert(required.value() == static_cast<IScheduler*>(&adapter.sched()));
    // The service is used through its own contract; Core did not start it.
    assert(!adapter.sched().running());
    Counter counter;
    const auto id = required.value()->create_task(TaskConfig{}, &count, &counter);
    assert(id && id.value() != 0);
    assert(required.value()->start());
    assert(adapter.sched().running() && counter.value == 1);
    assert(required.value()->stop());
}

void test_clock_access() {
    SpyAdapter adapter;
    const PlatformContext context(adapter);
    const auto required = context.require_clock();
    assert(required.has_value() && required.value() == context.clock());
    assert(required.value()->now().domain() == ClockDomain::MONOTONIC);
}

void test_timer_access() {
    SpyAdapter adapter;
    const PlatformContext context(adapter);
    const auto required = context.require_timer();
    assert(required.has_value() && required.value() == context.timer());
    assert(!adapter.tim().running());                                         // obtaining the timer did not start it
    Counter counter;
    assert(required.value()->start(Duration::from_milliseconds(10), TimerMode::ONE_SHOT, Callback{&count, &counter}));
    adapter.tim().advance(10 * MS);
    assert(counter.value == 1);
    assert(required.value()->stop());
}

void test_watchdog_access() {
    SpyAdapter adapter;
    const PlatformContext context(adapter);
    const auto required = context.require_watchdog();
    assert(required.has_value() && required.value() == context.watchdog());
    assert(!adapter.dog().running());                                         // obtaining the watchdog did not start it
    assert(required.value()->start(Duration::from_milliseconds(100)));
    assert(required.value()->kick());
    assert(required.value()->stop());
}

void test_unavailable_services_are_deterministic_unsupported_results() {
    // Unattached: every service is unavailable.
    const PlatformContext none;
    expect_unsupported(none.require_scheduler().error(), "scheduler", "no platform is attached");
    expect_unsupported(none.require_clock().error(), "clock", "no platform is attached");
    expect_unsupported(none.require_timer().error(), "timer", "no platform is attached");
    expect_unsupported(none.require_watchdog().error(), "watchdog", "no platform is attached");
    assert(!none.require_scheduler().has_value());

    // Attached, but the platform does not provide the service.
    ReferenceAdapter bare(PlatformInfo{"bare", Version{}}, {false, false, false, false});
    const PlatformContext context(bare);
    expect_unsupported(context.require_scheduler().error(), "scheduler", "not provided by the platform");
    expect_unsupported(context.require_clock().error(), "clock", "not provided by the platform");
    expect_unsupported(context.require_timer().error(), "timer", "not provided by the platform");
    expect_unsupported(context.require_watchdog().error(), "watchdog", "not provided by the platform");

    // Each service is independent: only the missing one is unavailable.
    ReferenceAdapter no_timer(PlatformInfo{"p", Version{}}, {true, true, false, true});
    const PlatformContext partial(no_timer);
    assert(partial.require_scheduler() && partial.require_clock() && partial.require_watchdog());
    assert(!partial.require_timer() && partial.require_timer().error().code == ErrorCode::UNSUPPORTED);

    // Deterministic: the same answer every time.
    assert(partial.require_timer().error().message == partial.require_timer().error().message);
    assert(none.require_clock().error().message == none.require_clock().error().message);
}

void test_every_service_combination_matches_the_adapter() {
    for (int mask = 0; mask < 16; ++mask) {
        const ReferenceAdapter::Provides provides{(mask & 1) != 0, (mask & 2) != 0, (mask & 4) != 0, (mask & 8) != 0};
        ReferenceAdapter adapter(PlatformInfo{"p", Version{}}, provides);
        const PlatformContext context(adapter);
        assert(context.require_scheduler().has_value() == provides.scheduler && context.require_scheduler().has_value() == context.supports(PlatformService::SCHEDULER));
        assert(context.require_clock().has_value() == provides.clock && context.require_clock().has_value() == context.supports(PlatformService::CLOCK));
        assert(context.require_timer().has_value() == provides.timer && context.require_timer().has_value() == context.supports(PlatformService::TIMER));
        assert(context.require_watchdog().has_value() == provides.watchdog && context.require_watchdog().has_value() == context.supports(PlatformService::WATCHDOG));
        if (provides.scheduler) assert(context.require_scheduler().value() == adapter.scheduler());   // success is never nullptr
    }
}

void test_requiring_is_a_query_with_no_side_effects() {
    SpyAdapter adapter;
    const PlatformContext context(adapter);
    assert(adapter.calls == 0);
    (void)context.require_scheduler();
    assert(adapter.calls == 1);                                               // exactly one accessor call per require
    (void)context.require_clock();
    (void)context.require_timer();
    (void)context.require_watchdog();
    assert(adapter.calls == 4);
    // Nothing was started, stopped, configured or created by asking.
    assert(adapter.activations() == 0);
    assert(adapter.sched().create_calls == 0 && adapter.sched().start_calls == 0 && adapter.sched().stop_calls == 0);
    assert(adapter.tim().start_calls == 0 && adapter.tim().stop_calls == 0);
    assert(adapter.dog().start_calls == 0 && adapter.dog().kick_calls == 0 && adapter.dog().stop_calls == 0);
    assert(!adapter.sched().running() && !adapter.tim().running() && !adapter.dog().running());

    // The failing path does not touch services either.
    SpyAdapter bare({false, false, false, false});
    const PlatformContext none_provided(bare);
    (void)none_provided.require_scheduler();
    (void)none_provided.require_watchdog();
    assert(bare.calls == 2);
}

void test_nothing_is_started_or_stopped_implicitly_and_state_survives_the_context() {
    SpyAdapter adapter;
    assert(adapter.watchdog()->start(Duration::from_milliseconds(100)));      // the integrator starts the watchdog itself
    adapter.calls = 0;
    {
        const PlatformContext context(adapter);
        const PlatformContext copy = context;
        (void)copy.require_watchdog();
        (void)copy.require_scheduler();
    }                                                                         // every context is gone
    assert(adapter.dog().running());                                          // the context neither stopped it ...
    assert(adapter.dog().stop_calls == 0 && adapter.dog().kick_calls == 0 && adapter.dog().start_calls == 1);
    assert(!adapter.sched().running());                                       // ... nor started anything else
}

void test_platform_service_errors_are_never_translated() {
    SpyAdapter adapter;
    adapter.tim().policy.resource_available = false;
    const PlatformContext context(adapter);
    const auto timer = context.require_timer();
    assert(timer.has_value());                                                // the service exists; it is the service that fails
    const auto started = timer.value()->start(Duration::from_milliseconds(10), TimerMode::ONE_SHOT, Callback{&count, nullptr});
    assert(!started);
    assert(started.error().code == ErrorCode::RESOURCE_UNAVAILABLE);          // the service's own code, severity, source and message
    assert(started.error().severity == ErrorSeverity::ERROR);
    assert(!started.error().source.valid());
    assert(started.error().message == "no timer resource");

    adapter.dog().policy.max_timeout_ns = 10 * MS;
    const auto watchdog = context.require_watchdog();
    const auto too_long = watchdog.value()->start(Duration::from_milliseconds(500));
    assert(!too_long && too_long.error().code == ErrorCode::UNSUPPORTED && too_long.error().message == "timeout outside supported range");
    const auto invalid = watchdog.value()->start(Duration::from_nanoseconds(0));
    assert(!invalid && invalid.error().code == ErrorCode::INVALID_ARGUMENT);   // each service's own codes pass straight through
    // Core's own UNSUPPORTED (service unavailable) is a different, distinguishable message from a service's UNSUPPORTED.
    const PlatformContext none;
    assert(none.require_watchdog().error().message != too_long.error().message);
}

// Component-level attribution: an integrator-written Component that propagates a platform Error sets its own source.
class PlatformUser final : public runtime::Component {
public:
    PlatformUser(runtime::ComponentInfo info, PlatformContext context) : Component(std::move(info)), context_(context) {}
    Result<void> configure(const Configuration&) override { return Result<void>::success(); }
    Result<void> initialize() override { return Result<void>::success(); }
    Result<void> start() override {
        const auto timer = context_.require_timer();
        if (!timer) return Result<void>::failure(attribute(timer.error()));                       // Core's UNSUPPORTED
        const auto started = timer.value()->start(Duration::from_milliseconds(10), TimerMode::ONE_SHOT, Callback{&count, nullptr});
        if (!started) return Result<void>::failure(attribute(started.error()));                    // the service's own Error
        return Result<void>::success();
    }
    Result<void> stop() override { return Result<void>::success(); }
    Result<void> shutdown() override { return Result<void>::success(); }
    LifecycleState lifecycle_state() const noexcept override { return LifecycleState::UNKNOWN; }
    Status status() const override { return Status{}; }
    Health health() const override { return Health{}; }
    CapabilitySet capabilities() const override { return CapabilitySet{}; }
private:
    Error attribute(Error error) const { error.source = info().id(); return error; }              // only the source changes
    PlatformContext context_;
};

void test_component_attributes_platform_errors_and_the_runtime_propagates_them_unchanged() {
    {   // a service failure
        SpyAdapter adapter;
        adapter.tim().policy.resource_available = false;
        runtime::RuntimeManager runtime;
        auto info = runtime::ComponentInfo::create(runtime::ComponentId{7}, "user");
        PlatformUser user(std::move(info).value(), PlatformContext(adapter));
        assert(runtime.register_component(user) && runtime.attach_platform(adapter));
        assert(runtime.initialize());
        const auto started = runtime.start();
        assert(!started);
        assert(started.error().code == ErrorCode::RESOURCE_UNAVAILABLE && started.error().message == "no timer resource");
        assert(started.error().source == runtime::ComponentId{7});                                   // attributed to the component
        assert(runtime.state() == LifecycleState::FAULT && runtime.fault_error()->code == ErrorCode::RESOURCE_UNAVAILABLE);
    }
    {   // an unavailable service
        ReferenceAdapter bare(PlatformInfo{"bare", Version{}}, {false, false, false, false});
        runtime::RuntimeManager runtime;
        auto info = runtime::ComponentInfo::create(runtime::ComponentId{8}, "user");
        PlatformUser user(std::move(info).value(), PlatformContext(bare));
        assert(runtime.register_component(user) && runtime.initialize());
        const auto started = runtime.start();
        assert(!started && started.error().code == ErrorCode::UNSUPPORTED && started.error().source == runtime::ComponentId{8});
        assert(started.error().message.find("timer") != std::string::npos);
        assert(runtime.reset() && runtime.state() == LifecycleState::STOPPED);                       // explicit recovery only
    }
}

// Re-entry rules of R0.4 are unchanged: a timer callback re-entering its own timer through a pointer obtained from a context.
struct Reenter { PlatformContext context; ErrorCode stop_code{ErrorCode::NONE}; ErrorCode start_code{ErrorCode::NONE}; };
void reenter(void* raw) {
    auto* r = static_cast<Reenter*>(raw);
    const auto timer = r->context.require_timer();
    const auto stopped = timer.value()->stop();
    r->stop_code = stopped ? ErrorCode::NONE : stopped.error().code;
    const auto started = timer.value()->start(Duration::from_milliseconds(10), TimerMode::ONE_SHOT, Callback{&count, nullptr});
    r->start_code = started ? ErrorCode::NONE : started.error().code;
}

void test_callback_and_reentry_rules_are_those_of_the_service_contracts() {
    SpyAdapter adapter;
    Reenter reentrant{PlatformContext(adapter)};                              // the context travels in the caller-owned callback context
    const auto timer = reentrant.context.require_timer();
    assert(timer.value()->start(Duration::from_milliseconds(10), TimerMode::ONE_SHOT, Callback{&reenter, &reentrant}));
    adapter.tim().advance(10 * MS);
    assert(reentrant.stop_code == ErrorCode::INVALID_STATE);                  // the R0.4 timer rule, unchanged
    assert(reentrant.start_code == ErrorCode::INVALID_STATE);
    assert(timer.value()->stop());
}

void test_requirements_and_consumption_work_together() {
    ReferenceAdapter adapter(PlatformInfo{"p", Version{}}, {true, true, false, true});
    const PlatformContext context(adapter);
    PlatformRequirements requirements;
    assert(requirements.add_service(PlatformService::SCHEDULER, Requirement::REQUIRED));
    assert(requirements.add_service(PlatformService::TIMER, Requirement::OPTIONAL));
    assert(check_required(requirements, context));                            // the required service is there
    assert(context.require_scheduler());
    assert(!context.require_timer());                                         // the optional one is not: consume it explicitly or skip it
    PlatformRequirements needs_timer;
    assert(needs_timer.add_service(PlatformService::TIMER, Requirement::REQUIRED));
    assert(!check_required(needs_timer, context));
}

} // namespace

int main() {
    test_shape();
    test_scheduler_access();
    test_clock_access();
    test_timer_access();
    test_watchdog_access();
    test_unavailable_services_are_deterministic_unsupported_results();
    test_every_service_combination_matches_the_adapter();
    test_requiring_is_a_query_with_no_side_effects();
    test_nothing_is_started_or_stopped_implicitly_and_state_survives_the_context();
    test_platform_service_errors_are_never_translated();
    test_component_attributes_platform_errors_and_the_runtime_propagates_them_unchanged();
    test_callback_and_reentry_rules_are_those_of_the_service_contracts();
    test_requirements_and_consumption_work_together();
    return 0;
}
