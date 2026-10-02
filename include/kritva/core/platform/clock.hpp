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

// Compatibility alias / migration path (CORE-PLAT-002).
//
// The canonical Core clock abstraction is kritva::core::time::IClock
// (time/clock.hpp). platform::IClock is the very same type, not a second or
// independent contract: it adds no members, no semantics, and cannot diverge.
// Platform adapters should implement kritva::core::time::IClock directly; new
// code should include "time/clock.hpp". The alias exists only so that code
// written against the R0.1 platform namespace keeps compiling. No removal date
// is set; removing it is a public API compatibility change requiring human
// review.
using IClock = kritva::core::time::IClock;

} // namespace kritva::core::platform
