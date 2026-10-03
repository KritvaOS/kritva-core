//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component_events_test.cpp
// Description : Contract tests for IEventSink and ComponentEventReporter.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-OPS-005, CORE-OPS-008
// API         : CORE-API-RUNTIME
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#include <cassert>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include <kritva/core/core.hpp>

using namespace kritva::core;
using namespace kritva::core::runtime;

namespace {

// Integrator-owned sink: records what it is given, can fail on demand, can re-enter, can throw.
class RecordingSink final : public IEventSink {
public:
    Result<void> report(const Event& event) override {
        ++calls;
        thread = std::this_thread::get_id();
        if (reentrant != nullptr && depth == 0) {
            ++depth;
            nested_result = reentrant->report(Event{Id{500}, {}, EventType::STATUS, {}, ErrorSeverity::INFO, {}});
            --depth;
        }
        if (throw_on_call) throw std::runtime_error("sink failure");
        if (fail_from != 0 && calls >= fail_from) return Result<void>::failure(failure);
        events.push_back(event);
        return Result<void>::success();
    }
    std::vector<Event> events;
    int calls{0};
    int fail_from{0};                          // fail every call from the Nth on
    Error failure{ErrorCode::RESOURCE_UNAVAILABLE, ErrorSeverity::WARNING, Id{77}, Timestamp{}, "sink full"};
    bool throw_on_call{false};
    std::thread::id thread{};
    ComponentEventReporter* reentrant{nullptr};
    Result<void> nested_result{Result<void>::success()};
    int depth{0};
};

class Probe final : public Component {
public:
    explicit Probe(ComponentInfo info) : Component(std::move(info)) {}
    Result<void> configure(const Configuration&) override { ++operations; return Result<void>::success(); }
    Result<void> initialize() override { ++operations; return Result<void>::success(); }
    Result<void> start() override { ++operations; return Result<void>::success(); }
    Result<void> stop() override { ++operations; return Result<void>::success(); }
    Result<void> shutdown() override { ++operations; return Result<void>::success(); }
    LifecycleState lifecycle_state() const noexcept override { return LifecycleState::UNKNOWN; }
    Status status() const override { return Status{}; }
    Health health() const override { return Health{}; }
    CapabilitySet capabilities() const override { return CapabilitySet{}; }
    int operations{0};
};

ComponentInfo make_info(std::uint64_t id) {
    auto info = ComponentInfo::create(ComponentId{id}, "ev");
    assert(info);
    return std::move(info).value();
}

Event make_event(std::uint64_t source) {
    return Event{Id{11}, Id{source}, EventType::HEALTH, Timestamp{123456789, ClockDomain::MONOTONIC}, ErrorSeverity::WARNING, Id{33}};
}

using R = ComponentEventReporter;

template <class T> concept HasSetSink = requires(T t, IEventSink& s) { t.set_sink(s); };
template <class T> concept HasBind = requires(T t, const ComponentInfo& i, IEventSink& s) { t.bind(i, s); };
template <class T> concept HasReset = requires(T t) { t.reset(); };
template <class T> concept HasSink = requires(const T t) { t.sink(); };
template <class T> concept HasInfoAccess = requires(const T t) { t.info(); };

void test_shape() {
    static_assert(std::is_abstract_v<IEventSink>);
    static_assert(std::has_virtual_destructor_v<IEventSink>);
    static_assert(sizeof(R) == 2 * sizeof(void*));
    static_assert(std::is_trivially_destructible_v<R>);
    static_assert(std::is_nothrow_default_constructible_v<R>);
    static_assert(std::is_nothrow_copy_constructible_v<R> && std::is_nothrow_move_constructible_v<R>);
    static_assert(!std::is_copy_assignable_v<R> && !std::is_move_assignable_v<R>);        // immutable: no rebinding
    static_assert(!HasSetSink<R> && !HasBind<R> && !HasReset<R>);
    static_assert(!HasSink<R> && !HasInfoAccess<R>);                                     // no path to the sink or the identity
    static_assert(std::is_constructible_v<R, const ComponentInfo&, IEventSink&>);
    static_assert(std::is_constructible_v<R, const Component&, IEventSink&>);
    static_assert(!std::is_constructible_v<R, ComponentInfo, IEventSink&>);              // temporary identity refused
    static_assert(!std::is_constructible_v<R, Probe, IEventSink&>);
    static_assert(!std::is_constructible_v<R, const ComponentInfo&, RecordingSink>);     // temporary sink refused
    static_assert(!std::is_constructible_v<R, const Component&, RecordingSink>);
    static_assert(!std::is_constructible_v<R, const ComponentInfo&>);                    // a sink is required
    static_assert(noexcept(std::declval<const R&>().bound()) && noexcept(std::declval<const R&>().id()));
    static_assert(!std::is_base_of_v<Component, R>);
}

void test_unbound_reporter_fails_without_calling_anything() {
    const R r;
    assert(!r.bound() && !r.id().valid());
    const auto result = r.report(make_event(0));
    assert(!result && result.error().code == ErrorCode::INVALID_STATE);
    const auto named = r.report(make_event(5));
    assert(!named && named.error().code == ErrorCode::INVALID_STATE);
}

void test_zero_source_is_stamped_with_the_component_id() {
    Probe c(make_info(7));
    RecordingSink sink;
    const R r(c, sink);
    assert(r.bound() && r.id() == ComponentId{7});
    Event e = make_event(0);
    assert(r.report(e));
    assert(sink.calls == 1 && sink.events.size() == 1);
    assert(sink.events[0].source_id == ComponentId{7});
    assert(!e.source_id.valid());                              // the caller's own copy is untouched
}

void test_matching_source_is_forwarded_unchanged() {
    Probe c(make_info(7));
    RecordingSink sink;
    const R r(c, sink);
    assert(r.report(make_event(7)));
    assert(sink.calls == 1 && sink.events[0].source_id == ComponentId{7});
}

void test_mismatching_source_is_rejected_and_never_forwarded() {
    Probe c(make_info(7));
    RecordingSink sink;
    const R r(c, sink);
    for (const std::uint64_t other : {1ull, 6ull, 8ull, 0xFFFFFFFFFFFFFFFFull}) {
        const auto result = r.report(make_event(other));
        assert(!result && result.error().code == ErrorCode::INVALID_ARGUMENT);
        assert(result.error().source == ComponentId{7});       // attributable to the reporting component
        assert(result.error().severity == ErrorSeverity::ERROR);
    }
    assert(sink.calls == 0 && sink.events.empty());            // nothing reached the sink
    assert(r.report(make_event(0)) && sink.calls == 1);        // and the reporter is still usable
}

void test_every_other_field_is_forwarded_exactly() {
    Probe c(make_info(9));
    RecordingSink sink;
    const R r(c, sink);
    for (const EventType type : {EventType::UNKNOWN, EventType::LIFECYCLE, EventType::STATUS, EventType::HEALTH, EventType::ERROR, EventType::CONFIGURATION, EventType::CAPABILITY}) {
        for (const ErrorSeverity severity : {ErrorSeverity::INFO, ErrorSeverity::WARNING, ErrorSeverity::ERROR, ErrorSeverity::CRITICAL}) {
            Event e{Id{1000 + static_cast<std::uint64_t>(type)}, Id{}, type, Timestamp{42, ClockDomain::REALTIME}, severity, Id{999}};
            assert(r.report(e));
            const Event& got = sink.events.back();
            assert(got.event_id == e.event_id && got.type == type && got.severity == severity);
            assert(got.timestamp == e.timestamp && got.correlation_id == e.correlation_id);
            assert(got.source_id == ComponentId{9});
        }
    }
    assert(sink.events.size() == 28);
    Event invalid_everything{};                                // nothing else is validated or rejected
    assert(r.report(invalid_everything) && sink.events.back().type == EventType::UNKNOWN);
}

void test_exactly_one_synchronous_call_on_the_callers_thread() {
    Probe c(make_info(7));
    RecordingSink sink;
    const R r(c, sink);
    assert(r.report(make_event(7)));
    assert(sink.calls == 1 && sink.thread == std::this_thread::get_id());
    assert(r.report(make_event(7)) && r.report(make_event(7)));
    assert(sink.calls == 3 && sink.events.size() == 3);
}

void test_sink_result_is_returned_unchanged_and_nothing_is_retried_or_buffered() {
    Probe c(make_info(7));
    RecordingSink sink;
    sink.fail_from = 2;                                        // the first succeeds, every later call fails
    const R r(c, sink);
    assert(r.report(make_event(7)));
    const auto failed = r.report(make_event(7));
    assert(!failed);
    assert(failed.error().code == ErrorCode::RESOURCE_UNAVAILABLE && failed.error().severity == ErrorSeverity::WARNING);
    assert(failed.error().source == Id{77} && failed.error().message == "sink full");   // NOT re-attributed to the component
    assert(sink.calls == 2);                                   // one attempt, no retry
    sink.fail_from = 0;                                        // the sink recovers ...
    assert(r.report(make_event(7)));
    assert(sink.calls == 3 && sink.events.size() == 2);        // ... and the failed event was kept nowhere
}

void test_event_never_commands_the_runtime() {
    Probe c(make_info(7));
    RecordingSink sink;
    const R r(c, sink);
    RuntimeManager runtime;
    assert(runtime.register_component(c));
    assert(runtime.initialize() && runtime.start());
    assert(sink.calls == 0);                                   // the Runtime never touches an integrator sink
    const int operations = c.operations;
    const LifecycleState state = runtime.state();
    const auto errors = runtime.statistics().error_count.value();
    const auto samples = runtime.statistics().sample_count.value();
    for (const EventType type : {EventType::LIFECYCLE, EventType::ERROR, EventType::HEALTH, EventType::STATUS, EventType::CONFIGURATION}) {
        assert(r.report(Event{Id{1}, {}, type, {}, ErrorSeverity::CRITICAL, {}}));
    }
    assert(sink.calls == 5);
    assert(runtime.state() == state && runtime.fault_error() == nullptr);              // no stop, reset, retry, fault
    assert(c.operations == operations);                                                // no lifecycle operation invoked
    assert(runtime.statistics().error_count.value() == errors && runtime.statistics().sample_count.value() == samples);
    assert(runtime.stop() && runtime.shutdown());
    assert(sink.calls == 5);                                   // the Runtime emitted nothing of its own either
}

void test_sink_may_report_again_reentrantly() {
    Probe c(make_info(7));
    RecordingSink sink;
    R r(c, sink);
    sink.reentrant = &r;
    assert(r.report(make_event(0)));
    assert(sink.nested_result);                                // the nested report went through
    assert(sink.calls == 2 && sink.events.size() == 2);        // nested first, then the outer one: no lock, no state
    assert(sink.events[0].event_id == Id{500} && sink.events[1].event_id == Id{11});
    assert(sink.events[0].source_id == ComponentId{7} && sink.events[1].source_id == ComponentId{7});
}

void test_exceptions_from_the_sink_propagate_and_leave_the_reporter_usable() {
    Probe c(make_info(7));
    RecordingSink sink;
    const R r(c, sink);
    sink.throw_on_call = true;
    bool thrown = false;
    try { (void)r.report(make_event(7)); } catch (const std::runtime_error&) { thrown = true; }
    assert(thrown && sink.calls == 1);
    sink.throw_on_call = false;
    assert(r.report(make_event(7)) && sink.calls == 2);
}

void test_copies_and_moves_share_the_same_identity_and_sink() {
    Probe c(make_info(7));
    RecordingSink sink;
    const R original(c, sink);
    const R copy = original;
    R moved = R(original);
    assert(copy.bound() && copy.id() == ComponentId{7} && moved.id() == ComponentId{7});
    assert(copy.report(make_event(0)) && moved.report(make_event(0)) && original.report(make_event(0)));
    assert(sink.calls == 3);                                   // all three reach the one integrator sink
    const R from_info(c.info(), sink);                         // by identity alone is equivalent
    assert(from_info.id() == ComponentId{7} && from_info.report(make_event(0)) && sink.calls == 4);
}

void test_two_components_report_through_one_sink_without_mixing() {
    Probe a(make_info(1)), b(make_info(2));
    RecordingSink sink;
    const R ra(a, sink), rb(b, sink);
    assert(ra.report(make_event(0)) && rb.report(make_event(0)));
    assert(!ra.report(make_event(2)) && !rb.report(make_event(1)));      // neither may speak for the other
    assert(sink.events.size() == 2);
    assert(sink.events[0].source_id == ComponentId{1} && sink.events[1].source_id == ComponentId{2});
}

} // namespace

int main() {
    test_shape();
    test_unbound_reporter_fails_without_calling_anything();
    test_zero_source_is_stamped_with_the_component_id();
    test_matching_source_is_forwarded_unchanged();
    test_mismatching_source_is_rejected_and_never_forwarded();
    test_every_other_field_is_forwarded_exactly();
    test_exactly_one_synchronous_call_on_the_callers_thread();
    test_sink_result_is_returned_unchanged_and_nothing_is_retried_or_buffered();
    test_event_never_commands_the_runtime();
    test_sink_may_report_again_reentrantly();
    test_exceptions_from_the_sink_propagate_and_leave_the_reporter_usable();
    test_copies_and_moves_share_the_same_identity_and_sink();
    test_two_components_report_through_one_sink_without_mixing();
    return 0;
}
