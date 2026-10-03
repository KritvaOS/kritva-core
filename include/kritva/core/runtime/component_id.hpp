//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component_id.hpp
// Description : Runtime component identifier.
//
// Component   : Kritva Core
// Module      : Runtime
// Layer       : Core Foundation
//
// Requirements: CORE-RT-001
// API         : CORE-API-RUNTIME
//
// Author      : KritvaOS Core Team
// Created     : 03-10-2026
//==============================================================================

#pragma once
#include "../types/id.hpp"
namespace kritva::core::runtime {

//------------------------------------------------------------------------------
// ComponentId (CORE-RT-001)
//
// Stable identity of a runtime component: the Core 64-bit Id (types/id.hpp).
//   - Valid iff value() != 0; 0 is the invalid / "no component" identity and
//     never identifies a component.
//   - Identity is the value only. Comparison (==, <=>) and hashing are
//     deterministic and total; the order has no meaning beyond being a stable
//     tie-break.
//   - Identity is assigned by the integrator and is platform independent: it
//     is never derived from a pointer, thread id, process id, OS handle or
//     other platform-specific value.
//   - Uniqueness within one runtime is enforced by the component registry
//     (KF-CORE-R03-002), not by this type.
// Like CapabilityId, this is an alias of Id, so it does not by itself prevent
// mixing with other Id-based identities.
//------------------------------------------------------------------------------
using ComponentId = kritva::core::Id;

} // namespace kritva::core::runtime
