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

// Scheduler platform contract (R0.2, CORE-PLAT-001).
//
// Core defines WHAT a scheduler provides; platform implementations decide HOW.
// This header carries no platform-specific behavior.
//
// TaskConfig
//   - name         : must be a NUL-terminated string that outlives the call to
//                    create_task(); implementations that retain it must copy it.
//   - priority     : higher value means higher priority. The mapping onto native
//                    priorities is implementation-defined; implementations must
//                    preserve relative ordering or fail with
//                    ErrorCode::UNSUPPORTED / INVALID_ARGUMENT.
//   - cpu_affinity : a CPU bitmask (bit N = CPU N). 0 means "no affinity
//                    requested" (the scheduler may run the task on any CPU).
//                    Unsupported masks are rejected, never silently ignored.
//   - period       : zero Duration means the task is not periodic (entry is
//                    invoked once per start()); otherwise entry is invoked every
//                    period. Negative Duration is invalid.
//
// IScheduler
//   - create_task() : registers a task and returns its TaskId; the task does not
//                     run until start(). Must be called before start() unless the
//                     implementation documents otherwise. `entry` must be
//                     non-null. `context` is borrowed: Core never takes ownership
//                     and the caller must keep it valid until stop() returns.
//   - start()       : begins execution of all created tasks. Calling start() when
//                     already started fails with ErrorCode::ALREADY_RUNNING.
//   - stop()        : stops all tasks and returns only after no task is executing
//                     `entry`; afterward `context` may be released. Calling stop()
//                     when not started is a no-op success or INVALID_STATE
//                     (implementation-defined, must be documented).
//   - Failure behavior: all methods report failure through Result and do not
//                     throw. A failed call leaves the scheduler state unchanged.
//   - Real-time: create_task/start/stop are setup/teardown operations and may
//                     allocate and block; they must not be called from a task
//                     entry. `entry` itself must not block unboundedly.
//   - Thread-safety: not guaranteed; callers serialize calls to one instance.
//   - Ownership     : the scheduler owns its internal task resources; the
//                     caller owns `context`.
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
