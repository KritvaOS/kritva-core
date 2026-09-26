//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : configuration.cpp
// Description : Configuration implementation.
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


#include <kritva/core/configuration/configuration.hpp>
namespace kritva::core {
Result<void> Configuration::set(Parameter parameter) {
    if (parameter.name.empty()) {
        return Result<void>::failure(Error{
            ErrorCode::INVALID_ARGUMENT, ErrorSeverity::ERROR, {}, {}, "Parameter name is empty"
        });
    }
    parameters_[parameter.name] = std::move(parameter);
    return Result<void>::success();
}
const Parameter* Configuration::get(const std::string& name) const noexcept {
    auto it = parameters_.find(name);
    return it == parameters_.end() ? nullptr : &it->second;
}
bool Configuration::contains(const std::string& name) const noexcept { return get(name) != nullptr; }

Result<void> Configuration::validate() const {
    for (const auto& parameter : parameters_) {
        if (parameter.second.name.empty()) {
            return Result<void>::failure(Error{
                ErrorCode::INVALID_ARGUMENT,
                ErrorSeverity::ERROR,
                {},
                {},
                "configuration parameter name must not be empty"
            });
        }
    }
    return Result<void>::success();
}
} // namespace kritva::core
