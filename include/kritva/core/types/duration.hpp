//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : duration.hpp
// Description : Duration type used for Core timing and scheduling intervals.
//
// Component   : Kritva Core
// Module      : Types
// Layer       : Core Foundation
//
// Requirements: CORE-TYPE-004
// API         : CORE-API-DURATION
//
// Author      : KritvaOS Core Team
// Created     : 2026-09-14
//==============================================================================

#pragma once

#include <chrono>
#include <cstdint>

namespace kritva::core {

/// @brief Nanosecond-based duration used by Kritva Core.
class Duration {
public:
    using value_type = std::int64_t;

    constexpr Duration() noexcept = default;
    explicit constexpr Duration(value_type nanoseconds) noexcept : value_(nanoseconds) {}

    [[nodiscard]] constexpr value_type nanoseconds() const noexcept { return value_; }

    static constexpr Duration from_nanoseconds(value_type value) noexcept {
        return Duration{value};
    }

    static constexpr Duration from_microseconds(value_type value) noexcept {
        return Duration{value * 1'000};
    }

    static constexpr Duration from_milliseconds(value_type value) noexcept {
        return Duration{value * 1'000'000};
    }

    friend constexpr bool operator==(Duration, Duration) noexcept = default;
    friend constexpr auto operator<=>(Duration, Duration) noexcept = default;

private:
    value_type value_{0};
};

} // namespace kritva::core
