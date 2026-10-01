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

// Monotonic unsigned event counter (R0.2 contract).
//   - Thread-safety: NOT thread-safe. Concurrent access requires external
//     synchronization. Not an atomic and not a real-time synchronization
//     primitive; atomic/lock-free variants may be added in a later release.
//   - Allocation: none. Blocking: none. Complexity: O(1).
//   - Overflow: increment() wraps modulo 2^64 (unsigned arithmetic).
//   - Failure behavior: none; all operations are noexcept.
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
