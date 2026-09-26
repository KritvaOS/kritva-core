//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : scheduler.hpp
// Description : Abstract scheduler contract implemented by platform adapters.
//
// Component   : Kritva Core
// Module      : Platform Contract
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-001
// API         : CORE-API-PLATFORM
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once

#include <cstdint>
#include "../error/result.hpp"
#include "../types/duration.hpp"
namespace kritva::core::platform {
using TaskId = std::uint64_t;
struct TaskConfig {
    const char* name{"kritva"};
    std::uint32_t priority{0};
    std::uint32_t cpu_affinity{0};
    Duration period{};
};
class IScheduler {
public:
    virtual ~IScheduler() = default;
    virtual Result<TaskId> create_task(const TaskConfig&, void (*entry)(void*), void* context) = 0;
    virtual Result<void> start() = 0;
    virtual Result<void> stop() = 0;
};
} // namespace kritva::core::platform
