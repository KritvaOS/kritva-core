//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : health_state.hpp
// Description : Health classification independent from operational status.
//
// Component   : Kritva Core
// Module      : Health
// Layer       : Core Foundation
//
// Requirements: CORE-HLT-001
// API         : CORE-API-HEALTH
//
// Author      : KritvaOS Core Team
// Created     : 2026-09-14
//==============================================================================

#pragma once

namespace kritva::core {

/// @brief Health classification; a component may be RUNNING while DEGRADED.
enum class HealthState {
    UNKNOWN,
    HEALTHY,
    DEGRADED,
    UNHEALTHY
};

} // namespace kritva::core
