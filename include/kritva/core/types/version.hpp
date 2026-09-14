//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : version.hpp
// Description : Semantic version representation for Kritva Core APIs and objects.
//
// Component   : Kritva Core
// Module      : Types
// Layer       : Core Foundation
//
// Requirements: CORE-TYPE-002
// API         : CORE-API-VERSION
//
// Author      : KritvaOS Core Team
// Created     : 2026-09-14
//==============================================================================

#pragma once

#include <cstdint>
#include <string>

namespace kritva::core {

/// @brief Represents a semantic version using major, minor, and patch components.
struct Version {
    std::uint32_t major{0};
    std::uint32_t minor{0};
    std::uint32_t patch{0};

    [[nodiscard]] std::string to_string() const;

    friend constexpr bool operator==(const Version&, const Version&) noexcept = default;
};

} // namespace kritva::core
