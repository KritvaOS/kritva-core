//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : health_state.hpp
// Description : Health classification independent from status.
//
// Component   : Kritva Core
// Module      : Health
// Layer       : Core Foundation
//
// Requirements: CORE-HEA-001
// API         : CORE-API-HEALTH
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once

#include <cstdint>

namespace kritva::core {
   enum class HealthState : std::uint8_t {
    UNKNOWN,
    HEALTHY,
    DEGRADED,
    UNHEALTHY
   };
} // namespace kritva::core
