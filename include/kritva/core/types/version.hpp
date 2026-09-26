//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : version.hpp
// Description : Semantic version representation.
//
// Component   : Kritva Core
// Module      : Types
// Layer       : Core Foundation
//
// Requirements: CORE-TYP-002
// API         : CORE-API-VERSION
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include <cstdint>
#include <string>

namespace kritva::core {
struct Version {
    std::uint32_t major{0};
    std::uint32_t minor{0};
    std::uint32_t patch{0};
    [[nodiscard]] std::string to_string() const;
    friend constexpr bool operator==(const Version&, const Version&) noexcept = default;
};
} // namespace kritva::core
