//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : error.hpp
// Description : Structured operational error with source and timing context.
//
// Component   : Kritva Core
// Module      : Error
// Layer       : Core Foundation
//
// Requirements: CORE-ERR-002
// API         : CORE-API-ERROR
//
// Author      : KritvaOS Core Team
// Created     : 2026-09-14
//==============================================================================

#pragma once

#include "error_code.hpp"
#include "../types/id.hpp"
#include "../types/timestamp.hpp"

#include <string>

namespace kritva::core {

/// @brief Structured description of an operational failure.
struct Error {
    ErrorCode code{ErrorCode::NONE};
    ErrorSeverity severity{ErrorSeverity::ERROR};
    Id source{};
    Timestamp timestamp{};
    std::string message;
};

} // namespace kritva::core
