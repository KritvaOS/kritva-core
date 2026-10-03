//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : dependency_graph.hpp
// Description : Component dependency graph and deterministic dependency order.
//
// Component   : Kritva Core
// Module      : Runtime
// Layer       : Core Foundation
//
// Requirements: CORE-RT-004, CORE-RT-005
// API         : CORE-API-RUNTIME
//
// Author      : KritvaOS Core Team
// Created     : 03-10-2026
//==============================================================================

#pragma once
#include "component_id.hpp"
#include "component_registry.hpp"
#include "../error/result.hpp"
#include <cstddef>
#include <map>
#include <set>
#include <vector>
namespace kritva::core::runtime {

//------------------------------------------------------------------------------
// DependencyGraph (CORE-RT-004, CORE-RT-005)
//
// Records which components depend on which others and computes a
// dependency-respecting order. It is pure data about ComponentIds: it holds no
// component, pointer, or registry, and it never calls a component, initializes,
// starts or stops anything, creates threads, or uses a scheduler.
//
// REPRESENTATION (CORE-RT-004)
//   - A dependency is a directed edge "dependent -> dependency" between two
//     ComponentIds, read as "dependent depends on dependency", so the
//     dependency must come first in the dependency order. Identity is the
//     ComponentId only; raw pointers and platform handles are never used.
//   - The graph is a set of such edges. It is a value type (copyable, movable)
//     and independent of any ComponentRegistry; ids need not be registered when
//     an edge is added (they are checked by order()).
//   - Internally ordered containers are used, so every observable result
//     (dependencies_of(), order(), error messages) is a pure function of the set
//     of edges and of the sequence of add_dependency() calls.
//
// add_dependency(dependent, dependency)
//   Adds one edge, or fails without changing the graph. All failures are
//   ErrorCode::INVALID_ARGUMENT with Error::source == dependent:
//     - either id is the invalid id (0);
//     - self dependency (dependent == dependency);
//     - duplicate edge: the same edge was already added. Duplicates are
//       REJECTED, never silently merged, so no duplicate edge can exist;
//     - the edge would create a dependency cycle. The message lists the cycle
//       as "a -> b -> ... -> a" in dependent-to-dependency direction, starting
//       at `dependent`; among several paths the one visiting the smallest
//       ComponentId first is reported. The graph is therefore always acyclic.
//   Cycle membership is reported in the error message only.
//
// order(registry)  (CORE-RT-005)
//   Returns every component registered in `registry`, exactly once, such that
//   each component appears AFTER all components it depends on ("dependencies
//   first"). Reverse the result for an order in which dependents come first
//   (for example for stopping). Components with no edges are included.
//   - Tie-break (frozen): among components that are simultaneously ready,
//     the one with the lowest ComponentId comes first. The result is therefore
//     the lexicographically smallest valid order by ComponentId, a pure function
//     of the registered set and the edges; it does not depend on registration
//     order, edge insertion order, or any container iteration order. For
//     A -> C and B -> C (A=1, B=2, C=3) the order is C, A, B.
//   - An empty registry yields an empty order; edges whose endpoints are not
//     registered make order() fail (below), so an empty registry with edges is
//     an error, not an empty order.
//   - Missing component: if an edge endpoint is not registered, order() fails
//     with ErrorCode::CONFIGURATION_ERROR and returns no order. The first
//     offending edge in ascending (dependent, dependency) order is reported;
//     Error::source is the dependent id and the message names the missing id
//     and whether it is the dependent or the dependency. Neither the graph nor
//     the registry is modified.
//   - No partial or invalid order is ever returned: the result is either a
//     complete valid order or an error. Because add_dependency() keeps the graph
//     acyclic, a cycle can never make order() fail.
//
// OWNERSHIP / REAL TIME / THREAD SAFETY
//   - order() reads the registry through its public API only and never calls
//     a component. It does not extend any component's lifetime.
//   - Not thread-safe: serialize calls on one graph. Concurrent const calls are
//     safe only if nobody is adding edges.
//   - Control-plane only: add_dependency() and order() allocate. add_dependency()
//     is O(V + E) in the worst case (cycle check); order() is O((V + E) log V).
//     No hard-real-time claim is made. Allocation failure throws std::bad_alloc
//     (graph unchanged after a failed add_dependency()).
//------------------------------------------------------------------------------
class DependencyGraph {
public:
    /// Add "dependent depends on dependency". See the contract above.
    Result<void> add_dependency(ComponentId dependent, ComponentId dependency);

    /// Direct dependencies of `dependent`, ascending. Empty if there are none.
    [[nodiscard]] std::vector<ComponentId> dependencies_of(ComponentId dependent) const;

    /// Number of edges.
    [[nodiscard]] std::size_t size() const noexcept { return edge_count_; }
    [[nodiscard]] bool empty() const noexcept { return edge_count_ == 0; }

    /// Dependency-respecting order of all components in `registry`
    /// (dependencies first, ties by ascending ComponentId), or an error.
    [[nodiscard]] Result<std::vector<ComponentId>> order(const ComponentRegistry& registry) const;

private:
    std::map<ComponentId, std::set<ComponentId>> dependencies_;  // dependent -> its dependencies
    std::size_t edge_count_{0};
};

} // namespace kritva::core::runtime
