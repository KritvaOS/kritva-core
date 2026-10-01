//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : clock.hpp
// Description : Platform adapter contract for the Core time service.
//
// Component   : Kritva Core
// Module      : Platform Contract
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-002
// API         : CORE-API-PLATFORM
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================

#pragma once

#include "../time/clock.hpp"

namespace kritva::core::platform {

/// Compatibility/adapter alias. Platform implementations may implement
/// kritva::core::time::IClock directly.
// DEPRECATED compatibility alias. The canonical Core clock abstraction is
// kritva::core::time::IClock (time/clock.hpp); this is NOT a second abstraction.
// New code should use time::IClock. Planned removal: R0.3 or later.
using IClock = kritva::core::time::IClock;

} // namespace kritva::core::platform
