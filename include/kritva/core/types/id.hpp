//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : id.hpp
// Description : Strongly typed identifier.
//
// Component   : Kritva Core
// Module      : Types
// Layer       : Core Foundation
//
// Requirements: CORE-TYP-001
// API         : CORE-API-ID
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include <compare>
#include <cstdint>
#include <functional>

namespace kritva::core {
class Id {
public:
    using value_type = std::uint64_t;
    constexpr Id() noexcept = default;
    explicit constexpr Id(value_type value) noexcept : value_(value) {}
    [[nodiscard]] constexpr value_type value() const noexcept { return value_; }
    [[nodiscard]] constexpr bool valid() const noexcept { return value_ != 0; }
    friend constexpr bool operator==(Id, Id) noexcept = default;
    friend constexpr auto operator<=>(Id, Id) noexcept = default;
private:
    value_type value_{0};
};
} // namespace kritva::core

template<> struct std::hash<kritva::core::Id> {
    std::size_t operator()(kritva::core::Id id) const noexcept {
        return std::hash<kritva::core::Id::value_type>{}(id.value());
    }
};
