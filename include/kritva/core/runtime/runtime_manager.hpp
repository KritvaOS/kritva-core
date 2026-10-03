//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : runtime_manager.hpp
// Description : Concrete synchronous Runtime Manager implementing runtime::Runtime.
//
// Component   : Kritva Core
// Module      : Runtime
// Layer       : Core Foundation
//
// Requirements: CORE-RT-006
// API         : CORE-API-RUNTIME
//
// Author      : KritvaOS Core Team
// Created     : 03-10-2026
//==============================================================================

#pragma once
#include "component.hpp"
#include "component_id.hpp"
#include "component_registry.hpp"
#include "dependency_graph.hpp"
#include "runtime.hpp"
#include "../error/result.hpp"
#include "../lifecycle/lifecycle.hpp"
#include <vector>
namespace kritva::core::runtime {

//------------------------------------------------------------------------------
// RuntimeManager
//
// The concrete, synchronous, platform-independent implementation of the existing
// runtime::Runtime interface (CORE-RT-002). Runtime itself is unchanged and is
// the only public Runtime abstraction; RuntimeManager adds no second one. It
// composes the frozen ComponentRegistry and DependencyGraph and owns neither the
// components nor any thread, scheduler, timer or executor.
//
// SCOPE OF THIS CLASS (KF-CORE-R03-004)
//   Composition, topology validation, a fixed component set, and the Runtime's
//   own lifecycle state. It makes NO call on any component: no configure,
//   initialize, start, stop or shutdown. Dependency-ordered and reverse-ordered
//   invocation of components is KF-CORE-R03-005; failure and recovery semantics
//   are KF-CORE-R03-006. Until then the Runtime state is the Runtime's own.
//
// SETUP AND FIXED TOPOLOGY
//   - register_component() and add_dependency() build the topology. They forward
//     to ComponentRegistry::register_component() and DependencyGraph::
//     add_dependency(), and return their Results UNCHANGED (same code, source
//     and message): duplicate id, self dependency, duplicate edge, cycle and
//     invalid id are rejected exactly as those classes reject them, leaving the
//     manager unchanged.
//   - Setup is allowed only until the first successful initialize(). That call
//     fixes the topology for the lifetime of the manager: afterwards both setup
//     operations fail with ErrorCode::INVALID_STATE and change nothing,
//     including after stop() and re-initialize(). There is no unregister and no
//     topology mutation while the runtime is READY, RUNNING or STOPPED.
//   - Components are held as non-owning references (see Component). The manager
//     never owns, copies, moves, deletes or calls a component; destroying the
//     manager never touches one. A registered component must outlive any use of
//     the manager, as for ComponentRegistry. The manager itself is neither
//     copyable nor movable.
//   - registry() and dependencies() expose read-only views for inspection (a
//     const registry still yields mutable Component*, see ComponentRegistry).
//
// TOPOLOGY VALIDATION AND ORDER
//   - component_order() is exactly DependencyGraph::order(registry): every
//     registered component once, dependencies first, ties by lowest ComponentId,
//     independent of registration and edge insertion order. RuntimeManager has
//     no ordering algorithm of its own. A dependency endpoint that is not
//     registered is reported with the graph's CONFIGURATION_ERROR (source = the
//     dependent) and no order is returned.
//   - initialize() validates the topology with that same call before anything
//     else happens, so an invalid topology can never reach a READY runtime.
//
// RUNTIME OPERATIONS (the Runtime interface)
//   The Runtime has the Core lifecycle states and follows the same operation
//   table as Component, but only its own state changes:
//
//     operation    valid from        on success   notes
//     ----------   ---------------   ----------   ----------------------------------
//     initialize   UNKNOWN           READY        validates topology, then fixes it
//                  STOPPED           READY        topology already fixed and valid
//     start        READY             RUNNING      no component is started (R03-005)
//     stop         READY, RUNNING    STOPPED      no component is stopped (R03-005)
//     shutdown     UNKNOWN, STOPPED  unchanged    idempotent no-op
//
//   - Any other operation/state pair fails with ErrorCode::INVALID_STATE and has
//     no effect. FAULT is part of the Core state set but nothing in this class
//     produces it; failure handling is R03-006, and shutdown() is therefore not
//     yet valid from FAULT.
//   - Failed topology validation in initialize() returns the validation Error
//     unchanged and leaves the runtime UNKNOWN (not running) with the topology
//     still open, so the caller may fix the topology and call initialize()
//     again. Setup never implicitly starts anything.
//   - state() reports the state after the last completed operation. Operations
//     are synchronous; the transient INITIALIZING state is never observable
//     after a call returns.
//   - Errors from the manager itself (INVALID_STATE) carry no ComponentId source
//     because no component is involved.
//
// THREADING, ALLOCATION, REAL TIME
//   - Synchronous and single-threaded: no thread, executor or timer is created
//     and no thread-safety guarantee is made; callers serialize all calls.
//   - Setup, initialize() and component_order() allocate; they are control-plane
//     operations. start() and stop() do not allocate. R03 makes no real-time
//     claim.
//------------------------------------------------------------------------------
class RuntimeManager final : public Runtime {
public:
    RuntimeManager() = default;
    ~RuntimeManager() override = default;
    RuntimeManager(const RuntimeManager&) = delete;
    RuntimeManager& operator=(const RuntimeManager&) = delete;

    // --- setup (valid until the first successful initialize()) -------------
    Result<void> register_component(Component& component);
    Result<void> add_dependency(ComponentId dependent, ComponentId dependency);

    /// True once initialize() has succeeded; setup is then closed for good.
    [[nodiscard]] bool topology_fixed() const noexcept { return topology_fixed_; }

    /// DependencyGraph::order() over the registered components.
    [[nodiscard]] Result<std::vector<ComponentId>> component_order() const;

    [[nodiscard]] const ComponentRegistry& registry() const noexcept { return registry_; }
    [[nodiscard]] const DependencyGraph& dependencies() const noexcept { return graph_; }

    // --- runtime::Runtime ---------------------------------------------------
    Result<void> initialize() override;
    Result<void> start() override;
    Result<void> stop() override;
    Result<void> shutdown() override;
    [[nodiscard]] LifecycleState state() const noexcept override { return lifecycle_.state(); }

private:
    Result<void> invalid_state(const char* operation) const;
    Result<void> setup_closed(const char* operation) const;
    void transition(LifecycleState target);

    ComponentRegistry registry_;
    DependencyGraph graph_;
    Lifecycle lifecycle_;
    bool topology_fixed_{false};
};

} // namespace kritva::core::runtime
