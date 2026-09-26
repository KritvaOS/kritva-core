//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : timer.hpp
// Description : Abstract timer contract.
//
// Component   : Kritva Core
// Module      : Time
// Layer       : Core Foundation
//
// Requirements: CORE-TIME-002
// API         : CORE-API-TIME
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include "../error/result.hpp"
#include "../types/duration.hpp"
namespace kritva::core::time {
class ITimer {
public:
    virtual ~ITimer() = default;
    virtual Result<void> start(Duration period) = 0;
    virtual Result<void> stop() = 0;
};
} // namespace kritva::core::time
