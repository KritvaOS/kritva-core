//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : capability_id.hpp
// Description : Identifier for a capability exposed by a Core-managed component.
//
// Component   : Kritva Core
// Module      : Capability
// Layer       : Core Foundation
//
// Requirements: CORE-CAP-001
// API         : CORE-API-CAPABILITY
//
// Author      : KritvaOS Core Team
// Created     : 2026-09-14
//==============================================================================

#pragma once

#include "../types/id.hpp"

namespace kritva::core {

/// @brief Capability identifier alias using the Core Id primitive.
using CapabilityId = Id;

} // namespace kritva::core
