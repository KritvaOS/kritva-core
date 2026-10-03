//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : reference_context_test.cpp
// Description : Tests of the test-only reference context harness and the reusable context contract checks.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-CTX-005
// API         : CORE-TEST-REFERENCE-CONTEXT-RUN
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <kritva/core/core.hpp>

#include "../contract/reference_adapter.hpp"
#include "../platform/context_conformance.hpp"
#include "../platform/reference_platform.hpp"
#include "../runtime/reference_context_component.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;
using namespace kritva::core::runtime::contract;
using namespace kritva::core::platform;
using kritva::core::platform::testing::Controls;
using kritva::core::platform::testing::LifetimeProbe;
using kritva::core::platform::testing::Method;
using kritva::core::platform::testing::ReferencePlatform;

namespace {

CapabilitySet capabilities_100() {
    CapabilitySet set;
    set.add(Capability{CapabilityId{100}, "gpio", Version{1, 0, 0}});
    return set;
}

ReferencePlatform::Config config(ReferencePlatform::Provides provides = {}, LifetimeProbe* probe = nullptr, bool with_capability = true) {
    ReferencePlatform::Config c;
    c.provides = provides;
    c.probe = probe;
    if (with_capability) c.capabilities = capabilities_100();
    return c;
}

ReferencePlatform::Provides provides_from(int mask) {
    return ReferencePlatform::Provides{(mask & 1) != 0, (mask & 2) != 0, (mask & 4) != 0, (mask & 8) != 0};
}

ComponentInfo make_info(std::uint64_t id = 7) {
    auto info = ComponentInfo::create(ComponentId{id}, "ref-ctx", Version{1, 0, 0});
    assert(info.has_value());
    return std::move(info).value();
}

using S = ContextStep;
const ContextPlans kFullPlans{
    {},                                                                                   // configure
    {S::REQUIRE_SCHEDULER, S::CREATE_TASK},                                               // initialize
    {S::START_SCHEDULER, S::START_TIMER, S::START_WATCHDOG, S::KICK_WATCHDOG, S::REQUIRE_CLOCK},   // start
    {S::STOP_WATCHDOG, S::STOP_TIMER, S::STOP_SCHEDULER},                                 // stop
    {},                                                                                   // shutdown
};

// ---- the reusable contract checks --------------------------------------------------------------

void test_the_contract_checks_pass_for_every_platform_and_every_kind_of_context() {
    const ComponentInfo info = make_info();
    std::size_t runs = 0;
    for (int mask = 0; mask < 16; ++mask) {
        for (const bool with_capability : {true, false}) {
            ReferencePlatform platform(config(provides_from(mask), nullptr, with_capability));
            // bound, with the platform
            {
                const ComponentContext context(info, PlatformContext(platform));
                conformance::Report report;
                conformance::check_component_context(context, &platform, report);
                assert(report.ok() && report.checks() > 30);
                ++runs;
            }
            // bound, built from a nullable pointer
            {
                const ComponentContext context(info, PlatformContext(static_cast<IPlatformAdapter*>(&platform)));
                conformance::Report report;
                conformance::check_component_context(context, &platform, report);
                assert(report.ok());
            }
            {   // copies and moves are contexts like any other
                const ComponentContext original(info, PlatformContext(platform));
                const ComponentContext copy(original);
                ComponentContext moved(ComponentContext(info, PlatformContext(platform)));
                conformance::Report copy_report, move_report;
                conformance::check_component_context(copy, &platform, copy_report);
                conformance::check_component_context(moved, &platform, move_report);
                assert(copy_report.ok() && move_report.ok());
            }
            assert(platform.controls().log().empty());                                     // checking started and used no service
        }
    }
    // bound without a platform, and unbound
    {
        const ComponentContext bare(info);
        conformance::Report report;
        conformance::check_component_context(bare, nullptr, report);
        assert(report.ok());
    }
    {
        const ComponentContext unbound;
        conformance::Report report;
        conformance::check_component_context(unbound, nullptr, report);
        assert(report.ok());
    }
    assert(runs == 32);
}

void test_the_contract_checks_notice_a_context_over_the_wrong_platform() {
    const ComponentInfo info = make_info();
    ReferencePlatform platform(config());
    ReferencePlatform other(config({false, false, false, false}));
    const ComponentContext context(info, PlatformContext(platform));
    conformance::Report wrong;
    conformance::check_component_context(context, &other, wrong);                          // claims a different adapter
    assert(!wrong.ok());
    conformance::Report none_claimed;
    conformance::check_component_context(context, nullptr, none_claimed);                 // claims no platform although one is attached
    assert(!none_claimed.ok());
    const ComponentContext bare(info);
    conformance::Report attached_claimed;
    conformance::check_component_context(bare, &platform, attached_claimed);              // claims a platform the context does not have
    assert(!attached_claimed.ok());
}

// ---- the reference component executes plans through its context ------------------------------------------

void test_a_scripted_plan_drives_the_platform_in_order_through_the_context() {
    ReferencePlatform platform(config());
    ReferenceContextComponent component(make_info(), PlatformContext(platform), kFullPlans);
    assert(component.context().bound() && component.context().info() == &component.info());   // built from its own identity
    Configuration configuration;
    assert(component.configure(configuration));
    assert(component.initialize() && component.start());
    assert(component.lifecycle_state() == LifecycleState::RUNNING);
    const std::vector<Method> expected = {Method::SCHEDULER_CREATE_TASK, Method::SCHEDULER_START, Method::TIMER_START, Method::WATCHDOG_START, Method::WATCHDOG_KICK};
    assert(platform.controls().log().size() == expected.size());
    for (std::size_t i = 0; i < expected.size(); ++i) assert(platform.controls().log()[i].method == expected[i] && platform.controls().log()[i].ok);
    assert(component.executed().size() == 2 + 5);                                          // every executed step is recorded
    for (const StepRecord& r : component.executed()) assert(r.ok && r.code == ErrorCode::NONE && !r.source.valid());
    assert(component.executed()[0].step == S::REQUIRE_SCHEDULER && component.executed()[1].step == S::CREATE_TASK);
    platform.controls().clear_log();
    assert(component.stop() && component.lifecycle_state() == LifecycleState::STOPPED);
    assert(platform.controls().log().size() == 3 && platform.controls().log()[0].method == Method::WATCHDOG_STOP
           && platform.controls().log()[1].method == Method::TIMER_STOP && platform.controls().log()[2].method == Method::SCHEDULER_STOP);   // reverse
    assert(component.shutdown());
    assert(component.task_runs() == 0);                                                    // nothing ran: time never advanced by itself
    platform.let_time_pass(Duration::from_milliseconds(1));
}

void test_every_unavailable_service_is_an_attributed_unsupported_failure_and_the_component_faults() {
    for (int mask = 0; mask < 16; ++mask) {
        const ReferencePlatform::Provides provides = provides_from(mask);
        ReferencePlatform platform(config(provides));
        ReferenceContextComponent component(make_info(9), PlatformContext(platform), kFullPlans);
        const auto init = component.initialize();
        if (!provides.scheduler) {                                                         // the first thing initialize() needs
            assert(!init && init.error().code == ErrorCode::UNSUPPORTED && init.error().source == ComponentId{9});
            assert(init.error().message.find("scheduler") != std::string::npos);
            assert(component.lifecycle_state() == LifecycleState::FAULT);                            // a failed operation leaves FAULT
            assert(component.executed().back().step == S::REQUIRE_SCHEDULER && !component.executed().back().ok && component.executed().back().source == ComponentId{9});
            continue;
        }
        assert(init);
        const auto start = component.start();
        // start() needs the scheduler, the timer, the watchdog and then the clock, in that order.
        const char* missing = !provides.timer ? "timer" : !provides.watchdog ? "watchdog" : !provides.clock ? "clock" : nullptr;
        if (missing != nullptr) {
            assert(!start && start.error().code == ErrorCode::UNSUPPORTED && start.error().source == ComponentId{9});
            assert(start.error().message.find(missing) != std::string::npos);
            assert(component.lifecycle_state() == LifecycleState::FAULT);
        } else {
            assert(start && component.lifecycle_state() == LifecycleState::RUNNING);
        }
    }
}

void test_a_platform_service_error_is_returned_unchanged_except_for_its_source() {
    ReferencePlatform platform(config());
    platform.controls().fail_nth(Method::TIMER_START, 1, ErrorCode::RESOURCE_UNAVAILABLE);
    ReferenceContextComponent component(make_info(5), PlatformContext(platform), kFullPlans);
    assert(component.initialize());
    const auto start = component.start();
    assert(!start);
    assert(start.error().code == ErrorCode::RESOURCE_UNAVAILABLE && start.error().message == "injected failure");   // the service's own code and message
    assert(start.error().severity == ErrorSeverity::ERROR);
    assert(start.error().source == ComponentId{5});                                        // attributed by the component through its context
    assert(component.lifecycle_state() == LifecycleState::FAULT);
    assert(component.executed().back().step == S::START_TIMER && component.executed().back().code == ErrorCode::RESOURCE_UNAVAILABLE);
}

void test_requirement_checks_run_through_the_context() {
    PlatformRequirements needs;
    assert(needs.add_service(PlatformService::SCHEDULER, Requirement::REQUIRED));
    assert(needs.add_capability(CapabilityId{100}, Requirement::REQUIRED));
    assert(needs.add_service(PlatformService::WATCHDOG, Requirement::OPTIONAL));
    const ContextPlans plans{{}, {S::CHECK_REQUIREMENTS, S::EVALUATE_REQUIREMENTS, S::QUERY_CAPABILITY, S::QUERY_SUPPORT}, {}, {}, {}};
    {
        ReferencePlatform ok(config({true, true, true, false}));                           // the optional watchdog is missing: fine
        ReferenceContextComponent component(make_info(), PlatformContext(ok), plans, needs);
        assert(component.initialize() && component.lifecycle_state() == LifecycleState::READY);
        assert(component.executed().size() == 4 && ok.controls().log().empty());          // queries only
    }
    {
        ReferencePlatform missing_capability(config({true, true, true, true}, nullptr, false));
        ReferenceContextComponent component(make_info(6), PlatformContext(missing_capability), plans, needs);
        const auto init = component.initialize();
        assert(!init && init.error().code == ErrorCode::UNSUPPORTED && init.error().source == ComponentId{6});
        assert(init.error().message.find("100") != std::string::npos);
        assert(component.executed().size() == 1);                                          // the first failing step ended the plan
    }
}

// ---- side effects are observed on the reference platform ------------------------------------------------

void test_every_context_operation_is_a_counted_side_effect_free_query() {
    ReferencePlatform platform(config());
    const ComponentInfo info = make_info();
    const Controls& controls = platform.controls();
    assert(controls.adapter_queries() == 0);
    const ComponentContext context(info, PlatformContext(platform));
    const ComponentContext copy = context;
    ComponentContext moved(ComponentContext(info, PlatformContext(platform)));
    (void)copy.bound(); (void)copy.id(); (void)copy.info(); (void)moved.platform().attached();
    assert(controls.adapter_queries() == 0);                                               // construction, copying and shape queries never touch the platform
    std::size_t before = controls.adapter_queries();
    (void)context.require_scheduler(); assert(controls.adapter_queries() == before + 1); before += 1;   // one accessor call each
    (void)context.require_clock();     assert(controls.adapter_queries() == before + 1); before += 1;
    (void)context.require_timer();     assert(controls.adapter_queries() == before + 1); before += 1;
    (void)context.require_watchdog();  assert(controls.adapter_queries() == before + 1); before += 1;
    (void)context.supports(PlatformService::TIMER); assert(controls.adapter_queries() == before + 1); before += 1;
    (void)context.has_capability(CapabilityId{100}); assert(controls.adapter_queries() == before + 1); before += 1;   // one capability snapshot
    (void)context.attribute(Error{ErrorCode::TIMEOUT, ErrorSeverity::ERROR, {}, {}, "x"});
    assert(controls.adapter_queries() == before);                                          // attribution never touches the platform
    PlatformRequirements needs;
    assert(needs.add_service(PlatformService::SCHEDULER, Requirement::REQUIRED) && needs.add_service(PlatformService::TIMER, Requirement::OPTIONAL));
    assert(needs.add_capability(CapabilityId{100}, Requirement::REQUIRED));
    (void)context.evaluate(needs);
    assert(controls.adapter_queries() == before + 3);                                      // supports() per declared service + one capability snapshot
    assert(controls.log().empty());                                                        // not one service was started, stopped or configured
    assert(!platform.fake_scheduler().running() && !platform.fake_timer().running() && !platform.fake_watchdog().running());
}

void test_the_runtime_never_touches_the_platform_behind_components_that_do_not_use_it() {
    for (const bool faulting : {false, true}) {
        ReferencePlatform platform(config());
        RuntimeManager runtime;
        ReferenceContextComponent first(make_info(1), PlatformContext(platform), {});             // holds a context, uses it for nothing
        ReferenceContextComponent second(make_info(2), PlatformContext(platform), {});
        assert(runtime.register_component(first) && runtime.register_component(second) && runtime.add_dependency(ComponentId{2}, ComponentId{1}));
        assert(runtime.attach_platform(platform));
        assert(runtime.configure(Configuration{}) && runtime.initialize());
        if (faulting) second.fail_next_start = ErrorCode::TIMEOUT;
        const auto started = runtime.start();
        assert(started.has_value() == !faulting);
        if (faulting) assert(runtime.state() == LifecycleState::FAULT && runtime.reset());
        else assert(runtime.stop() && runtime.shutdown());
        assert(platform.controls().adapter_queries() == 0);                                // the Runtime never queried the adapter, in any state
        assert(platform.controls().log().empty());                                         // and never started, stopped, created or used a service
        assert(platform.fake_clock().now().nanoseconds() == 0);                            // nor read or advanced time
    }
}

// ---- ownership, lifetime and determinism --------------------------------------------------------------------

void test_lifetime_ownership_and_destruction_are_observable() {
    LifetimeProbe probe;
    bool component_destroyed = false;
    auto platform = std::make_unique<ReferencePlatform>(config({}, &probe));
    {
        ReferenceContextComponent component(make_info(), PlatformContext(*platform), kFullPlans, {}, &component_destroyed);
        assert(component.initialize() && component.start() && component.stop() && component.shutdown());
        const ComponentContext copy = component.context();
        assert(copy.info() == &component.info());                                          // valid for as long as the component lives
        assert(!probe.platform_destroyed && !component_destroyed);
    }                                                                                      // the component and its context end first
    assert(component_destroyed);
    assert(!probe.platform_destroyed && probe.services_destroyed == 0);                    // the context never destroyed the platform or a service
    platform.reset();                                                                      // the integrator ends the platform
    assert(probe.platform_destroyed && probe.services_destroyed == 4);
}

void test_replay_is_deterministic() {
    auto run = [] {
        ReferencePlatform platform(config());
        platform.controls().fail_nth(Method::WATCHDOG_START, 1, ErrorCode::TIMEOUT);
        ReferenceContextComponent component(make_info(3), PlatformContext(platform), kFullPlans);
        std::string trace;
        auto note = [&](const Result<void>& r) { trace += r ? "ok;" : std::to_string(static_cast<int>(r.error().code)) + "/" + std::to_string(r.error().source.value()) + ";"; };
        note(component.initialize()); note(component.start());
        for (const StepRecord& s : component.executed()) trace += std::to_string(static_cast<int>(s.step)) + (s.ok ? "+" : "-") + std::to_string(static_cast<int>(s.code)) + ";";
        for (const auto& r : platform.controls().log()) trace += std::to_string(static_cast<int>(r.method)) + (r.ok ? "+" : "-") + ";";
        return trace;
    };
    assert(run() == run());
}

void test_the_harness_runs_under_a_runtime_like_any_component() {
    ReferencePlatform platform(config());
    RuntimeManager runtime;
    ReferenceContextComponent component(make_info(8), PlatformContext(platform), kFullPlans);
    assert(runtime.register_component(component) && runtime.attach_platform(platform));
    assert(runtime.initialize() && runtime.start() && runtime.state() == LifecycleState::RUNNING);
    assert(runtime.stop() && runtime.shutdown());
    assert(runtime.statistics().sample_count.value() == 4 && runtime.statistics().error_count.value() == 0);
    // And a failure inside a plan is an ordinary component failure.
    ReferencePlatform failing(config({true, true, false, true}));
    RuntimeManager runtime2;
    ReferenceContextComponent needs_timer(make_info(8), PlatformContext(failing), kFullPlans);
    assert(runtime2.register_component(needs_timer) && runtime2.initialize());
    const auto started = runtime2.start();
    assert(!started && started.error().code == ErrorCode::UNSUPPORTED && started.error().source == ComponentId{8});
    assert(runtime2.state() == LifecycleState::FAULT && runtime2.reset());
}

} // namespace

int main() {
    test_the_contract_checks_pass_for_every_platform_and_every_kind_of_context();
    test_the_contract_checks_notice_a_context_over_the_wrong_platform();
    test_a_scripted_plan_drives_the_platform_in_order_through_the_context();
    test_every_unavailable_service_is_an_attributed_unsupported_failure_and_the_component_faults();
    test_a_platform_service_error_is_returned_unchanged_except_for_its_source();
    test_requirement_checks_run_through_the_context();
    test_every_context_operation_is_a_counted_side_effect_free_query();
    test_the_runtime_never_touches_the_platform_behind_components_that_do_not_use_it();
    test_lifetime_ownership_and_destruction_are_observable();
    test_replay_is_deterministic();
    test_the_harness_runs_under_a_runtime_like_any_component();
    return 0;
}
