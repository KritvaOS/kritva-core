//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : reference_operational.hpp
// Description : Test-only reference operational harness (component, sink, provider, observer).
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

#pragma once

// TEST SUPPORT ONLY (never compiled into or installed with the production library).
//
// A reusable, deterministic harness for the R0.7 operational contracts, built only on public
// APIs. It adds nothing to Core and models no real component:
//   - RecordingEventSink       an integrator-owned IEventSink that records every event and every
//                              direct call (count, thread, order), can fail on chosen calls, throw,
//                              or re-enter, and can append to a shared ordered log;
//   - ReferenceStatisticsProvider  an optional Component-owned statistics provider counting reads;
//   - ReferenceOperationalComponent  a conforming Component (the plain ReferenceComponent for
//                              lifecycle) that owns scripted Status/Health, an optional
//                              statistics provider and a ComponentEventReporter, and whose
//                              lifecycle operations first run the same operation as the plain
//                              ReferenceComponent and then a SCRIPTED action (set Status/Health,
//                              update statistics, report events) at that exact point;
//   - RecordingObserver        calls observe() and keeps every ComponentObservation;
//   - RuntimeProbe             a value capturing the Runtime facts an observation must never alter.

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <kritva/core/core.hpp>

#include "../contract/reference_component.hpp"

namespace kritva::core::runtime::contract {

/// Ordered log shared between a component and a sink, so "event during start()" is observable.
using SharedLog = std::vector<std::string>;

class RecordingEventSink final : public IEventSink {
public:
    Result<void> report(const Event& event) override {
        ++calls;
        last_thread = std::this_thread::get_id();
        if (log != nullptr) log->push_back("sink:report#" + std::to_string(calls));
        if (on_report) on_report(event);                         // re-entrancy / interleaving hook
        if (throw_on_call != 0 && calls == throw_on_call) throw std::runtime_error("sink exception");
        if (fail_all || fail_on_calls.count(calls) != 0) return Result<void>::failure(failure);
        events.push_back(event);
        return Result<void>::success();
    }

    std::vector<Event> events;                                   // events the sink ACCEPTED
    int calls{0};                                                // every call, accepted or failed
    std::thread::id last_thread{};
    SharedLog* log{nullptr};
    std::function<void(const Event&)> on_report;
    bool fail_all{false};
    std::set<int> fail_on_calls;
    int throw_on_call{0};
    Error failure{ErrorCode::RESOURCE_UNAVAILABLE, ErrorSeverity::WARNING, Id{900}, Timestamp{}, "sink rejected the event"};
};

class ReferenceStatisticsProvider final : public IComponentStatistics {
public:
    [[nodiscard]] Statistics statistics() const override { ++reads; return value; }
    Statistics value{};
    mutable int reads{0};
};

/// What a scripted action does at its point; every part is optional.
struct OperationalAction {
    std::optional<Status> status;
    std::optional<Health> health;
    std::vector<Event> events;                                   // reported through the component's reporter, in order
    std::uint64_t samples{0};
    std::uint64_t errors{0};
    std::optional<std::int64_t> queue_depth;
};

enum class OperationPoint : std::uint8_t { CONFIGURE, INITIALIZE, START, STOP, SHUTDOWN };

class ReferenceOperationalComponent : public ReferenceComponent {
public:
    /// `sink == nullptr` leaves the reporter unbound; `with_statistics` makes provider() non-null.
    ReferenceOperationalComponent(ComponentInfo info, IEventSink* sink = nullptr, bool with_statistics = false)
        : ReferenceComponent(std::move(info)),
          reporter_(sink != nullptr ? ComponentEventReporter(this->info(), *sink) : ComponentEventReporter()),
          with_statistics_(with_statistics) {}

    Result<void> configure(const Configuration& c) override { return then(OperationPoint::CONFIGURE, ReferenceComponent::configure(c)); }
    Result<void> initialize() override { return then(OperationPoint::INITIALIZE, ReferenceComponent::initialize()); }
    Result<void> start() override { return then(OperationPoint::START, ReferenceComponent::start()); }
    Result<void> stop() override { return then(OperationPoint::STOP, ReferenceComponent::stop()); }
    Result<void> shutdown() override { return then(OperationPoint::SHUTDOWN, ReferenceComponent::shutdown()); }

    /// Scripted reports win over the plain reference behavior (Status/Health follow the owner).
    [[nodiscard]] Status status() const override { ++status_reads; return status_override_ ? *status_override_ : ReferenceComponent::status(); }
    [[nodiscard]] Health health() const override { ++health_reads; return health_override_ ? *health_override_ : ReferenceComponent::health(); }

    void set_status(Status s) { status_override_ = std::move(s); }
    void set_health(Health h) { health_override_ = std::move(h); }
    void clear_reports() { status_override_.reset(); health_override_.reset(); }

    /// Optional statistics: nullptr unless this component opted in.
    [[nodiscard]] const IComponentStatistics* provider() const noexcept { return with_statistics_ ? &stats : nullptr; }
    [[nodiscard]] const ComponentEventReporter& reporter() const noexcept { return reporter_; }

    /// Report one event through this component's reporter and remember the outcome.
    Result<void> report(Event e) {
        Result<void> r = reporter_.report(std::move(e));
        report_codes.push_back(r ? ErrorCode::NONE : r.error().code);
        return r;
    }

    [[nodiscard]] ComponentObservation observe_self() const { return observe(*this, provider()); }

    /// Run `action` immediately after the next successful or failed call of `point` (every time until cleared).
    void script(OperationPoint point, OperationalAction action) { plan_[point] = std::move(action); }
    void clear_script() { plan_.clear(); }

    ReferenceStatisticsProvider stats;
    std::vector<ErrorCode> report_codes;                         // outcome of each report (NONE = accepted)
    SharedLog* shared_log{nullptr};
    mutable int status_reads{0}, health_reads{0};

private:
    Result<void> then(OperationPoint point, Result<void> result) {
        if (shared_log != nullptr) shared_log->push_back("component:" + std::to_string(info().id().value()) + ":" + name(point));
        const auto it = plan_.find(point);
        if (it != plan_.end()) run(it->second);
        return result;                                           // the operation's own result, unchanged
    }
    void run(const OperationalAction& a) {
        if (a.status) status_override_ = *a.status;
        if (a.health) health_override_ = *a.health;
        stats.value.sample_count.increment(a.samples);
        stats.value.error_count.increment(a.errors);
        if (a.queue_depth) stats.value.queue_depth.set(*a.queue_depth);
        for (const Event& e : a.events) (void)report(e);
    }
    static const char* name(OperationPoint p) {
        switch (p) {
            case OperationPoint::CONFIGURE: return "configure";
            case OperationPoint::INITIALIZE: return "initialize";
            case OperationPoint::START: return "start";
            case OperationPoint::STOP: return "stop";
            case OperationPoint::SHUTDOWN: return "shutdown";
        }
        return "?";
    }

    ComponentEventReporter reporter_;
    bool with_statistics_;
    std::optional<Status> status_override_;
    std::optional<Health> health_override_;
    std::map<OperationPoint, OperationalAction> plan_;
};

/// Collects observations: the harness's observer. It only calls observe().
class RecordingObserver {
public:
    const ComponentObservation& sample(const Component& component, const IComponentStatistics* provider = nullptr) {
        snapshots.push_back(observe(component, provider));
        return snapshots.back();
    }
    std::vector<ComponentObservation> snapshots;
};

/// The Runtime facts an observation or a report must never alter.
struct RuntimeProbe {
    LifecycleState state{LifecycleState::UNKNOWN};
    bool has_fault{false};
    ErrorCode fault_code{ErrorCode::NONE};
    std::uint64_t errors{0};
    std::uint64_t samples{0};

    static RuntimeProbe capture(const RuntimeManager& runtime) {
        RuntimeProbe p;
        p.state = runtime.state();
        p.has_fault = runtime.fault_error() != nullptr;
        p.fault_code = p.has_fault ? runtime.fault_error()->code : ErrorCode::NONE;
        p.errors = runtime.statistics().error_count.value();
        p.samples = runtime.statistics().sample_count.value();
        return p;
    }
    friend bool operator==(const RuntimeProbe&, const RuntimeProbe&) = default;
};
} // namespace kritva::core::runtime::contract
