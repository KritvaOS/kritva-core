//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : gauge.hpp
// Description : Instantaneous signed measurement.
//
// Component   : Kritva Core
// Module      : Statistics
// Layer       : Core Foundation
//
// Requirements: CORE-STS-002
// API         : CORE-API-STATISTICS
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include <cstdint>
namespace kritva::core {

//------------------------------------------------------------------------------
// Gauge (CORE-STS-002)
//
// Signed instantaneous value, 0 after construction. The unit is defined by
// whoever owns the Gauge (see Statistics for the Core-defined fields).
//
// Contract:
//   - set(v) replaces the previous value with v; value() returns the last v.
//   - Any std::int64_t is accepted and stored exactly. No clamping, range
//     check, or arithmetic is performed, so there is no overflow behavior.
//   - Behavior is fully deterministic.
//
// Real-time notes:
//   - Allocation: none. Blocking / synchronization: none. Complexity: O(1).
//   - Thread-safety: NOT thread-safe; same rules as Counter. A Gauge is a
//     plain integer, not an atomic and not a synchronization primitive.
//   - Not a hard-real-time guarantee.
//   - Exceptions: none (all operations noexcept).
//------------------------------------------------------------------------------
class Gauge {
public:
    using value_type = std::int64_t;
    constexpr void set(value_type value) noexcept { value_ = value; }
    [[nodiscard]] constexpr value_type value() const noexcept { return value_; }
private:
    value_type value_{0};
};
} // namespace kritva::core
