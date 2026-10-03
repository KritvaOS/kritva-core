//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component_registry.hpp
// Description : Deterministic non-owning registry of runtime components.
//
// Component   : Kritva Core
// Module      : Runtime
// Layer       : Core Foundation
//
// Requirements: CORE-RT-003
// API         : CORE-API-RUNTIME
//
// Author      : KritvaOS Core Team
// Created     : 03-10-2026
//==============================================================================

#pragma once
#include "component.hpp"
#include "component_id.hpp"
#include "../error/result.hpp"
#include <cstddef>
#include <map>
#include <vector>
namespace kritva::core::runtime {

//------------------------------------------------------------------------------
// ComponentRegistry (CORE-RT-003)
//
// Keeps track of which components exist, keyed by ComponentId. It does nothing
// else: it never initializes, starts, stops, configures, orders or otherwise
// calls a component, creates threads, or uses a scheduler.
//
// OWNERSHIP AND LIFETIME (preserves the Component ownership model)
//   - The registry is NON-OWNING. It stores a reference to each registered
//     component; the application/runtime owner keeps owning it. The registry
//     never owns, copies, moves, or deletes a component, and does not make a
//     component's address or lifetime part of any contract beyond the rule
//     below. It must not be changed into an owning container.
//   - A registered component must stay alive, at the same address, for as long
//     as the registry is used. There is no unregister operation (see below), so
//     in practice: destroy the registry no earlier than you stop using it, and
//     do not destroy a registered component while the registry is still in use.
//     Using the registry after one of its components was destroyed is a
//     precondition violation (undefined behavior); the registry cannot detect it.
//   - Destroying the registry is always safe and never touches any component,
//     so components may be destroyed before or after the registry once it is no
//     longer used.
//   - Pointers returned by find() and components() point at the registered
//     components themselves (not copies) and are valid exactly as long as the
//     component is alive. They are never invalidated by later registrations,
//     because the registry stores no component by value.
//   - The registry is neither copyable nor movable: it is the single holder of
//     its registration state.
//
// REGISTRATION
//   - register_component(component) uses component.info().id() as the key. A
//     ComponentInfo can only hold a valid (non-zero) id and a Component cannot
//     be built without one, so an invalid identity cannot reach the registry
//     and no null component can be passed (a reference cannot be null).
//   - Registration is allowed regardless of the component's lifecycle state and
//     does not change it.
//   - A second registration with an id that is already registered fails with
//     ErrorCode::INVALID_ARGUMENT and Error::source == the id. It never
//     replaces the existing registration, whether it is the same object again or
//     a different object with the same id, and the registry is unchanged.
//   - Failures are reported through Result. Allocation failure while
//     registering throws std::bad_alloc, as for the other Core containers; in
//     that case the registry is unchanged (strong guarantee).
//   - There is no unregister operation. It is deliberately out of scope: no R03
//     requirement needs it, and the runtime's components are fixed before it
//     starts. Adding it would be an explicit, reviewed API extension.
//
// LOOKUP
//   - find(id) returns the registered component or nullptr if there is none
//     (including for the invalid id). contains(id) is the same test as a bool.
//     Both are side-effect free.
//   - The registry is shallow-const: a const registry still hands out mutable
//     component pointers, because it does not own what it points to.
//
// ENUMERATION AND ORDER
//   - components() returns a snapshot (a vector of non-owning pointers) with each
//     registered component exactly once, in ASCENDING ComponentId order
//     (ComponentId::value(), unsigned). The order depends only on the set of
//     ids, never on registration order or on the hash/bucket layout of any
//     container, so it is identical on every run and platform. Later changes to
//     the registry do not alter a snapshot already returned.
//   - The container used internally is not exposed.
//
// REAL-TIME / THREAD-SAFETY
//   - Not thread-safe: callers serialize all calls on one registry. Concurrent
//     const calls (find, contains, size, empty, components) are safe only if no
//     thread is registering.
//   - Control-plane only: register_component() and components() allocate;
//     find()/contains() do not allocate and are O(log n) with no blocking. No
//     hard-real-time claim is made.
//------------------------------------------------------------------------------
class ComponentRegistry {
public:
    ComponentRegistry() = default;
    ~ComponentRegistry() = default;
    ComponentRegistry(const ComponentRegistry&) = delete;
    ComponentRegistry& operator=(const ComponentRegistry&) = delete;

    /// Register `component` under component.info().id(). Fails with
    /// ErrorCode::INVALID_ARGUMENT if that id is already registered.
    Result<void> register_component(Component& component);

    /// The registered component with this id, or nullptr.
    [[nodiscard]] Component* find(ComponentId id) const noexcept;

    [[nodiscard]] bool contains(ComponentId id) const noexcept;

    [[nodiscard]] std::size_t size() const noexcept { return components_.size(); }
    [[nodiscard]] bool empty() const noexcept { return components_.empty(); }

    /// Snapshot of all registered components in ascending ComponentId order.
    [[nodiscard]] std::vector<Component*> components() const;

private:
    std::map<ComponentId, Component*> components_;
};

} // namespace kritva::core::runtime
