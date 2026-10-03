//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : runtime_platform_lifecycle_test.cpp
// Description : The Runtime and platform lifecycles are separate: attachment rules, no implicit
//               service lifecycle, and Runtime behavior identical whatever the platform does.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-015
// API         : CORE-TEST-RUNTIME-PLATFORM-LIFECYCLE
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#include <atomic>
#include <cassert>
#include <cstdint>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include <kritva/core/core.hpp>

#include "../contract/reference_adapter.hpp"
#include "../integration/runtime_scenarios.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;
using namespace kritva::core::runtime::scenarios;
using kritva::core::platform::contract::ReferenceAdapter;
using kritva::core::platform::contract::ReferenceScheduler;
using kritva::core::platform::contract::ReferenceWatchdog;
using kritva::core::time::contract::ReferenceTimer;
using kritva::core::time::TimerMode;

namespace {

constexpr std::int64_t MS = 1'000'000;

struct Counter { std::atomic<int> value{0}; };
void count(void* context) { if (context != nullptr) ++static_cast<Counter*>(context)->value; }

CapabilitySet make_capabilities() {
    CapabilitySet set;
    set.add(Capability{CapabilityId{100}, "gpio", Version{1, 0, 0}});
    return set;
}

// A reference platform that counts every call made to it and exposes the state of its services.
class Platform final : public ReferenceAdapter {
public:
    explicit Platform(Provides provides = {}) : ReferenceAdapter(platform::PlatformInfo{"platform", Version{1, 0, 0}}, provides, make_capabilities()) {}
    [[nodiscard]] const platform::PlatformInfo& info() const noexcept override { ++calls; return ReferenceAdapter::info(); }
    [[nodiscard]] platform::IScheduler* scheduler() const noexcept override { ++calls; return ReferenceAdapter::scheduler(); }
    [[nodiscard]] time::IClock* clock() const noexcept override { ++calls; return ReferenceAdapter::clock(); }
    [[nodiscard]] time::ITimer* timer() const noexcept override { ++calls; return ReferenceAdapter::timer(); }
    [[nodiscard]] platform::IWatchdog* watchdog() const noexcept override { ++calls; return ReferenceAdapter::watchdog(); }
    [[nodiscard]] CapabilitySet capabilities() const override { ++calls; return ReferenceAdapter::capabilities(); }
    ReferenceScheduler& sched() const { return *static_cast<ReferenceScheduler*>(ReferenceAdapter::scheduler()); }
    ReferenceTimer& tim() const { return *static_cast<ReferenceTimer*>(ReferenceAdapter::timer()); }
    ReferenceWatchdog& dog() const { return *static_cast<ReferenceWatchdog*>(ReferenceAdapter::watchdog()); }
    mutable int calls{0};

    // The integrator starts every service itself, before or after attaching the platform to a Runtime.
    void start_all_services(Counter& task_runs) {
        assert(sched().create_task(platform::TaskConfig{"t", 0, 0, Duration::from_milliseconds(1)}, &count, &task_runs));
        assert(sched().start());
        assert(tim().start(Duration::from_milliseconds(5), TimerMode::PERIODIC, Callback{&count, &task_runs}));
        assert(dog().start(Duration::from_milliseconds(1000)));
    }
};

// Everything observable about the services' own lifecycle.
struct ServiceState {
    bool sched_running, tim_running, dog_running;
    int create, sched_start, sched_stop, tim_start, tim_stop, dog_start, dog_kick, dog_stop;
    std::uint64_t tim_fired, dog_expiries;
    int task_runs;
    bool operator==(const ServiceState&) const = default;
};
ServiceState state_of(const Platform& p, const Counter& runs) {
    return ServiceState{p.sched().running(), p.tim().running(), p.dog().running(),
                        p.sched().create_calls, p.sched().start_calls, p.sched().stop_calls, p.tim().start_calls, p.tim().stop_calls,
                        p.dog().start_calls, p.dog().kick_calls, p.dog().stop_calls, p.tim().fired(), p.dog().expiries(), runs.value};
}

Script fixed_script(std::size_t n, std::vector<Step> steps) {
    Script script;
    script.n = n;
    for (std::size_t i = 1; i <= n; ++i) script.registration.push_back(i);
    script.steps = std::move(steps);
    return script;
}

// ---- attachment stays setup-only in every Runtime state --------------------------------------------

void expect_attach_rejected(World& world, Platform& adapter, platform::IPlatformAdapter* expected) {
    const auto r = world.runtime.attach_platform(adapter);
    assert(!r && r.error().code == ErrorCode::INVALID_STATE);
    assert(world.runtime.platform() == expected);                            // nothing changed
}

void test_attach_rules_in_every_runtime_state() {
    Platform platform_a, platform_b;
    {
        // Before the first successful initialize(): allowed, and it changes nothing about the Runtime.
        World world(fixed_script(3, {}), nullptr);
        assert(world.runtime.platform() == nullptr && world.runtime.state() == LifecycleState::UNKNOWN && !world.runtime.topology_fixed());
        assert(world.runtime.configure(Configuration{}));                      // configure() does not fix the topology
        assert(world.runtime.attach_platform(platform_a));
        assert(world.runtime.platform() == &platform_a);
        assert(world.runtime.state() == LifecycleState::UNKNOWN && !world.runtime.topology_fixed());
        assert(world.runtime.initialize());
        // From here on, in every state: INVALID_STATE, adapter unchanged.
        expect_attach_rejected(world, platform_b, &platform_a);                // READY
        assert(world.runtime.state() == LifecycleState::READY);
        assert(world.runtime.start());
        expect_attach_rejected(world, platform_b, &platform_a);                // RUNNING
        assert(world.runtime.stop());
        expect_attach_rejected(world, platform_b, &platform_a);                // STOPPED
        assert(world.runtime.shutdown());
        expect_attach_rejected(world, platform_b, &platform_a);                // STOPPED after shutdown
        assert(world.runtime.initialize() && world.runtime.start());           // a second live period
        expect_attach_rejected(world, platform_b, &platform_a);
        assert(platform_a.calls == 0 && platform_b.calls == 0);
    }
    {
        // A component failure fixes the topology too (initialize() validated it), so FAULT and reset() are closed.
        Script script = fixed_script(3, {});
        World world(script, nullptr);
        world.components[1]->fail_next_initialize = ErrorCode::TIMEOUT;
        assert(!world.runtime.initialize() && world.runtime.state() == LifecycleState::FAULT);
        expect_attach_rejected(world, platform_b, nullptr);                    // FAULT: no adapter, still rejected
        assert(world.runtime.reset() && world.runtime.state() == LifecycleState::STOPPED);
        expect_attach_rejected(world, platform_b, nullptr);                    // after reset()
        assert(world.runtime.initialize());
        expect_attach_rejected(world, platform_b, nullptr);
    }
    {
        // An invalid topology makes initialize() fail without fixing it: setup, and so attachment, stays open.
        World world(fixed_script(2, {}), nullptr);
        assert(world.runtime.add_dependency(ComponentId{1}, ComponentId{9}));  // 9 is not registered
        assert(!world.runtime.initialize() && !world.runtime.topology_fixed() && world.runtime.state() == LifecycleState::UNKNOWN);
        assert(world.runtime.attach_platform(platform_a) && world.runtime.platform() == &platform_a);
    }
    {
        // A second attachment never replaces the first, even before initialize().
        World world(fixed_script(1, {}), &platform_a);
        expect_attach_rejected(world, platform_b, &platform_a);
        assert(world.runtime.platform() == &platform_a && !world.runtime.topology_fixed());
    }
}

// ---- the Runtime never starts, stops, configures, polls or advances a platform service --------------

void test_no_implicit_service_lifecycle() {
    std::mt19937 rng(20261006);
    for (int scenario = 0; scenario < 60; ++scenario) {
        const Script script = random_script(rng);
        Platform platform;
        Counter runs;
        platform.start_all_services(runs);
        const ServiceState before = state_of(platform, runs);
        assert(before.sched_running && before.tim_running && before.dog_running);
        World world(script, &platform);
        const int calls_at_attach = platform.calls;
        Hooks hooks;
        hooks.after = [&](World& w, std::size_t) {
            assert(w.runtime.platform() == &platform);                           // the attachment itself survives every operation, reset() included
            assert(state_of(platform, runs) == before);                          // after every Runtime operation, in every state
            assert(platform.calls == calls_at_attach);                           // and the Runtime never called the platform
        };
        (void)run(world, script, hooks);
        assert(state_of(platform, runs) == before);
        assert(platform.sched().running() && platform.tim().running() && platform.dog().running());   // nothing was stopped, even by FAULT or shutdown
    }
}

void test_no_implicit_service_lifecycle_when_services_start_late_or_never() {
    // The integrator starts nothing: the Runtime does not either, in any state.
    Platform platform;
    Script script = fixed_script(2, {Step{1}, Step{2}, Step{3}, Step{4}, Step{1}, Step{2}});
    World world(script, &platform);
    (void)run(world, script);
    assert(!platform.sched().running() && !platform.tim().running() && !platform.dog().running());
    assert(platform.sched().create_calls == 0 && platform.sched().start_calls == 0 && platform.tim().start_calls == 0 && platform.dog().start_calls == 0);
    assert(platform.calls == 0);
    // Services started AFTER a Runtime is RUNNING are not disturbed by its later stop() and shutdown().
    Counter runs;
    platform.start_all_services(runs);
    const ServiceState started = state_of(platform, runs);
    assert(world.runtime.state() == LifecycleState::RUNNING);
    assert(world.runtime.stop() && world.runtime.shutdown());
    assert(state_of(platform, runs) == started);
}

void test_no_background_execution_services_advance_only_when_the_integrator_advances_them() {
    Platform platform;
    Counter runs;
    platform.start_all_services(runs);
    const Script script = fixed_script(3, {Step{1}, Step{2}, Step{3}, Step{4}});
    World world(script, &platform);
    (void)run(world, script);
    assert(runs.value == 0);                                                  // the Runtime ran no task, fired no timer, ticked nothing
    platform.sched().tick();                                                  // the integrator advances time: now the periodic task runs
    platform.tim().advance(5 * MS);
    assert(runs.value == 2);                                                  // one task tick + one timer firing, caused by the test only
    (void)world;
}

// ---- Runtime behavior is identical whatever the platform is or does -----------------------------------

void use_the_platform_between_steps(World& world, Platform* platform) {
    // What an integrator may do at any time: build a context from the Runtime's pointer and use it.
    const platform::PlatformContext context(world.runtime.platform());
    (void)context.supports(platform::PlatformService::TIMER);
    (void)context.require_scheduler();
    (void)context.require_clock();
    (void)context.require_timer();
    (void)context.require_watchdog();
    (void)context.has_capability(CapabilityId{100});
    platform::PlatformRequirements requirements;
    (void)requirements.add_service(platform::PlatformService::WATCHDOG, platform::Requirement::REQUIRED);
    (void)requirements.add_capability(CapabilityId{100}, platform::Requirement::OPTIONAL);
    (void)platform::check_required(requirements, context);
    if (platform != nullptr) platform->calls += 0;
}

void test_runtime_is_identical_for_every_platform_and_every_use_of_it() {
    std::mt19937 rng(20261007);
    std::size_t operations = 0, failed_or_invalid = 0;
    for (int scenario = 0; scenario < 30; ++scenario) {
        const Script script = random_script(rng);
        World baseline(script, nullptr);
        const Trace expected = run(baseline, script);
        for (const std::string& line : expected) if (line.find("-> ok") == std::string::npos && line.find("->") != std::string::npos) ++failed_or_invalid;
        operations += script.steps.size();

        for (int mask = 0; mask < 16; ++mask) {                                // every combination of provided services
            Platform platform({(mask & 1) != 0, (mask & 2) != 0, (mask & 4) != 0, (mask & 8) != 0});
            World world(script, &platform);
            int before_calls = 0;
            Hooks hooks;
            hooks.before = [&](World&, std::size_t) { before_calls = platform.calls; };
            hooks.after = [&](World& w, std::size_t) {
                assert(platform.calls == before_calls);                        // the Runtime made no call to the platform
                use_the_platform_between_steps(w, &platform);                  // the integrator may; it must change nothing
            };
            const Trace actual = run(world, script, hooks);
            assert(actual == expected);                                        // results, states, faults, statistics and invocation trace
            assert(platform.activations() == 0);
        }
    }
    assert(operations == 30u * 40u && failed_or_invalid > 100);              // the scenarios cover many failure and invalid-state paths
}

// ---- a platform failure is an ordinary component failure -----------------------------------------------

class PlatformUser final : public Component {
public:
    PlatformUser(ComponentInfo info, platform::PlatformContext context, ErrorCode* trace_code) : Component(std::move(info)), context_(context), trace_code_(trace_code) {}
    Result<void> configure(const Configuration&) override { return Result<void>::success(); }
    Result<void> initialize() override { return Result<void>::success(); }
    Result<void> start() override {
        const auto timer = context_.require_timer();
        if (!timer) return fail(timer.error());
        const auto started = timer.value()->start(Duration::from_milliseconds(10), TimerMode::ONE_SHOT, Callback{&count, nullptr});
        if (!started) return fail(started.error());
        return Result<void>::success();
    }
    Result<void> stop() override { return Result<void>::success(); }
    Result<void> shutdown() override { return Result<void>::success(); }
    LifecycleState lifecycle_state() const noexcept override { return LifecycleState::UNKNOWN; }
    Status status() const override { return Status{}; }
    Health health() const override { return Health{}; }
    CapabilitySet capabilities() const override { return CapabilitySet{}; }
private:
    Result<void> fail(Error error) const { *trace_code_ = error.code; error.source = info().id(); return Result<void>::failure(error); }
    platform::PlatformContext context_;
    ErrorCode* trace_code_;
};

struct Outcome {
    std::vector<std::string> lines;
    bool operator==(const Outcome&) const = default;
};

// Code, source, state, fault and statistics of a fixed scenario; messages are excluded (they name the component).
std::string outcome_of(const RuntimeManager& runtime, const Result<void>& r, const char* what) {
    const Statistics& st = runtime.statistics();
    const Error* fault = runtime.fault_error();
    return std::string(what) + " " + (r ? "ok" : std::to_string(static_cast<int>(r.error().code)) + "/" + std::to_string(r.error().source.value())) +
           " state=" + std::to_string(static_cast<int>(runtime.state())) + " fault=" + (fault ? std::to_string(static_cast<int>(fault->code)) + "/" + std::to_string(fault->source.value()) : "none") +
           " samples=" + std::to_string(st.sample_count.value()) + " errors=" + std::to_string(st.error_count.value());
}

Outcome scenario_with_platform_failure(Platform& platform) {
    // Components 1 and 3 are reference components; component 2 uses the platform's timer and fails when it is unavailable.
    RuntimeManager runtime;
    Trace trace;
    ErrorCode seen = ErrorCode::NONE;
    auto i1 = ComponentInfo::create(ComponentId{1}, "c"), i2 = ComponentInfo::create(ComponentId{2}, "c"), i3 = ComponentInfo::create(ComponentId{3}, "c");
    contract::ReferenceComponent c1(std::move(i1).value()), c3(std::move(i3).value());
    PlatformUser c2(std::move(i2).value(), platform::PlatformContext(&platform), &seen);
    c1.trace = c3.trace = &trace;
    assert(runtime.register_component(c1) && runtime.register_component(c2) && runtime.register_component(c3));
    assert(runtime.add_dependency(ComponentId{2}, ComponentId{1}) && runtime.add_dependency(ComponentId{3}, ComponentId{2}));
    assert(runtime.attach_platform(platform));
    Outcome out;
    out.lines.push_back(outcome_of(runtime, runtime.initialize(), "init"));
    out.lines.push_back(outcome_of(runtime, runtime.start(), "start"));
    out.lines.push_back(outcome_of(runtime, runtime.stop(), "stop"));
    out.lines.push_back(outcome_of(runtime, runtime.reset(), "reset"));
    out.lines.push_back(outcome_of(runtime, runtime.shutdown(), "shutdown"));
    out.lines.push_back(std::to_string(trace.size()));
    assert(seen == ErrorCode::UNSUPPORTED);                                    // the platform's own UNSUPPORTED reached the component
    return out;
}

Outcome scenario_with_injected_failure() {
    // The same graph with no platform at all and component 2's start() failing with the same code.
    RuntimeManager runtime;
    Trace trace;
    auto i1 = ComponentInfo::create(ComponentId{1}, "c"), i2 = ComponentInfo::create(ComponentId{2}, "c"), i3 = ComponentInfo::create(ComponentId{3}, "c");
    contract::ReferenceComponent c1(std::move(i1).value()), c2(std::move(i2).value()), c3(std::move(i3).value());
    c1.trace = c2.trace = c3.trace = &trace;
    assert(runtime.register_component(c1) && runtime.register_component(c2) && runtime.register_component(c3));
    assert(runtime.add_dependency(ComponentId{2}, ComponentId{1}) && runtime.add_dependency(ComponentId{3}, ComponentId{2}));
    c2.fail_next_start = ErrorCode::UNSUPPORTED;
    Outcome out;
    out.lines.push_back(outcome_of(runtime, runtime.initialize(), "init"));
    out.lines.push_back(outcome_of(runtime, runtime.start(), "start"));
    out.lines.push_back(outcome_of(runtime, runtime.stop(), "stop"));
    out.lines.push_back(outcome_of(runtime, runtime.reset(), "reset"));
    out.lines.push_back(outcome_of(runtime, runtime.shutdown(), "shutdown"));
    // Component 2 records a trace line for start/stop/shutdown/initialize; PlatformUser records none: compare the
    // reference components' lines only (components 1 and 3), which must match line for line.
    std::size_t reference_only = 0;
    for (const std::string& line : trace) if (line.rfind("2:", 0) != 0) ++reference_only;
    out.lines.push_back(std::to_string(reference_only));
    return out;
}

void test_a_platform_failure_is_an_ordinary_component_failure() {
    Platform no_timer({true, true, false, true});                              // the platform provides no timer
    const Outcome with_platform = scenario_with_platform_failure(no_timer);
    const Outcome injected = scenario_with_injected_failure();
    assert(with_platform == injected);                                         // same states, fault, statistics, source and reference-component trace
    const std::string unsupported = std::to_string(static_cast<int>(ErrorCode::UNSUPPORTED));
    assert(with_platform.lines[1].find("start " + unsupported + "/2") != std::string::npos);   // the platform's code, attributed to component 2
}

// ---- watchdog expiry and Runtime FAULT never interact ---------------------------------------------------

void test_watchdog_expiry_never_changes_the_runtime_in_any_state() {
    std::mt19937 rng(20261008);
    for (int scenario = 0; scenario < 30; ++scenario) {
        const Script script = random_script(rng);
        World baseline(script, nullptr);
        const Trace expected = run(baseline, script);
        Platform platform;
        assert(platform.dog().start(Duration::from_milliseconds(10)));
        World world(script, &platform);
        Hooks hooks;
        hooks.before = [&](World&, std::size_t) { platform.dog().advance(1000 * MS); };   // expires and stays expired in every state
        hooks.after = [&](World&, std::size_t) { platform.dog().advance(1000 * MS); };
        const Trace actual = run(world, script, hooks);
        assert(platform.dog().expired() && platform.dog().expiries() == 1);
        assert(actual == expected);                                            // no state, FAULT, reset or statistic differs
        assert(platform.dog().kick_calls == 0 && platform.dog().stop_calls == 0 && platform.dog().start_calls == 1);   // and the Runtime never touched it
    }
}

} // namespace

int main() {
    test_attach_rules_in_every_runtime_state();
    test_no_implicit_service_lifecycle();
    test_no_implicit_service_lifecycle_when_services_start_late_or_never();
    test_no_background_execution_services_advance_only_when_the_integrator_advances_them();
    test_runtime_is_identical_for_every_platform_and_every_use_of_it();
    test_a_platform_failure_is_an_ordinary_component_failure();
    test_watchdog_expiry_never_changes_the_runtime_in_any_state();
    return 0;
}
