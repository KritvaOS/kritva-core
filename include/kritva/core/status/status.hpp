//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : status.hpp
// Description : Operational status value.
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
#include "status_code.hpp"
#include <string>
#include <utility>
namespace kritva::core {

//------------------------------------------------------------------------------
// Status (CORE-STA-001)
//
// A StatusCode plus an optional human-readable message.
//
// Construction:
//   - Status()                  -> code UNKNOWN, empty message.
//   - explicit Status(code)     -> given code, empty message.
//   Use set_message() to attach detail. Status is copyable and movable.
//
// Mutability:
//   - set_code() and set_message() mutate the object in place. The two fields
//     are independent: neither call validates or resets the other, and an empty
//     message is valid for any code (OK does not imply an empty message).
//   - message() returns a reference valid until the next set_message() call or
//     destruction/move of this Status.
//
// Real-time notes:
//   - Allocation: Status(), Status(code), code(), set_code() never allocate.
//     set_message(), copy construction/assignment may allocate (std::string).
//     Treat those as control-plane operations.
//   - Blocking / synchronization: none.
//   - Thread-safety: not thread-safe. Concurrent const access is safe;
//     any concurrent mutation needs external synchronization.
//   - Complexity: O(1) except message copy/move, which is O(message length)
//     for copy and O(1) for move.
//   - Exceptions: only set_message() and copies can throw (std::bad_alloc).
//------------------------------------------------------------------------------
class Status {
public:
    Status() noexcept = default;

    explicit Status(StatusCode code) noexcept
        : code_(code) {}

    [[nodiscard]] constexpr StatusCode code() const noexcept {
        return code_;
    }

    [[nodiscard]] const std::string& message() const noexcept {
        return message_;
    }

    void set_code(StatusCode code) noexcept {
        code_ = code;
    }

    void set_message(std::string message) {
        message_ = std::move(message);
    }

private:
    StatusCode code_{StatusCode::UNKNOWN};
    std::string message_;
};
} // namespace kritva::core
