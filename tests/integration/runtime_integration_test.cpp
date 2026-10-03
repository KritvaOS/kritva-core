//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : runtime_integration_test.cpp
// Description : End-to-end integration tests of the frozen R03 runtime contracts.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-RT-009
// API         : CORE-TEST-RUNTIME-INTEGRATION
//
// Author      : KritvaOS Core Team
// Created     : 03-10-2026
//==============================================================================

// These tests consume only public APIs (Component, ComponentRegistry via RuntimeManager,
// DependencyGraph via RuntimeManager, RuntimeManager) with reference components and
// test-only fault injection. They assert observable contract behavior: invocation traces,
// results, states, statistics. Nothing here changes or depends on private details, and no
// result depends on registration or container iteration order.

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <kritva/core/core.hpp>

#include "../contract/reference_component.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;
using kritva::core::runtime::contract::ReferenceComponent;

namespace {

using Ids = std::vector<std::uint64_t>;
using Edge = std::pair<std::uint64_t, std::uint64_t>;   // (dependent, dependency)
using Trace = std::vector<std::string>;
using S = LifecycleState;

Trace expect(const Ids& ids, const char* operation) {
    Trace t;
    for (std::uint64_t id : ids) t.push_back(std::to_string(id) + ":" + operation);
    return t;
}
Trace join(Trace a, const Trace& b) { a.insert(a.end(), b.begin(), b.end()); return a; }
Ids reversed(Ids ids) { std::reverse(ids.begin(), ids.end()); return ids; }

// Independent oracle for the frozen ordering rule: dependencies first, ties by lowest id.
Ids oracle_order(std::size_t n, const std::vector<Edge>& edges) {
    std::map<std::uint64_t, std::set<std::uint64_t>> deps;
    for (const Edge& e : edges) deps[e.first].insert(e.second);
    Ids order;
    std::set<std::uint64_t> placed;
    while (order.size() < n) {
        for (std::uint64_t id = 1; id <= n; ++id) {
            if (placed.count(id) != 0) continue;
            bool ready = true;
            for (std::uint64_t d : deps[id]) ready = ready && placed.count(d) != 0;
            if (ready) { order.push_back(id); placed.insert(id); break; }   // lowest eligible id
        }
    }
    return order;
}

// A runtime plus its reference components, topology and shared invocation trace.
struct Rig {
    Trace trace;
    std::vector<std::unique_ptr<ReferenceComponent>> owned;   // index = id - 1
    RuntimeManager runtime;
    Ids forward;

    Rig(std::size_t n, const std::vector<Edge>& edges, const Ids& registration, const std::vector<Edge>& edge_order) {
        owned.resize(n);
        for (std::uint64_t id : registration) {
            auto info = ComponentInfo::create(ComponentId{id}, "c");
            assert(info.has_value());
            owned[id - 1] = std::make_unique<ReferenceComponent>(std::move(info).value());
            owned[id - 1]->trace = &trace;
            assert(runtime.register_component(*owned[id - 1]));
        }
        for (const Edge& e : edge_order) assert(runtime.add_dependency(ComponentId{e.first}, ComponentId{e.second}));
        forward = oracle_order(n, edges);
    }
    Rig(std::size_t n, const std::vector<Edge>& edges) : Rig(n, edges, iota(n), edges) {}

    static Ids iota(std::size_t n) { Ids ids(n); for (std::size_t i = 0; i < n; ++i) ids[i] = i + 1; return ids; }
    ReferenceComponent& c(std::uint64_t id) { return *owned[id - 1]; }
    Ids reverse_order() const { return reversed(forward); }
    int calls() const {
        int n = 0;
        for (const auto& x : owned) n += x->configure_calls + x->initialize_calls + x->start_calls + x->stop_calls + x->shutdown_calls;
        return n;
    }
    std::size_t failures_injected_pending() const {
        std::size_t n = 0;
        for (const auto& x : owned)
            for (ErrorCode code : {x->fail_next_configure, x->fail_next_initialize, x->fail_next_start, x->fail_next_stop, x->fail_next_shutdown})
                if (code != ErrorCode::NONE) ++n;
        return n;
    }
    void clear_injections() {
        for (auto& x : owned) x->fail_next_configure = x->fail_next_initialize = x->fail_next_start = x->fail_next_stop = x->fail_next_shutdown = ErrorCode::NONE;
    }
    void expect_statistics_match_calls() {
        const Statistics& st = runtime.statistics();
        assert(st.sample_count.value() + st.error_count.value() == static_cast<std::uint64_t>(calls()));   // every call counted once
        assert(st.retry_count.value() == 0 && st.drop_count.value() == 0);
        assert(st.queue_depth.value() == 0 && st.utilization.value() == 0);
    }
};

// Topologies. Eight components: 1..8.
//   fan-in/fan-out/diamond/chain branches in one graph:
//   1 -> 4, 1 -> 5        (1 depends on 4 and 5)
//   2 -> 5                (2 depends on 5)
//   3 -> 6                (3 depends on 6)
//   4 -> 7, 5 -> 7        (diamond below 1)
//   6 -> 8, 7 -> 8        (everything funnels into 8)
const std::vector<Edge> kBranches = {{1, 4}, {1, 5}, {2, 5}, {3, 6}, {4, 7}, {5, 7}, {6, 8}, {7, 8}};

// -----------------------------------------------------------------------------
// AC-01 / AC-03: complete lifecycle across live periods
// -----------------------------------------------------------------------------

void test_full_lifecycle_across_two_live_periods() {
    Rig r(8, kBranches);
    const Ids& fwd = r.forward;
    assert((fwd == Ids{8, 6, 3, 7, 4, 5, 1, 2}));                        // oracle sanity: matches the frozen rule
    assert(r.runtime.state() == S::UNKNOWN && !r.runtime.topology_fixed());

    assert(r.runtime.configure(Configuration{}));
    assert(r.trace == expect(fwd, "configure") && r.runtime.state() == S::UNKNOWN && !r.runtime.topology_fixed());
    r.trace.clear();

    assert(r.runtime.initialize() && r.runtime.state() == S::READY && r.runtime.topology_fixed());
    assert(r.trace == expect(fwd, "initialize"));
    r.trace.clear();
    assert(r.runtime.start() && r.runtime.state() == S::RUNNING);
    assert(r.trace == expect(fwd, "start"));
    for (std::uint64_t id : fwd) assert(r.c(id).lifecycle_state() == S::RUNNING);
    r.trace.clear();
    assert(r.runtime.stop() && r.runtime.state() == S::STOPPED);
    assert(r.trace == expect(r.reverse_order(), "stop"));
    for (std::uint64_t id : fwd) assert(r.c(id).lifecycle_state() == S::STOPPED);
    r.trace.clear();
    assert(r.runtime.shutdown() && r.runtime.state() == S::STOPPED);
    assert(r.trace == expect(r.reverse_order(), "shutdown"));
    r.trace.clear();

    // A second live period after a completed one: new explicit initialize/start.
    assert(r.runtime.initialize() && r.runtime.start() && r.runtime.state() == S::RUNNING);
    assert(r.trace == join(expect(fwd, "initialize"), expect(fwd, "start")));
    assert(r.runtime.stop() && r.runtime.shutdown());

    for (std::uint64_t id = 1; id <= 8; ++id) {
        assert(r.c(id).configure_calls == 1 && r.c(id).initialize_calls == 2 && r.c(id).start_calls == 2);
        assert(r.c(id).stop_calls == 2 && r.c(id).shutdown_calls == 2);          // exactly once per live period
    }
    assert(r.runtime.statistics().sample_count.value() == 8 * (1 + 2 + 2 + 2 + 2));
    assert(r.runtime.statistics().error_count.value() == 0);
    r.expect_statistics_match_calls();
}

// -----------------------------------------------------------------------------
// AC-01 / AC-02: branch shapes
// -----------------------------------------------------------------------------

void test_branch_shapes_order_forward_and_reverse() {
    struct Shape { std::size_t n; std::vector<Edge> edges; Ids forward; };
    const std::vector<Shape> shapes = {
        {5, {{1, 2}, {1, 3}, {1, 4}, {1, 5}}, {2, 3, 4, 5, 1}},                         // fan-in: one depends on many
        {5, {{2, 1}, {3, 1}, {4, 1}, {5, 1}}, {1, 2, 3, 4, 5}},                         // fan-out: many depend on one
        {5, {{1, 2}, {2, 3}, {3, 4}, {4, 5}}, {5, 4, 3, 2, 1}},                         // deep chain, ids against the order
        {4, {{1, 2}, {1, 3}, {2, 4}, {3, 4}}, {4, 2, 3, 1}},                            // diamond
        {6, {{3, 1}, {6, 4}}, {1, 2, 3, 4, 5, 6}},                                      // independent branches, lowest id first
    };
    for (const Shape& shape : shapes) {
        Rig r(shape.n, shape.edges);
        assert(r.forward == shape.forward);
        assert(r.runtime.initialize() && r.runtime.start() && r.runtime.stop() && r.runtime.shutdown());
        const Trace expected = join(join(expect(shape.forward, "initialize"), expect(shape.forward, "start")),
                                    join(expect(reversed(shape.forward), "stop"), expect(reversed(shape.forward), "shutdown")));
        assert(r.trace == expected);
        // Dependencies are always initialized before and stopped after their dependents.
        for (const Edge& e : shape.edges) {
            const auto pos = [&](const char* op, std::uint64_t id) {
                const auto it = std::find(r.trace.begin(), r.trace.end(), std::to_string(id) + ":" + op);
                return it - r.trace.begin();
            };
            assert(pos("initialize", e.second) < pos("initialize", e.first));
            assert(pos("stop", e.first) < pos("stop", e.second));
        }
    }
}

// -----------------------------------------------------------------------------
// AC-02: determinism across registration and edge insertion order
// -----------------------------------------------------------------------------

Trace full_cycle_trace(Rig& r) {
    assert(r.runtime.configure(Configuration{}) && r.runtime.initialize() && r.runtime.start());
    assert(r.runtime.stop() && r.runtime.shutdown());
    return r.trace;
}

void test_registration_and_edge_order_permutations_are_unobservable() {
    // Exhaustive: 4 edges (24 insertion orders) x 5 registration orders.
    const std::vector<Edge> edges = {{1, 3}, {2, 3}, {3, 4}, {5, 2}};
    Trace expected;
    {
        Rig base(5, edges);
        expected = full_cycle_trace(base);
    }
    std::vector<Edge> perm = edges;
    std::sort(perm.begin(), perm.end());
    std::mt19937 rng(7);
    int checked = 0;
    do {
        for (int k = 0; k < 5; ++k) {
            Ids registration = Rig::iota(5);
            std::shuffle(registration.begin(), registration.end(), rng);
            Rig r(5, edges, registration, perm);
            assert(full_cycle_trace(r) == expected);
            ++checked;
        }
    } while (std::next_permutation(perm.begin(), perm.end()));
    assert(checked == 120);

    // Randomized on the 8-component branch topology.
    Trace expected8;
    {
        Rig base(8, kBranches);
        expected8 = full_cycle_trace(base);
    }
    for (int round = 0; round < 200; ++round) {
        Ids registration = Rig::iota(8);
        std::shuffle(registration.begin(), registration.end(), rng);
        std::vector<Edge> edge_order = kBranches;
        std::shuffle(edge_order.begin(), edge_order.end(), rng);
        Rig r(8, kBranches, registration, edge_order);
        assert(full_cycle_trace(r) == expected8);
    }
}

// -----------------------------------------------------------------------------
// AC-04: invalid operations invoke nothing and change no progress
// -----------------------------------------------------------------------------

void test_invalid_operations_do_not_disturb_a_lifecycle() {
    Rig clean(8, kBranches);
    const Trace expected = full_cycle_trace(clean);

    Rig r(8, kBranches);
    auto invalid_calls = [&](bool configure_ok, bool initialize_ok, bool start_ok, bool stop_ok, bool shutdown_ok) {
        const int before = r.calls();
        const S state = r.runtime.state();
        if (!configure_ok) { const auto x = r.runtime.configure(Configuration{}); assert(!x && x.error().code == ErrorCode::INVALID_STATE); }
        if (!initialize_ok) { const auto x = r.runtime.initialize(); assert(!x && x.error().code == ErrorCode::INVALID_STATE); }
        if (!start_ok) { const auto x = r.runtime.start(); assert(!x && x.error().code == ErrorCode::INVALID_STATE); }
        if (!stop_ok) { const auto x = r.runtime.stop(); assert(!x && x.error().code == ErrorCode::INVALID_STATE); }
        if (!shutdown_ok) { const auto x = r.runtime.shutdown(); assert(!x && x.error().code == ErrorCode::INVALID_STATE); }
        const auto reset = r.runtime.reset();                                  // reset is invalid outside FAULT
        assert(!reset && reset.error().code == ErrorCode::INVALID_STATE);
        assert(r.calls() == before && r.runtime.state() == state);
    };
    invalid_calls(true, true, false, false, true);                              // UNKNOWN: start/stop invalid
    assert(r.runtime.configure(Configuration{}));
    invalid_calls(true, true, false, false, true);
    assert(r.runtime.initialize());
    invalid_calls(false, false, true, true, false);                             // READY
    assert(r.runtime.start());
    invalid_calls(false, false, false, true, false);                            // RUNNING
    assert(r.runtime.stop());
    invalid_calls(true, true, false, false, true);                              // STOPPED
    assert(r.runtime.shutdown());
    assert(r.trace == join(join(expect(r.forward, "configure"), expect(r.forward, "initialize")),
                           join(join(expect(r.forward, "start"), expect(r.reverse_order(), "stop")), expect(r.reverse_order(), "shutdown"))));
    (void)expected;
    r.expect_statistics_match_calls();
}

// -----------------------------------------------------------------------------
// AC-05 / AC-06: representative failures and reset
// -----------------------------------------------------------------------------

enum class Op { Initialize, Start, Stop };

void inject(ReferenceComponent& c, Op op, ErrorCode code) {
    if (op == Op::Initialize) c.fail_next_initialize = code;
    else if (op == Op::Start) c.fail_next_start = code;
    else c.fail_next_stop = code;
}
Result<void> run(Rig& r, Op op) { return op == Op::Initialize ? r.runtime.initialize() : op == Op::Start ? r.runtime.start() : r.runtime.stop(); }
const char* name(Op op) { return op == Op::Initialize ? "initialize" : op == Op::Start ? "start" : "stop"; }

void test_failure_then_reset_then_new_attempt_at_representative_positions() {
    for (Op op : {Op::Initialize, Op::Start, Op::Stop}) {
        Rig probe(8, kBranches);
        const Ids order = op == Op::Stop ? probe.reverse_order() : probe.forward;
        for (std::size_t position : {std::size_t{0}, order.size() / 2, order.size() - 1}) {     // first, middle, last
            Rig r(8, kBranches);
            if (op != Op::Initialize) assert(r.runtime.initialize());
            if (op == Op::Stop) assert(r.runtime.start());
            const std::uint64_t failing = order[position];
            inject(r.c(failing), op, ErrorCode::TIMEOUT);
            r.trace.clear();
            const std::uint64_t samples_before = r.runtime.statistics().sample_count.value();

            const auto result = run(r, op);
            // Original error preserved; fail-fast; FAULT.
            assert(!result && result.error().code == ErrorCode::TIMEOUT && result.error().source == ComponentId{failing});
            assert(result.error().severity == ErrorSeverity::ERROR && result.error().message == std::string(name(op)) + " failed");
            Trace prefix;
            for (std::size_t i = 0; i <= position; ++i) prefix.push_back(std::to_string(order[i]) + ":" + name(op));
            assert(r.trace == prefix);                                       // later components not invoked
            assert(r.runtime.state() == S::FAULT);
            assert(r.runtime.fault_error() != nullptr && r.runtime.fault_error()->source == ComponentId{failing});
            assert(r.runtime.statistics().sample_count.value() == samples_before + position);
            assert(r.runtime.statistics().error_count.value() == 1);
            // Only reset() is accepted; nothing is invoked meanwhile.
            const int calls = r.calls();
            assert(!r.runtime.initialize() && !r.runtime.start() && !r.runtime.stop() && !r.runtime.shutdown());
            assert(r.calls() == calls);

            // reset(): two-pass reverse cleanup.
            r.trace.clear();
            assert(r.runtime.reset());
            std::map<std::uint64_t, int> stage;                              // 0 none,1 initialized,2 started,3 stopped,4 faulted
            for (std::uint64_t id : r.forward) stage[id] = op == Op::Initialize ? 0 : op == Op::Start ? 1 : 2;
            for (std::size_t i = 0; i < position; ++i) stage[order[i]] = op == Op::Initialize ? 1 : op == Op::Start ? 2 : 3;
            stage[failing] = 4;
            Trace cleanup;
            const Ids rev = r.reverse_order();
            for (std::uint64_t id : rev) if (stage[id] == 1 || stage[id] == 2) { cleanup.push_back(std::to_string(id) + ":stop"); stage[id] = 3; }
            for (std::uint64_t id : rev) if (stage[id] == 3 || stage[id] == 4) cleanup.push_back(std::to_string(id) + ":shutdown");
            assert(r.trace == cleanup);
            assert(r.runtime.state() == S::STOPPED && r.runtime.fault_error() == nullptr);
            // The failed operation was attempted exactly once and not retried by reset().
            const ReferenceComponent& f = r.c(failing);
            assert((op == Op::Initialize ? f.initialize_calls : op == Op::Start ? f.start_calls : f.stop_calls) == 1);

            // A subsequent initialize() is a new, explicit attempt over the whole forward order.
            r.trace.clear();
            assert(r.runtime.initialize() && r.runtime.start() && r.runtime.state() == S::RUNNING);
            assert(r.trace == join(expect(r.forward, "initialize"), expect(r.forward, "start")));
            r.expect_statistics_match_calls();
        }
    }
}

void test_failed_reset_preserves_fault_and_resumes_without_repeating_cleanup() {
    Rig r(8, kBranches);
    assert(r.runtime.initialize());
    r.c(1).fail_next_start = ErrorCode::TIMEOUT;                           // forward 8,6,3,7,4,5,1(fails),2
    assert(!r.runtime.start());
    const Error original = *r.runtime.fault_error();

    // Cleanup pass 1 in reverse (2,1,5,4,7,3,6,8): 1 is faulted and is not stopped; 4's stop fails.
    r.c(4).fail_next_stop = ErrorCode::RESOURCE_UNAVAILABLE;
    r.trace.clear();
    const auto first = r.runtime.reset();
    assert(!first && first.error().source == ComponentId{4} && first.error().code == ErrorCode::RESOURCE_UNAVAILABLE);
    assert((r.trace == Trace{"2:stop", "5:stop", "4:stop"}));
    assert(r.runtime.state() == S::FAULT);
    assert(r.runtime.fault_error()->code == original.code && r.runtime.fault_error()->source == original.source
           && r.runtime.fault_error()->message == original.message);              // the original fault, not the cleanup error

    // Pass 1 resumes: 2 and 5 are not stopped again; 4 is now faulted (shut down, not stopped again).
    r.trace.clear();
    assert(r.runtime.reset());
    assert((r.trace == Trace{"7:stop", "3:stop", "6:stop", "8:stop",
                              "2:shutdown", "1:shutdown", "5:shutdown", "4:shutdown", "7:shutdown", "3:shutdown", "6:shutdown", "8:shutdown"}));
    assert(r.runtime.state() == S::STOPPED && r.runtime.fault_error() == nullptr);

    // No component was cleaned up twice, and no failed operation was retried.
    for (std::uint64_t id = 1; id <= 8; ++id) {
        assert(r.c(id).stop_calls == (id == 1 ? 0 : 1));           // 1 faulted at start: shut down, never stopped
        assert(r.c(id).shutdown_calls == 1);
        assert(r.c(id).initialize_calls == 1);
        assert(r.c(id).start_calls == (id == 2 ? 0 : 1));        // 2 follows the failure and was never started; 1's failed start was not retried
    }
    r.expect_statistics_match_calls();
}

void test_repeated_cleanup_is_never_invoked_twice() {
    Rig r(5, {{1, 2}, {2, 3}});
    assert(r.runtime.initialize() && r.runtime.start() && r.runtime.stop());
    r.c(1).fail_next_shutdown = ErrorCode::TIMEOUT;                          // reverse: 5,4,1(fails),2,3 -> order is 3,2,1,4,5
    assert(!r.runtime.shutdown());                                           // 5,4 done; 1 fails
    r.trace.clear();
    assert(r.runtime.shutdown());                                            // resumes at 1: 1,2,3
    assert(r.trace == expect({1, 2, 3}, "shutdown"));
    r.trace.clear();
    assert(r.runtime.shutdown() && r.runtime.shutdown());                    // fully released: nothing to invoke
    assert(r.trace.empty());
    for (std::uint64_t id : {2u, 3u, 4u, 5u}) assert(r.c(id).shutdown_calls == 1);
    assert(r.c(1).shutdown_calls == 2);                                      // only the failed one is attempted again
}

// -----------------------------------------------------------------------------
// AC-07: cross-contract interactions
// -----------------------------------------------------------------------------

void test_topology_freeze_interacts_correctly_with_failure_and_recovery() {
    auto info = ComponentInfo::create(ComponentId{99}, "late");
    assert(info.has_value());
    ReferenceComponent late(std::move(info).value());

    Rig r(4, {{1, 2}, {3, 4}});
    // configure does not freeze; failed validation does not freeze; setup still allowed.
    assert(r.runtime.configure(Configuration{}) && !r.runtime.topology_fixed());
    assert(r.runtime.add_dependency(ComponentId{2}, ComponentId{9}));          // dangling: 9 is not registered
    assert(!r.runtime.initialize());                                          // validation failure, graph error unchanged
    assert(r.runtime.state() == S::UNKNOWN && !r.runtime.topology_fixed());
    assert(r.runtime.registry().size() == 4);
    assert(r.runtime.register_component(late));                               // setup open
    assert(r.runtime.add_dependency(ComponentId{9}, ComponentId{3}) && !r.runtime.initialize());   // 9 still not a registered edge target
    // (9 is not registered; late has id 99.) Fix the dangling edge by registering the real component 9.
    auto info9 = ComponentInfo::create(ComponentId{9}, "nine");
    assert(info9.has_value());
    ReferenceComponent nine(std::move(info9).value());
    nine.trace = &r.trace;
    assert(r.runtime.register_component(nine));
    assert(r.runtime.initialize() && r.runtime.topology_fixed());

    // A fixed topology stays fixed through failure, FAULT and recovery.
    nine.fail_next_start = ErrorCode::TIMEOUT;
    assert(!r.runtime.start() && r.runtime.state() == S::FAULT);
    auto info7 = ComponentInfo::create(ComponentId{7}, "seven");
    ReferenceComponent seven(std::move(info7).value());
    assert(!r.runtime.register_component(seven) && !r.runtime.add_dependency(ComponentId{1}, ComponentId{3}));
    assert(r.runtime.reset() && r.runtime.state() == S::STOPPED);
    const auto reg = r.runtime.register_component(seven);
    assert(!reg && reg.error().code == ErrorCode::INVALID_STATE);
    assert(r.runtime.registry().size() == 6 && !r.runtime.registry().contains(ComponentId{7}));
    assert(r.runtime.initialize() && r.runtime.start() && r.runtime.state() == S::RUNNING);
    assert(r.runtime.stop() && r.runtime.shutdown());
}

void test_statistics_account_for_every_component_call() {
    Rig r(8, kBranches);
    r.c(5).fail_next_initialize = ErrorCode::NOT_READY;
    assert(!r.runtime.initialize());
    r.expect_statistics_match_calls();
    r.c(8).fail_next_shutdown = ErrorCode::TIMEOUT;                          // cleanup failure on the last pass-2 step
    assert(!r.runtime.reset());
    r.expect_statistics_match_calls();
    assert(r.runtime.reset());
    r.expect_statistics_match_calls();
    assert(r.runtime.statistics().error_count.value() == 2);
    assert(r.runtime.initialize() && r.runtime.start() && r.runtime.stop() && r.runtime.shutdown());
    r.expect_statistics_match_calls();
    assert(r.runtime.statistics().error_count.value() == 2);
}

// Records its own destruction.
class LifetimeProbe final : public Component {
public:
    LifetimeProbe(ComponentInfo info, bool* destroyed) : Component(std::move(info)), destroyed_(destroyed) {}
    ~LifetimeProbe() override { *destroyed_ = true; }
    Result<void> configure(const Configuration&) override { return Result<void>::success(); }
    Result<void> initialize() override { return Result<void>::success(); }
    Result<void> start() override { return Result<void>::success(); }
    Result<void> stop() override { return Result<void>::success(); }
    Result<void> shutdown() override { return Result<void>::success(); }
    LifecycleState lifecycle_state() const noexcept override { return LifecycleState::UNKNOWN; }
    Status status() const override { return Status{}; }
    Health health() const override { return Health{}; }
    CapabilitySet capabilities() const override { return CapabilitySet{}; }
private:
    bool* destroyed_;
};

void test_runtime_never_owns_or_destroys_components() {
    bool a_gone = false, b_gone = false;
    auto ia = ComponentInfo::create(ComponentId{1}, "a");
    auto ib = ComponentInfo::create(ComponentId{2}, "b");
    auto a = std::make_unique<LifetimeProbe>(std::move(ia).value(), &a_gone);
    auto b = std::make_unique<LifetimeProbe>(std::move(ib).value(), &b_gone);
    {
        RuntimeManager runtime;
        assert(runtime.register_component(*a) && runtime.register_component(*b));
        assert(runtime.add_dependency(ComponentId{2}, ComponentId{1}));
        assert(runtime.initialize() && runtime.start());                       // destroyed while RUNNING
    }
    assert(!a_gone && !b_gone);
    assert(a->info().id() == ComponentId{1});                                  // still valid and owned by the caller
    a.reset();
    assert(a_gone && !b_gone);
    b.reset();
    assert(b_gone);

    // The owner may also destroy a runtime that is in FAULT; components stay valid.
    Rig r(3, {{1, 2}});
    r.c(2).fail_next_initialize = ErrorCode::TIMEOUT;
    {
        RuntimeManager local;
        assert(local.register_component(r.c(1)) && local.register_component(r.c(2)) && local.add_dependency(ComponentId{1}, ComponentId{2}));
        assert(!local.initialize() && local.state() == S::FAULT);
    }
    assert(r.c(2).lifecycle_state() == S::FAULT && r.c(2).initialize_calls == 1);
}

// -----------------------------------------------------------------------------
// AC-08: no health / logging / background side effects
// -----------------------------------------------------------------------------

void test_health_does_not_drive_the_runtime() {
    Rig r(6, {{1, 2}, {2, 3}, {4, 3}});
    assert(r.runtime.initialize() && r.runtime.start());
    // A faulted component reports UNHEALTHY through its own contract only after a failure; the
    // runtime state is changed only by returned errors, never by what components report.
    for (int i = 0; i < 1000; ++i) assert(r.runtime.state() == S::RUNNING);
    const int calls = r.calls();
    assert(r.runtime.fault_error() == nullptr && r.calls() == calls);
    r.c(3).fail_next_stop = ErrorCode::TIMEOUT;
    assert(!r.runtime.stop());
    assert(r.c(3).health().state() == HealthState::UNHEALTHY);                 // component health reflects its own fault
    const int after = r.calls();
    for (int i = 0; i < 1000; ++i) assert(r.runtime.state() == S::FAULT);
    assert(r.calls() == after);                                                // nothing recovers by itself
    assert(r.runtime.statistics().retry_count.value() == 0);
}

// -----------------------------------------------------------------------------
// Model-based test: random topologies, random operations, random failures, compared with
// an independent model of the frozen contract at every step.
// -----------------------------------------------------------------------------

struct Model {
    enum class St { None, Init, Started, Stopped, Faulted, Down };
    struct CompModel { S state{S::UNKNOWN}; };

    std::size_t n;
    Ids forward;
    S state{S::UNKNOWN};
    bool fixed{false};
    bool live{false};
    std::map<std::uint64_t, St> stage;
    std::map<std::uint64_t, S> comp;
    bool has_fault{false};
    ErrorCode fault_code{ErrorCode::NONE};
    std::uint64_t fault_source{0};
    std::uint64_t samples{0}, errors{0};
    Trace trace;

    // Injected failure for the current runtime operation: component -> (op name, code).
    struct Injection { std::uint64_t id{0}; std::string op; ErrorCode code{ErrorCode::NONE}; };
    Injection inj;

    Model(std::size_t count, Ids order) : n(count), forward(std::move(order)) {
        for (std::uint64_t id = 1; id <= n; ++id) { stage[id] = St::None; comp[id] = S::UNKNOWN; }
    }

    struct Out { bool ok; ErrorCode code; std::uint64_t source; };

    // The reference component's own contract (tests/contract/reference_component.hpp).
    Out call(std::uint64_t id, const std::string& op) {
        trace.push_back(std::to_string(id) + ":" + op);
        const bool injected = inj.id == id && inj.op == op && inj.code != ErrorCode::NONE;
        S& cs = comp[id];
        auto bad = [&] { ++errors; return Out{false, ErrorCode::INVALID_STATE, id}; };
        auto failed = [&] { ++errors; return Out{false, inj.code, id}; };
        if (op == "configure") {
            if (cs != S::UNKNOWN && cs != S::STOPPED) return bad();
            if (injected) return failed();
        } else if (op == "initialize") {
            if (cs != S::UNKNOWN && cs != S::STOPPED) return bad();
            if (injected) { cs = S::FAULT; return failed(); }
            cs = S::READY;
        } else if (op == "start") {
            if (cs != S::READY) return bad();
            if (injected) { cs = S::FAULT; return failed(); }
            cs = S::RUNNING;
        } else if (op == "stop") {
            if (cs != S::READY && cs != S::RUNNING) return bad();
            if (injected) { cs = S::FAULT; return failed(); }
            cs = S::STOPPED;
        } else {   // shutdown
            if (cs != S::UNKNOWN && cs != S::STOPPED && cs != S::FAULT) return bad();
            if (injected) return failed();
            if (cs == S::FAULT) cs = S::STOPPED;
        }
        ++samples;
        return Out{true, ErrorCode::NONE, 0};
    }

    Out invalid() { return Out{false, ErrorCode::INVALID_STATE, 0}; }
    Out ok() { return Out{true, ErrorCode::NONE, 0}; }
    void enter_fault(const Out& o) { state = S::FAULT; has_fault = true; fault_code = o.code; fault_source = o.source; }

    Out sequence(const char* op, bool reverse_order, St success_stage, bool track_stage) {
        Ids order = reverse_order ? Ids(forward.rbegin(), forward.rend()) : forward;
        for (std::uint64_t id : order) {
            const Out o = call(id, op);
            if (!o.ok) { if (track_stage) stage[id] = St::Faulted; return o; }
            if (track_stage) stage[id] = success_stage;
        }
        return ok();
    }

    Out configure() {
        if (state != S::UNKNOWN && state != S::STOPPED) return invalid();
        return sequence("configure", false, St::None, false);
    }
    Out initialize() {
        if (state != S::UNKNOWN && state != S::STOPPED) return invalid();
        fixed = true; live = true;
        for (auto& kv : stage) kv.second = St::None;
        const Out o = sequence("initialize", false, St::Init, true);
        if (!o.ok) { enter_fault(o); return o; }
        state = S::READY;
        return o;
    }
    Out start() {
        if (state != S::READY) return invalid();
        const Out o = sequence("start", false, St::Started, true);
        if (!o.ok) { enter_fault(o); return o; }
        state = S::RUNNING;
        return o;
    }
    Out stop() {
        if (state != S::READY && state != S::RUNNING) return invalid();
        const Out o = sequence("stop", true, St::Stopped, true);
        if (!o.ok) { enter_fault(o); return o; }
        state = S::STOPPED;
        return o;
    }
    Out shutdown() {
        if (state != S::UNKNOWN && state != S::STOPPED) return invalid();
        if (!live) return ok();
        for (auto it = forward.rbegin(); it != forward.rend(); ++it) {
            if (stage[*it] == St::Down || stage[*it] == St::None) continue;
            const Out o = call(*it, "shutdown");
            if (!o.ok) return o;
            stage[*it] = St::Down;
        }
        live = false;
        return ok();
    }
    Out reset() {
        if (state != S::FAULT) return invalid();
        for (auto it = forward.rbegin(); it != forward.rend(); ++it) {
            if (stage[*it] != St::Init && stage[*it] != St::Started) continue;
            const Out o = call(*it, "stop");
            if (!o.ok) { stage[*it] = St::Faulted; return o; }
            stage[*it] = St::Stopped;
        }
        for (auto it = forward.rbegin(); it != forward.rend(); ++it) {
            if (stage[*it] != St::Stopped && stage[*it] != St::Faulted) continue;
            const Out o = call(*it, "shutdown");
            if (!o.ok) return o;
            stage[*it] = St::Down;
        }
        state = S::STOPPED; live = false; has_fault = false; fault_code = ErrorCode::NONE; fault_source = 0;
        return ok();
    }
};

void test_model_based_random_scenarios() {
    std::mt19937 rng(20261004);
    const std::vector<ErrorCode> codes = {ErrorCode::TIMEOUT, ErrorCode::NOT_READY, ErrorCode::RESOURCE_UNAVAILABLE, ErrorCode::INTERNAL_ERROR};
    std::uint64_t operations = 0, failures = 0, resets = 0;

    for (int scenario = 0; scenario < 400; ++scenario) {
        const std::size_t n = 1 + rng() % 7;
        // Random DAG: hidden rank guarantees acyclic edges.
        Ids rank = Rig::iota(n);
        std::shuffle(rank.begin(), rank.end(), rng);
        std::vector<Edge> edges;
        for (std::size_t a = 0; a < n; ++a)
            for (std::size_t b = 0; b < n; ++b)
                if (rank[a] > rank[b] && rng() % 3 == 0) edges.push_back({a + 1, b + 1});
        Ids registration = Rig::iota(n);
        std::shuffle(registration.begin(), registration.end(), rng);
        std::vector<Edge> edge_order = edges;
        std::shuffle(edge_order.begin(), edge_order.end(), rng);

        Rig rig(n, edges, registration, edge_order);
        Model model(n, rig.forward);

        for (int step = 0; step < 40; ++step) {
            const int op = static_cast<int>(rng() % 6);
            static const char* names[] = {"configure", "initialize", "start", "stop", "shutdown", "reset"};
            const std::string name = names[op];

            // Random one-shot failure on a random component for operations that can be injected.
            rig.clear_injections();
            model.inj = Model::Injection{};
            if (rng() % 4 == 0) {
                const std::uint64_t target = 1 + rng() % n;
                std::string inj_op;
                if (op == 0) inj_op = "configure";
                else if (op == 1) inj_op = "initialize";
                else if (op == 2) inj_op = "start";
                else if (op == 3) inj_op = "stop";
                else if (op == 4) inj_op = "shutdown";
                else inj_op = (rng() % 2 == 0) ? "stop" : "shutdown";
                const ErrorCode code = codes[rng() % codes.size()];
                ReferenceComponent& c = rig.c(target);
                if (inj_op == "configure") c.fail_next_configure = code;
                else if (inj_op == "initialize") c.fail_next_initialize = code;
                else if (inj_op == "start") c.fail_next_start = code;
                else if (inj_op == "stop") c.fail_next_stop = code;
                else c.fail_next_shutdown = code;
                model.inj = Model::Injection{target, inj_op, code};
            }

            rig.trace.clear();
            model.trace.clear();
            Result<void> actual = Result<void>::success();
            Model::Out expected{true, ErrorCode::NONE, 0};
            switch (op) {
                case 0: actual = rig.runtime.configure(Configuration{}); expected = model.configure(); break;
                case 1: actual = rig.runtime.initialize(); expected = model.initialize(); break;
                case 2: actual = rig.runtime.start(); expected = model.start(); break;
                case 3: actual = rig.runtime.stop(); expected = model.stop(); break;
                case 4: actual = rig.runtime.shutdown(); expected = model.shutdown(); break;
                default: actual = rig.runtime.reset(); expected = model.reset(); ++resets; break;
            }
            ++operations;

            // Result.
            assert(actual.has_value() == expected.ok);
            if (!expected.ok) {
                ++failures;
                assert(actual.error().code == expected.code);
                assert(actual.error().source == ComponentId{expected.source});   // 0 = no component source
            }
            // Exact component invocation trace, state, topology, fault and statistics.
            assert(rig.trace == model.trace);
            assert(rig.runtime.state() == model.state);
            assert(rig.runtime.topology_fixed() == model.fixed);
            assert((rig.runtime.fault_error() != nullptr) == model.has_fault);
            if (model.has_fault) {
                assert(rig.runtime.fault_error()->code == model.fault_code);
                assert(rig.runtime.fault_error()->source == ComponentId{model.fault_source});
            }
            assert(rig.runtime.statistics().sample_count.value() == model.samples);
            assert(rig.runtime.statistics().error_count.value() == model.errors);
            for (std::uint64_t id = 1; id <= n; ++id) assert(rig.c(id).lifecycle_state() == model.comp[id]);
            rig.expect_statistics_match_calls();
        }
    }
    // The scenario generator really exercised failures and recovery.
    assert(operations == 400 * 40 && failures > 2000 && resets > 1000);
}

} // namespace

int main() {
    test_full_lifecycle_across_two_live_periods();
    test_branch_shapes_order_forward_and_reverse();
    test_registration_and_edge_order_permutations_are_unobservable();
    test_invalid_operations_do_not_disturb_a_lifecycle();
    test_failure_then_reset_then_new_attempt_at_representative_positions();
    test_failed_reset_preserves_fault_and_resumes_without_repeating_cleanup();
    test_repeated_cleanup_is_never_invoked_twice();
    test_topology_freeze_interacts_correctly_with_failure_and_recovery();
    test_statistics_account_for_every_component_call();
    test_runtime_never_owns_or_destroys_components();
    test_health_does_not_drive_the_runtime();
    test_model_based_random_scenarios();
    return 0;
}
