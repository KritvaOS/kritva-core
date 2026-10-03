//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : dependency_graph_test.cpp
// Description : DependencyGraph contract tests.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-RT-004, CORE-RT-005
// API         : CORE-TEST-DEPENDENCY-GRAPH
//
// Author      : KritvaOS Core Team
// Created     : 03-10-2026
//==============================================================================

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <map>
#include <memory>
#include <numeric>
#include <random>
#include <set>
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

// Owns reference components for the duration of a test and registers them.
struct World {
    ComponentRegistry registry;
    std::vector<std::unique_ptr<ReferenceComponent>> owned;

    explicit World(const Ids& ids) {
        for (std::uint64_t id : ids) {
            auto info = ComponentInfo::create(ComponentId{id}, "c");
            assert(info.has_value());
            owned.push_back(std::make_unique<ReferenceComponent>(std::move(info).value()));
            const auto r = registry.register_component(*owned.back());
            assert(r.has_value());
            (void)r;
        }
    }
    int lifecycle_calls() const {
        int n = 0;
        for (const auto& c : owned) n += c->configure_calls + c->initialize_calls + c->start_calls + c->stop_calls + c->shutdown_calls;
        return n;
    }
};

Result<void> dep(DependencyGraph& g, std::uint64_t dependent, std::uint64_t dependency) {
    return g.add_dependency(ComponentId{dependent}, ComponentId{dependency});
}

Ids order_of(const DependencyGraph& g, const ComponentRegistry& r) {
    const auto result = g.order(r);
    assert(result.has_value());
    Ids ids;
    for (ComponentId id : result.value()) ids.push_back(id.value());
    return ids;
}

// -----------------------------------------------------------------------------
// Representation (AC-003-01)
// -----------------------------------------------------------------------------

void test_representation_uses_component_ids_only() {
    static_assert(std::is_copy_constructible_v<DependencyGraph>);   // plain data of ids
    static_assert(std::is_same_v<decltype(std::declval<const DependencyGraph&>().dependencies_of(ComponentId{})),
                                 std::vector<ComponentId>>);
    static_assert(std::is_same_v<decltype(std::declval<DependencyGraph&>().add_dependency(ComponentId{}, ComponentId{})),
                                 Result<void>>);

    DependencyGraph g;
    assert(g.empty() && g.size() == 0);
    assert(dep(g, 1, 2) && dep(g, 1, 3) && dep(g, 2, 3));
    assert(g.size() == 3 && !g.empty());
    assert((g.dependencies_of(ComponentId{1}) == std::vector<ComponentId>{ComponentId{2}, ComponentId{3}}));
    assert((g.dependencies_of(ComponentId{2}) == std::vector<ComponentId>{ComponentId{3}}));
    assert(g.dependencies_of(ComponentId{3}).empty());          // no dependencies
    assert(g.dependencies_of(ComponentId{99}).empty());         // unknown id

    // Insertion order does not change the (ascending) representation.
    DependencyGraph h;
    assert(dep(h, 1, 3) && dep(h, 2, 3) && dep(h, 1, 2));
    assert(h.dependencies_of(ComponentId{1}) == g.dependencies_of(ComponentId{1}));

    // Value semantics: a copy is independent.
    DependencyGraph copy = g;
    assert(dep(copy, 4, 1));
    assert(copy.size() == 4 && g.size() == 3);
}

void test_invalid_ids_are_rejected() {
    DependencyGraph g;
    const Result<void> r1 = g.add_dependency(ComponentId{}, ComponentId{2});
    assert(!r1 && r1.error().code == ErrorCode::INVALID_ARGUMENT);
    const Result<void> r2 = g.add_dependency(ComponentId{1}, ComponentId{});
    assert(!r2 && r2.error().code == ErrorCode::INVALID_ARGUMENT && r2.error().source == ComponentId{1});
    assert(!g.add_dependency(ComponentId{}, ComponentId{}));
    assert(g.empty());
}

// -----------------------------------------------------------------------------
// Self, duplicate (AC-003-03, AC-003-04)
// -----------------------------------------------------------------------------

void test_self_dependency_is_rejected() {
    DependencyGraph g;
    assert(dep(g, 1, 2));
    const Result<void> r = dep(g, 3, 3);
    assert(!r);
    assert(r.error().code == ErrorCode::INVALID_ARGUMENT);
    assert(r.error().source == ComponentId{3});
    assert(r.error().message.find("3") != std::string::npos);
    assert(g.size() == 1);                                       // unchanged
    assert(g.dependencies_of(ComponentId{3}).empty());
    assert(dep(g, 3, 2));                                         // still usable
}

void test_duplicate_dependency_is_rejected_not_merged() {
    DependencyGraph g;
    assert(dep(g, 1, 2));
    const Result<void> r = dep(g, 1, 2);
    assert(!r);
    assert(r.error().code == ErrorCode::INVALID_ARGUMENT);
    assert(r.error().source == ComponentId{1});
    assert(g.size() == 1);                                       // no duplicate edge exists
    assert((g.dependencies_of(ComponentId{1}) == std::vector<ComponentId>{ComponentId{2}}));
    const Result<void> again = dep(g, 1, 2);                      // deterministic
    assert(!again && again.error().message == r.error().message);
    assert(dep(g, 1, 3));                                         // other edges unaffected
}

// -----------------------------------------------------------------------------
// Cycles (AC-003-05)
// -----------------------------------------------------------------------------

void test_cycles_are_rejected_and_identified() {
    DependencyGraph g;
    assert(dep(g, 1, 2) && dep(g, 2, 3));
    const Result<void> r = dep(g, 3, 1);                          // A -> B -> C -> A
    assert(!r);
    assert(r.error().code == ErrorCode::INVALID_ARGUMENT);
    assert(r.error().source == ComponentId{3});
    assert(r.error().message.find("cycle: 3 -> 1 -> 2 -> 3") != std::string::npos);
    assert(g.size() == 2);                                       // graph unchanged
    assert(g.dependencies_of(ComponentId{3}).empty());

    // Two-node cycle.
    DependencyGraph two;
    assert(dep(two, 1, 2));
    const Result<void> back = dep(two, 2, 1);
    assert(!back && back.error().message.find("cycle: 2 -> 1 -> 2") != std::string::npos);

    // Longer cycle with a side branch; the reported path is deterministic.
    DependencyGraph g4;
    assert(dep(g4, 1, 2) && dep(g4, 2, 3) && dep(g4, 2, 5) && dep(g4, 3, 4) && dep(g4, 5, 4));
    const Result<void> c4 = dep(g4, 4, 1);
    assert(!c4);
    assert(c4.error().message.find("cycle: 4 -> 1 -> 2 -> 3 -> 4") != std::string::npos);  // via 3, the smaller id
    assert(g4.size() == 5);

    // A rejected edge leaves a valid graph: order() still works and is unchanged.
    World world({1, 2, 3, 4, 5});
    assert((order_of(g4, world.registry) == Ids{4, 3, 5, 2, 1}));
    // Diamond without a cycle is accepted.
    assert(dep(g4, 1, 4));
}

// -----------------------------------------------------------------------------
// Ordering (AC-003-06, 07, 08)
// -----------------------------------------------------------------------------

void test_empty_and_simple_graphs() {
    DependencyGraph none;
    World empty({});
    assert(order_of(none, empty.registry).empty());               // empty graph, empty registry

    World one({7});
    assert((order_of(none, one.registry) == Ids{7}));             // one independent component

    World several({30, 10, 20});                                  // registered out of order
    assert((order_of(none, several.registry) == Ids{10, 20, 30}));  // independent: ascending id
}

void test_single_and_multi_level_dependencies() {
    World world({1, 2, 3, 4});
    DependencyGraph single;
    assert(dep(single, 1, 2));                                    // 1 depends on 2
    assert((order_of(single, world.registry) == Ids{2, 1, 3, 4}));

    // Chain A -> B -> C -> D with ids that disagree with the dependency direction.
    DependencyGraph chain;
    assert(dep(chain, 1, 2) && dep(chain, 2, 3) && dep(chain, 3, 4));
    assert((order_of(chain, world.registry) == Ids{4, 3, 2, 1}));  // D, C, B, A

    // Multiple dependencies of one component.
    DependencyGraph many;
    assert(dep(many, 1, 3) && dep(many, 1, 4) && dep(many, 1, 2));
    assert((order_of(many, world.registry) == Ids{2, 3, 4, 1}));
}

void test_tie_break_is_lowest_component_id() {
    // A -> C and B -> C with A=1, B=2, C=3: C first, then A, then B.
    World world({1, 2, 3});
    DependencyGraph g;
    assert(dep(g, 1, 3) && dep(g, 2, 3));
    assert((order_of(g, world.registry) == Ids{3, 1, 2}));

    // Same shape with the ids reversed: the lower id still wins the tie.
    World reversed({1, 2, 3});
    DependencyGraph r;
    assert(dep(r, 3, 1) && dep(r, 2, 1));
    assert((order_of(r, reversed.registry) == Ids{1, 2, 3}));

    // Ready components interleave with unlocked ones by lowest id at every step.
    World nine({1, 2, 3, 4, 5, 6});
    DependencyGraph h;
    assert(dep(h, 1, 6) && dep(h, 2, 5) && dep(h, 3, 4));
    assert((order_of(h, nine.registry) == Ids{4, 3, 5, 2, 6, 1}));
}

void test_order_is_independent_of_registration_and_insertion_order() {
    const std::vector<std::pair<std::uint64_t, std::uint64_t>> edges = {{1, 3}, {2, 3}, {3, 4}, {5, 2}};
    Ids expected;
    {
        World w({1, 2, 3, 4, 5});
        DependencyGraph g;
        for (const auto& e : edges) assert(dep(g, e.first, e.second));
        expected = order_of(g, w.registry);
        assert((expected == Ids{4, 3, 1, 2, 5}));
    }
    auto perm = edges;
    std::sort(perm.begin(), perm.end());
    int checked = 0;
    do {
        DependencyGraph g;
        for (const auto& e : perm) assert(dep(g, e.first, e.second));   // every insertion order
        Ids ids = {1, 2, 3, 4, 5};
        std::mt19937 rng(static_cast<unsigned>(checked));
        std::shuffle(ids.begin(), ids.end(), rng);                        // and registration order
        World w(ids);
        assert(order_of(g, w.registry) == expected);
        ++checked;
    } while (std::next_permutation(perm.begin(), perm.end()));
    assert(checked == 24);
}

// Property check against an independent oracle: for random acyclic graphs the
// result must equal the lexicographically smallest valid topological order,
// found by brute force over all permutations.
void test_order_matches_brute_force_oracle() {
    std::mt19937 rng(20261003);
    for (int round = 0; round < 300; ++round) {
        const std::size_t n = 1 + rng() % 6;
        Ids ids(n);
        std::iota(ids.begin(), ids.end(), std::uint64_t{1});
        // Hidden topological rank makes every generated edge acyclic.
        Ids rank = ids;
        std::shuffle(rank.begin(), rank.end(), rng);
        std::set<std::pair<std::uint64_t, std::uint64_t>> edges;     // (dependent, dependency)
        for (std::size_t a = 0; a < n; ++a)
            for (std::size_t b = 0; b < n; ++b)
                if (rank[a] > rank[b] && rng() % 3 == 0) edges.insert({ids[a], ids[b]});

        std::vector<std::pair<std::uint64_t, std::uint64_t>> shuffled(edges.begin(), edges.end());
        std::shuffle(shuffled.begin(), shuffled.end(), rng);
        DependencyGraph g;
        for (const auto& e : shuffled) assert(dep(g, e.first, e.second));   // all acyclic: all accepted
        assert(g.size() == edges.size());

        World world(ids);
        const Ids got = order_of(g, world.registry);

        // Oracle: first permutation (in lexicographic order) that satisfies every edge.
        Ids candidate = ids;
        Ids best;
        do {
            std::map<std::uint64_t, std::size_t> position;
            for (std::size_t i = 0; i < candidate.size(); ++i) position[candidate[i]] = i;
            bool ok = true;
            for (const auto& e : edges) ok = ok && position[e.second] < position[e.first];
            if (ok) { best = candidate; break; }
        } while (std::next_permutation(candidate.begin(), candidate.end()));
        assert(!best.empty());
        assert(got == best);
    }
}

// -----------------------------------------------------------------------------
// Missing dependency (AC-003-02)
// -----------------------------------------------------------------------------

void test_missing_dependency_fails_without_an_order() {
    World world({1, 2});                                          // 3 is not registered
    DependencyGraph g;
    assert(dep(g, 1, 3));

    const auto result = g.order(world.registry);
    assert(!result.has_value());
    assert(result.error().code == ErrorCode::CONFIGURATION_ERROR);
    assert(result.error().source == ComponentId{1});               // the dependent
    assert(result.error().message.find("3") != std::string::npos); // names the missing id
    assert(result.error().message.find("unregistered") != std::string::npos);

    // Nothing was modified.
    assert(g.size() == 1 && world.registry.size() == 2);
    assert(world.lifecycle_calls() == 0);

    // Deterministic: the same error every time, and the smallest offending edge first.
    DependencyGraph many;
    assert(dep(many, 2, 9) && dep(many, 1, 8) && dep(many, 1, 7));
    const auto first = many.order(world.registry);
    assert(!first && first.error().source == ComponentId{1});
    assert(first.error().message.find("7") != std::string::npos);  // (1,7) before (1,8) and (2,9)
    assert(many.order(world.registry).error().message == first.error().message);

    // Registering the missing component fixes it: a complete valid order, never a partial one.
    ComponentRegistry& registry = world.registry;
    auto info = ComponentInfo::create(ComponentId{3}, "late");
    assert(info.has_value());
    ReferenceComponent late(std::move(info).value());
    assert(registry.register_component(late));
    assert((order_of(g, registry) == Ids{2, 3, 1}));   // ready {2,3}: lowest id first, then 1
}

void test_unregistered_dependent_and_empty_registry() {
    World world({2});
    DependencyGraph g;
    assert(dep(g, 5, 2));                                         // 5 (the dependent) is unregistered
    const auto r = g.order(world.registry);
    assert(!r && r.error().code == ErrorCode::CONFIGURATION_ERROR && r.error().source == ComponentId{5});
    assert(r.error().message.find("not registered") != std::string::npos);

    World empty({});
    const auto e = g.order(empty.registry);                       // edges but nothing registered: error, not []
    assert(!e && e.error().code == ErrorCode::CONFIGURATION_ERROR);
}

// -----------------------------------------------------------------------------
// No runtime orchestration (AC-003-09) and public-API integration
// -----------------------------------------------------------------------------

void test_graph_never_drives_components_and_is_consumed_by_the_owner() {
    World world({10, 20, 30, 40});
    DependencyGraph g;
    assert(dep(g, 10, 20) && dep(g, 20, 30) && dep(g, 40, 30));
    const auto result = g.order(world.registry);
    assert(result.has_value());
    assert(world.lifecycle_calls() == 0);                          // order() called no lifecycle operation
    for (const auto& c : world.owned) assert(c->lifecycle_state() == LifecycleState::UNKNOWN);

    // The owner (not the graph) drives components in the computed order.
    std::vector<std::uint64_t> started;
    for (ComponentId id : result.value()) {
        Component* c = world.registry.find(id);
        assert(c != nullptr);
        assert(c->initialize() && c->start());
        started.push_back(id.value());
    }
    assert((started == Ids{30, 20, 10, 40}));
    // Every component started after all of its dependencies.
    auto pos = [&](std::uint64_t id) { return std::find(started.begin(), started.end(), id) - started.begin(); };
    assert(pos(30) < pos(20) && pos(20) < pos(10) && pos(30) < pos(40));
}

void test_graph_does_not_extend_or_depend_on_component_lifetime() {
    DependencyGraph g;
    assert(dep(g, 1, 2));
    {
        World world({1, 2});
        assert((order_of(g, world.registry) == Ids{2, 1}));
    }                                                              // components and registry gone
    assert(g.size() == 1);                                         // the graph holds ids only
    assert((g.dependencies_of(ComponentId{1}) == std::vector<ComponentId>{ComponentId{2}}));
}

} // namespace

int main() {
    test_representation_uses_component_ids_only();
    test_invalid_ids_are_rejected();
    test_self_dependency_is_rejected();
    test_duplicate_dependency_is_rejected_not_merged();
    test_cycles_are_rejected_and_identified();
    test_empty_and_simple_graphs();
    test_single_and_multi_level_dependencies();
    test_tie_break_is_lowest_component_id();
    test_order_is_independent_of_registration_and_insertion_order();
    test_order_matches_brute_force_oracle();
    test_missing_dependency_fails_without_an_order();
    test_unregistered_dependent_and_empty_registry();
    test_graph_never_drives_components_and_is_consumed_by_the_owner();
    test_graph_does_not_extend_or_depend_on_component_lifetime();
    return 0;
}
