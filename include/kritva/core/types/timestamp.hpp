//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : timestamp.hpp
// Description : Monotonic timestamp type for deterministic Core timing.
//
// Component   : Kritva Core
// Module      : Types
// Layer       : Core Foundation
//
// Requirements: CORE-TYPE-003
// API         : CORE-API-TIMESTAMP
//
// Author      : KritvaOS Core Team
// Created     : 2026-09-14
//==============================================================================

#pragma once

#include <chrono>
#include <cstdint>

namespace kritva::core {

/// @brief Nanosecond-resolution monotonic timestamp.
class Timestamp {
public:
    using clock = std::chrono::steady_clock;
    using duration = std::chrono::nanoseconds;
    using value_type = std::int64_t;

    constexpr Timestamp() noexcept = default;
    explicit constexpr Timestamp(value_type nanoseconds) noexcept : value_(nanoseconds) {}

    [[nodiscard]] constexpr value_type nanoseconds() const noexcept { return value_; }

    static Timestamp now() noexcept;

    friend constexpr bool operator==(Timestamp, Timestamp) noexcept = default;
    friend constexpr auto operator<=>(Timestamp, Timestamp) noexcept = default;

private:
    value_type value_{0};
};

} // namespace kritva::core
