//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : counter.hpp
// Description : Monotonic counter primitive for Core statistics.
//
// Component   : Kritva Core
// Module      : Statistics
// Layer       : Core Foundation
//
// Requirements: CORE-STAT-001
// API         : CORE-API-STATISTICS
//
// Author      : KritvaOS Core Team
// Created     : 2026-09-14
//==============================================================================

#pragma once

#include <cstdint>

namespace kritva::core {

/// @brief Unsigned monotonic counter.
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
