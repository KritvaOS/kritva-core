//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : runtime.hpp
// Description : Top-level runtime contract.
//
// Component   : Kritva Core
// Module      : Runtime
// Layer       : Core Foundation
//
// Requirements: CORE-RT-002
// API         : CORE-API-RUNTIME
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include "../error/result.hpp"
#include "../lifecycle/lifecycle_state.hpp"
namespace kritva::core::runtime {
class Runtime {
public:
    virtual ~Runtime() = default;
    virtual Result<void> initialize() = 0;
    virtual Result<void> start() = 0;
    virtual Result<void> stop() = 0;
    virtual Result<void> shutdown() = 0;
    [[nodiscard]] virtual LifecycleState state() const noexcept = 0;
};
} // namespace kritva::core::runtime
