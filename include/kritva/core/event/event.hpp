//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : event.hpp
// Description : Event envelope carrying source, type, time, severity and correlation.
//
// Component   : Kritva Core
// Module      : Event
// Layer       : Core Foundation
//
// Requirements: CORE-EVT-002
// API         : CORE-API-EVENT
//
// Author      : KritvaOS Core Team
// Created     : 2026-09-14
//==============================================================================

#pragma once

#include "event_type.hpp"
#include "../error/error_code.hpp"
#include "../types/id.hpp"
#include "../types/timestamp.hpp"

namespace kritva::core {

/// @brief Common envelope for asynchronous Core events.
struct Event {
    Id event_id{};
    Id source_id{};
    EventType type{EventType::UNKNOWN};
    Timestamp timestamp{};
    ErrorSeverity severity{ErrorSeverity::INFO};
    Id correlation_id{};
};

} // namespace kritva::core
