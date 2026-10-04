//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : capability.hpp
// Description : Capability metadata.
//
// Component   : Kritva Core
// Module      : Capability
// Layer       : Core Foundation
//
// Requirements: CORE-CAP-002; CORE-CAP-004
// API         : CORE-API-CAPABILITY
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include "capability_id.hpp"
#include "../types/version.hpp"
#include <string>
namespace kritva::core {

//------------------------------------------------------------------------------
// CAPABILITY CONTRACT: IDENTITY AND PROVIDER SEMANTICS (CORE-CAP-004)
//
// WHAT A CAPABILITY IS
//   A Capability is a plain, copyable VALUE that DESCRIBES functionality an entity
//   reports as providing: a CapabilityId (identity), a human-readable name and the
//   Version of the capability contract being provided. It is descriptive metadata
//   about a contract. It is NOT an authentication credential, an authorization
//   token, proof of trust, a lifecycle state, an availability or readiness flag or
//   a health signal, and carrying one grants nobody any authority.
//
// IDENTITY IS AUTHORITATIVE
//   CapabilityId (capability_id.hpp, an alias of Id) is the ONLY identity of a
//   capability: it is valid iff non-zero, and two capabilities denote the same
//   capability exactly when their CapabilityIds are equal. Everything else is
//   metadata:
//     - name is for humans (logs, diagnostics, documentation). It is not unique,
//       may be empty, may repeat across different ids, is never parsed, compared
//       for matching or used to derive an id, and Core assigns no meaning to its
//       spelling (a dotted or vendor-looking name is just text).
//     - version describes the provided capability contract (see
//       the CapabilitySet contract); it is not runtime state, availability,
//       health, a configuration revision, an ordering value or evidence.
//   Core defines no numeric ranges, namespaces, registries or vendor encodings for
//   CapabilityId; who allocates ids is the integrator's concern. The identity
//   implies nothing about a platform, vendor, operating system or hardware.
//
// PROVIDER SEMANTICS
//   An entity (an integrator-written Component through Component::capabilities(),
//   a platform adapter through IPlatformAdapter::capabilities(), or any
//   integrator code) PROVIDES capabilities by returning a CapabilitySet BY VALUE:
//   a snapshot of its declaration at that moment.
//     - The declaration is the provider's CLAIM. Core does not verify, probe,
//       grant, revoke, negotiate, cache, track or notify about it, and a snapshot
//       says nothing about whether the functionality currently works.
//     - A conforming provider publishes only valid CapabilityIds. (The container
//       can physically hold an entry whose id is invalid, because
//       CapabilitySet::add() accepts the value unchanged; such an entry is
//       storable data, not an authoritative capability declaration, and it can
//       never satisfy a requirement, because requirements cannot name an invalid
//       id.)
//     - PROVISION is distinct from REQUIREMENT: what a provider declares and what
//       a consumer needs are separate things, related only by an explicit,
//       identity-based check made by the consumer or integrator. Core never
//       resolves, binds, injects or orders anything because a capability exists.
//
// OWNERSHIP, THREADS, REAL TIME
//   A Capability owns its name and refers to nothing: copies are independent, no
//   pointer or reference into one is retained by Core. It is a plain value type:
//   constructing, copying and moving never block and use no thread; copying a name
//   may allocate. Concurrent const use is safe, concurrent mutation needs external
//   synchronization. No real-time claim is made for operations that copy names.
//
// EXCLUDED
//   No capability credentials or tokens, authentication or authorization, dynamic
//   discovery or change notification, service registry or locator, automatic
//   dependency resolution or readiness calculation, and no platform-, vendor- or
//   hardware-specific capability definitions.
//------------------------------------------------------------------------------
struct Capability {
    CapabilityId id{};
    std::string name;
    Version version{};
};
} // namespace kritva::core
