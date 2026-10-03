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
// Requirements: CORE-RT-006, CORE-RT-007, CORE-RT-008
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

Result<void> RuntimeManager::call(ComponentId id, Step step, const Configuration* configuration) {
    Component* component = registry_.find(id);
    assert(component != nullptr);                         // order_ only lists registered components
    Result<void> result = Result<void>::success();
    switch (step) {
        case Step::CONFIGURE:  result = component->configure(*configuration); break;
        case Step::INITIALIZE: result = component->initialize(); break;
        case Step::START:      result = component->start(); break;
        case Step::STOP:       result = component->stop(); break;
        case Step::SHUTDOWN:   result = component->shutdown(); break;
    }

    if (!result.has_value()) {
        statistics_.error_count.increment();
        // A failed initialize/start/stop leaves the component in FAULT (Component contract):
        // it is recorded as faulted, never as having completed. configure and shutdown
        // failures leave the recorded stage unchanged so a later explicit attempt can resume.
        if (step == Step::INITIALIZE || step == Step::START || step == Step::STOP) {
            stage_[id] = Stage::FAULTED;
        }
        return result;                                    // the component's own Error, unchanged
    }

    statistics_.sample_count.increment();
    switch (step) {
        case Step::CONFIGURE:  break;
        case Step::INITIALIZE: stage_[id] = Stage::INITIALIZED; break;
        case Step::START:      stage_[id] = Stage::STARTED; break;
        case Step::STOP:       stage_[id] = Stage::STOPPED; break;
        case Step::SHUTDOWN:   stage_[id] = Stage::SHUT_DOWN; break;
    }
    return result;
}

Result<void> RuntimeManager::run(const std::vector<ComponentId>& ids, Step step, bool reverse,
                                 const Configuration* configuration) {
    // First failure ends the sequence; that component's error is returned unchanged.
    if (reverse) {
        for (auto it = ids.rbegin(); it != ids.rend(); ++it) {
            if (auto r = call(*it, step, configuration); !r.has_value()) return r;
        }
    } else {
        for (const ComponentId id : ids) {
            if (auto r = call(id, step, configuration); !r.has_value()) return r;
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
    for (const ComponentId id : order_) stage_[id] = Stage::NONE;   // a new live period starts here
    if (auto r = run(order_, Step::INITIALIZE, false, nullptr); !r.has_value()) {
        enter_fault(r.error());
        return r;
    }
    transition(LifecycleState::READY);
    return Result<void>::success();
}

Result<void> RuntimeManager::start() {
    if (lifecycle_.state() != LifecycleState::READY) return invalid_state("start");
    if (auto r = run(order_, Step::START, false, nullptr); !r.has_value()) {
        enter_fault(r.error());
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
        enter_fault(r.error());
        return r;
    }
    transition(LifecycleState::STOPPED);
    return Result<void>::success();
}

Result<void> RuntimeManager::shutdown() {
    const LifecycleState s = lifecycle_.state();
    if (s != LifecycleState::UNKNOWN && s != LifecycleState::STOPPED) return invalid_state("shutdown");
    if (!components_live_) return Result<void>::success();   // nothing to release: no component is invoked

    // Reverse order, skipping components that never ran or whose shutdown already succeeded in
    // this live period. The first failure ends the call (state unchanged); progress made so
    // far is kept, so a retry resumes with the failing component and never repeats a success.
    for (auto it = order_.rbegin(); it != order_.rend(); ++it) {
        const Stage stage = stage_[*it];
        if (stage == Stage::SHUT_DOWN || stage == Stage::NONE) continue;
        if (auto r = call(*it, Step::SHUTDOWN, nullptr); !r.has_value()) return r;
    }
    components_live_ = false;
    return Result<void>::success();
}

void RuntimeManager::enter_fault(const Error& error) {
    fault_ = error;                                       // the original failure stays observable
    transition(LifecycleState::FAULT);
}

Result<void> RuntimeManager::reset() {
    if (lifecycle_.state() != LifecycleState::FAULT) return invalid_state("reset");

    // Explicit, caller-requested cleanup. Everything is in reverse dependency order, each
    // step at most once, and the failed operation itself is never retried.
    // Pass 1: stop what was initialized or started. A component that failed (faulted) or
    // never ran is not stopped.
    for (auto it = order_.rbegin(); it != order_.rend(); ++it) {
        const Stage stage = stage_[*it];
        if (stage != Stage::INITIALIZED && stage != Stage::STARTED) continue;
        if (auto r = call(*it, Step::STOP, nullptr); !r.has_value()) return r;   // stays FAULT; fault_ kept
    }
    // Pass 2: release what stopped or faulted (a faulted component leaves FAULT by shutdown()).
    for (auto it = order_.rbegin(); it != order_.rend(); ++it) {
        const Stage stage = stage_[*it];
        if (stage != Stage::STOPPED && stage != Stage::FAULTED) continue;
        if (auto r = call(*it, Step::SHUTDOWN, nullptr); !r.has_value()) return r;
    }

    transition(LifecycleState::STOPPED);                  // FAULT -> STOPPED: an edge of the Core table
    components_live_ = false;
    fault_.reset();
    return Result<void>::success();
}

} // namespace kritva::core::runtime
