//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : foundation_contract_test.cpp
// Description : R0.2 foundation contract tests through the public umbrella header.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-LIF-001, CORE-LIF-002, CORE-LIF-003, CORE-HEA-001,
//               CORE-HEA-002, CORE-ERR-001, CORE-ERR-002, CORE-ERR-004,
//               CORE-CAP-001, CORE-CAP-002, CORE-CAP-003, CORE-CFG-001,
//               CORE-CFG-002, CORE-CFG-003, CORE-EVT-001, CORE-EVT-002,
//               CORE-EVT-003, CORE-EVT-004, CORE-STS-001, CORE-STS-002,
//               CORE-STS-003, CORE-TYP-001, CORE-TYP-002, CORE-TYP-003,
//               CORE-TYP-004, CORE-STA-001
// API         : CORE-TEST-FOUNDATION-CONTRACT
//
// Author      : KritvaOS Core Team
// Created     : 02-10-2026
//==============================================================================
//
// Contract tests consume only the public umbrella header and verify documented
// behavior, not implementation details. They are deterministic: no wall-clock
// timing, network, hardware, threads, or vendor SDKs.
//
// Each section states the contract it checks. The expectations are written as
// independent tables here (not derived from the implementation), so that an
// implementation change that alters a contract fails this test.

#include <array>
#include <limits>
#include <cassert>
#include <cstdint>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>

#include <kritva/core/core.hpp>

using namespace kritva::core;

namespace {

// -----------------------------------------------------------------------------
// Lifecycle (CORE-LIF-001..003): the complete 8x8 transition matrix.
// The table is the documented one in ARCHITECTURE.md ("Lifecycle transitions").
// -----------------------------------------------------------------------------

constexpr std::array<LifecycleState, 8> kStates = {
    LifecycleState::UNKNOWN,  LifecycleState::INITIALIZING, LifecycleState::READY,
    LifecycleState::RUNNING,  LifecycleState::STOPPING,     LifecycleState::STOPPED,
    LifecycleState::FAULT,    LifecycleState::RECOVERING};

constexpr bool allowed(LifecycleState from, LifecycleState to) {
    using S = LifecycleState;
    switch (from) {
        case S::UNKNOWN:      return to == S::INITIALIZING;
        case S::INITIALIZING: return to == S::READY || to == S::FAULT;
        case S::READY:        return to == S::RUNNING || to == S::STOPPED || to == S::FAULT;
        case S::RUNNING:      return to == S::STOPPING || to == S::FAULT;
        case S::STOPPING:     return to == S::STOPPED || to == S::FAULT;
        case S::STOPPED:      return to == S::INITIALIZING;
        case S::FAULT:        return to == S::RECOVERING || to == S::STOPPED;
        case S::RECOVERING:   return to == S::READY || to == S::FAULT;
    }
    return false;
}

// Drive a Lifecycle into `target` along documented transitions only.
Lifecycle lifecycle_in(LifecycleState target) {
    using S = LifecycleState;
    Lifecycle lc;
    auto go = [&lc](std::initializer_list<S> path) {
        for (S s : path) { auto r = lc.transition_to(s); assert(r.has_value()); (void)r; }
    };
    switch (target) {
        case S::UNKNOWN:      break;
        case S::INITIALIZING: go({S::INITIALIZING}); break;
        case S::READY:        go({S::INITIALIZING, S::READY}); break;
        case S::RUNNING:      go({S::INITIALIZING, S::READY, S::RUNNING}); break;
        case S::STOPPING:     go({S::INITIALIZING, S::READY, S::RUNNING, S::STOPPING}); break;
        case S::STOPPED:      go({S::INITIALIZING, S::READY, S::STOPPED}); break;
        case S::FAULT:        go({S::INITIALIZING, S::FAULT}); break;
        case S::RECOVERING:   go({S::INITIALIZING, S::FAULT, S::RECOVERING}); break;
    }
    assert(lc.state() == target);
    return lc;
}

void test_lifecycle_full_transition_matrix() {
    int accepted = 0, rejected = 0;
    for (LifecycleState from : kStates) {
        for (LifecycleState to : kStates) {
            Lifecycle lc = lifecycle_in(from);
            const Result<void> r = lc.transition_to(to);
            if (allowed(from, to)) {
                ++accepted;
                assert(r.has_value());
                assert(lc.state() == to);
            } else {
                ++rejected;
                // CORE-LIF-003: rejected with INVALID_STATE, state unchanged.
                assert(!r.has_value());
                assert(r.error().code == ErrorCode::INVALID_STATE);
                assert(r.error().severity == ErrorSeverity::ERROR);
                assert(!r.error().message.empty());
                assert(lc.state() == from);
            }
        }
    }
    assert(accepted + rejected == 64);
    assert(accepted == 15);  // size of the documented table
}

void test_lifecycle_contract_properties() {
    Lifecycle lc;
    assert(lc.state() == LifecycleState::UNKNOWN);   // initial state
    assert(!lc.is_running());

    // Self-transitions are never valid.
    for (LifecycleState s : kStates) assert(!allowed(s, s));

    // Failure is recoverable: FAULT -> RECOVERING -> READY -> RUNNING.
    Lifecycle faulted = lifecycle_in(LifecycleState::FAULT);
    assert(faulted.transition_to(LifecycleState::RECOVERING));
    assert(faulted.transition_to(LifecycleState::READY));
    assert(faulted.transition_to(LifecycleState::RUNNING));
    assert(faulted.is_running());

    // A rejected transition does not poison later valid ones.
    Lifecycle ready = lifecycle_in(LifecycleState::READY);
    assert(!ready.transition_to(LifecycleState::STOPPING));
    assert(ready.state() == LifecycleState::READY);
    assert(ready.transition_to(LifecycleState::RUNNING));

    // is_running() is true only in RUNNING.
    for (LifecycleState s : kStates) {
        assert(lifecycle_in(s).is_running() == (s == LifecycleState::RUNNING));
    }
    // A stopped component can be re-initialized.
    Lifecycle stopped = lifecycle_in(LifecycleState::STOPPED);
    assert(stopped.transition_to(LifecycleState::INITIALIZING));

    static_assert(noexcept(std::declval<Lifecycle&>().transition_to(LifecycleState::READY)));
    static_assert(noexcept(std::declval<const Lifecycle&>().state()));
}

// -----------------------------------------------------------------------------
// Status (CORE-STA-001) and Health (CORE-HEA-001/002)
// -----------------------------------------------------------------------------

void test_status_and_health_contract() {
    // Status: UNKNOWN default, independent fields, explicit construction.
    Status status;
    assert(status.code() == StatusCode::UNKNOWN && status.message().empty());
    status.set_message("detail");
    status.set_code(StatusCode::OK);
    assert(status.code() == StatusCode::OK && status.message() == "detail");
    static_assert(!std::is_convertible_v<StatusCode, Status>);

    // Health: same shape. UNKNOWN default; state and detail independent.
    Health health;
    assert(health.state() == HealthState::UNKNOWN && health.detail().empty());
    for (HealthState s : {HealthState::HEALTHY, HealthState::DEGRADED,
                          HealthState::UNHEALTHY, HealthState::UNKNOWN}) {
        health.set_state(s);
        assert(health.state() == s);
    }
    health.set_detail("sensor lag");
    health.set_state(HealthState::DEGRADED);
    assert(health.detail() == "sensor lag");   // set_state keeps detail
    health.set_detail("");
    assert(health.state() == HealthState::DEGRADED && health.detail().empty());

    const Health constructed(HealthState::HEALTHY);
    assert(constructed.state() == HealthState::HEALTHY && constructed.detail().empty());
    static_assert(!std::is_convertible_v<HealthState, Health>);
    static_assert(std::is_nothrow_default_constructible_v<Health>);
    static_assert(std::is_nothrow_move_constructible_v<Health>);

    // Health states are distinct and UNKNOWN is the zero value.
    assert(static_cast<unsigned>(HealthState::UNKNOWN) == 0u);
    assert(HealthState::HEALTHY != HealthState::DEGRADED);
    assert(HealthState::DEGRADED != HealthState::UNHEALTHY);
}

// -----------------------------------------------------------------------------
// Error (CORE-ERR-001/002) and Result (CORE-ERR-004)
// -----------------------------------------------------------------------------

void test_error_and_result_contract() {
    // NONE is the zero value and the default; all codes are distinct.
    assert(static_cast<unsigned>(ErrorCode::NONE) == 0u);
    const std::array<ErrorCode, 12> codes = {
        ErrorCode::NONE, ErrorCode::UNKNOWN, ErrorCode::INVALID_ARGUMENT,
        ErrorCode::INVALID_STATE, ErrorCode::NOT_INITIALIZED, ErrorCode::NOT_READY,
        ErrorCode::ALREADY_RUNNING, ErrorCode::TIMEOUT, ErrorCode::RESOURCE_UNAVAILABLE,
        ErrorCode::CONFIGURATION_ERROR, ErrorCode::UNSUPPORTED, ErrorCode::INTERNAL_ERROR};
    std::unordered_set<std::uint32_t> seen;
    for (ErrorCode c : codes) assert(seen.insert(static_cast<std::uint32_t>(c)).second);

    // Severity is ordered from least to most severe.
    assert(ErrorSeverity::INFO < ErrorSeverity::WARNING);
    assert(ErrorSeverity::WARNING < ErrorSeverity::ERROR);
    assert(ErrorSeverity::ERROR < ErrorSeverity::CRITICAL);

    // Error is a copyable value and a Result carries it unchanged.
    Error e{ErrorCode::TIMEOUT, ErrorSeverity::CRITICAL, Id{9},
            Timestamp{5, ClockDomain::REALTIME}, "slow"};
    const Result<int> failed = Result<int>::failure(e);
    assert(!failed.has_value());
    assert(failed.error().code == ErrorCode::TIMEOUT);
    assert(failed.error().severity == ErrorSeverity::CRITICAL);
    assert(failed.error().source == Id{9});
    assert(failed.error().timestamp == (Timestamp{5, ClockDomain::REALTIME}));
    assert(failed.error().message == "slow");

    // Failures produced by Core APIs are real Results with real Errors.
    Lifecycle lc;
    const Result<void> bad = lc.transition_to(LifecycleState::RUNNING);
    assert(!bad && bad.error().code == ErrorCode::INVALID_STATE);
    Configuration cfg;
    const Result<void> bad_cfg = cfg.set(Parameter{"", ParameterValue{true}, ""});
    assert(!bad_cfg && bad_cfg.error().code == ErrorCode::INVALID_ARGUMENT);
    assert(Result<void>::success().has_value());
}

// -----------------------------------------------------------------------------
// Capability (CORE-CAP-001..003)
// -----------------------------------------------------------------------------

void test_capability_set_contract() {
    static_assert(std::is_same_v<CapabilityId, Id>);

    CapabilitySet set;
    assert(set.empty() && set.size() == 0);
    assert(!set.contains(CapabilityId{1}));
    assert(set.find(CapabilityId{1}) == nullptr);

    set.add(Capability{CapabilityId{1}, "camera", Version{1, 0, 0}});
    set.add(Capability{CapabilityId{2}, "lidar", Version{2, 1, 0}});
    assert(set.size() == 2);
    assert(set.contains(CapabilityId{1}) && set.contains(CapabilityId{2}));
    assert(!set.contains(CapabilityId{3}));

    // Adding an existing identity replaces it (CORE-CAP-003): same size,
    // same position, new content.
    set.add(Capability{CapabilityId{1}, "camera-v2", Version{1, 1, 0}});
    assert(set.size() == 2);
    const Capability* camera = set.find(CapabilityId{1});
    assert(camera != nullptr && camera->name == "camera-v2" && camera->version == (Version{1, 1, 0}));
    assert(set.all()[0].id == CapabilityId{1});
    assert(set.all()[1].id == CapabilityId{2});
    assert(set.find(CapabilityId{2})->name == "lidar");   // others untouched

    // Insertion order is preserved for distinct identities.
    set.add(Capability{CapabilityId{7}, "imu", Version{}});
    assert(set.size() == 3 && set.all()[2].id == CapabilityId{7});
}

// -----------------------------------------------------------------------------
// Configuration (CORE-CFG-001..003)
// -----------------------------------------------------------------------------

void test_configuration_contract() {
    static_assert(std::is_same_v<ConfigurationVersion, Version>);

    Configuration cfg;
    assert(cfg.size() == 0 && !cfg.contains("a") && cfg.get("a") == nullptr);
    assert(cfg.validate().has_value());              // empty configuration is valid

    // Typed values round-trip with their alternative preserved.
    assert(cfg.set(Parameter{"flag", ParameterValue{true}, "bool"}));
    assert(cfg.set(Parameter{"count", ParameterValue{std::int64_t{-3}}, "int"}));
    assert(cfg.set(Parameter{"gain", ParameterValue{1.5}, "double"}));
    assert(cfg.set(Parameter{"mode", ParameterValue{std::string{"fast"}}, "string"}));
    assert(cfg.size() == 4);
    assert(std::get<bool>(cfg.get("flag")->value) == true);
    assert(std::get<std::int64_t>(cfg.get("count")->value) == -3);
    assert(std::get<double>(cfg.get("gain")->value) == 1.5);
    assert(std::get<std::string>(cfg.get("mode")->value) == "fast");
    assert(cfg.get("mode")->description == "string");
    assert(cfg.get("missing") == nullptr);

    // Setting an existing name replaces it, including its type.
    assert(cfg.set(Parameter{"flag", ParameterValue{std::int64_t{1}}, "now int"}));
    assert(cfg.size() == 4);
    assert(std::holds_alternative<std::int64_t>(cfg.get("flag")->value));

    // Invalid input is rejected and leaves the configuration unchanged.
    const auto rejected = cfg.set(Parameter{"", ParameterValue{true}, ""});
    assert(!rejected && rejected.error().code == ErrorCode::INVALID_ARGUMENT);
    assert(cfg.size() == 4 && !cfg.contains(""));
    assert(cfg.validate().has_value());              // structural validation passes

    // Names are case-sensitive.
    assert(!cfg.contains("FLAG"));
}

// -----------------------------------------------------------------------------
// Event (CORE-EVT-001..004)
// -----------------------------------------------------------------------------

void test_event_envelope_contract() {
    const Event blank{};
    assert(!blank.event_id.valid() && !blank.source_id.valid() && !blank.correlation_id.valid());
    assert(blank.type == EventType::UNKNOWN);
    assert(static_cast<unsigned>(EventType::UNKNOWN) == 0u);
    assert(blank.timestamp == Timestamp{});
    assert(blank.severity == ErrorSeverity::INFO);

    Event e;
    e.event_id = Id{10};
    e.source_id = Id{20};
    e.correlation_id = Id{30};
    e.type = EventType::HEALTH;
    e.timestamp = Timestamp{77, ClockDomain::REALTIME};
    e.severity = ErrorSeverity::WARNING;

    // Identities are independent: event, source, and correlation never alias.
    assert(e.event_id.value() == 10 && e.source_id.value() == 20 && e.correlation_id.value() == 30);
    assert(e.timestamp.domain() == ClockDomain::REALTIME);   // domain survives
    const Event copy = e;
    assert(copy.type == EventType::HEALTH && copy.severity == ErrorSeverity::WARNING);

    // Event types are distinct.
    const std::array<EventType, 7> types = {
        EventType::UNKNOWN, EventType::LIFECYCLE, EventType::STATUS, EventType::HEALTH,
        EventType::ERROR, EventType::CONFIGURATION, EventType::CAPABILITY};
    std::unordered_set<std::uint32_t> seen;
    for (EventType t : types) assert(seen.insert(static_cast<std::uint32_t>(t)).second);
}

// -----------------------------------------------------------------------------
// Statistics (CORE-STS-001..003) through the umbrella header
// -----------------------------------------------------------------------------

void test_statistics_contract() {
    Statistics stats{};
    stats.sample_count.increment(3);
    stats.error_count.increment();
    stats.queue_depth.set(-2);
    assert(stats.sample_count.value() == 3 && stats.error_count.value() == 1);
    assert(stats.retry_count.value() == 0 && stats.drop_count.value() == 0);
    assert(stats.queue_depth.value() == -2 && stats.utilization.value() == 0);

    Counter c;
    c.increment(std::numeric_limits<Counter::value_type>::max());
    c.increment(2);
    assert(c.value() == 1);                          // modulo 2^64
    static_assert(std::is_trivially_copyable_v<Statistics>);
}

// -----------------------------------------------------------------------------
// Types (CORE-TYP-001..004)
// -----------------------------------------------------------------------------

void test_types_contract() {
    // Id: zero is invalid; ordering and hashing are value based.
    assert(!Id{}.valid() && !Id{0}.valid() && Id{1}.valid());
    assert(Id{1} < Id{2} && Id{2} == Id{2} && Id{3} != Id{4});
    assert(std::hash<Id>{}(Id{5}) == std::hash<Id>{}(Id{5}));
    static_assert(Id{}.value() == 0);
    static_assert(std::is_trivially_copyable_v<Id>);

    // Version: defaults to 0.0.0; formats as major.minor.patch; value equality.
    assert(Version{}.to_string() == "0.0.0");
    assert((Version{1, 2, 3}).to_string() == "1.2.3");
    assert((Version{10, 0, 255}).to_string() == "10.0.255");
    assert((Version{1, 2, 3}) == (Version{1, 2, 3}));
    assert(!((Version{1, 2, 3}) == (Version{1, 2, 4})));

    // Duration: nanosecond resolution, exact unit conversion, signed, ordered.
    assert(Duration{}.nanoseconds() == 0);
    assert(Duration::from_microseconds(1).nanoseconds() == 1'000);
    assert(Duration::from_milliseconds(1).nanoseconds() == 1'000'000);
    assert(Duration::from_milliseconds(-2).nanoseconds() == -2'000'000);
    assert(Duration::from_nanoseconds(1) < Duration::from_microseconds(1));
    assert(Duration::from_milliseconds(1) == Duration::from_microseconds(1000));

    // Metadata: set replaces, get/contains agree, missing keys are absent.
    Metadata md;
    assert(md.empty() && md.size() == 0 && md.get("k") == nullptr && !md.contains("k"));
    md.set("k", "v1");
    md.set("k", "v2");
    md.set("empty", "");
    assert(md.size() == 2 && *md.get("k") == "v2");
    assert(md.contains("empty") && md.get("empty")->empty());   // empty value is present
    assert(md.get("K") == nullptr);                              // case-sensitive
}

} // namespace

int main() {
    test_lifecycle_full_transition_matrix();
    test_lifecycle_contract_properties();
    test_status_and_health_contract();
    test_error_and_result_contract();
    test_capability_set_contract();
    test_configuration_contract();
    test_event_envelope_contract();
    test_statistics_contract();
    test_types_contract();
    return 0;
}
