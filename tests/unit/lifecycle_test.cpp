//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : lifecycle_test.cpp
// Description : Lifecycle state and transition contract tests.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-LIF-002; CORE-LIF-003
// API         : CORE-TEST-LIFECYCLE
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================

#include <cassert>

#include <kritva/core/lifecycle/lifecycle.hpp>

int main() {
    using namespace kritva::core;

    //--------------------------------------------------------------------------
    // Initial state
    //--------------------------------------------------------------------------

    Lifecycle lifecycle;

    assert(lifecycle.state() == LifecycleState::UNKNOWN);
    assert(!lifecycle.is_running());

    //--------------------------------------------------------------------------
    // UNKNOWN -> INITIALIZING
    //--------------------------------------------------------------------------

    auto result = lifecycle.transition_to(LifecycleState::INITIALIZING);

    assert(result.has_value());
    assert(lifecycle.state() == LifecycleState::INITIALIZING);
    assert(!lifecycle.is_running());

    //--------------------------------------------------------------------------
    // INITIALIZING -> READY
    //--------------------------------------------------------------------------

    result = lifecycle.transition_to(LifecycleState::READY);

    assert(result.has_value());
    assert(lifecycle.state() == LifecycleState::READY);

    //--------------------------------------------------------------------------
    // READY -> RUNNING
    //--------------------------------------------------------------------------

    result = lifecycle.transition_to(LifecycleState::RUNNING);

    assert(result.has_value());
    assert(lifecycle.state() == LifecycleState::RUNNING);
    assert(lifecycle.is_running());

    //--------------------------------------------------------------------------
    // RUNNING -> STOPPING
    //--------------------------------------------------------------------------

    result = lifecycle.transition_to(LifecycleState::STOPPING);

    assert(result.has_value());
    assert(lifecycle.state() == LifecycleState::STOPPING);
    assert(!lifecycle.is_running());

    //--------------------------------------------------------------------------
    // STOPPING -> STOPPED
    //--------------------------------------------------------------------------

    result = lifecycle.transition_to(LifecycleState::STOPPED);

    assert(result.has_value());
    assert(lifecycle.state() == LifecycleState::STOPPED);

    //--------------------------------------------------------------------------
    // STOPPED -> INITIALIZING
    //--------------------------------------------------------------------------

    result = lifecycle.transition_to(LifecycleState::INITIALIZING);

    assert(result.has_value());
    assert(lifecycle.state() == LifecycleState::INITIALIZING);

    //--------------------------------------------------------------------------
    // INITIALIZING -> FAULT
    //--------------------------------------------------------------------------

    result = lifecycle.transition_to(LifecycleState::FAULT);

    assert(result.has_value());
    assert(lifecycle.state() == LifecycleState::FAULT);

    //--------------------------------------------------------------------------
    // FAULT -> RECOVERING
    //--------------------------------------------------------------------------

    result = lifecycle.transition_to(LifecycleState::RECOVERING);

    assert(result.has_value());
    assert(lifecycle.state() == LifecycleState::RECOVERING);

    //--------------------------------------------------------------------------
    // RECOVERING -> READY
    //--------------------------------------------------------------------------

    result = lifecycle.transition_to(LifecycleState::READY);

    assert(result.has_value());
    assert(lifecycle.state() == LifecycleState::READY);

    //--------------------------------------------------------------------------
    // READY -> STOPPED
    //--------------------------------------------------------------------------

    result = lifecycle.transition_to(LifecycleState::STOPPED);

    assert(result.has_value());
    assert(lifecycle.state() == LifecycleState::STOPPED);

    //--------------------------------------------------------------------------
    // STOPPED -> INITIALIZING -> READY -> FAULT
    //--------------------------------------------------------------------------

    assert(lifecycle.transition_to(LifecycleState::INITIALIZING).has_value());
    assert(lifecycle.transition_to(LifecycleState::READY).has_value());

    result = lifecycle.transition_to(LifecycleState::FAULT);

    assert(result.has_value());
    assert(lifecycle.state() == LifecycleState::FAULT);

    //--------------------------------------------------------------------------
    // FAULT -> STOPPED
    //--------------------------------------------------------------------------

    result = lifecycle.transition_to(LifecycleState::STOPPED);

    assert(result.has_value());
    assert(lifecycle.state() == LifecycleState::STOPPED);

    //--------------------------------------------------------------------------
    // RECOVERING -> FAULT
    //--------------------------------------------------------------------------
    
    assert(lifecycle.transition_to(LifecycleState::INITIALIZING).has_value());
    assert(lifecycle.transition_to(LifecycleState::FAULT).has_value());
    assert(lifecycle.transition_to(LifecycleState::RECOVERING).has_value());

    result = lifecycle.transition_to(LifecycleState::FAULT);

    assert(result.has_value());
    assert(lifecycle.state() == LifecycleState::FAULT);

    //--------------------------------------------------------------------------
    // RUNNING -> FAULT
    //--------------------------------------------------------------------------

    assert(lifecycle.transition_to(LifecycleState::STOPPED).has_value());
    assert(lifecycle.transition_to(LifecycleState::INITIALIZING).has_value());
    assert(lifecycle.transition_to(LifecycleState::READY).has_value());
    assert(lifecycle.transition_to(LifecycleState::RUNNING).has_value());

    result = lifecycle.transition_to(LifecycleState::FAULT);

    assert(result.has_value());
    assert(lifecycle.state() == LifecycleState::FAULT);

    //--------------------------------------------------------------------------
    // STOPPING -> FAULT
    //--------------------------------------------------------------------------

    assert(lifecycle.transition_to(LifecycleState::RECOVERING).has_value());
    assert(lifecycle.transition_to(LifecycleState::READY).has_value());
    assert(lifecycle.transition_to(LifecycleState::RUNNING).has_value());
    assert(lifecycle.transition_to(LifecycleState::STOPPING).has_value());

    result = lifecycle.transition_to(LifecycleState::FAULT);

    assert(result.has_value());
    assert(lifecycle.state() == LifecycleState::FAULT);

    //--------------------------------------------------------------------------
    // Invalid transition must fail and preserve state.
    //--------------------------------------------------------------------------

    result = lifecycle.transition_to(LifecycleState::RUNNING);

    assert(!result.has_value());
    assert(result.error().code == ErrorCode::INVALID_STATE);
    assert(lifecycle.state() == LifecycleState::FAULT);

    //--------------------------------------------------------------------------
    // Invalid transition from UNKNOWN.
    //--------------------------------------------------------------------------

    Lifecycle fresh;

    result = fresh.transition_to(LifecycleState::RUNNING);

    assert(!result.has_value());
    assert(result.error().code == ErrorCode::INVALID_STATE);
    assert(fresh.state() == LifecycleState::UNKNOWN);
    assert(!fresh.is_running());

    //--------------------------------------------------------------------------
    // Invalid transition from READY.
    //--------------------------------------------------------------------------

    assert(fresh.transition_to(LifecycleState::INITIALIZING).has_value());
    assert(fresh.transition_to(LifecycleState::READY).has_value());

    result = fresh.transition_to(LifecycleState::STOPPING);

    assert(!result.has_value());
    assert(result.error().code == ErrorCode::INVALID_STATE);
    assert(fresh.state() == LifecycleState::READY);

    return 0;
}
