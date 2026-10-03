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
// Requirements: CORE-TIME-002, CORE-PLAT-006
// API         : CORE-API-TIME
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include "../error/result.hpp"
#include "../types/callback.hpp"
#include "../types/duration.hpp"
#include <cstdint>
namespace kritva::core::time {

/// How a started timer fires.
enum class TimerMode : std::uint8_t {
    ONE_SHOT,   ///< Fires once, one period after start(), then the timer is stopped.
    PERIODIC    ///< Fires every period until stop().
};

//------------------------------------------------------------------------------
// ITimer (CORE-TIME-002, CORE-PLAT-006)
//
// The platform-neutral timer contract. Core defines it; platform adapters
// implement it (rules shared by all platform contracts: platform/boundary.hpp).
// Core contains no timer implementation, no worker thread and no OS timer.
//
// RELATION TO THE CLOCK
//   time::IClock (time/clock.hpp) is the only clock abstraction. A timer measures
//   ELAPSED time on a monotonic time base. It is independent of IClock and of the
//   REALTIME clock domain: a wall-clock step never lengthens or shortens a
//   period. Timer durations carry no ClockDomain.
//
// STATE AND ACTIVATION
//   A timer is either STOPPED (initial) or RUNNING.
//
//   start(period, mode, callback)
//     - Valid only while STOPPED. The activation is atomic and self-contained:
//       the period, the mode and the callback belong to this start() and end at
//       the next stop() (or, for ONE_SHOT, when it has fired).
//     - Requests that fail return an Error and have no effect (the timer stays
//       STOPPED):
//         INVALID_ARGUMENT     period <= 0 (zero or negative), or
//                              callback.function == nullptr. A null
//                              callback.context is valid.
//         INVALID_STATE        the timer is already RUNNING (including a call
//                              from inside the timer's own callback).
//         UNSUPPORTED          the adapter cannot provide the requested mode or
//                              this period (for example finer than its
//                              resolution, if it documents that as unsupported).
//         RESOURCE_UNAVAILABLE no timer resource is available.
//     - The adapter documents its resolution, the largest supported period and
//       how a period that is not a multiple of its resolution is treated.
//
//   Firing
//     - ONE_SHOT: after a successful start() the callback is invoked exactly
//       once, no earlier than one period later, unless stop() returned first.
//       When it has returned the timer is STOPPED and may be started again.
//     - PERIODIC: the callback is invoked zero or more times, roughly every
//       period, until stop(). Timing accuracy, jitter, and the handling of
//       overruns (callback still executing, or missed periods) are
//       adapter-defined and documented by the adapter. Invocations of one timer
//       never overlap each other.
//
//   stop()
//     - Idempotent: stopping a STOPPED timer succeeds and changes nothing.
//     - Synchronous. When stop() returns successfully the timer is STOPPED, no
//       callback invocation is executing, and none will begin: stop() does not
//       return until a callback that was executing has returned. The caller may
//       then release the callback context.
//     - Must not be called from inside the timer's own callback: it would have
//       to wait for itself. It fails with INVALID_STATE instead of deadlocking.
//
// CALLBACKS AND CONTEXT (kritva::core::Callback, types/callback.hpp)
//   - Who invokes it: the adapter, never Core. Where it runs is adapter-defined
//     (an adapter thread, an interrupt, a tick hook) and implies no Core-owned
//     thread.
//   - A portable callback is short: it shall not throw, shall not block, shall
//     not allocate, and shall not call back into the same timer (start() and
//     stop() fail with INVALID_STATE there). An adapter may permit more and then
//     documents it.
//   - The context is opaque, non-owning and caller-owned; Core and the timer
//     never dereference, copy or free it. It must remain valid until stop() has
//     returned successfully or, for a ONE_SHOT timer, until its callback has
//     returned.
//
// OWNERSHIP, TEARDOWN, THREADS
//   - The integrator owns the timer; Core types hold only non-owning references.
//     A timer destroyed or torn down while RUNNING must reach an orderly stop
//     (no callback executing or starting) before it releases resources; Core
//     does not specify adapter destructors.
//   - Whether start() and stop() may be called concurrently with each other is
//     adapter-defined. Operations are control-plane (they may allocate and
//     block) and Core makes no hard-real-time, latency or jitter guarantee.
//   - Timer expiry has no effect on the Runtime: it never triggers Runtime
//     recovery.
//------------------------------------------------------------------------------
class ITimer {
public:
    virtual ~ITimer() = default;
    virtual Result<void> start(Duration period, TimerMode mode, Callback callback) = 0;
    virtual Result<void> stop() = 0;
};
} // namespace kritva::core::time
