//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : capability_set.cpp
// Description : Capability collection implementation.
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


#include <kritva/core/capability/capability_set.hpp>
namespace kritva::core {
void CapabilitySet::add(Capability capability) { 
    if (const auto* existing = find(capability.id); existing != nullptr) {
        // Replace existing capability with the same identity.
        for (auto& item : capabilities_) if (item.id == capability.id) { item = std::move(capability); return; }
    }
    capabilities_.push_back(std::move(capability));
}
bool CapabilitySet::contains(CapabilityId id) const noexcept { return find(id) != nullptr; }
const Capability* CapabilitySet::find(CapabilityId id) const noexcept {
    for (const auto& item : capabilities_) if (item.id == id) return &item;
    return nullptr;
}
} // namespace kritva::core
