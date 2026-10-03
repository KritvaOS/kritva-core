//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component_observation.hpp
// Description : Read-only, detached observation of a Component's operational information.
//
// Component   : Kritva Core
// Module      : Runtime
// Layer       : Core Foundation
//
// Requirements: CORE-OPS-001, CORE-OPS-006
// API         : CORE-API-RUNTIME
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#pragma once
#include "component.hpp"
#include "component_statistics.hpp"
#include <optional>
namespace kritva::core::runtime {

//------------------------------------------------------------------------------
// ComponentObservation (CORE-OPS-001)
//
// A DETACHED VALUE: what an observer learned about one Component at one
// moment, assembled from the Component's own existing accessors. It owns every
// field, never refers into the Component (it cannot dangle) and is copyable.
//
//   id          the component's immutable identity (info().id()).
//   lifecycle   lifecycle_state(): where the Runtime-controlled lifecycle is.
//   status      status(): the component's coarse operational condition.
//   health      health(): the component's own health assessment.
//   statistics  std::nullopt when no provider was supplied; otherwise engaged
//               with exactly the value the provider returned. An engaged
//               Statistics{} carries no meaning beyond "the provider returned
//               it": the provider owns what its numbers mean.
//
// There is NO new operational state machine. Lifecycle, Status and Health are
// three distinct, independent pieces of information (see below).
//
// AUTHORITY (CORE-OPS-001)
//   The Component is authoritative for its operational information. Core keeps
//   no second copy and no cache: every observation asks the component again and
//   an old ComponentObservation never updates itself. The Runtime remains
//   authoritative for lifecycle orchestration and for its own statistics; it
//   neither produces nor consumes ComponentObservation.
//
// STATUS AND HEALTH (contract completed by R07-002)
//   - Status() and Health() are value snapshots: independent copies, valid
//     forever, never referring into the component.
//   - Lifecycle, Status and Health are INDEPENDENT. Core imposes no relation
//     between them: any combination a component reports is legal and is passed
//     on unchanged (for example Health UNHEALTHY while RUNNING, Health HEALTHY while the
//     lifecycle is FAULT, Status NOT_READY while RUNNING). An empty Status message or
//     Health detail is valid for every code or state. Core never validates,
//     normalizes, derives or reconciles one from another.
//   - Health is component-reported INFORMATION. It is independent of a Runtime
//     FAULT (which is a lifecycle state caused by a failed initialize/start/
//     stop) and is never interpreted by Core as a recovery, restart, retry or
//     reconfiguration trigger. Nothing in Core reads a Health or Status and
//     reacts to it.
//
// observe() (CORE-OPS-006)
//   Calls, once each and in this documented order:
//     1. component.info().id()
//     2. component.lifecycle_state()
//     3. component.status()
//     4. component.health()
//     5. provider->statistics()        only when a provider is supplied
//   It returns a ComponentObservation holding the results and does nothing
//   else: it changes no Component, Runtime or provider state of its own, calls
//   no other function, creates no thread, starts no polling and keeps nothing
//   after it returns. It does not involve a Runtime.
//   - Purity of the underlying accessors is a CONTRACT ON CONFORMING
//     implementations (status(), health(), lifecycle_state() and statistics()
//     are const queries without observable side effect), not something Core can
//     enforce: observe() itself introduces no side effect, but it does invoke
//     the virtual const accessors the component and the provider supply, and a
//     non-conforming implementation can do anything.
//   - The provider is supplied by the caller, who is responsible for passing the
//     one that belongs to this component; the interface carries no identity and
//     Core does not check the association.
//   - No cross-property atomicity: the five reads happen one after another and
//     are NOT a single atomic snapshot. If another thread changes the component
//     meanwhile, the fields may describe different instants; the caller
//     serializes access if it needs coherency. Core provides no lock.
//   - Deterministic: for a component and provider that return the same values,
//     observe() returns the same observation, and repeated calls without
//     intervening change return equal observations.
//   - Allocation / real time: observe() allocates only to copy the Status
//     message and Health detail strings (and whatever the accessors do). It is a
//     control-plane operation, not for real-time paths; Core makes no real-time
//     claim. It blocks nothing itself and has O(length of the two strings)
//     complexity plus the accessors' own.
//   - Failure: observe() has no error path of its own. The accessors do not
//     return Result; an exception thrown by an implementation (for example
//     std::bad_alloc while copying a string) propagates unchanged and leaves
//     everything unchanged.
//   - There is no Runtime polling, no event emission, no export and no logging.
//------------------------------------------------------------------------------
struct ComponentObservation {
    ComponentId id{};
    LifecycleState lifecycle{LifecycleState::UNKNOWN};
    Status status{};
    Health health{};
    std::optional<Statistics> statistics{};
};

/// Read-only observation of `component`; see the contract above.
[[nodiscard]] inline ComponentObservation observe(const Component& component,
                                                  const IComponentStatistics* provider = nullptr) {
    ComponentObservation observation;
    observation.id = component.info().id();
    observation.lifecycle = component.lifecycle_state();
    observation.status = component.status();
    observation.health = component.health();
    if (provider != nullptr) observation.statistics = provider->statistics();
    return observation;
}
} // namespace kritva::core::runtime
