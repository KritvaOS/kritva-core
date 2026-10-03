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
#include <utility>

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

Result<void> RuntimeManager::run(const std::vector<ComponentId>& ids, Step step, bool reverse,
                                 const Configuration* configuration) {
    const auto invoke = [&](ComponentId id) -> Result<void> {
        Component* component = registry_.find(id);
        assert(component != nullptr);                     // order() only lists registered components
        switch (step) {
            case Step::CONFIGURE:  return component->configure(*configuration);
            case Step::INITIALIZE: return component->initialize();
            case Step::START:      return component->start();
            case Step::STOP:       return component->stop();
        }
        return Result<void>::success();
    };
    // First failure ends the sequence; that component's error is returned unchanged.
    if (reverse) {
        for (auto it = ids.rbegin(); it != ids.rend(); ++it) {
            if (auto r = invoke(*it); !r.has_value()) return r;
        }
    } else {
        for (const ComponentId id : ids) {
            if (auto r = invoke(id); !r.has_value()) return r;
        }
    }
    return Result<void>::success();
}

Result<void> RuntimeManager::configure(const Configuration& configuration) {
    const LifecycleState s = lifecycle_.state();
    if (s != LifecycleState::UNKNOWN && s != LifecycleState::STOPPED) return invalid_state("configure");

    if (topology_fixed_) return run(order_, Step::CONFIGURE, false, &configuration);
    auto order = component_order();                       // current order; the topology is not fixed here
    if (!order.has_value()) return Result<void>::failure(order.error());
    return run(order.value(), Step::CONFIGURE, false, &configuration);
}

Result<void> RuntimeManager::initialize() {
    const LifecycleState s = lifecycle_.state();
    if (s != LifecycleState::UNKNOWN && s != LifecycleState::STOPPED) return invalid_state("initialize");

    if (!topology_fixed_) {
        // Validate before anything changes: an invalid topology never reaches READY
        // and no component is invoked.
        auto order = component_order();
        if (!order.has_value()) return Result<void>::failure(order.error());   // error preserved
        order_ = std::move(order).value();
        topology_fixed_ = true;
    }
    transition(LifecycleState::INITIALIZING);
    components_live_ = true;                              // components may now hold resources
    shut_down_.clear();                                   // a new live period: no shutdown has happened in it
    if (auto r = run(order_, Step::INITIALIZE, false, nullptr); !r.has_value()) {
        transition(LifecycleState::FAULT);
        return r;
    }
    transition(LifecycleState::READY);
    return Result<void>::success();
}

Result<void> RuntimeManager::start() {
    if (lifecycle_.state() != LifecycleState::READY) return invalid_state("start");
    if (auto r = run(order_, Step::START, false, nullptr); !r.has_value()) {
        transition(LifecycleState::FAULT);
        return r;
    }
    transition(LifecycleState::RUNNING);                  // only after every start() succeeded
    return Result<void>::success();
}

Result<void> RuntimeManager::stop() {
    const LifecycleState s = lifecycle_.state();
    if (s != LifecycleState::READY && s != LifecycleState::RUNNING) return invalid_state("stop");
    if (s == LifecycleState::RUNNING) transition(LifecycleState::STOPPING);
    if (auto r = run(order_, Step::STOP, true, nullptr); !r.has_value()) {
        transition(LifecycleState::FAULT);
        return r;
    }
    transition(LifecycleState::STOPPED);
    return Result<void>::success();
}

Result<void> RuntimeManager::shutdown() {
    const LifecycleState s = lifecycle_.state();
    if (s != LifecycleState::UNKNOWN && s != LifecycleState::STOPPED) return invalid_state("shutdown");
    if (!components_live_) return Result<void>::success();   // nothing to release: no component is invoked

    // Reverse order, skipping components whose shutdown already succeeded in this live
    // period. The first failure ends the call (state unchanged); progress made so far is
    // kept, so a retry resumes with the failing component and never repeats a success.
    for (auto it = order_.rbegin(); it != order_.rend(); ++it) {
        if (shut_down_.count(*it) != 0) continue;
        Component* component = registry_.find(*it);
        assert(component != nullptr);
        if (auto r = component->shutdown(); !r.has_value()) return r;
        shut_down_.insert(*it);
    }
    components_live_ = false;
    shut_down_.clear();
    return Result<void>::success();
}

} // namespace kritva::core::runtime
