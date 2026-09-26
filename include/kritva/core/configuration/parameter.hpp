//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : parameter.hpp
// Description : Typed scalar configuration parameter.
//
// Component   : Kritva Core
// Module      : Configuration
// Layer       : Core Foundation
//
// Requirements: CORE-CFG-001
// API         : CORE-API-CONFIGURATION
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include <cstdint>
#include <string>
#include <variant>
namespace kritva::core {
using ParameterValue = std::variant<bool, std::int64_t, double, std::string>;
struct Parameter {
    std::string name;
    ParameterValue value;
    std::string description;
};
} // namespace kritva::core
