//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component_status_health_test.cpp
// Description : Status and Health reporting contract tests (independence, snapshots, Runtime FAULT).
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-OPS-002, CORE-OPS-003
// API         : CORE-API-RUNTIME
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#include <cassert>
#include <string>
#include <utility>

#include <kritva/core/core.hpp>

#include "../runtime/status_health_conformance.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;
using kritva::core::runtime::conformance::check_status_health_independence;

namespace {

ComponentInfo make_info(std::uint64_t id) {
    auto info = ComponentInfo::create(ComponentId{id}, "sh");
    assert(info);
    return std::move(info).value();
}

// A conforming component: its three pieces of information are set independently by its owner.
// It counts every read so the Runtime's behavior can be observed. A failing operation can be armed.
class ReportingComponent final : public Component {
public:
    explicit ReportingComponent(ComponentInfo info) : Component(std::move(info)) {}
    Result<void> configure(const Configuration&) override { return step(); }
    Result<void> initialize() override { auto r = step(); if (r) state = LifecycleState::READY; else state = LifecycleState::FAULT; return r; }
    Result<void> start() override { auto r = step(); if (r) state = LifecycleState::RUNNING; else state = LifecycleState::FAULT; return r; }
    Result<void> stop() override { auto r = step(); if (r) state = LifecycleState::STOPPED; else state = LifecycleState::FAULT; return r; }
    Result<void> shutdown() override { auto r = step(); if (r && state == LifecycleState::FAULT) state = LifecycleState::STOPPED; return r; }
    LifecycleState lifecycle_state() const noexcept override { ++reads; return state; }
    Status status() const override { ++status_reads; return status_value; }
    Health health() const override { ++health_reads; return health_value; }
    CapabilitySet capabilities() const override { ++capability_reads; return CapabilitySet{}; }

    LifecycleState state{LifecycleState::UNKNOWN};
    Status status_value{};
    Health health_value{};
    int fail_start{0};          // fail the next start() when > 0
    int operations{0};
    mutable int reads{0}, status_reads{0}, health_reads{0}, capability_reads{0};
private:
    Result<void> step() { ++operations; return Result<void>::success(); }
};

// start() fails on demand
class FailingStart final : public Component {
public:
    explicit FailingStart(ComponentInfo info) : Component(std::move(info)) {}
    Result<void> configure(const Configuration&) override { return Result<void>::success(); }
    Result<void> initialize() override { state = LifecycleState::READY; return Result<void>::success(); }
    Result<void> start() override {
        state = LifecycleState::FAULT;
        return Result<void>::failure(Error{ErrorCode::INTERNAL_ERROR, ErrorSeverity::ERROR, info().id(), {}, "start failed"});
    }
    Result<void> stop() override { state = LifecycleState::STOPPED; return Result<void>::success(); }
    Result<void> shutdown() override { state = LifecycleState::STOPPED; return Result<void>::success(); }
    LifecycleState lifecycle_state() const noexcept override { return state; }
    Status status() const override { ++status_reads; return status_value; }
    Health health() const override { ++health_reads; return health_value; }
    CapabilitySet capabilities() const override { return CapabilitySet{}; }
    LifecycleState state{LifecycleState::UNKNOWN};
    Status status_value{StatusCode::OK};
    Health health_value{HealthState::HEALTHY};
    mutable int status_reads{0}, health_reads{0};
};

struct GoodFixture {
    ReportingComponent c{make_info(1)};
    const Component& component() { return c; }
    void set_status(Status s) { c.status_value = std::move(s); }
    void set_health(Health h) { c.health_value = std::move(h); }
    void set_lifecycle(LifecycleState l) { c.state = l; }
};

// Non-conforming components, to prove the conformance check detects each violation.
struct HealthFromStatus {                        // health derived from status
    ReportingComponent c{make_info(2)};
    const Component& component() { return c; }
    void set_status(Status s) { c.status_value = s; c.health_value = Health(s.code() == StatusCode::OK ? HealthState::HEALTHY : HealthState::UNHEALTHY); }
    void set_health(Health h) { c.health_value = std::move(h); }
    void set_lifecycle(LifecycleState l) { c.state = l; }
};
struct StatusFromLifecycle {                     // status forced to NOT_READY unless RUNNING
    ReportingComponent c{make_info(3)};
    const Component& component() { return c; }
    void set_status(Status s) { c.status_value = std::move(s); }
    void set_health(Health h) { c.health_value = std::move(h); }
    void set_lifecycle(LifecycleState l) { c.state = l; if (l != LifecycleState::RUNNING) c.status_value.set_code(StatusCode::NOT_READY); }
};
struct HealthResetOnFault {                      // health forced to UNHEALTHY in FAULT
    ReportingComponent c{make_info(4)};
    const Component& component() { return c; }
    void set_status(Status s) { c.status_value = std::move(s); }
    void set_health(Health h) { c.health_value = std::move(h); }
    void set_lifecycle(LifecycleState l) { c.state = l; if (l == LifecycleState::FAULT) c.health_value.set_state(HealthState::UNHEALTHY); }
};
struct DropsDetail {                             // empty detail is "normalized" to a text
    ReportingComponent c{make_info(5)};
    const Component& component() { return c; }
    void set_status(Status s) { if (s.message().empty()) s.set_message("n/a"); c.status_value = std::move(s); }
    void set_health(Health h) { c.health_value = std::move(h); }
    void set_lifecycle(LifecycleState l) { c.state = l; }
};

void test_conforming_component_passes_the_check() {
    GoodFixture f;
    const auto v = check_status_health_independence(f);
    assert(v.ok());
}

void test_check_detects_each_kind_of_violation() {
    HealthFromStatus a;            assert(!check_status_health_independence(a).ok());
    StatusFromLifecycle b;         assert(!check_status_health_independence(b).ok());
    HealthResetOnFault c;          assert(!check_status_health_independence(c).ok());
    DropsDetail d;                 assert(!check_status_health_independence(d).ok());
}

void test_check_runs_the_full_combination_space() {
    using namespace kritva::core::runtime::conformance;
    assert(all_status_codes().size() == 10);       // every StatusCode
    assert(all_health_states().size() == 4);
    assert(all_lifecycle_states().size() == 8);    // every LifecycleState, including transient ones
}

void test_every_combination_is_legal_and_passed_on_unchanged() {
    ReportingComponent c(make_info(10));
    // The surprising-but-legal combinations called out by the contract.
    c.state = LifecycleState::RUNNING;  c.health_value = Health(HealthState::UNHEALTHY);
    ComponentObservation o = observe(c);
    assert(o.lifecycle == LifecycleState::RUNNING && o.health.state() == HealthState::UNHEALTHY);
    c.state = LifecycleState::FAULT;    c.health_value = Health(HealthState::HEALTHY);
    o = observe(c);
    assert(o.lifecycle == LifecycleState::FAULT && o.health.state() == HealthState::HEALTHY);
    c.state = LifecycleState::READY;    c.status_value = Status(StatusCode::INTERNAL_ERROR);
    o = observe(c);
    assert(o.lifecycle == LifecycleState::READY && o.status.code() == StatusCode::INTERNAL_ERROR);
    c.status_value = Status(StatusCode::OK);
    assert(observe(c).status.message().empty());                 // OK does not imply or require text
    c.status_value.set_message("fine");
    assert(observe(c).status.code() == StatusCode::OK && observe(c).status.message() == "fine");
    c.health_value = Health(HealthState::UNKNOWN);               // empty detail valid for every state
    assert(observe(c).health.detail().empty());
}

void test_runtime_never_reads_status_or_health() {
    ReportingComponent c(make_info(11));
    c.status_value = Status(StatusCode::INTERNAL_ERROR);
    c.health_value = Health(HealthState::UNHEALTHY);             // the worst possible report ...
    RuntimeManager runtime;
    assert(runtime.register_component(c));
    assert(runtime.configure(Configuration{}));
    assert(runtime.initialize() && runtime.start());
    assert(runtime.state() == LifecycleState::RUNNING);          // ... changes nothing: Health is information
    assert(runtime.stop() && runtime.shutdown());
    assert(c.status_reads == 0 && c.health_reads == 0 && c.capability_reads == 0);   // never even read
}

void test_unhealthy_never_triggers_recovery_or_lifecycle_action() {
    ReportingComponent c(make_info(12));
    RuntimeManager runtime;
    assert(runtime.register_component(c));
    assert(runtime.initialize() && runtime.start());
    const int operations = c.operations;
    const auto errors = runtime.statistics().error_count.value();
    const auto samples = runtime.statistics().sample_count.value();
    c.health_value = Health(HealthState::UNHEALTHY);
    c.health_value.set_detail("sensor lost");
    c.status_value = Status(StatusCode::INTERNAL_ERROR);
    for (int i = 0; i < 20; ++i) (void)observe(c);
    assert(runtime.state() == LifecycleState::RUNNING);          // no stop, reset, restart or fault
    assert(runtime.fault_error() == nullptr);
    assert(c.operations == operations);                          // no lifecycle operation was invoked
    assert(runtime.statistics().error_count.value() == errors && runtime.statistics().sample_count.value() == samples);
}

void test_health_is_independent_of_runtime_fault() {
    FailingStart c(make_info(13));                                // reports HEALTHY / OK throughout
    RuntimeManager runtime;
    assert(runtime.register_component(c));
    assert(runtime.initialize());
    assert(!runtime.start());                                     // the component's start() fails
    assert(runtime.state() == LifecycleState::FAULT);             // the Runtime is in FAULT ...
    const ComponentObservation faulted = observe(c);
    assert(faulted.lifecycle == LifecycleState::FAULT);
    assert(faulted.health.state() == HealthState::HEALTHY);       // ... yet the component's health is its own word
    assert(faulted.status.code() == StatusCode::OK);

    c.health_value = Health(HealthState::UNHEALTHY);              // and an UNHEALTHY report does not clear or deepen it
    assert(runtime.state() == LifecycleState::FAULT);
    assert(runtime.reset());                                      // explicit reset: Runtime recovers
    assert(runtime.state() == LifecycleState::STOPPED);
    assert(observe(c).health.state() == HealthState::UNHEALTHY);  // the Runtime never rewrote the component's Health
    assert(runtime.statistics().error_count.value() >= 1);        // the failed start is a Runtime error ...
}

void test_reported_health_never_changes_how_a_failure_is_handled() {
    for (const HealthState reported : {HealthState::UNKNOWN, HealthState::HEALTHY, HealthState::DEGRADED, HealthState::UNHEALTHY}) {
        FailingStart c(make_info(15));
        c.health_value = Health(reported);
        c.status_value = Status(reported == HealthState::HEALTHY ? StatusCode::OK : StatusCode::INTERNAL_ERROR);
        RuntimeManager runtime;
        assert(runtime.register_component(c));
        assert(runtime.initialize());
        const auto r = runtime.start();
        assert(!r && r.error().code == ErrorCode::INTERNAL_ERROR && r.error().source == ComponentId{15});   // unchanged
        assert(runtime.state() == LifecycleState::FAULT && runtime.fault_error() != nullptr);              // whatever Health says
        assert(runtime.statistics().error_count.value() == 1);
        assert(runtime.reset() && runtime.state() == LifecycleState::STOPPED);                              // explicit reset only
        assert(c.status_reads == 0 && c.health_reads == 0);                                                 // never consulted
    }
}

void test_snapshots_by_value() {
    ReportingComponent c(make_info(14));
    c.status_value.set_message(std::string(300, 'a'));
    c.health_value.set_detail(std::string(300, 'b'));
    const Status s = c.status();
    const Health h = c.health();
    c.status_value.set_message("changed");
    c.health_value.set_detail("changed");
    assert(s.message() == std::string(300, 'a') && h.detail() == std::string(300, 'b'));
    Status moved = c.status();
    c.status_value.set_message("again");
    assert(moved.message() == "changed");
}

} // namespace

int main() {
    test_conforming_component_passes_the_check();
    test_check_detects_each_kind_of_violation();
    test_check_runs_the_full_combination_space();
    test_every_combination_is_legal_and_passed_on_unchanged();
    test_runtime_never_reads_status_or_health();
    test_unhealthy_never_triggers_recovery_or_lifecycle_action();
    test_health_is_independent_of_runtime_fault();
    test_reported_health_never_changes_how_a_failure_is_handled();
    test_snapshots_by_value();
    return 0;
}
