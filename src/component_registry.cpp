//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component_registry.cpp
// Description : ComponentRegistry implementation.
//
// Component   : Kritva Core
// Module      : Runtime
// Layer       : Core Foundation
//
// Requirements: CORE-RT-003
// API         : CORE-API-RUNTIME
//
// Author      : KritvaOS Core Team
// Created     : 03-10-2026
//==============================================================================

#include <kritva/core/runtime/component_registry.hpp>

#include <string>
#include <utility>

namespace kritva::core::runtime {

Result<void> ComponentRegistry::register_component(Component& component) {
    const ComponentId id = component.info().id();
    // try_emplace leaves the map untouched if the key exists or allocation fails.
    const auto [position, inserted] = components_.try_emplace(id, &component);
    if (!inserted) {
        return Result<void>::failure(Error{
            ErrorCode::INVALID_ARGUMENT, ErrorSeverity::ERROR, id, {},
            "component id " + std::to_string(id.value()) + " is already registered"});
    }
    (void)position;
    return Result<void>::success();
}

Component* ComponentRegistry::find(ComponentId id) const noexcept {
    const auto it = components_.find(id);
    return it == components_.end() ? nullptr : it->second;
}

bool ComponentRegistry::contains(ComponentId id) const noexcept {
    return components_.find(id) != components_.end();
}

std::vector<Component*> ComponentRegistry::components() const {
    std::vector<Component*> snapshot;
    snapshot.reserve(components_.size());
    for (const auto& entry : components_) {   // std::map iterates in ascending key order
        snapshot.push_back(entry.second);
    }
    return snapshot;
}

} // namespace kritva::core::runtime
