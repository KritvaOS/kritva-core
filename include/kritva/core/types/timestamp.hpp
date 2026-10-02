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

//------------------------------------------------------------------------------
// ClockDomain (CORE-TIME-001)
//
// Identifies the time base of a Timestamp. See time::IClock for the full
// semantics.
//   MONOTONIC - non-decreasing, unaffected by wall-clock adjustment, unspecified
//               epoch; only differences are meaningful.
//   REALTIME  - wall-clock time; may jump backward or forward.
// Timestamps of different domains are incomparable.
//------------------------------------------------------------------------------
enum class ClockDomain : std::uint8_t {
    MONOTONIC = 0,
    REALTIME = 1,
};

//------------------------------------------------------------------------------
// Timestamp (CORE-TIME-001)
//
// A point on a clock: signed nanoseconds plus the ClockDomain they belong to.
// Default value is 0 ns in the MONOTONIC domain. Plain value type (trivially
// copyable, no allocation, thread-safe).
//
// Equality compares nanoseconds AND domain, so timestamps from different
// domains are never equal even when their nanosecond counts match. This is
// not an ordering: Timestamp intentionally has no operator<, no <=>, and no
// subtraction, so a monotonic and a realtime value cannot be ordered or
// subtracted by accident. Callers doing arithmetic on nanoseconds() must check
// domain() first.
//------------------------------------------------------------------------------
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
