//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : lifecycle_state.hpp
// Description : Lifecycle states shared by Kritva Core-managed components.
//
// Component   : Kritva Core
// Module      : Lifecycle
// Layer       : Core Foundation
//
// Requirements: CORE-LC-001
// API         : CORE-API-LIFECYCLE
//
// Author      : KritvaOS Core Team
// Created     : 2026-09-14
//==============================================================================

#pragma once

namespace kritva::core {

/// @brief Standard lifecycle state of a Core-managed component.
enum class LifecycleState {
    UNKNOWN,
    INITIALIZING,
    READY,
    RUNNING,
    STOPPING,
    STOPPED,
    FAULT,
    RECOVERING
};

} // namespace kritva::core
