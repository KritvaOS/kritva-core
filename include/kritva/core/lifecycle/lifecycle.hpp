//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : lifecycle.hpp
// Description : Lifecycle state holder and transition interface.
//
// Component   : Kritva Core
// Module      : Lifecycle
// Layer       : Core Foundation
//
// Requirements: CORE-LC-002
// API         : CORE-API-LIFECYCLE
//
// Author      : KritvaOS Core Team
// Created     : 2026-09-14
//==============================================================================

#pragma once

#include "lifecycle_state.hpp"

namespace kritva::core {

/// @brief Manages the observable lifecycle state of a component.
class Lifecycle {
public:
    [[nodiscard]] LifecycleState state() const noexcept { return state_; }
    [[nodiscard]] bool is_running() const noexcept {
        return state_ == LifecycleState::RUNNING;
    }

    void set_state(LifecycleState state) noexcept { state_ = state; }

private:
    LifecycleState state_{LifecycleState::UNKNOWN};
};

} // namespace kritva::core
