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
// Requirements: CORE-RT-006, CORE-RT-007
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
#include "../configuration/configuration.hpp"
#include "../lifecycle/lifecycle.hpp"
#include <set>
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
// SETUP AND FIXED TOPOLOGY (KF-CORE-R03-004)
//   - register_component() and add_dependency() build the topology. They forward
//     to ComponentRegistry::register_component() and DependencyGraph::
//     add_dependency(), and return their Results UNCHANGED (same code, source
//     and message): duplicate id, self dependency, duplicate edge, cycle and
//     invalid id are rejected exactly as those classes reject them, leaving the
//     manager unchanged.
//   - Setup is allowed only until the first successful initialize() has
//     validated the topology. That call fixes the topology for the lifetime of
//     the manager: afterwards both setup operations fail with
//     ErrorCode::INVALID_STATE and change nothing, including after stop() and
//     re-initialize(). There is no unregister and no topology mutation.
//   - Components are held as non-owning references (see Component). The manager
//     never owns, copies, moves or deletes a component; destroying the manager
//     never touches one. A registered component must outlive any use of the
//     manager. The manager itself is neither copyable nor movable.
//   - registry() and dependencies() expose read-only views for inspection (a
//     const registry still yields mutable Component*, see ComponentRegistry).
//   - component_order() is exactly DependencyGraph::order(registry): every
//     registered component once, dependencies first, ties by lowest ComponentId,
//     independent of registration and edge insertion order. RuntimeManager has
//     no ordering algorithm of its own.
//
// LIFECYCLE ORCHESTRATION (KF-CORE-R03-005)
//   The Runtime has the Core lifecycle states and follows the same operation
//   table as Component. Each operation first checks the Runtime's own state;
//   an invalid call fails with ErrorCode::INVALID_STATE (no component source),
//   changes nothing and invokes NO component. Otherwise it invokes the one
//   corresponding Component operation, once per component, in a fixed order:
//
//     operation    valid from        component call   order      on success
//     ----------   ---------------   --------------   -------    ----------------
//     configure    UNKNOWN, STOPPED  configure(cfg)    forward    state unchanged
//     initialize   UNKNOWN, STOPPED  initialize()      forward    READY
//     start        READY             start()           forward    RUNNING
//     stop         READY, RUNNING    stop()            reverse    STOPPED
//     shutdown     UNKNOWN, STOPPED  shutdown()        reverse    state unchanged
//
//   "forward" is dependency order (dependencies before dependents, ties by lowest
//   ComponentId); "reverse" is exactly the reverse sequence, so dependents are
//   torn down before their dependencies. The order is the one validated by the
//   first initialize() and is never recomputed afterwards.
//
//   - configure(const Configuration&) is a RuntimeManager operation (the Runtime
//     interface has none). Every component receives the same Configuration. It
//     does not initialize or start anything, does not change the Runtime state,
//     and does not fix the topology: it uses the current order, so components
//     registered afterwards are not configured. It fails without invoking any
//     component if the topology is invalid (the graph's CONFIGURATION_ERROR,
//     unchanged).
//   - initialize() first validates and, the first time, fixes the topology as in
//     R03-004; a validation failure returns the graph's error unchanged, leaves
//     the Runtime UNKNOWN and invokes no component. Then it initializes every
//     component forward.
//   - The Runtime is never RUNNING before every start() has succeeded, and never
//     READY before every initialize() has succeeded: its state changes only
//     after the whole sequence succeeded.
//   - Components have a lifecycle progress that the Runtime records itself and
//     never infers from Component::lifecycle_state() (STOPPED alone cannot tell
//     "never initialized" from "shut down"): never initialized, initialized
//     (live), shut down. initialize() makes every component live again.
//   - shutdown() invokes only live components that have not yet completed a
//     shutdown during the current live period, in reverse order. A component
//     whose shutdown has already succeeded is NOT invoked by a later shutdown()
//     attempt. In UNKNOWN (never initialized) or after every component has been
//     shut down it is a no-op that invokes nothing, so no component is ever
//     shut down twice per live period.
//   - The Runtime invokes only the operation being orchestrated and never reads
//     or changes a component's state itself.
//
//   FAILURE BOUNDARY (detailed recovery and reset are KF-CORE-R03-006)
//   - The first component whose operation fails ends the sequence: remaining
//     components are NOT invoked, nothing is retried, and no completed step is
//     rolled back or compensated. The component's own Error is returned
//     UNCHANGED (code, severity, source = that component's id, message).
//   - configure() and shutdown() failures leave the Runtime state unchanged.
//   - Shutdown progress is preserved across a failed shutdown(): components
//     shut down before the failure stay recorded as shut down, and the failing
//     component and every component after it in reverse order are still
//     pending. Calling shutdown() again retries exactly those, in reverse order
//     (the failing component first), and never repeats a successful shutdown.
//     Example, reverse order D, C, B, A: first call D ok, C ok, B fails (A not
//     invoked); retry invokes B then A only. There is no rollback.
//   - A failure in initialize(), start() or stop() moves the Runtime to FAULT
//     (the Core table allows INITIALIZING/READY/RUNNING/STOPPING -> FAULT). In
//     FAULT every operation, including shutdown(), fails with INVALID_STATE
//     and invokes nothing: leaving FAULT is defined by R03-006. Components that
//     completed the operation before the failure keep their state, and the
//     failing component is in its own Component-contract state (normally FAULT).
//   - Nothing is retried or recovered automatically, and no timer, watchdog,
//     thread or background work exists.
//
//   state() reports the state after the last completed operation. Operations are
//   synchronous; the transient INITIALIZING and STOPPING states are never
//   observable after a call returns.
//
// THREADING, ALLOCATION, REAL TIME
//   - Synchronous and single-threaded: no thread, executor or timer is created
//     and no thread-safety guarantee is made; callers serialize all calls.
//   - Setup, configure(), initialize() and component_order() allocate; they are
//     control-plane operations. start(), stop() and shutdown() do not allocate
//     beyond what the components themselves do. R03 makes no real-time claim.
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

    // --- lifecycle orchestration (see the contract above) --------------------
    /// Configure every component, in dependency order, with the same Configuration.
    Result<void> configure(const Configuration& configuration);

    // --- runtime::Runtime ---------------------------------------------------
    Result<void> initialize() override;
    Result<void> start() override;
    Result<void> stop() override;
    Result<void> shutdown() override;
    [[nodiscard]] LifecycleState state() const noexcept override { return lifecycle_.state(); }

private:
    enum class Step { CONFIGURE, INITIALIZE, START, STOP };

    Result<void> invalid_state(const char* operation) const;
    Result<void> setup_closed(const char* operation) const;
    void transition(LifecycleState target);
    /// Invoke `step` on each component of `ids`, forward or reverse, stopping at
    /// the first failure and returning that component's error unchanged.
    Result<void> run(const std::vector<ComponentId>& ids, Step step, bool reverse,
                     const Configuration* configuration);

    ComponentRegistry registry_;
    DependencyGraph graph_;
    Lifecycle lifecycle_;
    std::vector<ComponentId> order_;   // validated forward order, set when the topology is fixed
    bool topology_fixed_{false};
    bool components_live_{false};      // initialized and not yet fully shut down
    std::set<ComponentId> shut_down_;  // components whose shutdown succeeded in this live period
};

} // namespace kritva::core::runtime
