//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component_statistics.hpp
// Description : Optional Component-owned operational statistics provider.
//
// Component   : Kritva Core
// Module      : Runtime
// Layer       : Core Foundation
//
// Requirements: CORE-OPS-001, CORE-OPS-004
// API         : CORE-API-RUNTIME
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#pragma once
#include "../statistics/statistics.hpp"
namespace kritva::core::runtime {

//------------------------------------------------------------------------------
// IComponentStatistics (CORE-OPS-004)
//
// An OPTIONAL interface through which an integrator-written Component (or any
// object the integrator chooses) offers its own operational Statistics. It is
// deliberately NOT a base of runtime::Component and Component has no
// statistics() member: statistics are not forced onto components for which
// they would be meaningless. A component opts in by also implementing this
// interface; observe() (component_observation.hpp) takes it as an explicit
// optional pointer, so Core needs no RTTI and no registration.
//
// OWNERSHIP AND SEPARATION
//   - The provider owns the numbers and their meaning (what an "item" or a
//     "resource" is). Core keeps no copy, accumulates nothing and mirrors
//     nothing. statistics() returns a Statistics BY VALUE: an independent
//     snapshot that never refers into the provider and cannot dangle.
//   - They are separate from the Runtime's own statistics
//     (RuntimeManager::statistics()), which count Runtime component calls. The
//     Runtime never reads, resets or combines a component's statistics, and
//     component statistics never change Runtime statistics.
//
//   - The pairing of a provider with its component is the CALLER's: the interface
//     carries no identity and Core does not check it (a component typically
//     inherits both, or an integrator-owned object forwards to the component's
//     counters). A component without statistics simply has no provider; Core never
//     invents or defaults one, and observe() then reports std::nullopt.
//   - Values pass through unchanged: Core does not clamp, normalize, derive, add
//     to or reset any field (a utilization outside 0..100 or a queue_depth below
//     zero is reported as the provider stored it).
//
// SEMANTICS
//   - statistics() is a const, read-only query: a conforming implementation has
//     no observable side effect (it does not reset counters, change lifecycle
//     state, call a service or report an Event). Core cannot enforce purity of
//     a virtual call; this is the contract a conforming implementation meets.
//   - Fields are independent and the copy is NOT an atomic snapshot across
//     fields (see Statistics). No ordering or relationship between successive
//     snapshots is promised beyond what the provider documents.
//   - The interface is control-plane: it may allocate (a conforming provider
//     typically does not) and is not for real-time paths. Thread-safety is that
//     of the provider and must be documented by it; Core provides none.
//   - Core never calls it on its own: no Runtime poll, no background thread, no
//     telemetry export. The integrator decides when to ask.
//------------------------------------------------------------------------------
class IComponentStatistics {
public:
    virtual ~IComponentStatistics() = default;

    [[nodiscard]] virtual Statistics statistics() const = 0;
};
} // namespace kritva::core::runtime
