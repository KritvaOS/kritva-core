//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : runtime_failure_test.cpp
// Description : RuntimeManager failure propagation and explicit recovery tests.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-RT-008
// API         : CORE-TEST-RUNTIME-FAILURE
//
// Author      : KritvaOS Core Team
// Created     : 03-10-2026
//==============================================================================

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <kritva/core/core.hpp>

#include "../contract/reference_component.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;
using kritva::core::runtime::contract::ReferenceComponent;

namespace {

using Trace = std::vector<std::string>;
using Edge = std::pair<std::uint64_t, std::uint64_t>;   // (dependent, dependency)
using Ids = std::vector<std::uint64_t>;

// Topology: 1 -> 3, 2 -> 3, 3 -> 4, 5 -> 2. Forward 4,3,1,2,5; reverse 5,2,1,3,4.
const std::vector<Edge> kEdges = {{1, 3}, {2, 3}, {3, 4}, {5, 2}};
const Ids kForward = {4, 3, 1, 2, 5};
const Ids kReverse = {5, 2, 1, 3, 4};

Trace expect(const Ids& ids, const char* operation) {
    Trace t;
    for (std::uint64_t id : ids) t.push_back(std::to_string(id) + ":" + operation);
    return t;
}

struct Fixture {
    Trace trace;
    std::vector<std::unique_ptr<ReferenceComponent>> owned;   // index = id - 1
    RuntimeManager runtime;

    Fixture() {
        for (std::uint64_t id = 1; id <= 5; ++id) {
            auto info = ComponentInfo::create(ComponentId{id}, "c");
            assert(info.has_value());
            owned.push_back(std::make_unique<ReferenceComponent>(std::move(info).value()));
            owned.back()->trace = &trace;
            assert(runtime.register_component(*owned.back()));
        }
        for (const Edge& e : kEdges) assert(runtime.add_dependency(ComponentId{e.first}, ComponentId{e.second}));
    }
    ReferenceComponent& component(std::uint64_t id) { return *owned[id - 1]; }
    int calls() const {
        int n = 0;
        for (const auto& c : owned) n += c->configure_calls + c->initialize_calls + c->start_calls + c->stop_calls + c->shutdown_calls;
        return n;
    }
};

enum class Op { Initialize, Start, Stop };
const char* name_of(Op op) { return op == Op::Initialize ? "initialize" : op == Op::Start ? "start" : "stop"; }

void inject(ReferenceComponent& c, Op op, ErrorCode code) {
    if (op == Op::Initialize) c.fail_next_initialize = code;
    else if (op == Op::Start) c.fail_next_start = code;
    else c.fail_next_stop = code;
}

Result<void> call(Fixture& f, Op op) {
    return op == Op::Initialize ? f.runtime.initialize() : op == Op::Start ? f.runtime.start() : f.runtime.stop();
}

void prepare(Fixture& f, Op op) {
    if (op == Op::Start || op == Op::Stop) assert(f.runtime.initialize());
    if (op == Op::Stop) assert(f.runtime.start());
}

const Ids& order_for(Op op) { return op == Op::Stop ? kReverse : kForward; }

// -----------------------------------------------------------------------------
// Failure propagation, fault state (AC-01, AC-02)
// -----------------------------------------------------------------------------

void test_failure_is_propagated_with_original_error_and_recorded_as_the_fault() {
    for (Op op : {Op::Initialize, Op::Start, Op::Stop}) {
        const Ids& order = order_for(op);
        for (std::size_t position = 0; position < order.size(); ++position) {
            Fixture f;
            prepare(f, op);
            const std::uint64_t failing = order[position];
            inject(f.component(failing), op, ErrorCode::TIMEOUT);
            assert(f.runtime.fault_error() == nullptr);          // no fault before the failure

            const auto r = call(f, op);
            assert(!r.has_value());
            assert(r.error().code == ErrorCode::TIMEOUT);
            assert(r.error().source == ComponentId{failing});
            assert(r.error().message == std::string(name_of(op)) + " failed");
            assert(f.runtime.state() == LifecycleState::FAULT);

            // The fault is the same Error, observable afterwards.
            const Error* fault = f.runtime.fault_error();
            assert(fault != nullptr);
            assert(fault->code == r.error().code && fault->source == r.error().source);
            assert(fault->message == r.error().message && fault->severity == r.error().severity);

            // The failed component follows the Component contract: FAULT.
            assert(f.component(failing).lifecycle_state() == LifecycleState::FAULT);
        }
    }
}

// -----------------------------------------------------------------------------
// Explicit recovery: cleanup order, partial progress (AC-03..AC-07)
// -----------------------------------------------------------------------------

// Independent model of what each component has completed when `op` fails at `position`.
enum class Stage { None, Initialized, Started, Stopped, Faulted };
std::map<std::uint64_t, Stage> stages_after(Op op, std::size_t position) {
    std::map<std::uint64_t, Stage> stage;
    const Ids& order = order_for(op);
    for (std::uint64_t id : kForward) stage[id] = Stage::None;
    if (op == Op::Initialize) {
        for (std::size_t i = 0; i < position; ++i) stage[order[i]] = Stage::Initialized;
    } else if (op == Op::Start) {
        for (std::uint64_t id : kForward) stage[id] = Stage::Initialized;
        for (std::size_t i = 0; i < position; ++i) stage[order[i]] = Stage::Started;
    } else {
        for (std::uint64_t id : kForward) stage[id] = Stage::Started;
        for (std::size_t i = 0; i < position; ++i) stage[order[i]] = Stage::Stopped;
    }
    stage[order[position]] = Stage::Faulted;
    return stage;
}

void test_reset_cleans_up_in_reverse_order_after_every_failure_point() {
    for (Op op : {Op::Initialize, Op::Start, Op::Stop}) {
        const Ids& order = order_for(op);
        for (std::size_t position = 0; position < order.size(); ++position) {
            Fixture f;
            prepare(f, op);
            inject(f.component(order[position]), op, ErrorCode::INTERNAL_ERROR);
            assert(!call(f, op));
            const Error original = *f.runtime.fault_error();
            const int calls_before = f.calls();
            f.trace.clear();

            // Expected cleanup from the model: pass 1 stops initialized/started components in
            // reverse order; pass 2 shuts down stopped (incl. just stopped) and faulted ones.
            auto stage = stages_after(op, position);
            Trace expected;
            for (std::uint64_t id : kReverse)
                if (stage[id] == Stage::Initialized || stage[id] == Stage::Started) { expected.push_back(std::to_string(id) + ":stop"); stage[id] = Stage::Stopped; }
            for (std::uint64_t id : kReverse)
                if (stage[id] == Stage::Stopped || stage[id] == Stage::Faulted) expected.push_back(std::to_string(id) + ":shutdown");

            const auto r = f.runtime.reset();
            assert(r.has_value());
            assert(f.trace == expected);
            assert(f.runtime.state() == LifecycleState::STOPPED);
            assert(f.runtime.fault_error() == nullptr);            // recovered: fault cleared
            assert(f.calls() == calls_before + static_cast<int>(expected.size()));

            // Every component that ran is released (STOPPED); never-invoked ones are untouched.
            const auto initial = stages_after(op, position);
            for (std::uint64_t id = 1; id <= 5; ++id) {
                const LifecycleState want = initial.at(id) == Stage::None ? LifecycleState::UNKNOWN : LifecycleState::STOPPED;
                assert(f.component(id).lifecycle_state() == want);
            }
            // The failed operation itself was not retried by reset().
            assert((op == Op::Initialize ? f.component(order[position]).initialize_calls
                  : op == Op::Start ? f.component(order[position]).start_calls
                  : f.component(order[position]).stop_calls) == 1);
            (void)original;

            // Recovery is complete: an explicit new attempt works from the beginning.
            f.trace.clear();
            assert(f.runtime.initialize());
            assert(f.trace == expect(kForward, "initialize"));
            assert(f.runtime.start());
            assert(f.runtime.state() == LifecycleState::RUNNING);
        }
    }
}

void test_reset_is_valid_only_in_fault_and_invokes_nothing_otherwise() {
    using S = LifecycleState;
    for (S s : {S::UNKNOWN, S::READY, S::RUNNING, S::STOPPED}) {
        Fixture f;
        if (s != S::UNKNOWN) assert(f.runtime.initialize());
        if (s == S::RUNNING) assert(f.runtime.start());
        if (s == S::STOPPED) assert(f.runtime.stop());
        assert(f.runtime.state() == s);
        const int calls = f.calls();
        f.trace.clear();
        const auto r = f.runtime.reset();
        assert(!r && r.error().code == ErrorCode::INVALID_STATE);
        assert(f.runtime.state() == s && f.calls() == calls && f.trace.empty());
    }
    // A successful reset leaves STOPPED: a second reset is invalid too.
    Fixture f;
    f.component(1).fail_next_initialize = ErrorCode::NOT_READY;
    assert(!f.runtime.initialize() && f.runtime.reset());
    const auto again = f.runtime.reset();
    assert(!again && again.error().code == ErrorCode::INVALID_STATE);
}

void test_fault_blocks_everything_except_reset() {
    Fixture f;
    f.component(3).fail_next_start = ErrorCode::TIMEOUT;
    assert(f.runtime.initialize() && !f.runtime.start());
    const int calls = f.calls();
    f.trace.clear();
    assert(!f.runtime.initialize() && !f.runtime.start() && !f.runtime.stop() && !f.runtime.shutdown());
    assert(!f.runtime.configure(Configuration{}));
    assert(f.calls() == calls && f.trace.empty());                 // nothing invoked, nothing retried
    assert(f.runtime.state() == LifecycleState::FAULT);
    assert(f.runtime.fault_error() != nullptr);
}

// -----------------------------------------------------------------------------
// Failures during recovery keep the original fault and the progress made
// -----------------------------------------------------------------------------

void test_failed_cleanup_stop_keeps_fault_and_resumes_without_repeats() {
    Fixture f;
    f.component(1).fail_next_start = ErrorCode::TIMEOUT;            // start order 4,3,1(fails),2,5
    assert(f.runtime.initialize() && !f.runtime.start());
    const Error original = *f.runtime.fault_error();

    // Cleanup stops initialized/started components in reverse: 5, 2 (initialized), 3, 4 (started);
    // component 1 is faulted and is not stopped. Make 2's stop fail.
    f.component(2).fail_next_stop = ErrorCode::RESOURCE_UNAVAILABLE;
    f.trace.clear();
    const auto r = f.runtime.reset();
    assert(!r && r.error().code == ErrorCode::RESOURCE_UNAVAILABLE && r.error().source == ComponentId{2});
    assert(f.trace == Trace({"5:stop", "2:stop"}));                 // stops at the failure
    assert(f.runtime.state() == LifecycleState::FAULT);
    const Error* fault = f.runtime.fault_error();                   // still the ORIGINAL failure
    assert(fault != nullptr && fault->code == original.code && fault->source == original.source && fault->message == original.message);

    // Explicit retry: 5 (stopped) is not stopped again; 2 failed its stop so it is faulted and is
    // shut down, not stopped again; 3 and 4 are stopped; then every stopped/faulted one is shut down.
    f.trace.clear();
    assert(f.runtime.reset());
    assert(f.trace == Trace({"3:stop", "4:stop", "5:shutdown", "2:shutdown", "1:shutdown", "3:shutdown", "4:shutdown"}));
    assert(f.component(5).stop_calls == 1 && f.component(2).stop_calls == 1);   // no duplicate attempts
    assert(f.component(1).start_calls == 1);                                   // failed start never retried
    assert(f.runtime.state() == LifecycleState::STOPPED && f.runtime.fault_error() == nullptr);
}

void test_failed_cleanup_shutdown_keeps_fault_and_resumes_without_repeats() {
    Fixture f;
    f.component(4).fail_next_initialize = ErrorCode::NOT_READY;     // first component fails
    assert(!f.runtime.initialize());
    // Only component 4 ran (faulted): cleanup is a single shutdown. Make it fail.
    f.component(4).fail_next_shutdown = ErrorCode::TIMEOUT;
    f.trace.clear();
    const auto r = f.runtime.reset();
    assert(!r && r.error().source == ComponentId{4} && r.error().code == ErrorCode::TIMEOUT);
    assert(f.trace == Trace({"4:shutdown"}));
    assert(f.runtime.state() == LifecycleState::FAULT);
    assert(f.runtime.fault_error() != nullptr && f.runtime.fault_error()->code == ErrorCode::NOT_READY);   // original

    f.trace.clear();
    assert(f.runtime.reset());                                      // explicit second request
    assert(f.trace == Trace({"4:shutdown"}));
    assert(f.component(4).initialize_calls == 1);                   // initialize is not retried
    assert(f.runtime.state() == LifecycleState::STOPPED);

    // Shutdown failure part-way through a larger cleanup resumes at the failing component.
    Fixture g;
    g.component(2).fail_next_start = ErrorCode::INTERNAL_ERROR;     // order 4,3,1,2(fails),5
    assert(g.runtime.initialize() && !g.runtime.start());
    g.component(1).fail_next_shutdown = ErrorCode::TIMEOUT;         // pass 2 reverse: 5,2,1(fails),3,4
    g.trace.clear();
    assert(!g.runtime.reset());
    assert(g.trace == Trace({"5:stop", "1:stop", "3:stop", "4:stop", "5:shutdown", "2:shutdown", "1:shutdown"}));
    g.trace.clear();
    assert(g.runtime.reset());
    assert(g.trace == Trace({"1:shutdown", "3:shutdown", "4:shutdown"}));      // resumes; 5 and 2 not repeated
}

// -----------------------------------------------------------------------------
// No automatic retry or background recovery (AC-05, AC-11)
// -----------------------------------------------------------------------------

void test_nothing_happens_without_an_explicit_request() {
    Fixture f;
    f.component(3).fail_next_initialize = ErrorCode::TIMEOUT;
    assert(!f.runtime.initialize());
    const Trace after = f.trace;
    const int calls = f.calls();
    for (int i = 0; i < 1000; ++i) {
        assert(f.runtime.state() == LifecycleState::FAULT);
        assert(f.runtime.fault_error() != nullptr);
        (void)f.runtime.statistics();
    }
    assert(f.trace == after && f.calls() == calls);                 // no retry, no background recovery
    assert(f.component(3).initialize_calls == 1);                   // the failed operation was attempted once
    assert(f.runtime.statistics().retry_count.value() == 0);
}

// -----------------------------------------------------------------------------
// Health and error are distinct (AC-08)
// -----------------------------------------------------------------------------

// Reports whatever health it is told to and counts every observer call.
class HealthProbe final : public Component {
public:
    explicit HealthProbe(ComponentInfo info) : Component(std::move(info)), inner_(info_copy()) {}
    Result<void> configure(const Configuration& c) override { return inner_.configure(c); }
    Result<void> initialize() override { return inner_.initialize(); }
    Result<void> start() override { return inner_.start(); }
    Result<void> stop() override { return inner_.stop(); }
    Result<void> shutdown() override { return inner_.shutdown(); }
    LifecycleState lifecycle_state() const noexcept override { ++observer_calls; return inner_.lifecycle_state(); }
    Status status() const override { ++observer_calls; return inner_.status(); }
    Health health() const override { ++observer_calls; return reported; }
    CapabilitySet capabilities() const override { ++observer_calls; return inner_.capabilities(); }
    Health reported{HealthState::HEALTHY};
    mutable int observer_calls{0};
private:
    ComponentInfo info_copy() const { return info(); }
    ReferenceComponent inner_;
};

void test_health_never_triggers_recovery_and_is_distinct_from_fault() {
    auto info_a = ComponentInfo::create(ComponentId{1}, "a");
    auto info_b = ComponentInfo::create(ComponentId{2}, "b");
    assert(info_a && info_b);
    HealthProbe a(std::move(info_a).value());
    HealthProbe b(std::move(info_b).value());
    RuntimeManager runtime;
    assert(runtime.register_component(a) && runtime.register_component(b));
    assert(runtime.add_dependency(ComponentId{2}, ComponentId{1}));

    a.reported = Health(HealthState::UNHEALTHY);                      // reports the worst health...
    b.reported = Health(HealthState::DEGRADED);
    assert(runtime.initialize() && runtime.start());
    assert(runtime.state() == LifecycleState::RUNNING);               // ...yet the runtime is not faulted
    assert(runtime.fault_error() == nullptr);
    assert(runtime.stop() && runtime.shutdown());

    // The Runtime never reads a component's state or health at all.
    assert(a.observer_calls == 0 && b.observer_calls == 0);
    assert(runtime.statistics().error_count.value() == 0);
}

// -----------------------------------------------------------------------------
// Statistics (AC-09)
// -----------------------------------------------------------------------------

void test_statistics_follow_the_documented_updates() {
    static_assert(std::is_same_v<decltype(std::declval<const RuntimeManager&>().statistics()), const Statistics&>);
    {
        Fixture f;
        assert(f.runtime.statistics().sample_count.value() == 0 && f.runtime.statistics().error_count.value() == 0);
        assert(f.runtime.initialize() && f.runtime.start() && f.runtime.stop() && f.runtime.shutdown());
        assert(f.runtime.statistics().sample_count.value() == 20);     // 4 operations x 5 components
        assert(f.runtime.statistics().error_count.value() == 0);
    }
    {
        Fixture f;
        f.component(1).fail_next_initialize = ErrorCode::TIMEOUT;     // order 4,3,1(fails),2,5
        assert(!f.runtime.initialize());
        assert(f.runtime.statistics().sample_count.value() == 2);      // 4 and 3 succeeded
        assert(f.runtime.statistics().error_count.value() == 1);
        assert(f.runtime.reset());                                     // stop 3,4 and shutdown 1,3,4: 5 successful calls
        assert(f.runtime.statistics().sample_count.value() == 2 + 5);
        assert(f.runtime.statistics().error_count.value() == 1);
        // Cleanup failures count too.
        Fixture g;
        g.component(4).fail_next_initialize = ErrorCode::TIMEOUT;
        assert(!g.runtime.initialize());
        g.component(4).fail_next_shutdown = ErrorCode::TIMEOUT;
        assert(!g.runtime.reset());
        assert(g.runtime.statistics().error_count.value() == 2);
    }
    // Retries are never made; unused fields stay zero; invalid calls and failed validation count nothing.
    Fixture f;
    f.component(5).fail_next_start = ErrorCode::TIMEOUT;
    assert(f.runtime.initialize() && !f.runtime.start() && f.runtime.reset());
    const Statistics& s = f.runtime.statistics();
    assert(s.retry_count.value() == 0 && s.drop_count.value() == 0);
    assert(s.queue_depth.value() == 0 && s.utilization.value() == 0);
    const auto before = s.sample_count.value();
    assert(!f.runtime.start() && !f.runtime.reset());                  // invalid calls
    assert(s.sample_count.value() == before && s.error_count.value() == 1);

    RuntimeManager bad;
    assert(bad.add_dependency(ComponentId{1}, ComponentId{2}));
    assert(!bad.initialize());                                         // validation failure
    assert(bad.statistics().sample_count.value() == 0 && bad.statistics().error_count.value() == 0);
}

// -----------------------------------------------------------------------------
// Dependency and dependent failures across branches
// -----------------------------------------------------------------------------

void test_dependency_failure_leaves_dependents_untouched() {
    Fixture f;
    f.component(3).fail_next_initialize = ErrorCode::TIMEOUT;         // 3 is a dependency of 1 and 2
    assert(!f.runtime.initialize());
    assert((f.trace == Trace{"4:initialize", "3:initialize"}));       // dependents 1, 2, 5 never invoked
    for (std::uint64_t id : {1u, 2u, 5u}) assert(f.component(id).lifecycle_state() == LifecycleState::UNKNOWN);
    f.trace.clear();
    assert(f.runtime.reset());
    assert((f.trace == Trace{"4:stop", "3:shutdown", "4:shutdown"}));
    assert(f.runtime.initialize() && f.runtime.start());              // recovery path works
}

void test_dependent_failure_unwinds_all_branches_in_reverse() {
    Fixture f;
    f.component(5).fail_next_initialize = ErrorCode::NOT_READY;       // last dependent fails
    assert(!f.runtime.initialize());
    f.trace.clear();
    assert(f.runtime.reset());
    // Stop initialized components dependents-first (2, 1, 3, 4), then release 5 (faulted) and the rest.
    assert((f.trace == Trace{"2:stop", "1:stop", "3:stop", "4:stop", "5:shutdown", "2:shutdown", "1:shutdown", "3:shutdown", "4:shutdown"}));
    for (std::uint64_t id = 1; id <= 5; ++id) assert(f.component(id).lifecycle_state() == LifecycleState::STOPPED);
}

} // namespace

int main() {
    test_failure_is_propagated_with_original_error_and_recorded_as_the_fault();
    test_reset_cleans_up_in_reverse_order_after_every_failure_point();
    test_reset_is_valid_only_in_fault_and_invokes_nothing_otherwise();
    test_fault_blocks_everything_except_reset();
    test_failed_cleanup_stop_keeps_fault_and_resumes_without_repeats();
    test_failed_cleanup_shutdown_keeps_fault_and_resumes_without_repeats();
    test_nothing_happens_without_an_explicit_request();
    test_health_never_triggers_recovery_and_is_distinct_from_fault();
    test_statistics_follow_the_documented_updates();
    test_dependency_failure_leaves_dependents_untouched();
    test_dependent_failure_unwinds_all_branches_in_reverse();
    return 0;
}
