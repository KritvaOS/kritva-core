//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : metadata.hpp
// Description : Lightweight metadata container for Core objects and events.
//
// Component   : Kritva Core
// Module      : Types
// Layer       : Core Foundation
//
// Requirements: CORE-TYPE-005
// API         : CORE-API-METADATA
//
// Author      : KritvaOS Core Team
// Created     : 2026-09-14
//==============================================================================

#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

namespace kritva::core {

/// @brief String key/value metadata associated with Core objects.
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
