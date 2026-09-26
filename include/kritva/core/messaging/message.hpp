//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : message.hpp
// Description : Platform-neutral message header.
//
// Component   : Kritva Core
// Module      : Messaging
// Layer       : Core Foundation
//
// Requirements: CORE-MSG-001
// API         : CORE-API-MESSAGING
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include "../types/id.hpp"
#include "../types/timestamp.hpp"
namespace kritva::core::messaging {
struct MessageHeader {
    Id message_id{};
    Id source_id{};
    Timestamp timestamp{};
};
} // namespace kritva::core::messaging
