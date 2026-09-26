//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : error_code.hpp
// Description : Standard error codes and severity.
//
// Component   : Kritva Core
// Module      : Error
// Layer       : Core Foundation
//
// Requirements: CORE-ERR-001
// API         : CORE-API-ERROR
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include <cstdint>
namespace kritva::core {
enum class ErrorSeverity : std::uint8_t { INFO, WARNING, ERROR, CRITICAL };
enum class ErrorCode : std::uint32_t {
    NONE = 0, UNKNOWN, INVALID_ARGUMENT, INVALID_STATE, NOT_INITIALIZED,
    NOT_READY, ALREADY_RUNNING, TIMEOUT, RESOURCE_UNAVAILABLE,
    CONFIGURATION_ERROR, UNSUPPORTED, INTERNAL_ERROR
};
} // namespace kritva::core
