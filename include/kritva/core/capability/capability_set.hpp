//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : capability_set.hpp
// Description : Capability collection with lookup.
//
// Component   : Kritva Core
// Module      : Capability
// Layer       : Core Foundation
//
// Requirements: CORE-CAP-003; CORE-CAP-005; CORE-CAP-006; CORE-CAP-007; CORE-CAP-008
// API         : CORE-API-CAPABILITY
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include "capability.hpp"
#include <cstddef>
#include <vector>
namespace kritva::core {

//------------------------------------------------------------------------------
// CAPABILITYSET CONTRACT: INVARIANTS, SNAPSHOTS AND VERSION SEMANTICS
// (CORE-CAP-005, CORE-CAP-006)
//
// WHAT A CAPABILITYSET IS
//   A CapabilitySet is a plain, copyable, ownership-safe VALUE: a collection of
//   Capability declarations used to declare what an entity provides and to look
//   capabilities up by identity. It is a SNAPSHOT, not a registry: there is no
//   global or static set, no discovery, no change notification and no hidden
//   state. Providers return one BY VALUE (CORE-CAP-004).
//
// INVARIANTS (all observable, all deterministic)
//   1. At most one entry per CapabilityId. add(c) with an id already present
//      REPLACES that entry (its name AND version) IN PLACE: the entry keeps its
//      position and size() does not change; otherwise it APPENDS. Latest wins and
//      no history is kept. (The invalid id, zero, is just another key: an entry
//      with it is storable data, not an authoritative capability: CORE-CAP-004.)
//   2. Observable order is FIRST-INSERTION order of the distinct ids, not sorted
//      and not affected by replacement. all() and any iteration over it follow
//      that order. The same sequence of add() calls always gives the same
//      contents and the same order; two sets with the same contents added in a
//      different order have a different observable order.
//   3. names and versions may repeat across different ids; only the id is a key.
//   4. add() never fails and validates nothing (no id, name or version check).
//      There is no remove, erase, clear, merge, union, priority or sorting
//      operation: a set only grows, and a different set is built to drop an entry.
//   5. size() is the number of distinct ids; empty() is size() == 0; a
//      default-constructed set is empty.
//   6. contains(id) and find(id) look up by IDENTITY ONLY (never by name or
//      version), linearly, noexcept and without allocation; find returns nullptr
//      for an id that is not present.
//
// LIFETIME AND OWNERSHIP
//   The set owns its entries (and their names). The pointer from find() and the
//   reference from all() are valid only until the next add() on that set, or until
//   it is moved from or destroyed: do not retain them. A copy is an independent
//   snapshot: changing either never changes the other, and a set never refers into
//   another object or into its source. Core keeps no copy of a provider's set.
//
// VERSION SEMANTICS (CORE-CAP-005)
//   Capability::version is the Version of the capability CONTRACT being provided:
//   which revision of the contract (the meaning of the identity) the provider
//   claims to implement. It is NOT runtime state, availability, health, a
//   configuration revision or ConfigurationVersion (the type is shared, the
//   meanings are not interchangeable), a build, package or firmware version of the
//   provider or of Core, an update counter, an ordering or dependency value, or
//   authorization or security evidence. Core never compares, orders, ranges,
//   defaults or interprets a capability version: Version has no ordering, matching
//   ignores it (identity-only matching), re-adding an id with a different version simply
//   replaces the old one (no maximum, minimum or merge), and no version-range
//   matching exists. Whether a provided version is acceptable to a consumer is the
//   consumer's or integrator's policy.
//
// THREADS, ALLOCATION, REAL TIME
//   Concurrent const use is safe; any concurrent mutation needs external
//   synchronization (there is no lock). add() and copying allocate (vector growth,
//   name copies) and may throw std::bad_alloc; lookups are O(n), allocation-free
//   and noexcept. All of it is control-plane: Core makes no real-time claim.
//
// EXCLUDED
//   No registry, locator, broker or global store, no dynamic discovery or change
//   events, no version-range matching and no capability credentials.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// CAPABILITY PROVISION, REQUIREMENT AND MATCHING BOUNDARY (CORE-CAP-007, CORE-CAP-008)
//
// This block states how a capability PROVIDED by an entity relates to a capability
// REQUIRED by a consumer. It adds no type: the existing mechanisms are used
// (CapabilitySet above, platform::PlatformRequirements / CapabilityRequirement /
// evaluate() / check_required() in platform/requirements.hpp, and the same model
// through runtime::ComponentContext). Those headers are unchanged.
//
// PROVISION IS NOT REQUIREMENT
//   Provision is what a provider DECLARES (a CapabilitySet snapshot). A
//   requirement is what a consumer NEEDS: an explicit, declarative item (a
//   CapabilityId and REQUIRED or OPTIONAL) in a PlatformRequirements the consumer
//   or integrator builds. The two are related ONLY by an explicit check that the
//   consumer or integrator makes. Core never derives a requirement from a
//   provision, a provision from a requirement, or either from a Component's type,
//   name or behavior. Evaluating a requirement is the consumer's or integrator's
//   act: the Runtime does not evaluate, resolve, bind, inject or satisfy any
//   capability requirement, ever, as part of any lifecycle operation.
//
// MATCHING IS IDENTITY-ONLY (CORE-CAP-008)
//   A requirement for CapabilityId X is satisfied exactly when the provider's
//   snapshot contains an entry with that CapabilityId (CapabilitySet::contains).
//   NOTHING ELSE is consulted: not the capability's name, not its version (no
//   version equality, range or compatibility test), not the provider's name,
//   version, vendor or kind (PlatformInfo), not the platform type, operating
//   system or hardware, not any spelling lookalike, and no inference of any kind.
//   An entry with an invalid id can never satisfy a requirement because a
//   requirement cannot name an invalid id (it is rejected when declared).
//   Whether a provided capability VERSION is acceptable is policy of the consumer
//   or integrator, applied by them after (and outside) the identity match.
//
// EVALUATION IS EXPLICIT, SIDE-EFFECT FREE AND DETERMINISTIC
//   evaluate() and check_required() only read: they take at most ONE capability
//   snapshot from the provider per evaluation, start, stop, configure, create or
//   change nothing, never touch the requirements, the context, the adapter or the
//   Runtime, and the same requirements against the same provision give the same
//   report. Declaring a requirement is atomic (a rejected declaration changes
//   nothing); a duplicate is decided by identity, not by (identity, level).
//   REQUIRED items decide success; OPTIONAL items are reported but never decide.
//
// CAPABILITY REQUIREMENTS ARE NOT COMPONENT DEPENDENCIES
//   runtime::DependencyGraph orders ComponentIds only and knows nothing about
//   capabilities: a capability being provided or required never adds, removes or
//   reorders a dependency edge, never changes the dependency order or the
//   registration of a Component, and a Component that happens to provide what
//   another requires is not thereby ordered before it. Ordering is declared by the
//   integrator with add_dependency(); requirements are checked by the integrator.
//   Both stay independent of lifecycle, readiness and health: whether a missing
//   prerequisite makes a Component's own lifecycle operation fail is decided by
//   that Component or the integrator, never by an automatic Core rule.
//------------------------------------------------------------------------------
class CapabilitySet {
public:
    void add(Capability capability);
    [[nodiscard]] bool contains(CapabilityId id) const noexcept;
    [[nodiscard]] const Capability* find(CapabilityId id) const noexcept;
    [[nodiscard]] const std::vector<Capability>& all() const noexcept { return capabilities_; }
    [[nodiscard]] bool empty() const noexcept { return capabilities_.empty(); }
    [[nodiscard]] std::size_t size() const noexcept { return capabilities_.size(); }
private:
    std::vector<Capability> capabilities_;
};
} // namespace kritva::core
