//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : event_type.hpp
// Description : Standard event classifications.
//
// Component   : Kritva Core
// Module      : Event
// Layer       : Core Foundation
//
// Requirements: CORE-EVT-001
// API         : CORE-API-EVENT
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include <cstdint>
namespace kritva::core {
enum class EventType : std::uint32_t {
    UNKNOWN = 0, LIFECYCLE, STATUS, HEALTH, ERROR, CONFIGURATION, CAPABILITY
};
} // namespace kritva::core
