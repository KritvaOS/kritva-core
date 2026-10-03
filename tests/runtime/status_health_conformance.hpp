//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : status_health_conformance.hpp
// Description : Reusable conformance check of Status/Health reporting independence (test only).
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-OPS-002, CORE-OPS-003
// API         : CORE-TEST-OPERATIONAL-CONFORMANCE
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#pragma once
#include <concepts>
#include <string>
#include <vector>

#include <kritva/core/core.hpp>

namespace kritva::core::runtime::conformance {

/// What a component under test must let the check drive. The check only reports what it saw; it
/// never decides on its own what a Component "should" report beyond the independence contract.
template <class Fixture>
concept StatusHealthFixture = requires(Fixture& f, Status s, Health h, LifecycleState l) {
    { f.component() } -> std::same_as<const Component&>;
    f.set_status(s);
    f.set_health(h);
    f.set_lifecycle(l);
};

/// Result of a conformance check: the first violation, or empty when the component conforms.
struct Verdict {
    std::string violation;
    [[nodiscard]] bool ok() const noexcept { return violation.empty(); }
};

inline const std::vector<StatusCode>& all_status_codes() {
    static const std::vector<StatusCode> codes = [] {
        std::vector<StatusCode> v;
        for (unsigned i = 0; i <= static_cast<unsigned>(StatusCode::INTERNAL_ERROR); ++i) v.push_back(static_cast<StatusCode>(i));
        return v;
    }();
    return codes;
}

inline const std::vector<HealthState>& all_health_states() {
    static const std::vector<HealthState> states{HealthState::UNKNOWN, HealthState::HEALTHY, HealthState::DEGRADED, HealthState::UNHEALTHY};
    return states;
}

inline const std::vector<LifecycleState>& all_lifecycle_states() {
    static const std::vector<LifecycleState> states = [] {
        std::vector<LifecycleState> v;
        for (unsigned i = 0; i <= static_cast<unsigned>(LifecycleState::RECOVERING); ++i) v.push_back(static_cast<LifecycleState>(i));
        return v;
    }();
    return states;
}

/// CORE-OPS-002 / CORE-OPS-003: lifecycle, Status and Health are independent and reported as set.
///  - every (lifecycle, status code, health state) combination is reported unchanged, with and
///    without a message/detail (nothing is validated, derived, normalized or reconciled);
///  - changing one of the three never changes another;
///  - Status and Health are value snapshots: a copy taken earlier never changes later.
template <StatusHealthFixture Fixture>
Verdict check_status_health_independence(Fixture& fixture) {
    const Component& c = fixture.component();
    for (const LifecycleState lifecycle : all_lifecycle_states()) {
        for (const StatusCode code : all_status_codes()) {
            for (const HealthState state : all_health_states()) {
                for (const bool with_text : {false, true}) {
                    Status status(code);
                    Health health(state);
                    if (with_text) { status.set_message("status text"); health.set_detail("health detail"); }
                    fixture.set_lifecycle(lifecycle);
                    fixture.set_status(status);
                    fixture.set_health(health);
                    const ComponentObservation o = observe(c);
                    if (o.lifecycle != lifecycle) return {"lifecycle was altered by Status/Health"};
                    if (o.status.code() != code) return {"status code was altered or derived"};
                    if (o.status.message() != status.message()) return {"status message was altered or derived"};
                    if (o.health.state() != state) return {"health state was altered or derived"};
                    if (o.health.detail() != health.detail()) return {"health detail was altered or derived"};
                }
            }
        }
    }

    // One change at a time: the other two stay exactly where they were.
    fixture.set_lifecycle(LifecycleState::RUNNING);
    fixture.set_status(Status(StatusCode::OK));
    fixture.set_health(Health(HealthState::HEALTHY));
    const ComponentObservation base = observe(c);
    fixture.set_health(Health(HealthState::UNHEALTHY));
    ComponentObservation o = observe(c);
    if (o.status.code() != base.status.code() || o.lifecycle != base.lifecycle) return {"a health change altered status or lifecycle"};
    fixture.set_health(Health(HealthState::HEALTHY));
    fixture.set_status(Status(StatusCode::INTERNAL_ERROR));
    o = observe(c);
    if (o.health.state() != base.health.state() || o.lifecycle != base.lifecycle) return {"a status change altered health or lifecycle"};
    fixture.set_status(Status(StatusCode::OK));
    fixture.set_lifecycle(LifecycleState::FAULT);
    o = observe(c);
    if (o.health.state() != base.health.state() || o.status.code() != base.status.code()) return {"a lifecycle change altered status or health"};

    // Snapshots by value: later reports never change an earlier copy.
    Status s(StatusCode::OK);
    s.set_message("first");
    fixture.set_status(s);
    const Status snapshot = c.status();
    s.set_message("second");
    fixture.set_status(s);
    if (snapshot.message() != "first") return {"a status snapshot changed after a later report"};
    Health h(HealthState::DEGRADED);
    h.set_detail("first");
    fixture.set_health(h);
    const Health health_snapshot = c.health();
    h.set_detail("second");
    fixture.set_health(h);
    if (health_snapshot.detail() != "first") return {"a health snapshot changed after a later report"};
    return {};
}
} // namespace kritva::core::runtime::conformance
