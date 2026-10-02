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

//------------------------------------------------------------------------------
// StatusCode (CORE-STA-001)
//
// Coarse operational outcome classification carried by Status. This header is
// the authoritative status-code enumeration.
//   - UNKNOWN is the zero value: "no status has been determined".
//   - OK is the only success code; every other code reports a non-success state.
//   - Enumerators are not an extension point for Core users and their numeric
//     values must not be persisted or transmitted; use only the names.
//   - Plain value type, trivially copyable, no allocation, thread-safe.
//------------------------------------------------------------------------------
enum class StatusCode : std::uint8_t {
    UNKNOWN,           ///< Status not determined (default).
    OK,                ///< Success.
    INVALID_ARGUMENT,  ///< Caller supplied an invalid argument.
    NOT_READY,         ///< Not in a state to perform the operation yet.
    BUSY,              ///< Resource temporarily in use; retry may succeed.
    TIMEOUT,           ///< Operation did not complete in time.
    FAILED,            ///< Operation failed; no more specific code applies.
    UNAVAILABLE,       ///< Resource or service is not available.
    UNSUPPORTED,       ///< Operation is not supported.
    INTERNAL_ERROR     ///< Internal inconsistency; indicates a defect.
};

} // namespace kritva::core
