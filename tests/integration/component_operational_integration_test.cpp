//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component_operational_integration_test.cpp
// Description : Runtime and operational information stay orthogonal (public APIs only).
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-OPS-007
// API         : CORE-API-RUNTIME
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

// Public APIs only: RuntimeManager, Component, observe(), IComponentStatistics, ComponentEventReporter, IEventSink,
// the test-only reference platform and the test-only operational harness. No private detail of the Runtime is touched.
//
// The central proof is ORTHOGONALITY: Component operational information (Status, Health, statistics, Events) flows
// OUTWARD to observers. Replaying the same seeded Runtime scenarios with plain reference components and with
// operational components that are observed, that report events and whose Status/Health/statistics are driven to the
// worst values between and during every step must give identical results, states, faults, statistics and invocation
// traces; the Runtime must never read, set, reset or react to any of it.

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
#include "../runtime/operational_conformance.hpp"
#include "runtime_scenarios.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;
using namespace kritva::core::runtime::scenarios;
using namespace kritva::core::runtime::contract;
using kritva::core::platform::testing::Method;
using kritva::core::platform::testing::ReferencePlatform;

namespace {

ComponentInfo make_info(std::uint64_t id) {
    auto info = ComponentInfo::create(ComponentId{id}, "c");
    assert(info.has_value());
    return std::move(info).value();
}

Event make_event(std::uint64_t id, EventType type, ErrorSeverity severity) {
    return Event{Id{id}, Id{}, type, Timestamp{static_cast<std::int64_t>(id)}, severity, Id{}};
}

const EventType kTypes[] = {EventType::LIFECYCLE, EventType::STATUS, EventType::HEALTH, EventType::ERROR, EventType::CONFIGURATION, EventType::CAPABILITY};

// The worst possible operational report, plus a flood of events of every type: none of it may matter.
void drive_to_the_worst(ReferenceOperationalComponent& c, std::uint64_t round) {
    c.set_health(Health(HealthState::UNHEALTHY));
    c.set_status(Status(StatusCode::INTERNAL_ERROR));
    c.stats.value.error_count.increment(1'000'000);
    c.stats.value.sample_count.increment(round);
    c.stats.value.queue_depth.set(-1'000'000);
    for (const EventType type : kTypes) (void)c.report(make_event(round, type, ErrorSeverity::CRITICAL));
}
void drive_to_the_best(ReferenceOperationalComponent& c) {
    c.set_health(Health(HealthState::HEALTHY));
    c.set_status(Status(StatusCode::OK));
}

// ---- 1. seeded differential: with and without operational activity -------------------------------
struct Variant {
    bool observe_between{false};     // observe every component before and after every Runtime step
    bool report_between{false};      // and report events / drive Status, Health, statistics
    bool scripted_inside{false};     // components also act inside their own lifecycle operations
    bool sink_fails{false};          // the integrator's sink rejects everything
};

Trace run_variant(const Script& script, const Variant& v, int* sink_calls = nullptr, std::uint64_t* provider_reads_by_runtime = nullptr) {
    RecordingEventSink sink;
    sink.fail_all = v.sink_fails;
    World::Factory factory;
    std::vector<ReferenceOperationalComponent*> ops;
    const bool operational = v.observe_between || v.report_between || v.scripted_inside;
    if (operational) {
        factory = [&](ComponentInfo info) -> std::unique_ptr<ReferenceComponent> {
            auto c = std::make_unique<ReferenceOperationalComponent>(std::move(info), &sink, /*with_statistics=*/true);
            if (v.scripted_inside) {
                OperationalAction a;
                a.status = Status(StatusCode::INTERNAL_ERROR);
                a.health = Health(HealthState::UNHEALTHY);
                a.events = {make_event(1, EventType::ERROR, ErrorSeverity::CRITICAL), make_event(2, EventType::LIFECYCLE, ErrorSeverity::INFO)};
                a.samples = 1;
                a.errors = 1;
                for (const OperationPoint p : {OperationPoint::CONFIGURE, OperationPoint::INITIALIZE, OperationPoint::START, OperationPoint::STOP, OperationPoint::SHUTDOWN}) c->script(p, a);
            }
            ops.push_back(c.get());
            return c;
        };
    }
    World world(script, nullptr, factory);
    Hooks hooks;
    if (operational) {
        hooks.before = [&](World& w, std::size_t i) {
            const RuntimeProbe before = RuntimeProbe::capture(w.runtime);
            for (ReferenceOperationalComponent* c : ops) {
                if (v.observe_between) { (void)c->observe_self(); (void)kritva::core::runtime::observe(*c); }
                if (v.report_between) drive_to_the_worst(*c, i + 1);
            }
            assert(RuntimeProbe::capture(w.runtime) == before);          // operational activity changed nothing in the Runtime
        };
        hooks.after = [&](World& w, std::size_t i) {
            const RuntimeProbe before = RuntimeProbe::capture(w.runtime);
            for (ReferenceOperationalComponent* c : ops) {
                if (v.observe_between) (void)c->observe_self();
                if (v.report_between) { if (i % 2 == 0) drive_to_the_best(*c); else drive_to_the_worst(*c, i + 100); }
            }
            assert(RuntimeProbe::capture(w.runtime) == before);
        };
    }
    Trace t = run(world, script, hooks);
    if (sink_calls != nullptr) *sink_calls = sink.calls;
    (void)provider_reads_by_runtime;
    return t;
}

void test_seeded_differential_with_and_without_operational_activity() {
    const Variant variants[] = {
        {true, false, false, false},     // observation only
        {false, true, false, false},     // reporting and worst values only
        {true, true, false, false},      // both, between steps
        {false, false, true, false},     // components act inside their own operations
        {true, true, true, false},       // everything
        {true, true, true, true},        // everything, and every sink call fails
    };
    int total_sink_calls = 0;
    for (unsigned seed = 1; seed <= 150; ++seed) {
        std::mt19937 rng(seed);
        const Script script = random_script(rng, 40);
        const Trace baseline = run_variant(script, Variant{});            // plain reference components
        for (const Variant& v : variants) {
            int sink_calls = 0;
            const Trace t = run_variant(script, v, &sink_calls);
            assert(t == baseline);                                       // identical results, states, faults, statistics, traces
            total_sink_calls += sink_calls;
        }
    }
    assert(total_sink_calls > 0);                                        // the operational activity really happened
}

// ---- 2. the Runtime never consumes operational information ----------------------------------------
void test_the_runtime_never_reads_or_calls_anything_operational() {
    for (unsigned seed = 1; seed <= 100; ++seed) {
        std::mt19937 rng(seed);
        const Script script = random_script(rng, 40);
        RecordingEventSink sink;
        std::vector<ReferenceOperationalComponent*> ops;
        World::Factory factory = [&](ComponentInfo info) -> std::unique_ptr<ReferenceComponent> {
            auto c = std::make_unique<ReferenceOperationalComponent>(std::move(info), &sink, true);
            ops.push_back(c.get());
            return c;
        };
        World world(script, nullptr, factory);
        (void)run(world, script);                                        // every operation, failure, fault and reset
        assert(sink.calls == 0);                                         // the Runtime never touches a sink
        for (const ReferenceOperationalComponent* c : ops) {
            assert(c->status_reads == 0 && c->health_reads == 0 && c->stats.reads == 0);
            assert(c->report_codes.empty());
            assert(c->stats.value.sample_count.value() == 0 && c->stats.value.error_count.value() == 0);   // nothing adjusted or reset
        }
    }
}

// ---- 3. Status/Health never alter Runtime state; failure/fault/reset stay R0.3-compatible ----------
void test_health_and_status_changes_do_not_alter_runtime_state() {
    RecordingEventSink sink;
    ReferenceOperationalComponent a(make_info(1), &sink, true), b(make_info(2), &sink, true);
    RuntimeManager runtime;
    assert(runtime.register_component(a) && runtime.register_component(b) && runtime.add_dependency(ComponentId{2}, ComponentId{1}));
    assert(runtime.configure(Configuration{}) && runtime.initialize() && runtime.start());
    assert(runtime.state() == LifecycleState::RUNNING);
    for (int i = 0; i < 25; ++i) { drive_to_the_worst(a, static_cast<std::uint64_t>(i)); drive_to_the_worst(b, static_cast<std::uint64_t>(i)); }
    assert(runtime.state() == LifecycleState::RUNNING && runtime.fault_error() == nullptr);   // UNHEALTHY / error status: information only
    assert(runtime.stop() && runtime.state() == LifecycleState::STOPPED);                      // normal stop, no special handling
    drive_to_the_best(a); drive_to_the_best(b);
    assert(runtime.shutdown() && runtime.state() == LifecycleState::STOPPED);
    assert(runtime.statistics().error_count.value() == 0);                                     // nothing was ever a Runtime error
}

void test_a_fault_ends_only_by_the_explicit_reset_whatever_health_reports() {
    SharedLog log;
    RecordingEventSink sink;
    sink.log = &log;
    ReferenceOperationalComponent a(make_info(1), &sink), b(make_info(2), &sink);
    a.trace = &log; b.trace = &log;
    b.fail_next_start = ErrorCode::INTERNAL_ERROR;
    RuntimeManager runtime;
    assert(runtime.register_component(a) && runtime.register_component(b) && runtime.add_dependency(ComponentId{2}, ComponentId{1}));
    assert(runtime.initialize());
    const auto r = runtime.start();
    assert(!r && r.error().code == ErrorCode::INTERNAL_ERROR && r.error().source == ComponentId{2});
    assert(runtime.state() == LifecycleState::FAULT);
    const std::size_t trace_size = log.size();
    // HEALTHY everywhere, a flood of events of every type: a FAULT neither clears nor deepens, and nothing retries.
    drive_to_the_best(a); drive_to_the_best(b);
    for (int i = 0; i < 10; ++i) for (const EventType type : kTypes) (void)b.report(make_event(static_cast<std::uint64_t>(i + 1), type, ErrorSeverity::CRITICAL));
    drive_to_the_worst(a, 1); drive_to_the_worst(b, 2);
    assert(runtime.state() == LifecycleState::FAULT && runtime.fault_error() != nullptr);
    assert(runtime.fault_error()->code == ErrorCode::INTERNAL_ERROR && runtime.fault_error()->source == ComponentId{2});
    for (std::size_t i = trace_size; i < log.size(); ++i) assert(log[i].rfind("sink:", 0) == 0);   // no component operation after the fault: only sink calls
    assert(runtime.reset() && runtime.state() == LifecycleState::STOPPED && runtime.fault_error() == nullptr);   // explicit, caller-requested
    assert(runtime.initialize() && runtime.state() == LifecycleState::READY);                       // and a new explicit attempt works
    assert(a.status_reads == 0 && a.health_reads == 0 && b.status_reads == 0 && b.health_reads == 0);
}

void test_an_event_describes_a_failure_it_never_replaces_it() {
    SharedLog log;
    RecordingEventSink sink;
    ReferenceOperationalComponent c(make_info(1), &sink);
    OperationalAction a;
    a.events = {make_event(7, EventType::ERROR, ErrorSeverity::CRITICAL)};   // the component reports the failure it is returning
    c.script(OperationPoint::START, a);
    c.fail_next_start = ErrorCode::TIMEOUT;
    RuntimeManager runtime;
    assert(runtime.register_component(c) && runtime.initialize());
    const auto r = runtime.start();
    assert(!r && r.error().code == ErrorCode::TIMEOUT && r.error().source == ComponentId{1});   // the Runtime sees the Result, not the event
    assert(runtime.fault_error() != nullptr && runtime.fault_error()->code == ErrorCode::TIMEOUT);
    assert(sink.calls == 1 && sink.events[0].type == EventType::ERROR && sink.events[0].source_id == ComponentId{1});
    assert(runtime.statistics().error_count.value() == 1);                // one failed operation; the event added no error
}

// ---- 4. order: operational reports follow the Runtime's own dependency order ----------------------
void test_events_reported_in_operations_follow_dependency_order() {
    SharedLog log;
    RecordingEventSink sink;
    sink.log = &log;
    ReferenceOperationalComponent a(make_info(1), &sink), b(make_info(2), &sink), c(make_info(3), &sink);
    for (ReferenceOperationalComponent* comp : {&a, &b, &c}) {
        comp->shared_log = &log;
        OperationalAction act;
        act.events = {make_event(comp->info().id().value(), EventType::LIFECYCLE, ErrorSeverity::INFO)};
        comp->script(OperationPoint::START, act);
        comp->script(OperationPoint::STOP, act);
    }
    RuntimeManager runtime;
    assert(runtime.register_component(c) && runtime.register_component(a) && runtime.register_component(b));   // registration order is irrelevant
    assert(runtime.add_dependency(ComponentId{3}, ComponentId{2}) && runtime.add_dependency(ComponentId{2}, ComponentId{1}));
    assert(runtime.initialize() && runtime.start());
    log.clear();
    assert(runtime.stop());
    // start events: dependency order 1, 2, 3; stop events: reverse 3, 2, 1
    assert(sink.events.size() == 6);
    assert(sink.events[0].source_id == ComponentId{1} && sink.events[1].source_id == ComponentId{2} && sink.events[2].source_id == ComponentId{3});
    assert(sink.events[3].source_id == ComponentId{3} && sink.events[4].source_id == ComponentId{2} && sink.events[5].source_id == ComponentId{1});
}

// ---- 5. statistics: Runtime-owned and Component-owned stay distinct ---------------------------------
void test_runtime_statistics_and_component_statistics_stay_distinct() {
    RecordingEventSink sink;
    ReferenceOperationalComponent a(make_info(1), &sink, true), b(make_info(2), &sink, true);
    a.stats.value.sample_count.increment(5'000'000);
    a.stats.value.error_count.increment(77);
    b.fail_next_start = ErrorCode::INTERNAL_ERROR;
    RuntimeManager runtime;
    assert(runtime.register_component(a) && runtime.register_component(b));
    assert(runtime.initialize() && !runtime.start() && runtime.reset());
    assert(runtime.statistics().error_count.value() == 1);               // exactly the Runtime's own failed call
    assert(runtime.statistics().sample_count.value() < 100);             // not influenced by 5,000,000
    assert(a.stats.value.sample_count.value() == 5'000'000 && a.stats.value.error_count.value() == 77);   // never reset or adjusted
    assert(b.stats.value.sample_count.value() == 0);
    const ComponentObservation oa = kritva::core::runtime::observe(a, a.provider());
    assert(oa.statistics->sample_count.value() == 5'000'000);            // the component's number is its own
    assert(a.stats.reads == 1);                                          // read once: by the observer, never by the Runtime
}

// ---- 6. no adapter/service ownership or platform lifecycle is introduced -----------------------------
void test_operational_activity_touches_no_platform_and_introduces_no_service_ownership() {
    ReferencePlatform platform;
    RecordingEventSink sink;
    ReferenceOperationalComponent a(make_info(1), &sink, true);
    RuntimeManager runtime;
    assert(runtime.register_component(a) && runtime.attach_platform(platform));
    assert(runtime.configure(Configuration{}) && runtime.initialize() && runtime.start());
    drive_to_the_worst(a, 1);
    (void)kritva::core::runtime::observe(a, a.provider());
    assert(a.report(make_event(1, EventType::HEALTH, ErrorSeverity::WARNING)));
    assert(runtime.stop() && runtime.shutdown());
    assert(platform.controls().adapter_queries() == 0);                  // no operational path queries the adapter
    assert(platform.controls().log().empty());                          // and no service call of any kind was made
    for (const Method m : {Method::SCHEDULER_CREATE_TASK, Method::TIMER_START, Method::WATCHDOG_START}) assert(platform.controls().calls(m) == 0);
    assert(runtime.attach_platform(platform).has_value() == false);      // attachment stays closed after the topology is fixed
}

// ---- 7. the harness conformance holds for a component that lives through a Runtime ---------------------
void test_the_conformance_checks_hold_for_a_runtime_managed_component() {
    RecordingEventSink sink;
    ReferenceOperationalComponent c(make_info(1), &sink, true);
    RuntimeManager runtime;
    assert(runtime.register_component(c) && runtime.initialize() && runtime.start());
    assert(conformance::check_event_reporter(c.reporter(), sink, ComponentId{1}).empty());
    struct Fix {
        ReferenceOperationalComponent& c;
        const IComponentStatistics& provider() { return c.stats; }
        void add_samples(std::uint64_t n) { c.stats.value.sample_count.increment(n); }
        void add_errors(std::uint64_t n) { c.stats.value.error_count.increment(n); }
        void set_queue_depth(std::int64_t g) { c.stats.value.queue_depth.set(g); }
    } fix{c};
    assert(conformance::check_statistics_provider(fix).empty());
    assert(runtime.state() == LifecycleState::RUNNING);                  // the checks changed nothing in the Runtime
    assert(runtime.stop() && runtime.shutdown());
}

} // namespace

int main() {
    test_seeded_differential_with_and_without_operational_activity();
    test_the_runtime_never_reads_or_calls_anything_operational();
    test_health_and_status_changes_do_not_alter_runtime_state();
    test_a_fault_ends_only_by_the_explicit_reset_whatever_health_reports();
    test_an_event_describes_a_failure_it_never_replaces_it();
    test_events_reported_in_operations_follow_dependency_order();
    test_runtime_statistics_and_component_statistics_stay_distinct();
    test_operational_activity_touches_no_platform_and_introduces_no_service_ownership();
    test_the_conformance_checks_hold_for_a_runtime_managed_component();
    std::puts("component operational integration: ok");
    return 0;
}
