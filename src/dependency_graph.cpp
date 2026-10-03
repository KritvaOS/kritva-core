//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : dependency_graph.cpp
// Description : DependencyGraph implementation.
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

#include <kritva/core/runtime/dependency_graph.hpp>

#include <cassert>
#include <string>
#include <utility>

namespace kritva::core::runtime {

namespace {

std::string text(ComponentId id) { return std::to_string(id.value()); }

Error invalid_edge(ComponentId dependent, std::string message) {
    return Error{ErrorCode::INVALID_ARGUMENT, ErrorSeverity::ERROR, dependent, {}, std::move(message)};
}

} // namespace

Result<void> DependencyGraph::add_dependency(ComponentId dependent, ComponentId dependency) {
    if (!dependent.valid() || !dependency.valid()) {
        return Result<void>::failure(invalid_edge(dependent, "dependency edge uses the invalid component id"));
    }
    if (dependent == dependency) {
        return Result<void>::failure(
            invalid_edge(dependent, "component " + text(dependent) + " cannot depend on itself"));
    }
    const auto existing = dependencies_.find(dependent);
    if (existing != dependencies_.end() && existing->second.count(dependency) != 0) {
        return Result<void>::failure(invalid_edge(
            dependent, "dependency " + text(dependent) + " -> " + text(dependency) + " already exists"));
    }

    // The edge closes a cycle iff `dependency` already (transitively) depends on
    // `dependent`. Depth-first search, smallest id first, so the reported path is
    // deterministic.
    std::vector<ComponentId> path{dependent, dependency};
    std::set<ComponentId> visited{dependency};
    struct Frame { ComponentId node; std::set<ComponentId>::const_iterator next, end; };
    std::vector<Frame> stack;
    const auto push = [&](ComponentId node) {
        const auto it = dependencies_.find(node);
        if (it == dependencies_.end()) {
            static const std::set<ComponentId> none;
            stack.push_back({node, none.begin(), none.end()});
        } else {
            stack.push_back({node, it->second.begin(), it->second.end()});
        }
    };
    push(dependency);
    while (!stack.empty()) {
        Frame& top = stack.back();
        if (top.next == top.end) {
            stack.pop_back();
            path.pop_back();
            continue;
        }
        const ComponentId next = *top.next++;
        if (next == dependent) {
            std::string cycle;
            for (const ComponentId id : path) cycle += text(id) + " -> ";
            cycle += text(dependent);
            return Result<void>::failure(invalid_edge(
                dependent, "dependency " + text(dependent) + " -> " + text(dependency) +
                               " would create a cycle: " + cycle));
        }
        if (visited.insert(next).second) {
            path.push_back(next);
            push(next);
        }
    }

    dependencies_[dependent].insert(dependency);   // strong guarantee: nothing else changed above
    ++edge_count_;
    return Result<void>::success();
}

std::vector<ComponentId> DependencyGraph::dependencies_of(ComponentId dependent) const {
    const auto it = dependencies_.find(dependent);
    if (it == dependencies_.end()) return {};
    return std::vector<ComponentId>(it->second.begin(), it->second.end());
}

Result<std::vector<ComponentId>> DependencyGraph::order(const ComponentRegistry& registry) const {
    // 1. Every edge endpoint must be registered. Edges are scanned in ascending
    //    (dependent, dependency) order so the reported failure is deterministic.
    for (const auto& [dependent, dependencies] : dependencies_) {
        if (!registry.contains(dependent)) {
            return Result<std::vector<ComponentId>>::failure(Error{
                ErrorCode::CONFIGURATION_ERROR, ErrorSeverity::ERROR, dependent, {},
                "component " + text(dependent) + " has dependencies but is not registered"});
        }
        for (const ComponentId dependency : dependencies) {
            if (!registry.contains(dependency)) {
                return Result<std::vector<ComponentId>>::failure(Error{
                    ErrorCode::CONFIGURATION_ERROR, ErrorSeverity::ERROR, dependent, {},
                    "component " + text(dependent) + " depends on unregistered component " +
                        text(dependency)});
            }
        }
    }

    // 2. Kahn's algorithm. `ready` is ordered, so the lowest ComponentId among the
    //    simultaneously ready components is always taken next.
    std::map<ComponentId, std::size_t> unmet;               // component -> dependencies not yet placed
    std::map<ComponentId, std::vector<ComponentId>> dependents_of;
    std::set<ComponentId> ready;
    for (const Component* component : registry.components()) {
        const ComponentId id = component->info().id();
        const auto it = dependencies_.find(id);
        const std::size_t count = it == dependencies_.end() ? 0 : it->second.size();
        unmet[id] = count;
        if (count == 0) ready.insert(id);
    }
    for (const auto& [dependent, dependencies] : dependencies_) {
        for (const ComponentId dependency : dependencies) dependents_of[dependency].push_back(dependent);
    }

    std::vector<ComponentId> ordered;
    ordered.reserve(unmet.size());
    while (!ready.empty()) {
        const ComponentId next = *ready.begin();
        ready.erase(ready.begin());
        ordered.push_back(next);
        const auto users = dependents_of.find(next);
        if (users == dependents_of.end()) continue;
        for (const ComponentId user : users->second) {
            if (--unmet[user] == 0) ready.insert(user);
        }
    }

    // add_dependency() keeps the graph acyclic, so every component is placed.
    assert(ordered.size() == unmet.size());
    return Result<std::vector<ComponentId>>::success(std::move(ordered));
}

} // namespace kritva::core::runtime
