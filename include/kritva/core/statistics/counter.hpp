//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : counter.hpp
// Description : Monotonic counter primitive.
//
// Component   : Kritva Core
// Module      : Statistics
// Layer       : Core Foundation
//
// Requirements: CORE-STS-001
// API         : CORE-API-STATISTICS
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include <cstdint>
namespace kritva::core {

//------------------------------------------------------------------------------
// Counter (CORE-STS-001)
//
// Monotonically increasing unsigned event count, 0 after construction.
//
// Contract:
//   - increment(n) adds n; increment() adds 1; increment(0) is a no-op.
//   - Overflow is defined: the value wraps modulo 2^64 (2^64 - 1 + 1 == 0).
//     Consumers that need rates must compute deltas with unsigned arithmetic.
//   - reset() sets the value to 0; it is the only operation that decreases it.
//   - Behavior is fully deterministic: no clock, no randomness, no I/O.
//
// Real-time notes:
//   - Allocation: none. Blocking / synchronization: none. Complexity: O(1).
//   - Thread-safety: NOT thread-safe. It is a plain integer, not an atomic and
//     not a synchronization primitive. Concurrent increment()/reset() or a
//     concurrent read during a write is a data race (undefined behavior)
//     unless the caller provides external synchronization, or each Counter has
//     a single writer and readers are synchronized with it.
//   - Not a hard-real-time guarantee: there is no bounded-latency or lock-free
//     claim beyond "a few instructions, no calls".
//   - Exceptions: none (all operations noexcept).
//------------------------------------------------------------------------------
class Counter {
public:
    using value_type = std::uint64_t;
    constexpr void increment(value_type amount = 1) noexcept { value_ += amount; }
    constexpr void reset() noexcept { value_ = 0; }
    [[nodiscard]] constexpr value_type value() const noexcept { return value_; }
private:
    value_type value_{0};
};
} // namespace kritva::core
