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
// Requirements: CORE-PLAT-003, CORE-PLAT-007
// API         : CORE-API-PLATFORM
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include "../error/result.hpp"
#include "../types/duration.hpp"
namespace kritva::core::platform {

//------------------------------------------------------------------------------
// IWatchdog (CORE-PLAT-003, CORE-PLAT-007)
//
// The platform-neutral watchdog contract. Core defines it; platform adapters
// implement it (rules shared by all platform contracts: platform/boundary.hpp).
// Core contains no watchdog driver, no background kicking service and no thread;
// whether the adapter is a hardware watchdog or a software one is not visible
// through this contract.
//
// STATE
//   A watchdog is either STOPPED (initial) or RUNNING.
//
//   start(timeout)
//     - Valid only while STOPPED. It begins the first timeout window: the
//       watchdog expires if kick() is not called within `timeout`.
//     - Failure returns an Error and has no effect (the watchdog stays STOPPED):
//         INVALID_ARGUMENT     timeout <= 0 (zero or negative).
//         INVALID_STATE        the watchdog is already RUNNING. A running
//                              watchdog is never silently reconfigured: the
//                              timeout of an activation is fixed until stop().
//         UNSUPPORTED          the adapter cannot provide this timeout (outside
//                              its supported range or resolution).
//         RESOURCE_UNAVAILABLE no watchdog resource is available.
//     - The adapter documents its supported timeout range and resolution.
//
//   kick()
//     - Valid only while RUNNING: it restarts the timeout window. kick() on a
//       STOPPED watchdog fails with INVALID_STATE and has no effect; it never
//       starts the watchdog.
//     - A successful kick() is a statement that the monitored activity is alive;
//       Core never kicks on the caller's behalf, because that would defeat the
//       watchdog. The caller owns the decision and the context from which it
//       kicks. Which contexts may kick (thread, interrupt) is adapter-defined.
//
//   stop()
//     - Idempotent: stopping a STOPPED watchdog succeeds and changes nothing.
//     - Stopping a RUNNING watchdog returns it to STOPPED, with no further
//       expiry from that activation. An adapter that cannot disable its
//       watchdog once started (typical for hardware) fails with UNSUPPORTED and
//       the watchdog stays RUNNING; the adapter documents this.
//
//   After stop() the watchdog may be started again with any valid timeout; the
//   new activation carries nothing over from the previous one.
//
// EXPIRY
//   What happens when the timeout elapses is adapter-defined and outside this
//   contract: a reset of the platform, a signal, a recorded fault, or any other
//   action the adapter documents. Core does not define, observe or require any
//   expiry action, and expiry has NO effect on Core: it never triggers
//   runtime::RuntimeManager::reset() or any other Runtime recovery, and the
//   Runtime never kicks, starts or stops a watchdog. Connecting a watchdog to
//   Runtime health is an integrator decision made with the public Runtime API.
//
// FAILURE, OWNERSHIP, THREADS
//   - Failures use Result/Error with the meanings above; they are atomic and
//     never throw.
//   - The integrator owns the watchdog; Core types hold only non-owning
//     references. Teardown of a RUNNING watchdog is adapter-defined (an adapter
//     that cannot stop documents the resulting platform behavior); Core does not
//     specify adapter destructors.
//   - Thread safety is adapter-defined. Operations are control-plane calls (they
//     may allocate and block unless the adapter documents otherwise). Core makes
//     no hard-real-time, latency or jitter guarantee, including for kick().
//------------------------------------------------------------------------------
class IWatchdog {
public:
    virtual ~IWatchdog() = default;
    virtual Result<void> start(Duration timeout) = 0;
    virtual Result<void> kick() = 0;
    virtual Result<void> stop() = 0;
};
} // namespace kritva::core::platform
