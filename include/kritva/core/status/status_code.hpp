//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : status_code.hpp
// Description : Standard operational status codes for Kritva Core.
//
// Component   : Kritva Core
// Module      : Status
// Layer       : Core Foundation
//
// Requirements: CORE-STS-001
// API         : CORE-API-STATUS
//
// Author      : KritvaOS Core Team
// Created     : 2026-09-14
//==============================================================================

#pragma once

namespace kritva::core {

/// @brief Result-independent operational status classification.
enum class StatusCode {
    UNKNOWN,
    OK,
    INITIALIZING,
    READY,
    RUNNING,
    STOPPING,
    STOPPED,
    DEGRADED,
    FAULT
};

} // namespace kritva::core
