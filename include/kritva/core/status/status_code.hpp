//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : status_code.hpp
// Description : Operational status classification.
//
// Component   : Kritva Core
// Module      : Status
// Layer       : Core Foundation
//
// Requirements: CORE-STA-001
// API         : CORE-API-STATUS
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include <cstdint>
namespace kritva::core {
   enum class StatusCode : std::uint8_t {
       UNKNOWN,
       OK,
       INVALID_ARGUMENT,
       NOT_READY,
       BUSY,
       TIMEOUT,
       FAILED,
       UNAVAILABLE,
       UNSUPPORTED,
       INTERNAL_ERROR
   };

} // namespace kritva::core
