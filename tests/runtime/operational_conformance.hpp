//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : operational_conformance.hpp
// Description : Reusable conformance check of an Event reporter and a combined operational check (test only).
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-OPS-009
// API         : CORE-TEST-OPERATIONAL-CONFORMANCE
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#pragma once
#include <concepts>
#include <string>

#include <kritva/core/core.hpp>

#include "reference_operational.hpp"
#include "status_health_conformance.hpp"
#include "statistics_conformance.hpp"

namespace kritva::core::runtime::conformance {

template <class Reporter>
concept EventReporterLike = requires(const Reporter& r, Event e) {
    { r.report(e) } -> std::same_as<Result<void>>;
    { r.bound() } -> std::convertible_to<bool>;
    { r.id() } -> std::same_as<ComponentId>;
};

/// CORE-OPS-005: a bound reporter for component `self` and a RecordingEventSink it forwards to.
/// Returns the first violation of the Event reporting contract, or an empty string.
template <EventReporterLike Reporter>
std::string check_event_reporter(const Reporter& reporter, contract::RecordingEventSink& sink, ComponentId self) {
    using contract::RecordingEventSink;
    if (!reporter.bound() || reporter.id() != self) return "the reporter is not bound to the component";
    sink.events.clear();
    sink.fail_all = false;
    sink.fail_on_calls.clear();
    sink.throw_on_call = 0;
    sink.on_report = nullptr;
    const int base = sink.calls;

    Event e{Id{11}, Id{}, EventType::HEALTH, Timestamp{123, ClockDomain::REALTIME}, ErrorSeverity::WARNING, Id{33}};
    // zero source: stamped with the component's id, nothing else touched, exactly one call
    if (!reporter.report(e)) return "an event with no source was rejected";
    if (sink.calls != base + 1 || sink.events.size() != 1) return "an accepted event was not forwarded exactly once";
    const Event& stamped = sink.events.back();
    if (stamped.source_id != self) return "a zero source was not stamped with the component id";
    if (stamped.event_id != e.event_id || stamped.type != e.type || stamped.timestamp != e.timestamp ||
        stamped.severity != e.severity || stamped.correlation_id != e.correlation_id) return "a field other than source_id was altered";
    // matching source: forwarded unchanged
    e.source_id = self;
    if (!reporter.report(e) || sink.events.size() != 2 || sink.events.back().source_id != self) return "a matching source was not forwarded unchanged";
    // mismatching source: INVALID_ARGUMENT attributed to the component, sink not called
    const int before = sink.calls;
    Event other = e;
    other.source_id = Id{self.value() + 1};
    const Result<void> mismatch = reporter.report(other);
    if (mismatch || mismatch.error().code != ErrorCode::INVALID_ARGUMENT) return "a mismatching source was not rejected with INVALID_ARGUMENT";
    if (mismatch.error().source != self) return "the mismatch error is not attributed to the reporting component";
    if (sink.calls != before || sink.events.size() != 2) return "a mismatching event reached the sink";
    // sink failure: returned unchanged, no retry, nothing kept
    sink.fail_all = true;
    const int failing_base = sink.calls;
    const Result<void> failed = reporter.report(e);
    sink.fail_all = false;
    if (failed || failed.error().code != sink.failure.code || failed.error().source != sink.failure.source ||
        failed.error().message != sink.failure.message || failed.error().severity != sink.failure.severity) return "the sink's failure was not returned unchanged";
    if (sink.calls != failing_base + 1) return "a failed report was retried or repeated";
    if (sink.events.size() != 2) return "a failed event was kept";
    // the reporter stays usable afterwards
    if (!reporter.report(e) || sink.events.size() != 3) return "the reporter is not usable after a sink failure";
    return {};
}
} // namespace kritva::core::runtime::conformance
