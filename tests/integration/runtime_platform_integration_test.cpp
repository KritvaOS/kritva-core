//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : runtime_platform_integration_test.cpp
// Description : Runtime and platform integration through public APIs and the reference platform.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-017
// API         : CORE-TEST-RUNTIME-PLATFORM-INTEGRATION
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

// Everything here uses only public APIs: RuntimeManager, PlatformContext, PlatformRequirements,
// the service contracts and the test-only reference platform (tests/platform). No private detail
// of the Runtime or of the platform is touched, and no hardware is needed.
//
// The central proof is EQUIVALENCE: integrator-written components that use the platform are run on
// a Runtime next to reference components; whatever the platform does (succeed, fail, be unavailable)
// the Runtime must see exactly what it would see from a plain component failing in the same place.

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include <kritva/core/core.hpp>

#include "../platform/reference_platform.hpp"
#include "runtime_scenarios.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;
using namespace kritva::core::runtime::scenarios;
using namespace kritva::core::platform;
using kritva::core::platform::testing::Controls;
using kritva::core::platform::testing::LifetimeProbe;
using kritva::core::platform::testing::Method;
using kritva::core::platform::testing::ReferencePlatform;
using kritva::core::time::TimerMode;

namespace {

constexpr std::int64_t MS = 1'000'000;

struct Counter { std::atomic<int> value{0}; };
void count(void* context) { if (context != nullptr) ++static_cast<Counter*>(context)->value; }

CapabilitySet capabilities_with_100() {
    CapabilitySet set;
    set.add(Capability{CapabilityId{100}, "gpio", Version{1, 0, 0}});
    return set;
}

ReferencePlatform::Config config(ReferencePlatform::Provides provides = {}, bool with_capability = true) {
    ReferencePlatform::Config c;
    c.provides = provides;
    if (with_capability) c.capabilities = capabilities_with_100();
    return c;
}

ReferencePlatform::Provides provides_from(int mask) {
    return ReferencePlatform::Provides{(mask & 1) != 0, (mask & 2) != 0, (mask & 4) != 0, (mask & 8) != 0};
}

// ---- an integrator-written component that uses every kind of platform service ----------------------

// initialize(): create a periodic task. start(): start the scheduler, a periodic timer and a watchdog,
// kick it, read the clock. stop(): the reverse. A platform failure is returned as this component's own
// failed Result (its source), with the platform's code and message untouched.
enum Role : unsigned { CREATE_TASK = 1, SCHEDULER = 2, TIMER = 4, WATCHDOG = 8, CLOCK = 16, ALL = 31 };

class AppComponent final : public Component {
public:
    AppComponent(ComponentInfo info, PlatformContext context, unsigned roles = ALL) : Component(std::move(info)), context_(context), roles_(roles) {}
    Result<void> configure(const Configuration&) override { return Result<void>::success(); }
    Result<void> initialize() override {
        if ((roles_ & CREATE_TASK) == 0) return Result<void>::success();
        const auto scheduler = context_.require_scheduler();
        if (!scheduler) return attribute(scheduler.error());
        const auto id = scheduler.value()->create_task(TaskConfig{"app", 0, 0, Duration::from_milliseconds(1)}, &count, &runs);
        if (!id) return attribute(id.error());
        return Result<void>::success();
    }
    Result<void> start() override {
        if ((roles_ & SCHEDULER) != 0) {
            const auto scheduler = context_.require_scheduler();
            if (!scheduler) return attribute(scheduler.error());
            if (auto r = scheduler.value()->start(); !r) return attribute(r.error());
        }
        if ((roles_ & TIMER) != 0) {
            const auto timer = context_.require_timer();
            if (!timer) return attribute(timer.error());
            if (auto r = timer.value()->start(Duration::from_milliseconds(5), TimerMode::PERIODIC, Callback{&count, &runs}); !r) return attribute(r.error());
        }
        if ((roles_ & WATCHDOG) != 0) {
            const auto watchdog = context_.require_watchdog();
            if (!watchdog) return attribute(watchdog.error());
            if (auto r = watchdog.value()->start(Duration::from_milliseconds(1000)); !r) return attribute(r.error());
            if (auto r = watchdog.value()->kick(); !r) return attribute(r.error());
        }
        if ((roles_ & CLOCK) != 0) {
            const auto clock = context_.require_clock();
            if (!clock) return attribute(clock.error());
            (void)clock.value()->now();
        }
        return Result<void>::success();
    }
    Result<void> stop() override {
        if ((roles_ & WATCHDOG) != 0) if (auto* watchdog = context_.watchdog()) if (auto r = watchdog->stop(); !r) return attribute(r.error());
        if ((roles_ & TIMER) != 0) if (auto* timer = context_.timer()) if (auto r = timer->stop(); !r) return attribute(r.error());
        if ((roles_ & SCHEDULER) != 0) if (auto* scheduler = context_.scheduler()) if (auto r = scheduler->stop(); !r) return attribute(r.error());
        return Result<void>::success();
    }
    Result<void> shutdown() override { return Result<void>::success(); }
    LifecycleState lifecycle_state() const noexcept override { return LifecycleState::UNKNOWN; }
    Status status() const override { return Status{}; }
    Health health() const override { return Health{}; }
    CapabilitySet capabilities() const override { return CapabilitySet{}; }
    Counter runs;

private:
    Result<void> attribute(Error error) const { error.source = info().id(); return Result<void>::failure(error); }
    PlatformContext context_;
    unsigned roles_;
};

// ---- the Runtime-visible outcome of a lifecycle, without messages (they name the component) ----------

std::string outcome(const RuntimeManager& runtime, const Result<void>& r, const char* what) {
    const Statistics& st = runtime.statistics();
    const Error* fault = runtime.fault_error();
    return std::string(what) + " " + (r ? "ok" : std::to_string(static_cast<int>(r.error().code)) + "/" + std::to_string(r.error().source.value())) +
           " state=" + std::to_string(static_cast<int>(runtime.state())) + " fixed=" + std::to_string(runtime.topology_fixed()) +
           " fault=" + (fault ? std::to_string(static_cast<int>(fault->code)) + "/" + std::to_string(fault->source.value()) : "none") +
           " samples=" + std::to_string(st.sample_count.value()) + " errors=" + std::to_string(st.error_count.value()) +
           " retries=" + std::to_string(st.retry_count.value());
}

const char* kOps[] = {"initialize", "start", "stop", "shutdown", "reset"};

// ---- 1. cross-service integration: services are used in dependency order and torn down in reverse -----

// Components 1..4 each play one role: 1 creates a task, 2 starts the scheduler, 3 starts a timer, 4 starts and kicks the
// watchdog and reads the clock; 2 depends on 1, 3 on 2, 4 on 3.
void test_cross_service_integration_follows_dependency_order() {
    ReferencePlatform platform(config());
    const PlatformContext context(platform);
    RuntimeManager runtime;
    const unsigned roles[] = {CREATE_TASK, SCHEDULER, TIMER, WATCHDOG | CLOCK};
    std::vector<std::unique_ptr<AppComponent>> apps;
    for (std::uint64_t id = 1; id <= 4; ++id) {
        auto info = ComponentInfo::create(ComponentId{id}, "app");
        apps.push_back(std::make_unique<AppComponent>(std::move(info).value(), context, roles[id - 1]));
        assert(runtime.register_component(*apps.back()));
    }
    for (std::uint64_t id = 2; id <= 4; ++id) assert(runtime.add_dependency(ComponentId{id}, ComponentId{id - 1}));
    assert(runtime.attach_platform(platform));

    ReferencePlatform other(config());
    assert(!runtime.attach_platform(other) && runtime.platform() == &platform);                // a second attach never replaces
    assert(runtime.initialize() && runtime.start());
    assert(!runtime.attach_platform(other) && runtime.platform() == &platform);                // and attachment is closed once the topology is fixed
    Controls& controls = platform.controls();
    assert(runtime.state() == LifecycleState::RUNNING);
    assert(platform.fake_scheduler().running() && platform.fake_timer().running() && platform.fake_watchdog().running());
    // The platform calls appear in dependency order: the Runtime asked each component once, forward.
    const std::vector<Method> forward = {Method::SCHEDULER_CREATE_TASK, Method::SCHEDULER_START, Method::TIMER_START, Method::WATCHDOG_START, Method::WATCHDOG_KICK};
    assert(controls.log().size() == forward.size());
    for (std::size_t i = 0; i < forward.size(); ++i) assert(controls.log()[i].method == forward[i] && controls.log()[i].ok);

    // Nothing runs until the integrator advances time; the Runtime never does.
    assert(apps[0]->runs.value == 0);
    platform.let_time_pass(Duration::from_milliseconds(5));
    assert(apps[0]->runs.value > 0);                                           // the task and timer run when the integrator lets time pass

    controls.clear_log();
    assert(runtime.stop());
    // Teardown is the reverse: the watchdog (component 4), then the timer (3), then the scheduler (2).
    assert(controls.log().size() == 3);
    assert(controls.log()[0].method == Method::WATCHDOG_STOP && controls.log()[1].method == Method::TIMER_STOP && controls.log()[2].method == Method::SCHEDULER_STOP);
    assert(runtime.shutdown());
    assert(!platform.fake_scheduler().running() && !platform.fake_timer().running() && !platform.fake_watchdog().running());   // components stopped what they started

    // A Runtime whose components do not use the platform makes no call to the adapter at all.
    ReferencePlatform quiet(config());
    RuntimeManager bare;
    auto info = ComponentInfo::create(ComponentId{1}, "plain");
    runtime::contract::ReferenceComponent plain(std::move(info).value());
    assert(bare.register_component(plain) && bare.attach_platform(quiet));
    assert(bare.initialize() && bare.start() && bare.stop() && bare.shutdown());
    assert(quiet.controls().adapter_queries() == 0 && quiet.controls().log().empty());
}

// ---- 2. a platform failure is exactly an ordinary component failure ----------------------------------

struct Where { Method method; const char* op; };
constexpr Where kWhere[] = {
    {Method::SCHEDULER_CREATE_TASK, "initialize"}, {Method::SCHEDULER_START, "start"}, {Method::TIMER_START, "start"},
    {Method::WATCHDOG_START, "start"}, {Method::WATCHDOG_KICK, "start"}, {Method::WATCHDOG_STOP, "stop"},
    {Method::TIMER_STOP, "stop"}, {Method::SCHEDULER_STOP, "stop"},
};

void arm(runtime::contract::ReferenceComponent& c, const std::string& op, ErrorCode code) {
    if (op == "initialize") c.fail_next_initialize = code;
    else if (op == "start") c.fail_next_start = code;
    else if (op == "stop") c.fail_next_stop = code;
}

// What the integrator does between attempts: it owns the platform, so it puts its services back into a known state.
// This goes straight to the underlying doubles, so it is neither counted nor subject to injected faults.
void integrator_heals(ReferencePlatform& platform) {
    (void)platform.fake_watchdog().kritva::core::platform::contract::ReferenceWatchdog::stop();
    (void)platform.fake_timer().kritva::core::time::contract::ReferenceTimer::stop();
    (void)platform.fake_scheduler().kritva::core::platform::contract::ReferenceScheduler::stop();
}

std::vector<std::string> lifecycle_with_platform(const Where& where, std::size_t nth, ErrorCode code) {
    ReferencePlatform platform(config());
    platform.controls().fail_nth(where.method, nth, code);
    RuntimeManager runtime;
    Trace trace;
    auto i1 = ComponentInfo::create(ComponentId{1}, "c"), i2 = ComponentInfo::create(ComponentId{2}, "c"), i3 = ComponentInfo::create(ComponentId{3}, "c");
    runtime::contract::ReferenceComponent c1(std::move(i1).value()), c3(std::move(i3).value());
    AppComponent c2(std::move(i2).value(), PlatformContext(platform));
    c1.trace = c3.trace = &trace;
    assert(runtime.register_component(c1) && runtime.register_component(c2) && runtime.register_component(c3));
    assert(runtime.add_dependency(ComponentId{2}, ComponentId{1}) && runtime.add_dependency(ComponentId{3}, ComponentId{2}));
    assert(runtime.attach_platform(platform));
    std::vector<std::string> out;
    for (std::size_t cycle = 1; cycle <= 3; ++cycle) {
        out.push_back(outcome(runtime, runtime.initialize(), "initialize"));
        out.push_back(outcome(runtime, runtime.start(), "start"));
        out.push_back(outcome(runtime, runtime.stop(), "stop"));
        out.push_back(outcome(runtime, runtime.shutdown(), "shutdown"));
        out.push_back(outcome(runtime, runtime.reset(), "reset"));
        integrator_heals(platform);
    }
    out.push_back("trace=" + std::to_string(trace.size()));
    return out;
}

std::vector<std::string> lifecycle_with_plain_failure(const Where& where, std::size_t nth, ErrorCode code) {
    RuntimeManager runtime;
    Trace trace;
    auto i1 = ComponentInfo::create(ComponentId{1}, "c"), i2 = ComponentInfo::create(ComponentId{2}, "c"), i3 = ComponentInfo::create(ComponentId{3}, "c");
    runtime::contract::ReferenceComponent c1(std::move(i1).value()), c2(std::move(i2).value()), c3(std::move(i3).value());
    c1.trace = c3.trace = &trace;                                                  // component 2 is not traced: it is the one that differs
    assert(runtime.register_component(c1) && runtime.register_component(c2) && runtime.register_component(c3));
    assert(runtime.add_dependency(ComponentId{2}, ComponentId{1}) && runtime.add_dependency(ComponentId{3}, ComponentId{2}));
    std::vector<std::string> out;
    for (std::size_t cycle = 1; cycle <= 3; ++cycle) {
        for (const char* op : kOps) {
            if (cycle == nth && std::string(op) == where.op) arm(c2, op, code);    // the same failure, in the same place, as a plain component failure
            Result<void> r = Result<void>::success();
            if (std::string(op) == "initialize") r = runtime.initialize();
            else if (std::string(op) == "start") r = runtime.start();
            else if (std::string(op) == "stop") r = runtime.stop();
            else if (std::string(op) == "shutdown") r = runtime.shutdown();
            else r = runtime.reset();
            out.push_back(outcome(runtime, r, op));
        }
    }
    out.push_back("trace=" + std::to_string(trace.size()));
    return out;
}

void test_a_platform_failure_is_exactly_an_ordinary_component_failure() {
    std::size_t compared = 0, faulting = 0;
    for (const Where& where : kWhere) {
        for (std::size_t nth = 1; nth <= 3; ++nth) {
            for (const ErrorCode code : {ErrorCode::TIMEOUT, ErrorCode::RESOURCE_UNAVAILABLE, ErrorCode::INTERNAL_ERROR}) {
                const auto platform_world = lifecycle_with_platform(where, nth, code);
                const auto plain_world = lifecycle_with_plain_failure(where, nth, code);
                if (platform_world != plain_world) {                              // show the first difference before failing
                    for (std::size_t i = 0; i < std::min(platform_world.size(), plain_world.size()); ++i)
                        if (platform_world[i] != plain_world[i]) { fprintf(stderr, "method %d nth %zu code %d line %zu\n  platform: %s\n  plain:    %s\n", static_cast<int>(where.method), nth, static_cast<int>(code), i, platform_world[i].c_str(), plain_world[i].c_str()); break; }
                }
                assert(platform_world == plain_world);                             // states, faults, sources, statistics and reference-component trace
                ++compared;
                for (const std::string& line : platform_world) if (line.find("state=" + std::to_string(static_cast<int>(LifecycleState::FAULT))) != std::string::npos) { ++faulting; break; }
            }
        }
    }
    assert(compared == 8u * 3u * 3u);
    assert(faulting == compared);                                                  // every injected platform failure really reached the Runtime as a fault
}

// ---- 3. service availability ------------------------------------------------------------------------

// What AppComponent needs, and the first step that fails, derived independently of the implementation.
struct Expected { LifecycleState final_state; const char* failing_op; ErrorCode code; };

Expected expected_for(const ReferencePlatform::Provides& p) {
    if (!p.scheduler) return {LifecycleState::FAULT, "initialize", ErrorCode::UNSUPPORTED};      // create_task needs a scheduler
    if (!p.timer || !p.watchdog || !p.clock) return {LifecycleState::FAULT, "start", ErrorCode::UNSUPPORTED};
    return {LifecycleState::RUNNING, "", ErrorCode::NONE};
}

void test_every_availability_combination_gives_the_derived_outcome_ungated() {
    for (int mask = 0; mask < 16; ++mask) {
        const ReferencePlatform::Provides provides = provides_from(mask);
        ReferencePlatform platform(config(provides));
        RuntimeManager runtime;
        auto info = ComponentInfo::create(ComponentId{5}, "app");
        AppComponent app(std::move(info).value(), PlatformContext(platform));
        assert(runtime.register_component(app) && runtime.attach_platform(platform));
        const Expected expected = expected_for(provides);
        const auto init = runtime.initialize();
        const auto start = init ? runtime.start() : Result<void>::success();
        if (std::string(expected.failing_op) == "initialize") {
            assert(!init && init.error().code == ErrorCode::UNSUPPORTED && init.error().source == ComponentId{5});
            assert(init.error().message.find("scheduler") != std::string::npos);
        } else if (std::string(expected.failing_op) == "start") {
            assert(init && !start && start.error().code == ErrorCode::UNSUPPORTED && start.error().source == ComponentId{5});
        } else {
            assert(init && start);
        }
        assert(runtime.state() == expected.final_state);
        assert(runtime.statistics().retry_count.value() == 0);
        if (expected.final_state == LifecycleState::FAULT) {
            assert(runtime.fault_error() != nullptr && runtime.fault_error()->code == ErrorCode::UNSUPPORTED);
            assert(runtime.reset() && runtime.state() == LifecycleState::STOPPED);                  // explicit recovery, no retry
        } else {
            assert(runtime.stop() && runtime.shutdown());
        }
    }
}

// The oracle for gating: a REQUIRED service or capability the platform lacks makes check_required fail.
struct Need { std::vector<std::pair<PlatformService, Requirement>> services; std::vector<std::pair<std::uint64_t, Requirement>> capabilities; };

bool oracle_satisfied(const Need& need, const ReferencePlatform::Provides& p, bool has_capability_100) {
    auto provided = [&](PlatformService s) {
        switch (s) { case PlatformService::SCHEDULER: return p.scheduler; case PlatformService::CLOCK: return p.clock;
                     case PlatformService::TIMER: return p.timer; case PlatformService::WATCHDOG: return p.watchdog; }
        return false;
    };
    for (const auto& [service, level] : need.services) if (level == Requirement::REQUIRED && !provided(service)) return false;
    for (const auto& [id, level] : need.capabilities) if (level == Requirement::REQUIRED && !(id == 100 && has_capability_100)) return false;
    return true;
}

void test_requirements_gate_the_runtime_deterministically_for_every_combination() {
    const std::vector<Need> needs = {
        {{{PlatformService::SCHEDULER, Requirement::REQUIRED}}, {}},
        {{{PlatformService::SCHEDULER, Requirement::REQUIRED}, {PlatformService::CLOCK, Requirement::REQUIRED}, {PlatformService::TIMER, Requirement::REQUIRED}, {PlatformService::WATCHDOG, Requirement::REQUIRED}}, {}},
        {{{PlatformService::TIMER, Requirement::OPTIONAL}}, {{100, Requirement::REQUIRED}}},
        {{{PlatformService::WATCHDOG, Requirement::OPTIONAL}, {PlatformService::CLOCK, Requirement::OPTIONAL}}, {{999, Requirement::OPTIONAL}}},
    };
    std::size_t gated_out = 0, gated_in = 0;
    for (int mask = 0; mask < 16; ++mask) {
        for (const bool has_capability : {true, false}) {
            for (const Need& need : needs) {
                ReferencePlatform platform(config(provides_from(mask), has_capability));
                const PlatformContext context(platform);
                PlatformRequirements requirements;
                for (const auto& [service, level] : need.services) assert(requirements.add_service(service, level));
                for (const auto& [id, level] : need.capabilities) assert(requirements.add_capability(CapabilityId{id}, level));
                const bool expected = oracle_satisfied(need, provides_from(mask), has_capability);
                const auto checked = check_required(requirements, context);
                assert(checked.has_value() == expected);
                assert(evaluate(requirements, context).satisfied() == expected);
                assert(platform.controls().log().empty());                                          // checking is a query: no service was touched
                if (!expected) {
                    // The integrator declines to build the Runtime: nothing was started, nothing is left running.
                    ++gated_out;
                    assert(checked.error().code == ErrorCode::UNSUPPORTED);
                    assert(!platform.fake_scheduler().running() && !platform.fake_timer().running() && !platform.fake_watchdog().running());
                } else {
                    ++gated_in;
                    RuntimeManager runtime;
                    auto info = ComponentInfo::create(ComponentId{1}, "c");
                    runtime::contract::ReferenceComponent component(std::move(info).value());
                    assert(runtime.register_component(component) && runtime.attach_platform(platform));
                    assert(runtime.initialize() && runtime.start() && runtime.state() == LifecycleState::RUNNING);
                    assert(runtime.stop() && runtime.shutdown());
                }
            }
        }
    }
    assert(gated_out > 20 && gated_in > 20);                                                        // both outcomes are well exercised
}

// ---- 4. differential model: the Runtime is identical with and without a platform ----------------------

void test_runtime_differential_with_the_reference_platform_and_injected_platform_faults() {
    std::mt19937 rng(20261009);
    std::size_t operations = 0, non_ok = 0;
    for (int scenario = 0; scenario < 40; ++scenario) {
        const Script script = random_script(rng);
        World baseline(script, nullptr);
        const Trace expected = run(baseline, script);
        for (const std::string& line : expected) if (line.find("-> ok") == std::string::npos && line.find("->") != std::string::npos) ++non_ok;
        operations += script.steps.size();

        for (int mask = 0; mask < 16; ++mask) {
            ReferencePlatform platform(config(provides_from(mask)));
            // Platform faults on every method, from the first call: irrelevant to a Runtime whose components do not use the platform.
            for (std::size_t m = 0; m < 8; ++m) platform.controls().fail_all(static_cast<Method>(m), ErrorCode::INTERNAL_ERROR);
            World world(script, &platform);
            Hooks hooks;
            std::size_t queries_before = 0;
            hooks.before = [&](World&, std::size_t) { queries_before = platform.controls().adapter_queries(); };
            hooks.after = [&](World& w, std::size_t) {
                assert(platform.controls().adapter_queries() == queries_before);          // the Runtime made no query to the adapter during the operation
                assert(platform.controls().log().empty());                                // and no call to any service, in any state (FAULT and reset included)
                assert(w.runtime.platform() == &platform);                                // the attachment survives every operation
                const PlatformContext context(w.runtime.platform());                      // the integrator keeps using the platform between steps
                (void)context.require_scheduler(); (void)context.require_timer(); (void)context.require_watchdog(); (void)context.require_clock();
                platform.let_time_pass(Duration::from_milliseconds(3));
            };
            const Trace actual = run(world, script, hooks);
            assert(actual == expected);                                                  // results, states, faults, statistics, invocation trace
            assert(platform.controls().calls(Method::SCHEDULER_START) == 0 && platform.controls().calls(Method::WATCHDOG_START) == 0);   // the Runtime started nothing
        }
    }
    assert(operations == 40u * 40u && non_ok > 200);
}

// ---- 5. lifecycle, fault, reset and recovery with a real use of the platform ----------------------------

void test_fault_reset_and_recovery_when_the_platform_recovers() {
    ReferencePlatform platform(config());
    RuntimeManager runtime;
    auto info = ComponentInfo::create(ComponentId{3}, "app");
    AppComponent app(std::move(info).value(), PlatformContext(platform));
    assert(runtime.register_component(app) && runtime.attach_platform(platform));
    platform.controls().fail_all(Method::TIMER_START, ErrorCode::RESOURCE_UNAVAILABLE);          // the platform cannot give a timer right now

    assert(runtime.initialize());
    const auto first = runtime.start();
    assert(!first && first.error().code == ErrorCode::RESOURCE_UNAVAILABLE && first.error().source == ComponentId{3});
    assert(first.error().message == "injected failure");                                         // the platform's own message, unchanged
    assert(runtime.state() == LifecycleState::FAULT && runtime.fault_error()->code == ErrorCode::RESOURCE_UNAVAILABLE);
    assert(!runtime.start() && !runtime.initialize() && !runtime.stop());                       // FAULT accepts only reset()
    assert(runtime.statistics().error_count.value() == 1 && runtime.statistics().sample_count.value() == 1);   // one failed call, and only the successful initialize() counted as a sample

    // Partial platform state is the component's business: the scheduler it had already started is still running.
    assert(platform.fake_scheduler().running());

    assert(runtime.reset() && runtime.state() == LifecycleState::STOPPED && runtime.fault_error() == nullptr);
    assert(platform.fake_scheduler().running());                                                 // the faulted component was shut down, not stopped: what it had started is still the integrator's to clean up
    integrator_heals(platform);                                                                  // the integrator owns the platform and puts its services back
    assert(!platform.fake_scheduler().running());
    platform.controls().clear_faults();                                                          // the platform recovers; the Runtime did not retry anything
    assert(runtime.statistics().retry_count.value() == 0);
    assert(runtime.initialize() && runtime.start() && runtime.state() == LifecycleState::RUNNING);   // a NEW explicit attempt
    assert(platform.fake_timer().running() && platform.fake_watchdog().running());
    assert(runtime.stop() && runtime.shutdown());
    assert(!platform.fake_timer().running() && !platform.fake_watchdog().running());
}

// ---- 6. statistics are the Runtime's own -----------------------------------------------------------------

void test_statistics_count_component_calls_not_platform_calls() {
    for (const bool with_platform_use : {false, true}) {
        ReferencePlatform platform(config());
        RuntimeManager runtime;
        auto i1 = ComponentInfo::create(ComponentId{1}, "c"), i2 = ComponentInfo::create(ComponentId{2}, "c");
        runtime::contract::ReferenceComponent plain(std::move(i1).value());
        AppComponent app(std::move(i2).value(), PlatformContext(platform));
        // The platform-using component makes many platform calls per operation; the plain one makes none.
        assert(runtime.register_component(plain));
        if (with_platform_use) assert(runtime.register_component(app));
        assert(runtime.attach_platform(platform));
        assert(runtime.initialize() && runtime.start() && runtime.stop() && runtime.shutdown());
        const std::uint64_t components = with_platform_use ? 2 : 1;
        assert(runtime.statistics().sample_count.value() == components * 4);                     // initialize, start, stop, shutdown per component
        assert(runtime.statistics().error_count.value() == 0 && runtime.statistics().retry_count.value() == 0);
        if (with_platform_use) assert(platform.controls().log().size() >= 8);                    // plenty of platform calls, none counted by the Runtime
    }
}

// ---- 7. independence: watchdog expiry and callbacks never reach the Runtime ----------------------------------

void test_watchdog_expiry_and_callbacks_never_enter_the_runtime() {
    ReferencePlatform platform(config());
    RuntimeManager runtime;
    auto info = ComponentInfo::create(ComponentId{1}, "app");
    AppComponent app(std::move(info).value(), PlatformContext(platform));
    assert(runtime.register_component(app) && runtime.attach_platform(platform));
    assert(runtime.initialize() && runtime.start());
    platform.fake_watchdog().advance(5000 * MS);                                                 // the watchdog expires and nobody kicks it
    platform.let_time_pass(Duration::from_milliseconds(50));                                     // callbacks and task entries run
    assert(platform.fake_watchdog().expired() && app.runs.value > 0);
    assert(runtime.state() == LifecycleState::RUNNING && runtime.fault_error() == nullptr);      // no FAULT, no reset, no recovery
    assert(runtime.statistics().error_count.value() == 0);
    assert(!runtime.reset());                                                                     // reset() is still explicit and FAULT-only
    assert(runtime.stop() && runtime.shutdown());
}

// ---- 8. lifetime across the integration ----------------------------------------------------------------------

void test_the_integrator_owns_the_platform_and_the_runtime_never_does() {
    LifetimeProbe probe;
    ReferencePlatform::Config c = config();
    c.probe = &probe;
    auto platform = std::make_unique<ReferencePlatform>(c);
    {
        RuntimeManager runtime;
        auto info = ComponentInfo::create(ComponentId{1}, "app");
        AppComponent app(std::move(info).value(), PlatformContext(*platform));
        assert(runtime.register_component(app) && runtime.attach_platform(*platform));
        assert(runtime.initialize() && runtime.start() && runtime.stop() && runtime.shutdown());
    }                                                                                              // Runtime and component are gone
    assert(!probe.platform_destroyed && probe.services_destroyed == 0);
    platform.reset();
    assert(probe.platform_destroyed && probe.services_destroyed == 4);
}

// ---- 9. attachment is closed once the topology is fixed, whatever happened since ---------------------------

void test_attachment_is_closed_after_the_topology_is_fixed_in_every_state() {
    ReferencePlatform platform(config());
    RuntimeManager runtime;
    auto info = ComponentInfo::create(ComponentId{1}, "c");
    runtime::contract::ReferenceComponent component(std::move(info).value());
    assert(runtime.register_component(component));
    assert(runtime.initialize());                                                                  // fixes the topology with NO adapter attached
    auto expect_closed = [&](const char*) {
        const auto r = runtime.attach_platform(platform);
        assert(!r && r.error().code == ErrorCode::INVALID_STATE && runtime.platform() == nullptr);
    };
    expect_closed("READY");
    assert(runtime.start());
    expect_closed("RUNNING");
    assert(runtime.stop());
    expect_closed("STOPPED");
    component.fail_next_start = ErrorCode::TIMEOUT;
    assert(runtime.initialize() && !runtime.start() && runtime.state() == LifecycleState::FAULT);
    expect_closed("FAULT");
    assert(runtime.reset());
    expect_closed("after reset");
    assert(platform.controls().adapter_queries() == 0);                                            // a rejected attach never touched the adapter
}

} // namespace

int main() {
    test_cross_service_integration_follows_dependency_order();
    test_a_platform_failure_is_exactly_an_ordinary_component_failure();
    test_every_availability_combination_gives_the_derived_outcome_ungated();
    test_requirements_gate_the_runtime_deterministically_for_every_combination();
    test_runtime_differential_with_the_reference_platform_and_injected_platform_faults();
    test_fault_reset_and_recovery_when_the_platform_recovers();
    test_statistics_count_component_calls_not_platform_calls();
    test_watchdog_expiry_and_callbacks_never_enter_the_runtime();
    test_the_integrator_owns_the_platform_and_the_runtime_never_does();
    test_attachment_is_closed_after_the_topology_is_fixed_in_every_state();
    return 0;
}
