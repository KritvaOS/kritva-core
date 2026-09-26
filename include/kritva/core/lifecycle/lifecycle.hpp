//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : lifecycle.hpp
// Description : Lifecycle state and validated transition contract.
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


#pragma once
#include "lifecycle_state.hpp"
#include "../error/result.hpp"
namespace kritva::core {
class Lifecycle {
public:
    [[nodiscard]] LifecycleState state() const noexcept { return state_; }
    [[nodiscard]] bool is_running() const noexcept { return state_ == LifecycleState::RUNNING; }
    [[nodiscard]] Result<void> transition_to(LifecycleState target) noexcept;
private:
    static bool valid_transition(LifecycleState from, LifecycleState to) noexcept;
    LifecycleState state_{LifecycleState::UNKNOWN};
};
} // namespace kritva::core
