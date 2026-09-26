//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : metadata.hpp
// Description : String metadata container.
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


#pragma once
#include <string>
#include <string_view>
#include <unordered_map>

namespace kritva::core {
class Metadata {
public:
    using map_type = std::unordered_map<std::string, std::string>;
    void set(std::string key, std::string value);
    [[nodiscard]] const std::string* get(std::string_view key) const noexcept;
    [[nodiscard]] bool contains(std::string_view key) const noexcept;
    [[nodiscard]] bool empty() const noexcept { return values_.empty(); }
    [[nodiscard]] std::size_t size() const noexcept { return values_.size(); }
private:
    map_type values_;
};
} // namespace kritva::core
