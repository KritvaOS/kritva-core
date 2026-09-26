//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : lifecycle_state.hpp
// Description : Lifecycle state enumeration.
//
// Component   : Kritva Core
// Module      : Lifecycle
// Layer       : Core Foundation
//
// Requirements: CORE-LIF-001
// API         : CORE-API-LIFECYCLE
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================

#pragma once

#include <cstdint>

namespace kritva::core {
   enum class LifecycleState : std::uint8_t {
       UNKNOWN,
       INITIALIZING,
       READY,
       RUNNING,
       STOPPING,
       STOPPED,
       FAULT,
       RECOVERING
   };
} // namespace kritva::core
