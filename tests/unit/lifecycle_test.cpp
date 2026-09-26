//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : lifecycle_test.cpp
// Description : Lifecycle contract smoke test.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-LIF-003
// API         : CORE-TEST-LIFECYCLE
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#include <cassert>
#include <kritva/core/lifecycle/lifecycle.hpp>
int main() {
    kritva::core::Lifecycle l;
    assert(l.transition_to(kritva::core::LifecycleState::INITIALIZING));
    assert(l.transition_to(kritva::core::LifecycleState::READY));
    assert(l.transition_to(kritva::core::LifecycleState::RUNNING));
    assert(!l.transition_to(kritva::core::LifecycleState::READY));
    return 0;
}
