//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : reference_operational_test.cpp
// Description : Tests of the test-only operational harness and its reusable conformance checks.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-OPS-009
// API         : CORE-TEST-OPERATIONAL-HARNESS
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <kritva/core/core.hpp>

#include "../runtime/operational_conformance.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;
using namespace kritva::core::runtime::contract;
namespace conf = kritva::core::runtime::conformance;

namespace {

ComponentInfo make_info(std::uint64_t id) {
    auto info = ComponentInfo::create(ComponentId{id}, "ops");
    assert(info);
    return std::move(info).value();
}

Event make_event(std::uint64_t id, EventType type = EventType::STATUS, ErrorSeverity severity = ErrorSeverity::INFO) {
    return Event{Id{id}, Id{}, type, Timestamp{static_cast<std::int64_t>(id)}, severity, Id{}};
}

// ---- fixtures that adapt the harness to the reusable checks -------------------------------------
struct StatusHealthFixture {
    ReferenceOperationalComponent c{make_info(1)};
    const Component& component() { return c; }
    void set_status(Status s) { c.set_status(std::move(s)); }
    void set_health(Health h) { c.set_health(std::move(h)); }
    void set_lifecycle(LifecycleState) {}                       // the reference component's lifecycle follows its operations
};
// The conformance check also sets lifecycle; the reference component derives it from its operations,
// so the lifecycle assertions use a fixture-local override component for that purpose:
class LifecycleSettable final : public ReferenceOperationalComponent {
public:
    using ReferenceOperationalComponent::ReferenceOperationalComponent;
    LifecycleState lifecycle_state() const noexcept override { return forced; }
    LifecycleState forced{LifecycleState::UNKNOWN};
};
struct FullStatusHealthFixture {
    LifecycleSettable c{make_info(2)};
    const Component& component() { return c; }
    void set_status(Status s) { c.set_status(std::move(s)); }
    void set_health(Health h) { c.set_health(std::move(h)); }
    void set_lifecycle(LifecycleState l) { c.forced = l; }
};
struct StatisticsFixture {
    ReferenceOperationalComponent c{make_info(3), nullptr, true};
    const IComponentStatistics& provider() { return c.stats; }
    void add_samples(std::uint64_t n) { c.stats.value.sample_count.increment(n); }
    void add_errors(std::uint64_t n) { c.stats.value.error_count.increment(n); }
    void set_queue_depth(std::int64_t g) { c.stats.value.queue_depth.set(g); }
};

// ---- non-conforming reporters, to prove the event check is sensitive ----------------------------
enum class Defect { NONE, OVERWRITES_MISMATCH, FORWARDS_MISMATCH, NO_STAMP, ALTERS_FIELD, SWALLOWS_FAILURE, RETRIES, DOUBLE_SEND, REATTRIBUTES, KEEPS_FAILED };
struct BrokenReporter {
    BrokenReporter(ComponentId self, IEventSink& sink, Defect defect) : self_(self), sink_(&sink), defect_(defect) {}
    bool bound() const { return true; }
    ComponentId id() const { return self_; }
    Result<void> report(Event e) const {
        const bool foreign = e.source_id.valid() && e.source_id != self_;
        if (foreign && defect_ != Defect::OVERWRITES_MISMATCH && defect_ != Defect::FORWARDS_MISMATCH) {
            return Result<void>::failure(Error{ErrorCode::INVALID_ARGUMENT, ErrorSeverity::ERROR, self_, {}, "mismatch"});
        }
        if (foreign && defect_ == Defect::OVERWRITES_MISMATCH) e.source_id = self_;
        if (!foreign || defect_ == Defect::OVERWRITES_MISMATCH) { if (defect_ != Defect::NO_STAMP && !e.source_id.valid()) e.source_id = self_; }
        if (defect_ == Defect::ALTERS_FIELD) e.severity = ErrorSeverity::INFO;
        Result<void> r = sink_->report(e);
        if (!r && defect_ == Defect::SWALLOWS_FAILURE) return Result<void>::success();
        if (!r && defect_ == Defect::RETRIES) return sink_->report(e);
        if (r && defect_ == Defect::DOUBLE_SEND) return sink_->report(e);
        if (!r && defect_ == Defect::REATTRIBUTES) { Error err = r.error(); err.source = self_; return Result<void>::failure(err); }
        return r;
    }
    ComponentId self_;
    IEventSink* sink_;
    Defect defect_;
};

void test_the_real_reporter_conforms_to_the_event_check() {
    RecordingEventSink sink;
    ReferenceOperationalComponent c(make_info(7), &sink);
    assert(conf::check_event_reporter(c.reporter(), sink, ComponentId{7}).empty());
    // and the check is repeatable on the same instances
    assert(conf::check_event_reporter(c.reporter(), sink, ComponentId{7}).empty());
}

void test_the_event_check_detects_every_kind_of_broken_reporter() {
    for (const Defect d : {Defect::OVERWRITES_MISMATCH, Defect::FORWARDS_MISMATCH, Defect::NO_STAMP, Defect::ALTERS_FIELD,
                           Defect::SWALLOWS_FAILURE, Defect::RETRIES, Defect::DOUBLE_SEND, Defect::REATTRIBUTES}) {
        RecordingEventSink sink;
        const BrokenReporter r(ComponentId{7}, sink, d);
        assert(!conf::check_event_reporter(r, sink, ComponentId{7}).empty());
    }
    RecordingEventSink sink;
    const BrokenReporter good(ComponentId{7}, sink, Defect::NONE);       // the model with no defect passes
    assert(conf::check_event_reporter(good, sink, ComponentId{7}).empty());
}

void test_the_event_check_rejects_an_unbound_or_wrong_reporter() {
    RecordingEventSink sink;
    ReferenceOperationalComponent c(make_info(7), &sink);
    assert(!conf::check_event_reporter(c.reporter(), sink, ComponentId{8}).empty());   // bound to a different component
    ReferenceOperationalComponent unbound(make_info(7));
    assert(!conf::check_event_reporter(unbound.reporter(), sink, ComponentId{7}).empty());
}

void test_the_reference_component_conforms_to_all_operational_checks() {
    FullStatusHealthFixture a;  assert(conf::check_status_health_independence(a).ok());
    StatisticsFixture b;        assert(conf::check_statistics_provider(b).empty());
    RecordingEventSink sink;
    ReferenceOperationalComponent c(make_info(4), &sink, true);
    assert(conf::check_event_reporter(c.reporter(), sink, ComponentId{4}).empty());
}

void test_the_status_health_fixture_without_lifecycle_control_still_runs_its_other_checks() {
    // The plain reference component derives its lifecycle from its operations, so it cannot be forced:
    // the check correctly reports that (a fixture must be able to drive every lifecycle state).
    StatusHealthFixture f;
    assert(!conf::check_status_health_independence(f).ok());
}

void test_scripted_actions_run_at_their_exact_point_and_in_order() {
    SharedLog log;
    RecordingEventSink sink;
    sink.log = &log;
    ReferenceOperationalComponent c(make_info(1), &sink, true);
    c.shared_log = &log;
    OperationalAction on_start;
    on_start.status = Status(StatusCode::NOT_READY);
    on_start.health = Health(HealthState::DEGRADED);
    on_start.events = {make_event(1), make_event(2, EventType::HEALTH, ErrorSeverity::WARNING)};
    on_start.samples = 3;
    on_start.errors = 1;
    on_start.queue_depth = 5;
    c.script(OperationPoint::START, on_start);

    assert(c.configure(Configuration{}) && c.initialize());
    assert(c.report_codes.empty() && sink.calls == 0);                 // nothing before the scripted point
    assert(c.start());
    const SharedLog expected{"component:1:configure", "component:1:initialize", "component:1:start", "sink:report#1", "sink:report#2"};
    assert(log == expected);                                           // the events occur inside start(), after it ran
    assert(sink.events.size() == 2 && sink.events[0].source_id == ComponentId{1} && sink.events[1].severity == ErrorSeverity::WARNING);
    const ComponentObservation o = c.observe_self();
    assert(o.lifecycle == LifecycleState::RUNNING);
    assert(o.status.code() == StatusCode::NOT_READY && o.health.state() == HealthState::DEGRADED);
    assert(o.statistics && o.statistics->sample_count.value() == 3 && o.statistics->error_count.value() == 1 && o.statistics->queue_depth.value() == 5);
    assert(c.stats.reads == 1);
}

void test_unscripted_defaults_follow_the_plain_reference_component() {
    ReferenceOperationalComponent c(make_info(1));
    assert(c.observe_self().status.code() == StatusCode::OK);          // the reference's own behavior
    assert(c.observe_self().health.state() == HealthState::HEALTHY);
    c.fail_next_initialize = ErrorCode::INTERNAL_ERROR;
    assert(!c.initialize());
    assert(c.observe_self().lifecycle == LifecycleState::FAULT && c.observe_self().health.state() == HealthState::UNHEALTHY);
    c.set_health(Health(HealthState::HEALTHY));                        // the owner overrides: legal, independent
    assert(c.observe_self().lifecycle == LifecycleState::FAULT && c.observe_self().health.state() == HealthState::HEALTHY);
    c.clear_reports();
    assert(c.observe_self().health.state() == HealthState::UNHEALTHY);
}

void test_optional_parts_are_really_optional() {
    ReferenceOperationalComponent bare(make_info(1));                  // no sink, no statistics
    assert(bare.provider() == nullptr && !bare.reporter().bound());
    assert(!bare.observe_self().statistics.has_value());
    const auto r = bare.report(make_event(1));
    assert(!r && r.error().code == ErrorCode::INVALID_STATE);
    assert(bare.report_codes.size() == 1 && bare.report_codes[0] == ErrorCode::INVALID_STATE);
    RecordingEventSink sink;
    ReferenceOperationalComponent with_sink(make_info(2), &sink);      // a sink but no statistics
    assert(with_sink.provider() == nullptr && with_sink.report(make_event(1)) && sink.calls == 1);
    ReferenceOperationalComponent with_stats(make_info(3), nullptr, true);   // statistics but no sink
    assert(with_stats.provider() == &with_stats.stats && !with_stats.reporter().bound());
    assert(with_stats.observe_self().statistics.has_value());
}

void test_the_operation_result_is_returned_unchanged_even_when_the_sink_fails() {
    RecordingEventSink sink;
    sink.fail_all = true;
    ReferenceOperationalComponent c(make_info(1), &sink);
    OperationalAction a;
    a.events = {make_event(1), make_event(2)};
    c.script(OperationPoint::INITIALIZE, a);
    c.fail_next_initialize = ErrorCode::TIMEOUT;
    const auto r = c.initialize();                                     // the operation fails for its own reason ...
    assert(!r && r.error().code == ErrorCode::TIMEOUT && r.error().source == ComponentId{1});
    assert(sink.calls == 2 && sink.events.empty());                    // ... both reports were attempted once and refused
    assert(c.report_codes.size() == 2 && c.report_codes[0] == ErrorCode::RESOURCE_UNAVAILABLE);
    assert(c.lifecycle_state() == LifecycleState::FAULT);              // the sink failure did not alter the component's state
    ReferenceOperationalComponent ok(make_info(2), &sink);
    ok.script(OperationPoint::START, a);
    assert(ok.initialize() && ok.start());                             // and a failing sink never fails a successful operation
}

void test_the_recording_sink_models_failure_reentrancy_and_exceptions() {
    RecordingEventSink sink;
    ReferenceOperationalComponent c(make_info(1), &sink);
    sink.fail_on_calls = {2};
    assert(c.report(make_event(1)) && !c.report(make_event(2)) && c.report(make_event(3)));
    assert(sink.calls == 3 && sink.events.size() == 2 && sink.events[1].event_id == Id{3});
    sink.on_report = [&](const Event& e) { if (e.event_id == Id{10}) (void)c.report(make_event(11)); };
    assert(c.report(make_event(10)));
    assert(sink.calls == 5 && sink.events[2].event_id == Id{11} && sink.events[3].event_id == Id{10});   // nested first
    sink.on_report = nullptr;
    sink.throw_on_call = sink.calls + 1;
    bool thrown = false;
    try { (void)c.report(make_event(20)); } catch (const std::runtime_error&) { thrown = true; }
    assert(thrown && c.report(make_event(21)));
}

void test_the_observer_records_detached_snapshots_in_order() {
    ReferenceOperationalComponent c(make_info(1), nullptr, true);
    RecordingObserver observer;
    c.set_status(Status(StatusCode::OK));
    c.stats.value.sample_count.increment(1);
    observer.sample(c, c.provider());
    c.set_status(Status(StatusCode::NOT_READY));
    c.stats.value.sample_count.increment(1);
    observer.sample(c, c.provider());
    observer.sample(c);                                                // no provider this time
    assert(observer.snapshots.size() == 3);
    assert(observer.snapshots[0].status.code() == StatusCode::OK && observer.snapshots[0].statistics->sample_count.value() == 1);
    assert(observer.snapshots[1].status.code() == StatusCode::NOT_READY && observer.snapshots[1].statistics->sample_count.value() == 2);
    assert(!observer.snapshots[2].statistics.has_value());
}

// Runs one fixed lifecycle over two components; `observe_and_report` interleaves observation and reports.
struct RunResult {
    std::vector<RuntimeProbe> probes;
    std::vector<std::string> trace;
    std::vector<std::string> results;
    int operational_reads{0};            // status/health/provider reads made by anything but the observer
};
RunResult run_lifecycle(bool observe_and_report, bool inject_fault) {
    RunResult out;
    RecordingEventSink sink;
    ReferenceOperationalComponent a(make_info(1), &sink, true), b(make_info(2), &sink, true);
    a.trace = &out.trace;
    b.trace = &out.trace;
    if (inject_fault) b.fail_next_start = ErrorCode::INTERNAL_ERROR;
    RuntimeManager runtime;
    assert(runtime.register_component(a) && runtime.register_component(b));
    assert(runtime.add_dependency(ComponentId{2}, ComponentId{1}));
    RecordingObserver observer;
    auto between = [&] {
        out.probes.push_back(RuntimeProbe::capture(runtime));
        if (!observe_and_report) return;
        for (ReferenceOperationalComponent* c : {&a, &b}) {
            observer.sample(*c, c->provider());
            (void)c->report(make_event(c->info().id().value(), EventType::LIFECYCLE, ErrorSeverity::CRITICAL));
            c->set_health(Health(HealthState::UNHEALTHY));
            c->set_status(Status(StatusCode::INTERNAL_ERROR));
            c->stats.value.error_count.increment(1000);
        }
        if (RuntimeProbe::capture(runtime) != out.probes.back()) out.results.push_back("RUNTIME CHANGED BY OBSERVATION");
    };
    auto note = [&](const char* what, const Result<void>& r) { out.results.push_back(std::string(what) + (r ? ":ok" : ":" + std::to_string(static_cast<int>(r.error().code)))); between(); };
    note("configure", runtime.configure(Configuration{}));
    note("initialize", runtime.initialize());
    note("start", runtime.start());
    if (inject_fault) { note("reset", runtime.reset()); }
    else { note("stop", runtime.stop()); note("shutdown", runtime.shutdown()); }
    out.operational_reads = a.status_reads + a.health_reads + b.status_reads + b.health_reads;
    if (!observe_and_report) out.operational_reads += a.stats.reads + b.stats.reads;
    return out;
}

void test_observation_and_reports_never_alter_lifecycle_or_runtime_state() {
    for (const bool fault : {false, true}) {
        const RunResult control = run_lifecycle(false, fault);
        const RunResult observed = run_lifecycle(true, fault);
        assert(control.probes == observed.probes);                     // identical state, fault, error_count, sample_count at every step
        assert(control.trace == observed.trace);                       // identical component invocation order
        assert(control.results == observed.results);                   // identical outcomes (and no "RUNTIME CHANGED" marker)
        assert(control.operational_reads == 0);                        // with nobody observing, nothing read Status/Health/statistics
    }
    const RunResult faulted = run_lifecycle(true, true);
    assert(faulted.probes.back().state == LifecycleState::STOPPED && faulted.probes[2].state == LifecycleState::FAULT);
}

void test_the_runtime_never_reads_the_harness_operational_surface() {
    RecordingEventSink sink;
    ReferenceOperationalComponent c(make_info(1), &sink, true);
    OperationalAction a;
    a.events = {make_event(1)};
    c.script(OperationPoint::START, a);
    RuntimeManager runtime;
    assert(runtime.register_component(c));
    assert(runtime.configure(Configuration{}) && runtime.initialize() && runtime.start() && runtime.stop() && runtime.shutdown());
    assert(c.status_reads == 0 && c.health_reads == 0 && c.stats.reads == 0);   // the Runtime read nothing operational
    assert(sink.calls == 1);                                                    // the only call is the component's own scripted report
}

void test_the_runtime_probe_is_sensitive_to_every_field() {
    RuntimeManager runtime;
    ReferenceOperationalComponent c(make_info(1));
    assert(runtime.register_component(c));
    const RuntimeProbe p0 = RuntimeProbe::capture(runtime);
    assert(runtime.initialize());
    const RuntimeProbe p1 = RuntimeProbe::capture(runtime);
    assert(p0 != p1 && p1.state == LifecycleState::READY && p1.samples == 1);
    c.fail_next_start = ErrorCode::INTERNAL_ERROR;
    assert(!runtime.start());
    const RuntimeProbe p2 = RuntimeProbe::capture(runtime);
    assert(p2.state == LifecycleState::FAULT && p2.has_fault && p2.fault_code == ErrorCode::INTERNAL_ERROR && p2.errors == 1);
}

} // namespace

int main() {
    test_the_real_reporter_conforms_to_the_event_check();
    test_the_event_check_detects_every_kind_of_broken_reporter();
    test_the_event_check_rejects_an_unbound_or_wrong_reporter();
    test_the_reference_component_conforms_to_all_operational_checks();
    test_the_status_health_fixture_without_lifecycle_control_still_runs_its_other_checks();
    test_scripted_actions_run_at_their_exact_point_and_in_order();
    test_unscripted_defaults_follow_the_plain_reference_component();
    test_optional_parts_are_really_optional();
    test_the_operation_result_is_returned_unchanged_even_when_the_sink_fails();
    test_the_recording_sink_models_failure_reentrancy_and_exceptions();
    test_the_observer_records_detached_snapshots_in_order();
    test_observation_and_reports_never_alter_lifecycle_or_runtime_state();
    test_the_runtime_never_reads_the_harness_operational_surface();
    test_the_runtime_probe_is_sensitive_to_every_field();
    return 0;
}
