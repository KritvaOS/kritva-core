//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : lifecycle.cpp
// Description : Validated lifecycle transition implementation.
//
// Component   : Kritva Core
// Module      : Lifecycle
// Layer       : Core Foundation
//
// Requirements: CORE-LIF-002; CORE-LIF-003
// API         : CORE-API-LIFECYCLE
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#include <kritva/core/lifecycle/lifecycle.hpp>
namespace kritva::core {
bool Lifecycle::valid_transition(LifecycleState from, LifecycleState to) noexcept {
    switch (from) {
        case LifecycleState::UNKNOWN: return to == LifecycleState::INITIALIZING;
        case LifecycleState::INITIALIZING: return to == LifecycleState::READY || to == LifecycleState::FAULT;
        case LifecycleState::READY: return to == LifecycleState::RUNNING || to == LifecycleState::STOPPED || to == LifecycleState::FAULT;
        case LifecycleState::RUNNING: return to == LifecycleState::STOPPING || to == LifecycleState::FAULT;
        case LifecycleState::STOPPING: return to == LifecycleState::STOPPED || to == LifecycleState::FAULT;
        case LifecycleState::STOPPED: return to == LifecycleState::INITIALIZING;
        case LifecycleState::FAULT: return to == LifecycleState::RECOVERING || to == LifecycleState::STOPPED;
        case LifecycleState::RECOVERING: return to == LifecycleState::READY || to == LifecycleState::FAULT;
    }
    return false;
}
Result<void> Lifecycle::transition_to(LifecycleState target) noexcept {
    if (!valid_transition(state_, target)) {
        return Result<void>::failure(Error{
            ErrorCode::INVALID_STATE, ErrorSeverity::ERROR, {}, {}, "Invalid lifecycle transition"
        });
    }
    state_ = target;
    return Result<void>::success();
}
} // namespace kritva::core
