//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : capability_id.hpp
// Description : Capability identifier.
//
// Component   : Kritva Core
// Module      : Capability
// Layer       : Core Foundation
//
// Requirements: CORE-CAP-001; CORE-CAP-004
// API         : CORE-API-CAPABILITY
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include "../types/id.hpp"
namespace kritva::core {
// CapabilityId (CORE-CAP-001, CORE-CAP-004): the authoritative identity of a capability. An alias of Id, so it is valid
// iff non-zero and compares by value only. It is contract identity, not an authenticated identity, credential or
// authorization token, it is not derived from a name, and it implies nothing about a platform, vendor or hardware. See the
// capability contract in capability.hpp. Like every Id alias it does not by itself prevent mixing with other Id-based
// identities (ComponentId, for example): the type system does not distinguish them, the contract does.
using CapabilityId = Id;
} // namespace kritva::core
