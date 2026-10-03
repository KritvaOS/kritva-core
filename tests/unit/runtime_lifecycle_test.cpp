//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : runtime_lifecycle_test.cpp
// Description : RuntimeManager lifecycle orchestration contract tests.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-RT-006
// API         : CORE-TEST-RUNTIME-LIFECYCLE
//
// Author      : KritvaOS Core Team
// Created     : 03-10-2026
//==============================================================================

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
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

// Topology used throughout: 1 -> 3, 2 -> 3, 3 -> 4, 5 -> 2 (a depends on b).
// Forward (dependencies first, lowest-id tie-break): 4, 3, 1, 2, 5.
const std::vector<Edge> kEdges = {{1, 3}, {2, 3}, {3, 4}, {5, 2}};
const std::vector<std::uint64_t> kForward = {4, 3, 1, 2, 5};
const std::vector<std::uint64_t> kReverse = {5, 2, 1, 3, 4};

Trace expect(const std::vector<std::uint64_t>& ids, const char* operation) {
    Trace t;
    for (std::uint64_t id : ids) t.push_back(std::to_string(id) + ":" + operation);
    return t;
}

Trace concat(Trace a, const Trace& b) { a.insert(a.end(), b.begin(), b.end()); return a; }

struct Fixture {
    Trace trace;
    std::vector<std::unique_ptr<ReferenceComponent>> owned;   // indexed by id - 1 (ids 1..5)
    RuntimeManager runtime;

    explicit Fixture(const std::vector<Edge>& edges = kEdges, std::vector<std::uint64_t> registration = {1, 2, 3, 4, 5}) {
        owned.resize(registration.size());
        for (std::uint64_t id : registration) {
            auto info = ComponentInfo::create(ComponentId{id}, "c");
            assert(info.has_value());
            owned[id - 1] = std::make_unique<ReferenceComponent>(std::move(info).value());
            owned[id - 1]->trace = &trace;
        }
        for (std::uint64_t id : registration) assert(runtime.register_component(*owned[id - 1]));
        for (const Edge& e : edges) assert(runtime.add_dependency(ComponentId{e.first}, ComponentId{e.second}));
    }
    ReferenceComponent& component(std::uint64_t id) { return *owned[id - 1]; }
    int total_calls() const {
        int n = 0;
        for (const auto& c : owned) n += c->configure_calls + c->initialize_calls + c->start_calls + c->stop_calls + c->shutdown_calls;
        return n;
    }
};

Configuration make_configuration() {
    Configuration c;
    assert(c.set(Parameter{"rate", ParameterValue{std::int64_t{10}}, "Hz"}).has_value());
    return c;
}

// -----------------------------------------------------------------------------
// Forward and reverse ordering (AC-01, AC-02) with exact invocation traces
// -----------------------------------------------------------------------------

void test_forward_operations_use_dependency_order() {
    Fixture f;
    assert(f.runtime.configure(make_configuration()));
    assert(f.trace == expect(kForward, "configure"));
    f.trace.clear();

    assert(f.runtime.initialize());
    assert(f.trace == expect(kForward, "initialize"));
    f.trace.clear();

    assert(f.runtime.start());
    assert(f.trace == expect(kForward, "start"));
    for (std::uint64_t id : kForward) assert(f.component(id).lifecycle_state() == LifecycleState::RUNNING);
}

void test_teardown_operations_use_reverse_dependency_order() {
    Fixture f;
    assert(f.runtime.initialize() && f.runtime.start());
    f.trace.clear();

    assert(f.runtime.stop());
    assert(f.trace == expect(kReverse, "stop"));              // dependents before dependencies
    f.trace.clear();
    assert(f.runtime.shutdown());
    assert(f.trace == expect(kReverse, "shutdown"));
    for (std::uint64_t id : kForward) assert(f.component(id).lifecycle_state() == LifecycleState::STOPPED);

    // Every component was invoked exactly once per operation.
    for (std::uint64_t id = 1; id <= 5; ++id) {
        const ReferenceComponent& c = f.component(id);
        assert(c.initialize_calls == 1 && c.start_calls == 1 && c.stop_calls == 1 && c.shutdown_calls == 1);
    }
}

void test_full_cycle_trace_and_runtime_states() {
    Fixture f;
    assert(f.runtime.state() == LifecycleState::UNKNOWN);
    assert(f.runtime.initialize() && f.runtime.state() == LifecycleState::READY);
    assert(f.runtime.start() && f.runtime.state() == LifecycleState::RUNNING);
    assert(f.runtime.stop() && f.runtime.state() == LifecycleState::STOPPED);
    assert(f.runtime.shutdown() && f.runtime.state() == LifecycleState::STOPPED);

    const Trace expected = concat(concat(expect(kForward, "initialize"), expect(kForward, "start")),
                                  concat(expect(kReverse, "stop"), expect(kReverse, "shutdown")));
    assert(f.trace == expected);
    assert(f.total_calls() == 20);
}

// A component that records what the Runtime reports while it is being called.
class ObservingComponent final : public Component {
public:
    ObservingComponent(ComponentInfo info, const Runtime& runtime, std::vector<LifecycleState>& seen)
        : Component(info), inner_(std::move(info)), runtime_(runtime), seen_(seen) {}
    Result<void> configure(const Configuration& c) override { seen_.push_back(runtime_.state()); return inner_.configure(c); }
    Result<void> initialize() override { seen_.push_back(runtime_.state()); return inner_.initialize(); }
    Result<void> start() override { seen_.push_back(runtime_.state()); return inner_.start(); }
    Result<void> stop() override { seen_.push_back(runtime_.state()); return inner_.stop(); }
    Result<void> shutdown() override { seen_.push_back(runtime_.state()); return inner_.shutdown(); }
    LifecycleState lifecycle_state() const noexcept override { return inner_.lifecycle_state(); }
    Status status() const override { return inner_.status(); }
    Health health() const override { return inner_.health(); }
    CapabilitySet capabilities() const override { return inner_.capabilities(); }
    ReferenceComponent& inner() { return inner_; }
private:
    ReferenceComponent inner_;
    const Runtime& runtime_;
    std::vector<LifecycleState>& seen_;
};

void test_runtime_state_changes_only_after_every_step_succeeded() {
    std::vector<LifecycleState> seen;
    RuntimeManager runtime;
    std::vector<std::unique_ptr<ObservingComponent>> owned;
    for (std::uint64_t id = 1; id <= 3; ++id) {
        auto info = ComponentInfo::create(ComponentId{id}, "o");
        assert(info);
        owned.push_back(std::make_unique<ObservingComponent>(std::move(info).value(), runtime, seen));
        assert(runtime.register_component(*owned.back()));
    }
    assert(runtime.initialize());
    // While components initialize the Runtime is INITIALIZING; it is READY only afterwards.
    assert(seen == std::vector<LifecycleState>(3, LifecycleState::INITIALIZING));
    seen.clear();
    assert(runtime.start());
    // While components start the Runtime is still READY: never RUNNING before all starts succeeded.
    assert(seen == std::vector<LifecycleState>(3, LifecycleState::READY));
    seen.clear();
    assert(runtime.stop());
    assert(seen == std::vector<LifecycleState>(3, LifecycleState::STOPPING));
    seen.clear();
    assert(runtime.shutdown());
    assert(seen == std::vector<LifecycleState>(3, LifecycleState::STOPPED));
}

void test_order_is_independent_of_registration_and_edge_insertion_order() {
    Trace expected;
    {
        Fixture f;
        assert(f.runtime.initialize() && f.runtime.start() && f.runtime.stop() && f.runtime.shutdown());
        expected = f.trace;
    }
    auto edges = kEdges;
    std::sort(edges.begin(), edges.end());
    std::vector<std::uint64_t> registration = {1, 2, 3, 4, 5};
    int checked = 0;
    do {
        std::next_permutation(registration.begin(), registration.end());
        Fixture f(edges, registration);
        assert(f.runtime.initialize() && f.runtime.start() && f.runtime.stop() && f.runtime.shutdown());
        assert(f.trace == expected);
        ++checked;
    } while (std::next_permutation(edges.begin(), edges.end()));
    assert(checked == 24);
}

void test_independent_branches_and_edge_cases() {
    // No dependencies: ascending id forward, descending id in reverse.
    Fixture none(std::vector<Edge>{});
    assert(none.runtime.initialize() && none.runtime.start() && none.runtime.stop());
    assert(none.trace == concat(concat(expect({1, 2, 3, 4, 5}, "initialize"), expect({1, 2, 3, 4, 5}, "start")),
                                expect({5, 4, 3, 2, 1}, "stop")));

    // An empty runtime has nothing to invoke but still follows the state table.
    RuntimeManager empty;
    assert(empty.initialize() && empty.start() && empty.stop() && empty.shutdown());
    assert(empty.state() == LifecycleState::STOPPED);

    // A single chain.
    Fixture chain({{1, 2}, {2, 3}, {3, 4}, {4, 5}});
    assert(chain.runtime.initialize());
    assert(chain.trace == expect({5, 4, 3, 2, 1}, "initialize"));
}

// -----------------------------------------------------------------------------
// Invalid and repeated calls (AC-09, AC-10): no component is invoked
// -----------------------------------------------------------------------------

void test_invalid_and_repeated_calls_invoke_no_component() {
    using S = LifecycleState;
    const auto drive_to = [](Fixture& f, S target) {
        if (target == S::READY || target == S::RUNNING || target == S::STOPPED) assert(f.runtime.initialize());
        if (target == S::RUNNING) assert(f.runtime.start());
        if (target == S::STOPPED) assert(f.runtime.stop());
        assert(f.runtime.state() == target);
        f.trace.clear();
    };
    for (S s : {S::UNKNOWN, S::READY, S::RUNNING, S::STOPPED}) {
        const bool can_configure = s == S::UNKNOWN || s == S::STOPPED;
        const bool can_initialize = can_configure;
        const bool can_start = s == S::READY;
        const bool can_stop = s == S::READY || s == S::RUNNING;
        if (!can_configure) { Fixture f; drive_to(f, s); const auto r = f.runtime.configure(make_configuration());
            assert(!r && r.error().code == ErrorCode::INVALID_STATE && f.trace.empty() && f.runtime.state() == s); }
        if (!can_initialize) { Fixture f; drive_to(f, s); const auto r = f.runtime.initialize();
            assert(!r && r.error().code == ErrorCode::INVALID_STATE && f.trace.empty() && f.runtime.state() == s); }
        if (!can_start) { Fixture f; drive_to(f, s); const auto r = f.runtime.start();
            assert(!r && r.error().code == ErrorCode::INVALID_STATE && f.trace.empty() && f.runtime.state() == s); }
        if (!can_stop) { Fixture f; drive_to(f, s); const auto r = f.runtime.stop();
            assert(!r && r.error().code == ErrorCode::INVALID_STATE && f.trace.empty() && f.runtime.state() == s); }
        if (s == S::READY || s == S::RUNNING) { Fixture f; drive_to(f, s); const auto r = f.runtime.shutdown();
            assert(!r && r.error().code == ErrorCode::INVALID_STATE && f.trace.empty() && f.runtime.state() == s); }
    }
}

void test_repeated_shutdown_never_invokes_a_component_twice() {
    Fixture never;                                                // never initialized: nothing to release
    assert(never.runtime.shutdown() && never.runtime.shutdown());
    assert(never.total_calls() == 0);

    Fixture f;
    assert(f.runtime.initialize() && f.runtime.stop());
    f.trace.clear();
    assert(f.runtime.shutdown());
    assert(f.trace == expect(kReverse, "shutdown"));
    f.trace.clear();
    assert(f.runtime.shutdown() && f.runtime.shutdown());         // repeated: no component call at all
    assert(f.trace.empty());
    for (std::uint64_t id = 1; id <= 5; ++id) assert(f.component(id).shutdown_calls == 1);

    // Re-initialization makes the components live again; the next shutdown invokes them again, once.
    assert(f.runtime.initialize() && f.runtime.stop() && f.runtime.shutdown());
    for (std::uint64_t id = 1; id <= 5; ++id) {
        assert(f.component(id).initialize_calls == 2 && f.component(id).shutdown_calls == 2);
    }
}

void test_stop_and_reinitialize_run_the_same_orders_again() {
    Fixture f;
    assert(f.runtime.initialize() && f.runtime.start() && f.runtime.stop());
    f.trace.clear();
    assert(f.runtime.initialize() && f.runtime.start());          // no implicit restart happened in stop()
    assert(f.trace == concat(expect(kForward, "initialize"), expect(kForward, "start")));
    assert(f.runtime.state() == LifecycleState::RUNNING);
}

// -----------------------------------------------------------------------------
// Configure (AC-04)
// -----------------------------------------------------------------------------

void test_configure_does_not_initialize_start_or_change_state() {
    Fixture f;
    assert(f.runtime.configure(make_configuration()));
    assert(f.runtime.state() == LifecycleState::UNKNOWN);          // documented state: unchanged
    for (std::uint64_t id = 1; id <= 5; ++id) {
        assert(f.component(id).configured);
        assert(f.component(id).lifecycle_state() == LifecycleState::UNKNOWN);   // not initialized or started
        assert(f.component(id).initialize_calls == 0 && f.component(id).start_calls == 0);
    }
    assert(!f.runtime.topology_fixed());                           // configure does not fix the topology

    // Valid again from STOPPED, in the same order.
    assert(f.runtime.initialize() && f.runtime.stop());
    f.trace.clear();
    assert(f.runtime.configure(make_configuration()));
    assert(f.trace == expect(kForward, "configure"));
    assert(f.runtime.state() == LifecycleState::STOPPED);
}

void test_configure_with_invalid_topology_invokes_nothing() {
    Fixture f({{1, 9}});                                           // 9 is not registered
    const auto r = f.runtime.configure(make_configuration());
    assert(!r && r.error().code == ErrorCode::CONFIGURATION_ERROR && r.error().source == ComponentId{1});
    assert(f.trace.empty() && f.total_calls() == 0);
    assert(f.runtime.state() == LifecycleState::UNKNOWN && !f.runtime.topology_fixed());
}

void test_configure_failure_stops_the_sequence_and_keeps_the_state() {
    Fixture f;
    f.component(1).fail_next_configure = ErrorCode::CONFIGURATION_ERROR;     // third in forward order
    const auto r = f.runtime.configure(make_configuration());
    assert(!r && r.error().code == ErrorCode::CONFIGURATION_ERROR && r.error().source == ComponentId{1});
    assert((f.trace == Trace{"4:configure", "3:configure", "1:configure"}));  // stops at the failure
    assert(f.runtime.state() == LifecycleState::UNKNOWN);
}

// -----------------------------------------------------------------------------
// Failure boundary (AC-03): fail-fast, original error, no retry, no rollback
// -----------------------------------------------------------------------------

enum class Op { Configure, Initialize, Start, Stop, Shutdown };

void inject(ReferenceComponent& c, Op op, ErrorCode code) {
    switch (op) {
        case Op::Configure:  c.fail_next_configure = code; break;
        case Op::Initialize: c.fail_next_initialize = code; break;
        case Op::Start:      c.fail_next_start = code; break;
        case Op::Stop:       c.fail_next_stop = code; break;
        case Op::Shutdown:   c.fail_next_shutdown = code; break;
    }
}

const char* name_of(Op op) {
    switch (op) {
        case Op::Configure: return "configure";  case Op::Initialize: return "initialize";
        case Op::Start: return "start";          case Op::Stop: return "stop";
        default: return "shutdown";
    }
}

Result<void> call(Fixture& f, Op op) {
    switch (op) {
        case Op::Configure:  return f.runtime.configure(make_configuration());
        case Op::Initialize: return f.runtime.initialize();
        case Op::Start:      return f.runtime.start();
        case Op::Stop:       return f.runtime.stop();
        default:             return f.runtime.shutdown();
    }
}

// Bring the fixture to the state in which `op` is valid.
void prepare(Fixture& f, Op op) {
    switch (op) {
        case Op::Configure:  case Op::Initialize: break;
        case Op::Start:      assert(f.runtime.initialize()); break;
        case Op::Stop:       assert(f.runtime.initialize() && f.runtime.start()); break;
        case Op::Shutdown:   assert(f.runtime.initialize() && f.runtime.stop()); break;
    }
}

void test_failure_at_every_position_of_every_operation() {
    for (Op op : {Op::Configure, Op::Initialize, Op::Start, Op::Stop, Op::Shutdown}) {
        const bool reverse = op == Op::Stop || op == Op::Shutdown;
        const auto& order = reverse ? kReverse : kForward;
        for (std::size_t position = 0; position < order.size(); ++position) {
            Fixture f;
            prepare(f, op);
            const LifecycleState before = f.runtime.state();
            f.trace.clear();
            const std::uint64_t failing = order[position];
            inject(f.component(failing), op, ErrorCode::TIMEOUT);

            const Result<void> r = call(f, op);

            // The component's own error, unchanged.
            assert(!r.has_value());
            assert(r.error().code == ErrorCode::TIMEOUT);
            assert(r.error().source == ComponentId{failing});
            assert(r.error().severity == ErrorSeverity::ERROR);
            assert(r.error().message == std::string(name_of(op)) + " failed");

            // Invocation order: everything up to and including the failure, nothing after, no retry.
            Trace expected;
            for (std::size_t i = 0; i <= position; ++i) expected.push_back(std::to_string(order[i]) + ":" + name_of(op));
            assert(f.trace == expected);

            // Runtime state at the failure boundary.
            if (op == Op::Configure || op == Op::Shutdown) assert(f.runtime.state() == before);
            else assert(f.runtime.state() == LifecycleState::FAULT);
            assert(f.runtime.state() != LifecycleState::RUNNING || op == Op::Configure);

            // Completed components keep their state; later ones were never invoked.
            for (std::size_t i = 0; i < order.size(); ++i) {
                const ReferenceComponent& c = f.component(order[i]);
                const int calls = op == Op::Configure ? c.configure_calls : op == Op::Initialize ? c.initialize_calls
                                : op == Op::Start ? c.start_calls : op == Op::Stop ? c.stop_calls : c.shutdown_calls;
                assert(calls == (i <= position ? 1 : 0));          // exactly once up to the failure, never after
            }
        }
    }
}

void test_faulted_runtime_rejects_every_operation_without_invoking_components() {
    for (Op op : {Op::Initialize, Op::Start, Op::Stop}) {
        Fixture f;
        prepare(f, op);
        inject(f.component(kForward[2]), op, ErrorCode::INTERNAL_ERROR);
        assert(!call(f, op));
        assert(f.runtime.state() == LifecycleState::FAULT);
        const int calls = f.total_calls();
        f.trace.clear();
        for (Op next : {Op::Configure, Op::Initialize, Op::Start, Op::Stop, Op::Shutdown}) {
            const auto r = call(f, next);
            assert(!r && r.error().code == ErrorCode::INVALID_STATE);      // recovery is R03-006
            assert(f.runtime.state() == LifecycleState::FAULT);
        }
        assert(f.trace.empty() && f.total_calls() == calls);               // nothing invoked, nothing retried
    }
}

void test_failed_initialize_keeps_earlier_components_initialized_without_rollback() {
    Fixture f;
    f.component(1).fail_next_initialize = ErrorCode::NOT_READY;            // 4, 3, then 1 fails
    assert(!f.runtime.initialize());
    assert(f.component(4).lifecycle_state() == LifecycleState::READY);      // completed: not rolled back
    assert(f.component(3).lifecycle_state() == LifecycleState::READY);
    assert(f.component(1).lifecycle_state() == LifecycleState::FAULT);      // failed: its own contract
    assert(f.component(2).lifecycle_state() == LifecycleState::UNKNOWN);    // never invoked
    assert(f.component(5).lifecycle_state() == LifecycleState::UNKNOWN);
    assert(f.component(4).stop_calls == 0 && f.component(4).shutdown_calls == 0);   // no compensation
    assert(f.runtime.topology_fixed());                                     // validation had succeeded
}

void test_failed_shutdown_leaves_state_and_may_be_retried_explicitly() {
    Fixture f;
    assert(f.runtime.initialize() && f.runtime.stop());
    f.component(1).fail_next_shutdown = ErrorCode::RESOURCE_UNAVAILABLE;
    f.trace.clear();
    const auto r = f.runtime.shutdown();
    assert(!r && r.error().source == ComponentId{1});
    assert(f.trace == Trace({"5:shutdown", "2:shutdown", "1:shutdown"}));   // reverse, stops at the failure
    assert(f.runtime.state() == LifecycleState::STOPPED);                   // unchanged

    // Explicit retry: invokes every live component again, in reverse order (documented).
    f.trace.clear();
    assert(f.runtime.shutdown());
    assert(f.trace == expect(kReverse, "shutdown"));
    f.trace.clear();
    assert(f.runtime.shutdown() && f.trace.empty());                        // now released: no-op
}

void test_validation_failure_invokes_no_component() {
    Fixture f({{1, 9}});
    const auto r = f.runtime.initialize();
    assert(!r && r.error().code == ErrorCode::CONFIGURATION_ERROR);
    assert(f.runtime.state() == LifecycleState::UNKNOWN);
    assert(f.total_calls() == 0);
    assert(f.runtime.shutdown() && f.total_calls() == 0);                    // nothing was initialized
}

void test_no_automatic_retry_or_background_activity() {
    Fixture f;
    f.component(3).fail_next_start = ErrorCode::TIMEOUT;
    assert(f.runtime.initialize());
    assert(!f.runtime.start());
    const Trace after_failure = f.trace;
    for (int i = 0; i < 1000; ++i) assert(f.runtime.state() == LifecycleState::FAULT);
    assert(f.trace == after_failure);                                        // no retry, no background work
    assert(f.component(3).start_calls == 1);
}

} // namespace

int main() {
    test_forward_operations_use_dependency_order();
    test_teardown_operations_use_reverse_dependency_order();
    test_full_cycle_trace_and_runtime_states();
    test_runtime_state_changes_only_after_every_step_succeeded();
    test_order_is_independent_of_registration_and_edge_insertion_order();
    test_independent_branches_and_edge_cases();
    test_invalid_and_repeated_calls_invoke_no_component();
    test_repeated_shutdown_never_invokes_a_component_twice();
    test_stop_and_reinitialize_run_the_same_orders_again();
    test_configure_does_not_initialize_start_or_change_state();
    test_configure_with_invalid_topology_invokes_nothing();
    test_configure_failure_stops_the_sequence_and_keeps_the_state();
    test_failure_at_every_position_of_every_operation();
    test_faulted_runtime_rejects_every_operation_without_invoking_components();
    test_failed_initialize_keeps_earlier_components_initialized_without_rollback();
    test_failed_shutdown_leaves_state_and_may_be_retried_explicitly();
    test_validation_failure_invokes_no_component();
    test_no_automatic_retry_or_background_activity();
    return 0;
}
