//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component_context_integration_test.cpp
// Description : Runtime, Component, ComponentContext and platform integration through public APIs.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-CTX-006
// API         : CORE-TEST-COMPONENT-CONTEXT-INTEGRATION
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

// Public APIs only: RuntimeManager, Component, ComponentContext, PlatformRequirements, the service contracts,
// the test-only reference platform and the test-only reference context component. No private detail of the
// Runtime, a context or a platform is touched and no hardware is needed.
//
// The central proof is EQUIVALENCE: components that use their ComponentContext are run on a Runtime next to plain
// reference components; whatever the platform does (succeed, fail, be unavailable) the Runtime must see exactly
// what it sees from a plain component failing in the same place, with the failing component as the source.

#include <algorithm>
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
#include "../runtime/reference_context_component.hpp"
#include "runtime_scenarios.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;
using namespace kritva::core::runtime::scenarios;
using namespace kritva::core::runtime::contract;
using namespace kritva::core::platform;
using kritva::core::platform::testing::LifetimeProbe;
using kritva::core::platform::testing::Method;
using kritva::core::platform::testing::ReferencePlatform;

namespace {

constexpr std::int64_t MS = 1'000'000;
using S = ContextStep;

CapabilitySet capabilities_100() {
    CapabilitySet set;
    set.add(Capability{CapabilityId{100}, "gpio", Version{1, 0, 0}});
    return set;
}

ReferencePlatform::Config config(ReferencePlatform::Provides provides = {}, bool with_capability = true, LifetimeProbe* probe = nullptr) {
    ReferencePlatform::Config c;
    c.provides = provides;
    c.probe = probe;
    if (with_capability) c.capabilities = capabilities_100();
    return c;
}

ReferencePlatform::Provides provides_from(int mask) {
    return ReferencePlatform::Provides{(mask & 1) != 0, (mask & 2) != 0, (mask & 4) != 0, (mask & 8) != 0};
}

ComponentInfo make_info(std::uint64_t id) {
    auto info = ComponentInfo::create(ComponentId{id}, "c");
    assert(info.has_value());
    return std::move(info).value();
}

// initialize: create a task; start: scheduler, timer, watchdog, kick, clock; stop: the reverse.
const ContextPlans kFullPlans{
    {},
    {S::REQUIRE_SCHEDULER, S::CREATE_TASK},
    {S::START_SCHEDULER, S::START_TIMER, S::START_WATCHDOG, S::KICK_WATCHDOG, S::REQUIRE_CLOCK},
    {S::STOP_WATCHDOG, S::STOP_TIMER, S::STOP_SCHEDULER},
    {},
};

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

Result<void> do_op(RuntimeManager& runtime, const std::string& op) {
    if (op == "initialize") return runtime.initialize();
    if (op == "start") return runtime.start();
    if (op == "stop") return runtime.stop();
    if (op == "shutdown") return runtime.shutdown();
    return runtime.reset();
}

// What the integrator does between attempts: it owns the platform, so it puts its services back into a known state.
// Straight to the underlying doubles: neither counted nor subject to injected faults.
void integrator_heals(ReferencePlatform& platform) {
    (void)platform.fake_watchdog().kritva::core::platform::contract::ReferenceWatchdog::stop();
    (void)platform.fake_timer().kritva::core::time::contract::ReferenceTimer::stop();
    (void)platform.fake_scheduler().kritva::core::platform::contract::ReferenceScheduler::stop();
}

// ---- 1. a platform failure through a context is exactly an ordinary component failure -----------------------

struct Where { Method method; const char* op; };
constexpr Where kWhere[] = {
    {Method::SCHEDULER_CREATE_TASK, "initialize"}, {Method::SCHEDULER_START, "start"}, {Method::TIMER_START, "start"},
    {Method::WATCHDOG_START, "start"}, {Method::WATCHDOG_KICK, "start"}, {Method::WATCHDOG_STOP, "stop"},
    {Method::TIMER_STOP, "stop"}, {Method::SCHEDULER_STOP, "stop"},
};

void arm(ReferenceComponent& c, const std::string& op, ErrorCode code) {
    if (op == "initialize") c.fail_next_initialize = code;
    else if (op == "start") c.fail_next_start = code;
    else if (op == "stop") c.fail_next_stop = code;
}

std::vector<std::string> chain_with_context(const Where& where, std::size_t nth, ErrorCode code) {
    ReferencePlatform platform(config());
    platform.controls().fail_nth(where.method, nth, code);
    RuntimeManager runtime;
    Trace trace;
    ReferenceComponent c1(make_info(1)), c3(make_info(3));
    ReferenceContextComponent c2(make_info(2), PlatformContext(platform), kFullPlans);
    c1.trace = c2.trace = c3.trace = &trace;                                        // all three are traced: the whole chain must match
    assert(runtime.register_component(c1) && runtime.register_component(c2) && runtime.register_component(c3));
    assert(runtime.add_dependency(ComponentId{2}, ComponentId{1}) && runtime.add_dependency(ComponentId{3}, ComponentId{2}));
    assert(runtime.attach_platform(platform));
    std::vector<std::string> out;
    for (std::size_t cycle = 1; cycle <= 3; ++cycle) {
        for (const char* op : kOps) out.push_back(outcome(runtime, do_op(runtime, op), op));
        integrator_heals(platform);
    }
    for (const std::string& line : trace) out.push_back(line);
    return out;
}

std::vector<std::string> chain_with_plain_failure(const Where& where, std::size_t nth, ErrorCode code) {
    RuntimeManager runtime;
    Trace trace;
    ReferenceComponent c1(make_info(1)), c2(make_info(2)), c3(make_info(3));
    c1.trace = c2.trace = c3.trace = &trace;
    assert(runtime.register_component(c1) && runtime.register_component(c2) && runtime.register_component(c3));
    assert(runtime.add_dependency(ComponentId{2}, ComponentId{1}) && runtime.add_dependency(ComponentId{3}, ComponentId{2}));
    std::vector<std::string> out;
    for (std::size_t cycle = 1; cycle <= 3; ++cycle) {
        for (const char* op : kOps) {
            if (cycle == nth && std::string(op) == where.op) arm(c2, op, code);     // the same failure, in the same place, as a plain component failure
            out.push_back(outcome(runtime, do_op(runtime, op), op));
        }
    }
    for (const std::string& line : trace) out.push_back(line);
    return out;
}

void test_a_platform_failure_through_a_context_is_exactly_an_ordinary_component_failure() {
    std::size_t compared = 0, faulting = 0;
    for (const Where& where : kWhere) {
        for (std::size_t nth = 1; nth <= 3; ++nth) {
            for (const ErrorCode code : {ErrorCode::TIMEOUT, ErrorCode::RESOURCE_UNAVAILABLE, ErrorCode::INTERNAL_ERROR}) {
                const auto with_context = chain_with_context(where, nth, code);
                const auto plain = chain_with_plain_failure(where, nth, code);
                if (with_context != plain) {                                        // show the first difference before failing
                    for (std::size_t i = 0; i < std::min(with_context.size(), plain.size()); ++i)
                        if (with_context[i] != plain[i]) { fprintf(stderr, "method %d nth %zu code %d line %zu\n  context: %s\n  plain:   %s\n", static_cast<int>(where.method), nth, static_cast<int>(code), i, with_context[i].c_str(), plain[i].c_str()); break; }
                }
                assert(with_context == plain);                                      // states, faults, code and source, statistics, reset cleanup and the full invocation trace
                ++compared;
                for (const std::string& line : with_context) if (line.find("state=" + std::to_string(static_cast<int>(LifecycleState::FAULT))) != std::string::npos) { ++faulting; break; }
            }
        }
    }
    assert(compared == 8u * 3u * 3u);
    assert(faulting == compared);                                                   // every injected platform failure really reached the Runtime as a fault
}

// ---- 2. service availability ----------------------------------------------------------------------------------

struct Expected { const char* failing_op; const char* missing; };

Expected expected_for(const ReferencePlatform::Provides& p) {
    if (!p.scheduler) return {"initialize", "scheduler"};                           // create_task needs a scheduler
    if (!p.timer) return {"start", "timer"};                                        // start needs scheduler, timer, watchdog, clock in that order
    if (!p.watchdog) return {"start", "watchdog"};
    if (!p.clock) return {"start", "clock"};
    return {"", ""};
}

void test_every_availability_combination_gives_the_derived_outcome() {
    for (int mask = 0; mask < 16; ++mask) {
        const ReferencePlatform::Provides provides = provides_from(mask);
        ReferencePlatform platform(config(provides));
        RuntimeManager runtime;
        ReferenceContextComponent component(make_info(5), PlatformContext(platform), kFullPlans);
        assert(runtime.register_component(component) && runtime.attach_platform(platform));
        const Expected expected = expected_for(provides);
        const auto init = runtime.initialize();
        const auto start = init ? runtime.start() : Result<void>::success();
        const std::string failing = expected.failing_op;
        const PlatformContext view(platform);
        if (failing == "initialize") {
            assert(!init && init.error().code == ErrorCode::UNSUPPORTED && init.error().source == ComponentId{5});
            assert(init.error().message.find(expected.missing) != std::string::npos);
            // Exactly the R0.5 error with only the source added: severity, timestamp and message are PlatformContext's.
            assert(init.error().severity == ErrorSeverity::ERROR && init.error().timestamp == Timestamp{});
            assert(init.error().message == view.require_scheduler().error().message);
        } else if (failing == "start") {
            assert(init && !start && start.error().code == ErrorCode::UNSUPPORTED && start.error().source == ComponentId{5});
            assert(start.error().message.find(expected.missing) != std::string::npos);
            assert(start.error().severity == ErrorSeverity::ERROR && start.error().timestamp == Timestamp{});
            const std::string r05 = std::string(expected.missing) == "timer" ? view.require_timer().error().message
                                  : std::string(expected.missing) == "watchdog" ? view.require_watchdog().error().message : view.require_clock().error().message;
            assert(start.error().message == r05);
        } else {
            assert(init && start && runtime.state() == LifecycleState::RUNNING);
        }
        if (failing.empty()) {
            assert(runtime.stop() && runtime.shutdown());
        } else {
            assert(runtime.state() == LifecycleState::FAULT && runtime.fault_error()->source == ComponentId{5});
            assert(runtime.reset() && runtime.state() == LifecycleState::STOPPED);
            assert(runtime.statistics().retry_count.value() == 0);                  // explicit recovery, no retry
        }
    }
}

// ---- 3. requirement checks through the context --------------------------------------------------------------------

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

void test_requirement_checks_through_a_context_gate_a_component_deterministically() {
    const std::vector<Need> needs = {
        {{{PlatformService::SCHEDULER, Requirement::REQUIRED}}, {}},
        {{{PlatformService::SCHEDULER, Requirement::REQUIRED}, {PlatformService::CLOCK, Requirement::REQUIRED}, {PlatformService::TIMER, Requirement::REQUIRED}, {PlatformService::WATCHDOG, Requirement::REQUIRED}}, {}},
        {{{PlatformService::TIMER, Requirement::OPTIONAL}}, {{100, Requirement::REQUIRED}}},
        {{{PlatformService::WATCHDOG, Requirement::OPTIONAL}, {PlatformService::CLOCK, Requirement::OPTIONAL}}, {{999, Requirement::OPTIONAL}}},
    };
    const ContextPlans plans{{}, {S::CHECK_REQUIREMENTS}, {}, {}, {}};
    std::size_t gated_out = 0, gated_in = 0;
    for (int mask = 0; mask < 16; ++mask) {
        for (const bool has_capability : {true, false}) {
            for (const Need& need : needs) {
                ReferencePlatform platform(config(provides_from(mask), has_capability));
                PlatformRequirements requirements;
                for (const auto& [service, level] : need.services) assert(requirements.add_service(service, level));
                for (const auto& [id, level] : need.capabilities) assert(requirements.add_capability(CapabilityId{id}, level));
                RuntimeManager runtime;
                ReferenceContextComponent component(make_info(4), PlatformContext(platform), plans, requirements);
                assert(runtime.register_component(component) && runtime.attach_platform(platform));
                const bool expected = oracle_satisfied(need, provides_from(mask), has_capability);
                const auto initialized = runtime.initialize();
                assert(initialized.has_value() == expected);
                assert(platform.controls().log().empty());                          // checking is a query: no service was touched
                if (expected) {
                    ++gated_in;
                    assert(runtime.state() == LifecycleState::READY && runtime.start() && runtime.stop() && runtime.shutdown());
                } else {
                    ++gated_out;
                    assert(initialized.error().code == ErrorCode::UNSUPPORTED && initialized.error().source == ComponentId{4});
                    assert(initialized.error().severity == ErrorSeverity::ERROR && initialized.error().timestamp == Timestamp{});
                    assert(initialized.error().message == check_required(requirements, PlatformContext(platform)).error().message);   // exactly R0.5's wording
                    assert(runtime.state() == LifecycleState::FAULT && runtime.statistics().error_count.value() == 1 && runtime.statistics().retry_count.value() == 0);
                    assert(runtime.reset() && runtime.state() == LifecycleState::STOPPED);
                }
            }
        }
    }
    assert(gated_out > 20 && gated_in > 20);                                        // both outcomes are well exercised
}

// ---- 4. differential model: the Runtime is identical whatever components do with their context ------------------------

void test_runtime_differential_with_components_that_use_their_context() {
    PlatformRequirements needs;
    assert(needs.add_service(PlatformService::SCHEDULER, Requirement::OPTIONAL) && needs.add_capability(CapabilityId{100}, Requirement::OPTIONAL));
    // Queries only: they never fail the operation, whatever the platform is or does.
    const ContextPlans queries{{S::QUERY_SUPPORT, S::QUERY_CAPABILITY, S::EVALUATE_REQUIREMENTS},
                               {S::QUERY_SUPPORT, S::QUERY_CAPABILITY, S::EVALUATE_REQUIREMENTS},
                               {S::QUERY_SUPPORT, S::QUERY_CAPABILITY, S::EVALUATE_REQUIREMENTS},
                               {S::QUERY_SUPPORT, S::QUERY_CAPABILITY, S::EVALUATE_REQUIREMENTS},
                               {S::QUERY_SUPPORT, S::QUERY_CAPABILITY, S::EVALUATE_REQUIREMENTS}};
    std::mt19937 rng(20261011);
    std::size_t operations = 0, non_ok = 0;
    for (int scenario = 0; scenario < 40; ++scenario) {
        const Script script = random_script(rng);
        World baseline(script, nullptr);
        const Trace expected = run(baseline, script);
        for (const std::string& line : expected) if (line.find("-> ok") == std::string::npos && line.find("->") != std::string::npos) ++non_ok;
        operations += script.steps.size();

        for (int mask = 0; mask < 16; ++mask) {
            ReferencePlatform platform(config(provides_from(mask)));
            for (std::size_t m = 0; m < 8; ++m) platform.controls().fail_all(static_cast<Method>(m), ErrorCode::INTERNAL_ERROR);   // every service method fails
            World world(script, &platform, [&](ComponentInfo info) -> std::unique_ptr<ReferenceComponent> {
                return std::make_unique<ReferenceContextComponent>(std::move(info), PlatformContext(platform), queries, needs);
            });
            const Trace actual = run(world, script);
            assert(actual == expected);                                             // results, states, faults, statistics, retries, invocation traces
            assert(platform.controls().log().empty());                              // the contexts used no service
        }
    }
    assert(operations == 40u * 40u && non_ok > 200);
}

// ---- 5. fault, reset and recovery -----------------------------------------------------------------------------------------

void test_fault_reset_and_recovery_when_the_platform_recovers() {
    ReferencePlatform platform(config());
    RuntimeManager runtime;
    ReferenceContextComponent component(make_info(3), PlatformContext(platform), kFullPlans);
    assert(runtime.register_component(component) && runtime.attach_platform(platform));
    platform.controls().fail_all(Method::TIMER_START, ErrorCode::RESOURCE_UNAVAILABLE);
    assert(runtime.initialize());
    const auto first = runtime.start();
    assert(!first && first.error().code == ErrorCode::RESOURCE_UNAVAILABLE && first.error().source == ComponentId{3});
    assert(first.error().message == "injected failure");                            // the platform's own message, unchanged
    assert(first.error().severity == ErrorSeverity::ERROR && first.error().timestamp == Timestamp{});   // and every other field of the service's error
    assert(runtime.state() == LifecycleState::FAULT && runtime.fault_error()->code == ErrorCode::RESOURCE_UNAVAILABLE);
    assert(!runtime.start() && !runtime.initialize() && !runtime.stop());           // FAULT accepts only reset()
    assert(runtime.statistics().error_count.value() == 1 && runtime.statistics().sample_count.value() == 1);
    assert(platform.fake_scheduler().running());                                    // the component's partial platform state is the integrator's to clean up
    assert(runtime.reset() && runtime.state() == LifecycleState::STOPPED && runtime.fault_error() == nullptr);
    integrator_heals(platform);
    platform.controls().clear_faults();                                             // the platform recovers; the Runtime retried nothing
    assert(runtime.statistics().retry_count.value() == 0);
    assert(runtime.initialize() && runtime.start() && runtime.state() == LifecycleState::RUNNING);   // a NEW explicit attempt
    assert(platform.fake_timer().running() && platform.fake_watchdog().running());
    assert(runtime.stop() && runtime.shutdown());
    assert(!platform.fake_timer().running() && !platform.fake_watchdog().running());
}

void test_a_failed_stop_is_recorded_as_faulted_and_reset_shuts_that_component_down() {
    ReferencePlatform platform(config());
    RuntimeManager runtime;
    ReferenceContextComponent first(make_info(1), PlatformContext(platform), ContextPlans{});
    ReferenceContextComponent second(make_info(2), PlatformContext(platform), kFullPlans);
    assert(runtime.register_component(first) && runtime.register_component(second) && runtime.add_dependency(ComponentId{2}, ComponentId{1}));
    platform.controls().fail_nth(Method::WATCHDOG_STOP, 1, ErrorCode::TIMEOUT);
    assert(runtime.initialize() && runtime.start());
    const auto stopped = runtime.stop();                                             // reverse order: component 2 stops first and fails
    assert(!stopped && stopped.error().source == ComponentId{2} && runtime.state() == LifecycleState::FAULT);
    assert(second.stop_calls == 1 && first.stop_calls == 0);                         // fail-fast: component 1 was never stopped
    assert(runtime.reset() && runtime.state() == LifecycleState::STOPPED);
    assert(second.stop_calls == 1);                                                  // the failed stop was recorded as faulted: reset() does not stop it again ...
    assert(second.shutdown_calls == 1);                                              // ... but shuts it down
    assert(first.stop_calls == 1 && first.shutdown_calls == 1);                      // component 1 (initialized/started) is stopped then shut down
}

// ---- 6. statistics -----------------------------------------------------------------------------------------------------------

void test_statistics_count_component_calls_not_context_queries() {
    for (const bool with_context_use : {false, true}) {
        ReferencePlatform platform(config());
        RuntimeManager runtime;
        const ContextPlans noisy{{S::QUERY_SUPPORT, S::QUERY_CAPABILITY, S::QUERY_SUPPORT}, {S::QUERY_SUPPORT, S::QUERY_CAPABILITY, S::QUERY_SUPPORT},
                                 {S::QUERY_SUPPORT, S::QUERY_CAPABILITY, S::QUERY_SUPPORT}, {S::QUERY_SUPPORT, S::QUERY_CAPABILITY, S::QUERY_SUPPORT},
                                 {S::QUERY_SUPPORT, S::QUERY_CAPABILITY, S::QUERY_SUPPORT}};
        ReferenceContextComponent chatty(make_info(1), PlatformContext(platform), with_context_use ? noisy : ContextPlans{});
        ReferenceContextComponent quiet(make_info(2), PlatformContext(platform), ContextPlans{});
        assert(runtime.register_component(chatty) && runtime.register_component(quiet) && runtime.attach_platform(platform));
        assert(runtime.configure(Configuration{}) && runtime.initialize() && runtime.start() && runtime.stop() && runtime.shutdown());
        assert(runtime.statistics().sample_count.value() == 2u * 5u);               // configure, initialize, start, stop, shutdown per component
        assert(runtime.statistics().error_count.value() == 0 && runtime.statistics().retry_count.value() == 0);
        if (with_context_use) assert(platform.controls().adapter_queries() == 15);  // 3 context queries x 5 operations, none counted by the Runtime
    }
}

// ---- 7. source attribution is always the failing component ------------------------------------------------------------------

void test_the_source_of_a_failure_is_always_the_failing_component() {
    // Four components in a chain; each in turn needs a service the platform lacks. The Runtime error always names that component.
    for (std::uint64_t failing = 1; failing <= 4; ++failing) {
        ReferencePlatform platform(config({true, true, false, true}));                      // no timer
        RuntimeManager runtime;
        std::vector<std::unique_ptr<ReferenceContextComponent>> components;
        for (std::uint64_t id = 1; id <= 4; ++id) {
            const ContextPlans plans{{}, {}, id == failing ? std::vector<ContextStep>{S::REQUIRE_TIMER} : std::vector<ContextStep>{S::REQUIRE_SCHEDULER}, {}, {}};
            components.push_back(std::make_unique<ReferenceContextComponent>(make_info(id), PlatformContext(platform), plans));
            assert(runtime.register_component(*components.back()));
        }
        for (std::uint64_t id = 2; id <= 4; ++id) assert(runtime.add_dependency(ComponentId{id}, ComponentId{id - 1}));
        assert(runtime.initialize());
        const auto started = runtime.start();
        assert(!started && started.error().code == ErrorCode::UNSUPPORTED && started.error().source == ComponentId{failing});
        assert(runtime.fault_error()->source == ComponentId{failing});
        // Fail-fast: components after the failing one were never started.
        for (std::uint64_t id = failing + 1; id <= 4; ++id) assert(components[id - 1]->start_calls == 0);
        for (std::uint64_t id = 1; id <= failing; ++id) assert(components[id - 1]->start_calls == 1);
        assert(runtime.reset());
    }
    // Two components sharing a platform attribute to their own ids: contexts do not leak into each other.
    ReferencePlatform platform(config({false, false, false, false}));
    ReferenceContextComponent a(make_info(10), PlatformContext(platform), kFullPlans), b(make_info(20), PlatformContext(platform), kFullPlans);
    const auto from_a = a.initialize();
    const auto from_b = b.initialize();
    assert(!from_a && from_a.error().source == ComponentId{10} && !from_b && from_b.error().source == ComponentId{20});
    assert(a.context().id() == ComponentId{10} && b.context().id() == ComponentId{20});
}

// ---- 8. watchdog expiry, callbacks and time never enter the Runtime ------------------------------------------------------------

void test_watchdog_expiry_and_callbacks_never_enter_the_runtime() {
    ReferencePlatform platform(config());
    RuntimeManager runtime;
    ReferenceContextComponent component(make_info(1), PlatformContext(platform), kFullPlans);
    assert(runtime.register_component(component) && runtime.attach_platform(platform));
    assert(runtime.initialize() && runtime.start());
    platform.fake_watchdog().advance(5000 * MS);                                            // the watchdog expires and nobody kicks it
    platform.let_time_pass(Duration::from_milliseconds(50));                                // callbacks and task entries run
    assert(platform.fake_watchdog().expired() && component.task_runs() > 0);
    assert(runtime.state() == LifecycleState::RUNNING && runtime.fault_error() == nullptr);   // no FAULT, no reset, no recovery
    assert(runtime.statistics().error_count.value() == 0 && !runtime.reset());
    assert(runtime.stop() && runtime.shutdown());
}

// ---- 9. ownership and lifetime across the integration --------------------------------------------------------------------------

void test_the_integrator_owns_the_platform_and_the_runtime_and_contexts_never_do() {
    LifetimeProbe probe;
    bool component_destroyed = false;
    auto platform = std::make_unique<ReferencePlatform>(config({}, true, &probe));
    {
        RuntimeManager runtime;
        ReferenceContextComponent component(make_info(1), PlatformContext(*platform), kFullPlans, {}, &component_destroyed);
        assert(runtime.register_component(component) && runtime.attach_platform(*platform));
        assert(runtime.initialize() && runtime.start() && runtime.stop() && runtime.shutdown());
        assert(component.context().info() == &component.info());                           // valid for as long as the component lives
    }                                                                                       // the Runtime and the component (and its context) are gone
    assert(component_destroyed && !probe.platform_destroyed && probe.services_destroyed == 0);
    platform.reset();
    assert(probe.platform_destroyed && probe.services_destroyed == 4);
}

} // namespace

int main() {
    test_a_platform_failure_through_a_context_is_exactly_an_ordinary_component_failure();
    test_every_availability_combination_gives_the_derived_outcome();
    test_requirement_checks_through_a_context_gate_a_component_deterministically();
    test_runtime_differential_with_components_that_use_their_context();
    test_fault_reset_and_recovery_when_the_platform_recovers();
    test_a_failed_stop_is_recorded_as_faulted_and_reset_shuts_that_component_down();
    test_statistics_count_component_calls_not_context_queries();
    test_the_source_of_a_failure_is_always_the_failing_component();
    test_watchdog_expiry_and_callbacks_never_enter_the_runtime();
    test_the_integrator_owns_the_platform_and_the_runtime_and_contexts_never_do();
    return 0;
}
