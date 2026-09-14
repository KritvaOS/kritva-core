//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : parameter.hpp
// Description : Typed configuration parameter metadata and value abstraction.
//
// Component   : Kritva Core
// Module      : Configuration
// Layer       : Core Foundation
//
// Requirements: CORE-CFG-001
// API         : CORE-API-CONFIGURATION
//
// Author      : KritvaOS Core Team
// Created     : 2026-09-14
//==============================================================================

#pragma once

#include <cstdint>
#include <string>
#include <variant>

namespace kritva::core {

/// @brief Initial scalar configuration value types supported by Core.
using ParameterValue = std::variant<bool, std::int64_t, double, std::string>;

/// @brief Describes one configurable parameter.
struct Parameter {
    std::string name;
    ParameterValue value;
    std::string description;
};

} // namespace kritva::core
