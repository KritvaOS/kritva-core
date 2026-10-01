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
