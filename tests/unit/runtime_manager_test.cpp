//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : runtime_manager_test.cpp
// Description : RuntimeManager contract tests.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-RT-002
// API         : CORE-TEST-RUNTIME-MANAGER
//
// Author      : KritvaOS Core Team
// Created     : 03-10-2026
//==============================================================================

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include <kritva/core/core.hpp>

#include "../contract/reference_component.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;
using kritva::core::runtime::contract::ReferenceComponent;

namespace {

using Ids = std::vector<std::uint64_t>;

// Reference component that reports its own destruction.
class ProbeComponent final : public Component {
public:
    ProbeComponent(ComponentInfo info, bool* destroyed) : Component(std::move(info)), destroyed_(destroyed) {}
    ~ProbeComponent() override { if (destroyed_) *destroyed_ = true; }
    Result<void> configure(const Configuration&) override { ++calls; return Result<void>::success(); }
    Result<void> initialize() override { ++calls; return Result<void>::success(); }
    Result<void> start() override { ++calls; return Result<void>::success(); }
    Result<void> stop() override { ++calls; return Result<void>::success(); }
    Result<void> shutdown() override { ++calls; return Result<void>::success(); }
    LifecycleState lifecycle_state() const noexcept override { return LifecycleState::UNKNOWN; }
    Status status() const override { return Status{}; }
    Health health() const override { return Health{}; }
    CapabilitySet capabilities() const override { return CapabilitySet{}; }
    int calls{0};
private:
    bool* destroyed_;
};

std::unique_ptr<ReferenceComponent> make_component(std::uint64_t id) {
    auto info = ComponentInfo::create(ComponentId{id}, "c");
    assert(info.has_value());
    return std::make_unique<ReferenceComponent>(std::move(info).value());
}

Ids order_of(const RuntimeManager& runtime) {
    const auto r = runtime.component_order();
    assert(r.has_value());
    Ids ids;
    for (ComponentId id : r.value()) ids.push_back(id.value());
    return ids;
}

int lifecycle_calls(const std::vector<std::unique_ptr<ReferenceComponent>>& owned) {
    int n = 0;
    for (const auto& c : owned)
        n += c->configure_calls + c->initialize_calls + c->start_calls + c->stop_calls + c->shutdown_calls;
    return n;
}

// -----------------------------------------------------------------------------
// Runtime interface conformance (AC-01)
// -----------------------------------------------------------------------------

void test_implements_the_existing_runtime_interface() {
    static_assert(std::is_base_of_v<Runtime, RuntimeManager>);
    static_assert(std::is_final_v<RuntimeManager>);
    static_assert(!std::is_abstract_v<RuntimeManager>);
    static_assert(!std::is_copy_constructible_v<RuntimeManager>);
    static_assert(!std::is_copy_assignable_v<RuntimeManager>);
    static_assert(!std::is_move_constructible_v<RuntimeManager>);
    static_assert(!std::is_move_assignable_v<RuntimeManager>);
    static_assert(noexcept(std::declval<const RuntimeManager&>().state()));

    // It is used through the unchanged Runtime interface.
    RuntimeManager manager;
    Runtime& runtime = manager;
    assert(runtime.state() == LifecycleState::UNKNOWN);               // initial state
    assert(runtime.initialize());
    assert(runtime.state() == LifecycleState::READY);
    assert(runtime.start());
    assert(runtime.state() == LifecycleState::RUNNING);
    assert(runtime.stop());
    assert(runtime.state() == LifecycleState::STOPPED);
    assert(runtime.shutdown());
    assert(runtime.state() == LifecycleState::STOPPED);

    // Deleting through the interface pointer is well defined.
    std::unique_ptr<Runtime> owned = std::make_unique<RuntimeManager>();
    assert(owned->state() == LifecycleState::UNKNOWN);
}

void test_runtime_operation_matrix() {
    using S = LifecycleState;
    auto in_state = [](S target) {
        auto m = std::make_unique<RuntimeManager>();
        if (target == S::READY || target == S::RUNNING || target == S::STOPPED) assert(m->initialize());
        if (target == S::RUNNING) assert(m->start());
        if (target == S::STOPPED) assert(m->stop());
        assert(m->state() == target);
        return m;
    };
    auto invalid = [](Runtime& r, const Result<void>& result, S before) {
        assert(!result.has_value());
        assert(result.error().code == ErrorCode::INVALID_STATE);
        assert(!result.error().message.empty());
        assert(r.state() == before);                                     // no effect
    };

    for (S s : {S::UNKNOWN, S::READY, S::RUNNING, S::STOPPED}) {
        {   // initialize: valid from UNKNOWN and STOPPED only
            auto m = in_state(s);
            const auto r = m->initialize();
            if (s == S::UNKNOWN || s == S::STOPPED) { assert(r && m->state() == S::READY); }
            else invalid(*m, r, s);
        }
        {   // start: valid from READY only
            auto m = in_state(s);
            const auto r = m->start();
            if (s == S::READY) { assert(r && m->state() == S::RUNNING); }
            else invalid(*m, r, s);
        }
        {   // stop: valid from READY and RUNNING
            auto m = in_state(s);
            const auto r = m->stop();
            if (s == S::READY || s == S::RUNNING) { assert(r && m->state() == S::STOPPED); }
            else invalid(*m, r, s);
        }
        {   // shutdown: idempotent no-op in UNKNOWN and STOPPED, invalid in READY/RUNNING
            auto m = in_state(s);
            const auto r = m->shutdown();
            if (s == S::UNKNOWN || s == S::STOPPED) { assert(r && m->state() == s); }
            else invalid(*m, r, s);
        }
    }

    // Re-initialization after stop works and the topology stays fixed.
    RuntimeManager m;
    assert(m.initialize() && m.stop() && m.initialize() && m.state() == S::READY && m.topology_fixed());
}

// -----------------------------------------------------------------------------
// Composition, ownership, fixed topology (AC-02, AC-03)
// -----------------------------------------------------------------------------

void test_composes_registry_and_graph_and_forwards_errors_unchanged() {
    auto a = make_component(1);
    auto b = make_component(2);
    auto dup = make_component(1);
    RuntimeManager runtime;
    assert(runtime.register_component(*a) && runtime.register_component(*b));
    assert(runtime.registry().size() == 2);
    assert(runtime.registry().find(ComponentId{1}) == a.get());

    // Registry error: identical to ComponentRegistry's own error.
    ComponentRegistry reference;
    assert(reference.register_component(*a));
    const auto expected = reference.register_component(*dup);
    const auto got = runtime.register_component(*dup);
    assert(!got && !expected);
    assert(got.error().code == expected.error().code);
    assert(got.error().source == expected.error().source);
    assert(got.error().message == expected.error().message);
    assert(runtime.registry().size() == 2);                              // unchanged

    // Graph errors: self, duplicate, cycle, invalid id, each as DependencyGraph reports them.
    assert(runtime.add_dependency(ComponentId{1}, ComponentId{2}));
    const auto self = runtime.add_dependency(ComponentId{2}, ComponentId{2});
    assert(!self && self.error().code == ErrorCode::INVALID_ARGUMENT && self.error().source == ComponentId{2});
    const auto twice = runtime.add_dependency(ComponentId{1}, ComponentId{2});
    assert(!twice && twice.error().code == ErrorCode::INVALID_ARGUMENT);
    const auto cycle = runtime.add_dependency(ComponentId{2}, ComponentId{1});
    assert(!cycle && cycle.error().message.find("cycle: 2 -> 1 -> 2") != std::string::npos);
    assert(!runtime.add_dependency(ComponentId{}, ComponentId{1}));
    assert(runtime.dependencies().size() == 1);                          // unchanged by every failure
    assert(runtime.state() == LifecycleState::UNKNOWN);                  // setup never starts anything
}

void test_topology_is_fixed_by_initialize() {
    auto a = make_component(1);
    auto b = make_component(2);
    auto late = make_component(3);
    RuntimeManager runtime;
    assert(!runtime.topology_fixed());
    assert(runtime.register_component(*a) && runtime.register_component(*b));
    assert(runtime.add_dependency(ComponentId{1}, ComponentId{2}));
    assert(runtime.initialize());
    assert(runtime.topology_fixed());

    for (int round = 0; round < 3; ++round) {                            // READY, RUNNING, STOPPED, ...
        const auto reg = runtime.register_component(*late);
        assert(!reg && reg.error().code == ErrorCode::INVALID_STATE);
        const auto edge = runtime.add_dependency(ComponentId{2}, ComponentId{1});
        assert(!edge && edge.error().code == ErrorCode::INVALID_STATE);
        assert(runtime.registry().size() == 2 && runtime.dependencies().size() == 1);
        assert(!runtime.registry().contains(ComponentId{3}));
        if (round == 0) assert(runtime.start());
        if (round == 1) assert(runtime.stop() && runtime.initialize());  // still fixed after re-initialize
    }
    assert((order_of(runtime) == Ids{2, 1}));                            // unchanged

    // The same applies to an empty topology: initialize() fixes it too.
    RuntimeManager empty;
    assert(empty.initialize());
    assert(!empty.register_component(*late));
}

void test_runtime_does_not_own_components() {
    bool destroyed_a = false, destroyed_b = false;
    auto info_a = ComponentInfo::create(ComponentId{1}, "a");
    auto info_b = ComponentInfo::create(ComponentId{2}, "b");
    assert(info_a && info_b);
    auto a = std::make_unique<ProbeComponent>(std::move(info_a).value(), &destroyed_a);
    auto b = std::make_unique<ProbeComponent>(std::move(info_b).value(), &destroyed_b);
    {
        RuntimeManager runtime;
        assert(runtime.register_component(*a) && runtime.register_component(*b));
        assert(runtime.add_dependency(ComponentId{1}, ComponentId{2}));
        assert(runtime.initialize() && runtime.start() && runtime.stop() && runtime.shutdown());
    }                                                                    // runtime destroyed
    assert(!destroyed_a && !destroyed_b);                                // never deleted
    assert(a->info().id() == ComponentId{1} && b->info().id() == ComponentId{2});   // still usable
    assert(a->calls == 0 && b->calls == 0);                              // and never driven (R03-005)
    a.reset();
    assert(destroyed_a && !destroyed_b);                                 // the owner decides
    b.reset();
    assert(destroyed_b);
}

// The read-only views cannot be used to get around the topology-freeze rule: the
// mutating operations are not callable through them, before or after the freeze.
template<class T> concept CanRegister = requires(T& registry, Component& c) { registry.register_component(c); };
template<class T> concept CanAddEdge = requires(T& graph) { graph.add_dependency(ComponentId{1}, ComponentId{2}); };

void test_views_cannot_bypass_the_topology_freeze() {
    using RegistryView = std::remove_reference_t<decltype(std::declval<RuntimeManager&>().registry())>;
    using GraphView = std::remove_reference_t<decltype(std::declval<RuntimeManager&>().dependencies())>;
    static_assert(std::is_const_v<RegistryView> && std::is_const_v<GraphView>);
    static_assert(!CanRegister<RegistryView>);       // register_component is non-const
    static_assert(!CanAddEdge<GraphView>);           // add_dependency is non-const
    static_assert(CanRegister<ComponentRegistry>);   // (the concepts do detect the operations)
    static_assert(CanAddEdge<DependencyGraph>);

    auto a = make_component(1);
    auto b = make_component(2);
    auto late = make_component(3);
    RuntimeManager runtime;
    assert(runtime.register_component(*a) && runtime.register_component(*b));
    assert(runtime.add_dependency(ComponentId{1}, ComponentId{2}));
    assert(runtime.initialize());

    // After the freeze both setup APIs are rejected and the underlying state is untouched.
    const std::size_t components = runtime.registry().size();
    const std::size_t edges = runtime.dependencies().size();
    const Ids order = order_of(runtime);
    const auto reg = runtime.register_component(*late);
    const auto edge = runtime.add_dependency(ComponentId{2}, ComponentId{3});
    assert(!reg && reg.error().code == ErrorCode::INVALID_STATE);
    assert(!edge && edge.error().code == ErrorCode::INVALID_STATE);
    assert(runtime.registry().size() == components && runtime.dependencies().size() == edges);
    assert(!runtime.registry().contains(ComponentId{3}));
    assert(runtime.dependencies().dependencies_of(ComponentId{2}).empty());
    assert(order_of(runtime) == order);
}

// -----------------------------------------------------------------------------
// Topology validation and order (AC-04, AC-05)
// -----------------------------------------------------------------------------

void test_valid_topology_is_accepted_and_ordered() {
    std::vector<std::unique_ptr<ReferenceComponent>> owned;
    RuntimeManager runtime;
    for (std::uint64_t id : {3u, 1u, 2u}) { owned.push_back(make_component(id)); assert(runtime.register_component(*owned.back())); }
    assert(runtime.add_dependency(ComponentId{1}, ComponentId{3}));       // 1 depends on 3
    assert(runtime.add_dependency(ComponentId{2}, ComponentId{3}));       // 2 depends on 3
    assert((order_of(runtime) == Ids{3, 1, 2}));                          // dependencies first, lowest id tie-break
    assert(runtime.initialize());
    assert((order_of(runtime) == Ids{3, 1, 2}));                          // same after fixing

    // Equal to the DependencyGraph/registry result: no second ordering algorithm.
    assert((runtime.dependencies().order(runtime.registry()).value() == runtime.component_order().value()));

    RuntimeManager empty;                                                 // an empty topology is valid
    assert(empty.component_order().value().empty());
    assert(empty.initialize() && empty.state() == LifecycleState::READY);
}

void test_missing_dependency_is_rejected_and_leaves_runtime_not_running() {
    auto a = make_component(1);
    auto c = make_component(3);
    RuntimeManager runtime;
    assert(runtime.register_component(*a));
    assert(runtime.add_dependency(ComponentId{1}, ComponentId{2}));       // 2 is never registered

    const auto r = runtime.initialize();
    assert(!r.has_value());
    assert(r.error().code == ErrorCode::CONFIGURATION_ERROR);             // the graph's own error
    assert(r.error().source == ComponentId{1});
    assert(r.error().message.find("2") != std::string::npos);
    // Identical to what the graph reports directly: not swallowed, not rewrapped.
    const auto direct = runtime.dependencies().order(runtime.registry());
    assert(!direct && r.error().code == direct.error().code &&
           r.error().source == direct.error().source && r.error().message == direct.error().message);

    // Documented non-running state: still UNKNOWN, topology still open, nothing started.
    assert(runtime.state() == LifecycleState::UNKNOWN);
    assert(!runtime.topology_fixed());
    assert(!runtime.start() && runtime.state() == LifecycleState::UNKNOWN);
    assert(!runtime.stop());

    // The caller fixes the topology and initializes again.
    auto b = make_component(2);
    assert(runtime.register_component(*b));
    assert(runtime.initialize());
    assert(runtime.state() == LifecycleState::READY && runtime.topology_fixed());
    (void)c;
}

void test_unregistered_dependent_is_rejected() {
    auto b = make_component(2);
    RuntimeManager runtime;
    assert(runtime.register_component(*b));
    assert(runtime.add_dependency(ComponentId{5}, ComponentId{2}));       // 5 (the dependent) is unregistered
    const auto r = runtime.initialize();
    assert(!r && r.error().code == ErrorCode::CONFIGURATION_ERROR && r.error().source == ComponentId{5});
    assert(runtime.state() == LifecycleState::UNKNOWN && !runtime.topology_fixed());
}

void test_order_is_independent_of_registration_and_edge_insertion_order() {
    const std::vector<std::pair<std::uint64_t, std::uint64_t>> edges = {{1, 3}, {2, 3}, {3, 4}, {5, 2}};
    Ids expected;
    {
        std::vector<std::unique_ptr<ReferenceComponent>> owned;
        RuntimeManager runtime;
        for (std::uint64_t id : {1u, 2u, 3u, 4u, 5u}) { owned.push_back(make_component(id)); assert(runtime.register_component(*owned.back())); }
        for (const auto& e : edges) assert(runtime.add_dependency(ComponentId{e.first}, ComponentId{e.second}));
        expected = order_of(runtime);
        assert((expected == Ids{4, 3, 1, 2, 5}));
    }
    auto perm = edges;
    std::sort(perm.begin(), perm.end());
    Ids ids = {1, 2, 3, 4, 5};
    int checked = 0;
    do {
        std::next_permutation(ids.begin(), ids.end());                    // vary registration order too
        std::vector<std::unique_ptr<ReferenceComponent>> owned;
        RuntimeManager runtime;
        for (std::uint64_t id : ids) { owned.push_back(make_component(id)); assert(runtime.register_component(*owned.back())); }
        for (const auto& e : perm) assert(runtime.add_dependency(ComponentId{e.first}, ComponentId{e.second}));
        assert(order_of(runtime) == expected);
        assert(runtime.initialize());
        assert(order_of(runtime) == expected);
        ++checked;
    } while (std::next_permutation(perm.begin(), perm.end()));
    assert(checked == 24);
}

// -----------------------------------------------------------------------------
// Lifecycle boundary (AC-06): no component is ever called by R03-004
// -----------------------------------------------------------------------------

void test_runtime_never_drives_components() {
    std::vector<std::unique_ptr<ReferenceComponent>> owned;
    RuntimeManager runtime;
    for (std::uint64_t id = 1; id <= 4; ++id) { owned.push_back(make_component(id)); assert(runtime.register_component(*owned.back())); }
    assert(runtime.add_dependency(ComponentId{1}, ComponentId{2}) && runtime.add_dependency(ComponentId{3}, ComponentId{2}));
    assert(runtime.initialize() && runtime.start());
    assert(runtime.stop() && runtime.initialize() && runtime.start() && runtime.stop() && runtime.shutdown());
    assert(!runtime.start());                                            // an invalid call, too
    assert(lifecycle_calls(owned) == 0);
    for (const auto& c : owned) assert(c->lifecycle_state() == LifecycleState::UNKNOWN);
}

} // namespace

int main() {
    test_implements_the_existing_runtime_interface();
    test_runtime_operation_matrix();
    test_composes_registry_and_graph_and_forwards_errors_unchanged();
    test_topology_is_fixed_by_initialize();
    test_runtime_does_not_own_components();
    test_views_cannot_bypass_the_topology_freeze();
    test_valid_topology_is_accepted_and_ordered();
    test_missing_dependency_is_rejected_and_leaves_runtime_not_running();
    test_unregistered_dependent_is_rejected();
    test_order_is_independent_of_registration_and_edge_insertion_order();
    test_runtime_never_drives_components();
    return 0;
}
