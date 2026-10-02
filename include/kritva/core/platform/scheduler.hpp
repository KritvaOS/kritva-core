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

//------------------------------------------------------------------------------
// TaskId (CORE-PLAT-001)
//
//   - Opaque handle returned by IScheduler::create_task(). The numeric value
//     has no meaning and no ordering; do not interpret or do arithmetic on it.
//   - 0 is reserved and is never returned for a successfully created task, so
//     callers may use 0 as "no task".
//   - Unique among all tasks created by one IScheduler instance, and never
//     reused during that instance's lifetime. Not unique across instances.
//   - Valid from the successful create_task() until the scheduler is destroyed
//     (there is no task-destroy operation in this contract).
//------------------------------------------------------------------------------
using TaskId = std::uint64_t;

//------------------------------------------------------------------------------
// TaskConfig (CORE-PLAT-001)
//
// Describes one task. Passed by const reference; the scheduler must copy
// whatever it needs before create_task() returns and must not retain the
// reference or the `name` pointer.
//
//   name         Non-null, NUL-terminated diagnostic name. Need only remain
//                valid for the duration of create_task(). An implementation
//                may truncate it to a platform limit. nullptr is invalid
//                (INVALID_ARGUMENT). Default: "kritva".
//
//   priority     Platform-neutral relative urgency. A larger value means more
//                urgent; equal values are peers. 0 is the default and the
//                least urgent level. Core assigns NO meaning to specific
//                numbers and no mapping to any OS priority range: adapters
//                map the supported range onto native priorities preserving
//                order, and document their maximum. A value above the
//                adapter's maximum is rejected with INVALID_ARGUMENT, never
//                silently clamped. Priority is a scheduling hint to the
//                platform; it is not a real-time guarantee.
//
//   cpu_affinity Bit mask of logical CPUs the task may run on: bit n set
//                means logical CPU n is allowed. The value 0 means "no
//                affinity constraint" (the platform may run the task on any
//                CPU); it does NOT mean "CPU 0" and does NOT mean "no CPU".
//                To pin to CPU 0, use 0x1. A non-zero mask is a request that
//                must be honoured or refused: an adapter without affinity
//                support returns UNSUPPORTED, and a mask naming only
//                non-existent CPUs returns INVALID_ARGUMENT. Affinity is
//                never silently ignored. The 32-bit mask addresses CPUs
//                0..31 only (known limitation of this contract revision).
//
//   period       Activation period. Duration{} (zero) means aperiodic: entry
//                is invoked once per start(). A positive value means
//                periodic: entry is invoked once per period, each invocation
//                running to completion and returning, until stop(). A
//                negative value is invalid (INVALID_ARGUMENT). Behavior on an
//                overrun (entry still running when the next period is due) is
//                implementation-defined and must be documented by the
//                adapter. Period accuracy and jitter are not guaranteed by
//                this contract.
//------------------------------------------------------------------------------
struct TaskConfig {
    const char* name{"kritva"};
    std::uint32_t priority{0};
    std::uint32_t cpu_affinity{0};
    Duration period{};
};

//------------------------------------------------------------------------------
// IScheduler (CORE-PLAT-001)
//
// Contract implemented by platform adapters (Linux, RTOS, ...). Core defines
// no implementation. Lifecycle of one scheduler instance:
//
//     create_task()*  ->  start()  ->  (running)  ->  stop()  ->  start() ...
//
// State: a scheduler is either STOPPED (initial) or RUNNING.
//
// create_task(config, entry, context)
//   - Registers a task in the STOPPED state. It does NOT start the task and
//     does NOT invoke `entry`; execution begins only at start().
//   - Valid only while the scheduler is STOPPED; while RUNNING it returns
//     INVALID_STATE and creates nothing.
//   - `entry` must be non-null (INVALID_ARGUMENT otherwise). The function must
//     remain valid for the scheduler's lifetime. It must not throw and must
//     not call stop() on its own scheduler.
//   - `context` is an opaque pointer, may be null, passed unchanged to every
//     invocation of `entry`. The scheduler never owns, copies, dereferences or
//     frees it. The caller must keep the pointee valid from create_task()
//     until the scheduler has been stop()ped (or destroyed), and is
//     responsible for synchronizing access to it between `entry` and any
//     other thread.
//   - Errors (no task is created, no TaskId is consumed, state is unchanged):
//       INVALID_ARGUMENT     null name/entry, negative period, priority above
//                            the adapter maximum, affinity naming no existing
//                            CPU
//       UNSUPPORTED          non-zero affinity requested but unsupported
//       INVALID_STATE        scheduler is RUNNING
//       RESOURCE_UNAVAILABLE no capacity left (task slots, memory, stacks,
//                            native handles). Exhaustion is reported, never
//                            by abort or exception.
//
// start()
//   - Starts every task created so far; transitions STOPPED -> RUNNING.
//     With zero tasks it succeeds and becomes RUNNING.
//   - Idempotent: calling start() while RUNNING succeeds and changes nothing.
//   - All-or-nothing: if any task cannot be started (e.g.
//     RESOURCE_UNAVAILABLE), tasks already started are stopped again, the
//     scheduler remains STOPPED, and the error is returned.
//   - May be called again after stop(); tasks persist and are restarted.
//
// stop()
//   - Transitions RUNNING -> STOPPED. On successful return no `entry`
//     invocation is in progress and none will begin, so the caller may then
//     release any `context`. It may block until running invocations return.
//   - Idempotent: calling stop() while STOPPED (including before any start())
//     succeeds and changes nothing.
//   - Must not be called from inside a task `entry` of the same scheduler:
//     implementations return INVALID_STATE rather than deadlock.
//   - Destroying a scheduler implies stop().
//
// Real-time and thread-safety:
//   - create_task(), start() and stop() are control-plane operations: they may
//     allocate and block and are not for use in real-time paths.
//   - The interface is not required to be thread-safe; callers serialize calls
//     on one scheduler unless an adapter documents otherwise.
//   - This interface makes no hard-real-time, latency, or jitter guarantee.
//     Adapters document what they actually provide.
//   - Exceptions: operations report failure through Result and do not throw.
//------------------------------------------------------------------------------
class IScheduler {
public:
    virtual ~IScheduler() = default;
    virtual Result<TaskId> create_task(const TaskConfig&, void (*entry)(void*), void* context) = 0;
    virtual Result<void> start() = 0;
    virtual Result<void> stop() = 0;
};
} // namespace kritva::core::platform
