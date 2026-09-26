//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : timestamp.hpp
// Description : Clock-domain-qualified timestamp value.
//
// Component   : Kritva Core
// Module      : Types
// Layer       : Core Foundation
//
// Requirements: CORE-TIME-001
// API         : CORE-API-TIME
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================

#pragma once

#include <cstdint>

namespace kritva::core {

enum class ClockDomain : std::uint8_t {
    MONOTONIC = 0,
    REALTIME = 1,
};

class Timestamp {
public:
    constexpr Timestamp() noexcept = default;
    constexpr explicit Timestamp(std::int64_t nanoseconds,
                                 ClockDomain domain = ClockDomain::MONOTONIC) noexcept
        : nanoseconds_(nanoseconds), domain_(domain) {}

    [[nodiscard]] constexpr std::int64_t nanoseconds() const noexcept {
        return nanoseconds_;
    }

    [[nodiscard]] constexpr ClockDomain domain() const noexcept {
        return domain_;
    }

    friend constexpr bool operator==(Timestamp lhs, Timestamp rhs) noexcept {
        return lhs.nanoseconds_ == rhs.nanoseconds_ && lhs.domain_ == rhs.domain_;
    }

    friend constexpr bool operator!=(Timestamp lhs, Timestamp rhs) noexcept {
        return !(lhs == rhs);
    }

private:
    std::int64_t nanoseconds_{0};
    ClockDomain domain_{ClockDomain::MONOTONIC};
};

} // namespace kritva::core
