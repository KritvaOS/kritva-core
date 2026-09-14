//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : error_code.hpp
// Description : Standard error classification for Kritva Core operations.
//
// Component   : Kritva Core
// Module      : Error
// Layer       : Core Foundation
//
// Requirements: CORE-ERR-001
// API         : CORE-API-ERROR
//
// Author      : KritvaOS Core Team
// Created     : 2026-09-14
//==============================================================================

#pragma once

#include <cstdint>

namespace kritva::core {

/// @brief Severity of an operational error.
enum class ErrorSeverity {
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

/// @brief Standard Core error codes.
enum class ErrorCode : std::uint32_t {
    NONE = 0,
    UNKNOWN,
    INVALID_ARGUMENT,
    NOT_INITIALIZED,
    NOT_READY,
    ALREADY_RUNNING,
    TIMEOUT,
    RESOURCE_UNAVAILABLE,
    CONFIGURATION_ERROR,
    INTERNAL_ERROR
};

} // namespace kritva::core
