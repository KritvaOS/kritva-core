//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : configuration.hpp
// Description : In-memory collection of typed Core configuration parameters.
//
// Component   : Kritva Core
// Module      : Configuration
// Layer       : Core Foundation
//
// Requirements: CORE-CFG-002
// API         : CORE-API-CONFIGURATION
//
// Author      : KritvaOS Core Team
// Created     : 2026-09-14
//==============================================================================

#pragma once

#include "parameter.hpp"

#include <cstddef>
#include <string>
#include <unordered_map>

namespace kritva::core {

/// @brief Version-independent in-memory configuration container.
class Configuration {
public:
    void set(Parameter parameter);
    [[nodiscard]] const Parameter* get(const std::string& name) const noexcept;
    [[nodiscard]] bool contains(const std::string& name) const noexcept;
    [[nodiscard]] std::size_t size() const noexcept { return parameters_.size(); }

private:
    std::unordered_map<std::string, Parameter> parameters_;
};

} // namespace kritva::core
