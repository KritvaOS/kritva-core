//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : capability_set.hpp
// Description : Capability collection with lookup.
//
// Component   : Kritva Core
// Module      : Capability
// Layer       : Core Foundation
//
// Requirements: CORE-CAP-003
// API         : CORE-API-CAPABILITY
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include "capability.hpp"
#include <cstddef>
#include <vector>
namespace kritva::core {
class CapabilitySet {
public:
    void add(Capability capability);
    [[nodiscard]] bool contains(CapabilityId id) const noexcept;
    [[nodiscard]] const Capability* find(CapabilityId id) const noexcept;
    [[nodiscard]] const std::vector<Capability>& all() const noexcept { return capabilities_; }
    [[nodiscard]] bool empty() const noexcept { return capabilities_.empty(); }
    [[nodiscard]] std::size_t size() const noexcept { return capabilities_.size(); }
private:
    std::vector<Capability> capabilities_;
};
} // namespace kritva::core
