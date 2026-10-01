//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : event.hpp
// Description : Common asynchronous event envelope.
//
// Component   : Kritva Core
// Module      : Event
// Layer       : Core Foundation
//
// Requirements: CORE-EVT-002; CORE-EVT-003; CORE-EVT-004
// API         : CORE-API-EVENT
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include "event_type.hpp"
#include "../error/error_code.hpp"
#include "../types/id.hpp"
#include "../types/timestamp.hpp"
namespace kritva::core {
struct Event {
    Id event_id{};
    Id source_id{};
    EventType type{EventType::UNKNOWN};
    Timestamp timestamp{};
    ErrorSeverity severity{ErrorSeverity::INFO};
    Id correlation_id{};
};
} // namespace kritva::core
