//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : watchdog.hpp
// Description : Abstract watchdog contract.
//
// Component   : Kritva Core
// Module      : Platform Contract
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-003
// API         : CORE-API-PLATFORM
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include "../error/result.hpp"
#include "../types/duration.hpp"
namespace kritva::core::platform {
class IWatchdog {
public:
    virtual ~IWatchdog() = default;
    virtual Result<void> start(Duration timeout) = 0;
    virtual Result<void> kick() = 0;
    virtual Result<void> stop() = 0;
};
} // namespace kritva::core::platform
