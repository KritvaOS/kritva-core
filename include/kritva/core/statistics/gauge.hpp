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
class Gauge {
public:
    using value_type = std::int64_t;
    constexpr void set(value_type value) noexcept { value_ = value; }
    [[nodiscard]] constexpr value_type value() const noexcept { return value_; }
private:
    value_type value_{0};
};
} // namespace kritva::core
