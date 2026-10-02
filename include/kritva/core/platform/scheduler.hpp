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
//   - 0 is the invalid / "no task" identifier and is never returned for a
//     successfully created task.
//   - Valid TaskIds are unique among the tasks that currently exist within one
//     IScheduler instance. Not unique across instances.
//   - A TaskId remains valid while its task exists. This contract has no
//     task-destroy operation, so in this revision a task exists until its
//     scheduler is torn down and ids are therefore never observed to repeat.
//     Whether an id may be reused after a future destroy operation is
//     implementation-defined and is NOT promised here; callers must not rely
//     on either behavior.
//   - Consequence for adapters: with no destroy operation, the number of
//     tasks (and ids) grows for the scheduler's lifetime; adapters document
//     their capacity (see RESOURCE_UNAVAILABLE).
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
//   priority     Implementation-independent relative priority. Within one
//                scheduler instance, a higher value represents greater
//                scheduling urgency than a lower value, and equal values are
//                peers. 0 is the default. Core defines NO OS-specific numeric
//                range, maximum, or mapping: the adapter maps this value to
//                its platform scheduler (whatever that platform's own
//                numeric direction or range is), preserving the relative
//                order, and documents its mapping and how it treats values
//                its platform cannot represent (for example rejecting them
//                with INVALID_ARGUMENT). Priority is a scheduling hint, not a
//                real-time guarantee.
//
//   cpu_affinity Bit mask of logical CPUs the task may run on: bit n set
//                means logical CPU n is allowed. The value 0 means "no
//                affinity constraint" (the platform may run the task on any
//                CPU); it does NOT mean "CPU 0" and does NOT mean "no CPU".
//                To pin to CPU 0, use 0x1. A non-zero mask is a request that
//                must be honoured or refused, never silently ignored. Two
//                distinct failures:
//                  INVALID_ARGUMENT  the mask itself is not valid for this
//                                    platform (it selects no CPU that
//                                    exists). Treatment of extra bits naming
//                                    non-existent CPUs alongside valid ones
//                                    is adapter-defined and documented.
//                  UNSUPPORTED       the mask is well-formed but the
//                                    adapter/platform cannot provide
//                                    affinity (for example no affinity
//                                    support at all).
//                R0.2 limitation: the 32-bit mask can address logical CPUs
//                0..31 only. This reflects the current TaskConfig type, not a
//                long-term Core architectural limit; a wider representation
//                may be introduced with the platform abstraction work
//                (KF-CORE-R04) and is out of scope for R0.2.
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
//   - Called while the scheduler is STOPPED: registers an inactive task. It
//     does NOT start the task and does NOT invoke `entry`; execution begins
//     only at start().
//   - Called while the scheduler is RUNNING: Core does not mandate a static
//     task set. This is an adapter policy and the adapter documents which it
//     provides:
//       (a) dynamic creation is supported: the task joins the running set
//           and the adapter documents when its first invocation may occur
//           (it may be before create_task() returns); or
//       (b) dynamic creation is not supported: the call fails with
//           INVALID_STATE and, atomically, creates nothing (no task, no
//           TaskId, no state change).
//     Portable callers that need a task before start() must create it while
//     STOPPED; callers that create tasks while RUNNING must handle (b).
//   - `entry` must be non-null (INVALID_ARGUMENT otherwise). The function must
//     remain valid for the scheduler's lifetime. It must not throw and must
//     not call stop() on its own scheduler.
//   - `context` is an opaque pointer, may be null, passed unchanged to every
//     invocation of `entry`. The scheduler never owns, copies, dereferences or
//     frees it. The caller must keep the pointee valid from create_task()
//     until the scheduler has been stop()ped (or destroyed), and is
//     responsible for synchronizing access to it between `entry` and any
//     other thread.
//   - Every error is atomic: no task is created, no TaskId is consumed, and
//     scheduler state is unchanged.
//       INVALID_ARGUMENT     null name/entry, negative period, affinity mask
//                            invalid for the platform, or a priority the
//                            adapter documents as not representable
//       UNSUPPORTED          well-formed non-zero affinity the adapter cannot
//                            provide
//       INVALID_STATE        dynamic creation while RUNNING is not supported
//                            by the adapter (policy (b) above)
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
//   - Must not be called from inside a task `entry` of the same scheduler.
//     Reason: stop() is synchronous and scheduler-wide, so it must wait for
//     every running `entry` to return, including the calling one, which would
//     wait on itself and deadlock. Implementations return INVALID_STATE
//     rather than deadlock. (To end work from within a task, signal the
//     owner through the caller's own `context` and have another thread stop.)
//
// Teardown:
//   - Core does not specify destructor behavior of an adapter class.
//     Implementations must not release scheduler resources (task stacks,
//     native handles, memory) while any `entry` is executing; they must reach
//     an orderly stop before resources are destroyed. Callers should call
//     stop() before destroying a scheduler and before releasing any `context`.
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
