//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : metadata.cpp
// Description : Metadata implementation.
//
// Component   : Kritva Core
// Module      : Types
// Layer       : Core Foundation
//
// Requirements: CORE-TYP-004
// API         : CORE-API-METADATA
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#include <kritva/core/types/metadata.hpp>
namespace kritva::core {
void Metadata::set(std::string key, std::string value) { values_[std::move(key)] = std::move(value); }
const std::string* Metadata::get(std::string_view key) const noexcept {
    auto it = values_.find(std::string(key));
    return it == values_.end() ? nullptr : &it->second;
}
bool Metadata::contains(std::string_view key) const noexcept { return get(key) != nullptr; }
} // namespace kritva::core
