//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : runtime_manager.cpp
// Description : RuntimeManager implementation.
//
// Component   : Kritva Core
// Module      : Runtime
// Layer       : Core Foundation
//
// Requirements: CORE-RT-006
// API         : CORE-API-RUNTIME
//
// Author      : KritvaOS Core Team
// Created     : 03-10-2026
//==============================================================================

#include <kritva/core/runtime/runtime_manager.hpp>

#include <cassert>
#include <string>

namespace kritva::core::runtime {

Result<void> RuntimeManager::invalid_state(const char* operation) const {
    return Result<void>::failure(Error{
        ErrorCode::INVALID_STATE, ErrorSeverity::ERROR, {}, {},
        std::string("runtime ") + operation + " is not valid in this lifecycle state"});
}

Result<void> RuntimeManager::setup_closed(const char* operation) const {
    return Result<void>::failure(Error{
        ErrorCode::INVALID_STATE, ErrorSeverity::ERROR, {}, {},
        std::string("runtime ") + operation + " is not allowed: the topology is fixed"});
}

void RuntimeManager::transition(LifecycleState target) {
    const auto r = lifecycle_.transition_to(target);   // reuse the Core transition table
    assert(r.has_value());
    (void)r;
}

Result<void> RuntimeManager::register_component(Component& component) {
    if (topology_fixed_) return setup_closed("register_component");
    return registry_.register_component(component);     // result returned unchanged
}

Result<void> RuntimeManager::add_dependency(ComponentId dependent, ComponentId dependency) {
    if (topology_fixed_) return setup_closed("add_dependency");
    return graph_.add_dependency(dependent, dependency);  // result returned unchanged
}

Result<std::vector<ComponentId>> RuntimeManager::component_order() const {
    return graph_.order(registry_);                       // the only ordering algorithm
}

Result<void> RuntimeManager::initialize() {
    const LifecycleState s = lifecycle_.state();
    if (s != LifecycleState::UNKNOWN && s != LifecycleState::STOPPED) return invalid_state("initialize");

    if (!topology_fixed_) {
        // Validate before anything changes: an invalid topology never reaches READY.
        auto order = component_order();
        if (!order.has_value()) return Result<void>::failure(order.error());   // error preserved
        topology_fixed_ = true;
    }
    transition(LifecycleState::INITIALIZING);
    transition(LifecycleState::READY);
    return Result<void>::success();
}

Result<void> RuntimeManager::start() {
    if (lifecycle_.state() != LifecycleState::READY) return invalid_state("start");
    transition(LifecycleState::RUNNING);
    return Result<void>::success();
}

Result<void> RuntimeManager::stop() {
    const LifecycleState s = lifecycle_.state();
    if (s != LifecycleState::READY && s != LifecycleState::RUNNING) return invalid_state("stop");
    if (s == LifecycleState::RUNNING) transition(LifecycleState::STOPPING);
    transition(LifecycleState::STOPPED);
    return Result<void>::success();
}

Result<void> RuntimeManager::shutdown() {
    const LifecycleState s = lifecycle_.state();
    if (s != LifecycleState::UNKNOWN && s != LifecycleState::STOPPED) return invalid_state("shutdown");
    return Result<void>::success();                       // nothing to release; state unchanged
}

} // namespace kritva::core::runtime
