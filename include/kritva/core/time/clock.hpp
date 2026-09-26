//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : clock.hpp
// Description : Platform-neutral clock contract exposed by Core.
//
// Component   : Kritva Core
// Module      : Time
// Layer       : Core Foundation
//
// Requirements: CORE-TIME-001
// API         : CORE-API-TIME
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================

#pragma once

#include "../types/timestamp.hpp"

namespace kritva::core::time {

/// Platform-neutral source of timestamps.
///
/// Implementations are provided by platform adapters. Core owns the contract;
/// Linux/RTOS/MCU/PTP-specific clock implementations must remain outside Core.
class IClock {
public:
    virtual ~IClock() = default;

    [[nodiscard]] virtual Timestamp now() const noexcept = 0;
};

} // namespace kritva::core::time
