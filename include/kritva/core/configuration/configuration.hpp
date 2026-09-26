//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : configuration.hpp
// Description : In-memory configuration container with basic validation.
//
// Component   : Kritva Core
// Module      : Configuration
// Layer       : Core Foundation
//
// Requirements: CORE-CFG-001; CORE-CFG-002
// API         : CORE-API-CONFIGURATION
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include "parameter.hpp"
#include "../error/result.hpp"
#include <cstddef>
#include <string>
#include <unordered_map>
namespace kritva::core {
class Configuration {
public:
    [[nodiscard]] Result<void> validate() const;
    Result<void> set(Parameter parameter);
    [[nodiscard]] const Parameter* get(const std::string& name) const noexcept;
    [[nodiscard]] bool contains(const std::string& name) const noexcept;
    [[nodiscard]] std::size_t size() const noexcept { return parameters_.size(); }
private:
    std::unordered_map<std::string, Parameter> parameters_;
};
} // namespace kritva::core
