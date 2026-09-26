//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : error.hpp
// Description : Structured operational error.
//
// Component   : Kritva Core
// Module      : Error
// Layer       : Core Foundation
//
// Requirements: CORE-ERR-002
// API         : CORE-API-ERROR
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include "error_code.hpp"
#include "../types/id.hpp"
#include "../types/timestamp.hpp"
#include <string>
namespace kritva::core {
struct Error {
    ErrorCode code{ErrorCode::NONE};
    ErrorSeverity severity{ErrorSeverity::ERROR};
    Id source{};
    Timestamp timestamp{};
    std::string message;
};
} // namespace kritva::core
