//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : event_type.hpp
// Description : Standard event classifications for Core event envelopes.
//
// Component   : Kritva Core
// Module      : Event
// Layer       : Core Foundation
//
// Requirements: CORE-EVT-001
// API         : CORE-API-EVENT
//
// Author      : KritvaOS Core Team
// Created     : 2026-09-14
//==============================================================================

#pragma once

#include <cstdint>

namespace kritva::core {

/// @brief Identifies the class of an emitted Core event.
enum class EventType : std::uint32_t {
    UNKNOWN = 0,
    LIFECYCLE,
    STATUS,
    HEALTH,
    ERROR,
    CONFIGURATION,
    CAPABILITY
};

} // namespace kritva::core
