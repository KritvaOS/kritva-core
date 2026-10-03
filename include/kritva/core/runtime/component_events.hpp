//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component_events.hpp
// Description : Explicit Component operational Event reporting to an integrator-owned sink.
//
// Component   : Kritva Core
// Module      : Runtime
// Layer       : Core Foundation
//
// Requirements: CORE-OPS-005, CORE-OPS-008
// API         : CORE-API-RUNTIME
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#pragma once
#include "component.hpp"
#include "../event/event.hpp"
#include "../error/result.hpp"
namespace kritva::core::runtime {

//------------------------------------------------------------------------------
// IEventSink (CORE-OPS-005, CORE-OPS-008)
//
// Where an integrator wants Component Events to go. It is implemented and OWNED
// by the integrator; Core never creates, selects, stores, buffers or destroys
// a sink and provides no implementation of it (no logger, queue, bus, exporter
// or file). report() receives the Event by const reference for the duration of
// the call only; a sink that wants to keep it copies it.
//
// A sink returns a Result<void>: success, or its own Error (for example
// RESOURCE_UNAVAILABLE for a full buffer). Core does not interpret that Result.
//------------------------------------------------------------------------------
class IEventSink {
public:
    virtual ~IEventSink() = default;

    virtual Result<void> report(const Event& event) = 0;
};

//------------------------------------------------------------------------------
// ComponentEventReporter (CORE-OPS-005, CORE-OPS-008)
//
// How an integrator-written Component explicitly reports an occurrence. It is
// a small copyable value of two NON-OWNING pointers (the component's immutable
// ComponentInfo and the IEventSink), immutable after construction exactly like
// ComponentContext: no setter, reset or rebinding, no copy or move assignment
// (copy and move construction are available), trivially destructible. A default
// reporter is UNBOUND. The identity and the sink must outlive every reporter
// that can invoke them, and a temporary of either is refused at compile time (a
// temporary ComponentInfo or Component by deleted overloads; a temporary sink
// because the constructor takes a non-const lvalue reference).
//
// LIFETIME (amendment): the sink is non-owning and integrator-managed. It must
// outlive every reporter that can invoke it. Core adds no ownership extension,
// reference counting or lifetime management; using a reporter after the sink or
// the identity is destroyed is undefined behavior.
//
// report(Event) (synchronous, exactly once, no side effect of its own)
//   unbound reporter                       INVALID_STATE, the sink is NOT called.
//   event.source_id is non-zero and is not
//     this component's id                  INVALID_ARGUMENT (source = this
//                                          component's id), the sink is NOT
//                                          called, nothing is forwarded.
//   event.source_id is zero                a copy with source_id set to this
//                                          component's id is forwarded.
//   event.source_id equals the component   forwarded unchanged.
//   Every forwarded Event therefore identifies the reporting component; a
//   producer bug that names another component is reported, never silently
//   overwritten. source_id is the ONLY field the reporter ever touches: event_id,
//   type, timestamp, severity and correlation_id are forwarded exactly as given
//   (Core has no clock here: the caller supplies the timestamp), and the
//   reporter never validates, filters, rewrites, orders, de-duplicates or
//   assigns identities.
//   The sink is called once, synchronously, on the caller's thread, and the
//   sink's Result<void> is returned UNCHANGED (its Error is not re-attributed).
//   Nothing is buffered, queued, retried, persisted, dispatched asynchronously
//   or dropped: if the sink fails, the Event is not kept and no second attempt
//   is made; what to do about it is the caller's and the integrator's decision.
//   An exception thrown by the sink propagates unchanged.
//
// EVENT VERSUS COMMAND (CORE-OPS-005)
//   An Event describes something that happened. Reporting one never commands or
//   triggers a start, stop, reset, retry, reconfiguration, recovery or watchdog
//   action: the reporter holds no Runtime, no registry and no other component,
//   the Runtime never receives or reacts to it, and an Event of type LIFECYCLE,
//   STATUS, HEALTH or ERROR is only a report. Core is not an event bus and
//   provides no queue, broker, subscription, dispatcher, background worker or
//   telemetry or logging backend; the integrator owns all of those, and any
//   policy that acts on an Event (CORE-OPS-008).
//
// THREADS, ALLOCATION, REAL TIME
//   report() allocates nothing itself, holds no lock and keeps no state, so it
//   is re-entrant (a sink may report again) and safe from concurrent callers
//   exactly when the sink is. It blocks only as the sink does. It is not a
//   real-time claim: the sink's cost is the sink's. Complexity is O(1) plus the
//   sink's.
//------------------------------------------------------------------------------
class ComponentEventReporter {
public:
    ComponentEventReporter() noexcept = default;
    ComponentEventReporter(const ComponentInfo& info, IEventSink& sink) noexcept : info_(&info), sink_(&sink) {}
    ComponentEventReporter(const Component& component, IEventSink& sink) noexcept : ComponentEventReporter(component.info(), sink) {}

    ComponentEventReporter(const ComponentInfo&&, IEventSink&) = delete;
    ComponentEventReporter(const Component&&, IEventSink&) = delete;

    ComponentEventReporter(const ComponentEventReporter&) noexcept = default;
    ComponentEventReporter(ComponentEventReporter&&) noexcept = default;
    ComponentEventReporter& operator=(const ComponentEventReporter&) = delete;
    ComponentEventReporter& operator=(ComponentEventReporter&&) = delete;

    /// True when this reporter has a component identity and a sink.
    [[nodiscard]] bool bound() const noexcept { return info_ != nullptr && sink_ != nullptr; }

    /// The reporting component's id; the invalid id when unbound.
    [[nodiscard]] ComponentId id() const noexcept { return info_ != nullptr ? info_->id() : ComponentId{}; }

    [[nodiscard]] Result<void> report(Event event) const {
        if (!bound()) {
            return Result<void>::failure(Error{ErrorCode::INVALID_STATE, ErrorSeverity::ERROR, {}, {}, "report: the event reporter is not bound to a component and a sink"});
        }
        const ComponentId self = info_->id();
        if (event.source_id.valid() && event.source_id != self) {
            return Result<void>::failure(Error{ErrorCode::INVALID_ARGUMENT, ErrorSeverity::ERROR, self, {}, "report: the event names a different source than the reporting component"});
        }
        event.source_id = self;
        return sink_->report(event);
    }

private:
    const ComponentInfo* info_{nullptr};
    IEventSink* sink_{nullptr};
};
} // namespace kritva::core::runtime
