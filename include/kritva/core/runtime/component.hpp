//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component.hpp
// Description : Base lifecycle-managed Core component contract.
//
// Component   : Kritva Core
// Module      : Runtime
// Layer       : Core Foundation
//
// Requirements: CORE-RT-001
// API         : CORE-API-RUNTIME
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include "component_id.hpp"
#include "component_info.hpp"
#include "../capability/capability_set.hpp"
#include "../configuration/configuration.hpp"
#include "../health/health.hpp"
#include "../lifecycle/lifecycle.hpp"
#include "../error/result.hpp"
#include "../status/status.hpp"
namespace kritva::core::runtime {

//------------------------------------------------------------------------------
// Component (CORE-RT-001)
//
// The platform-independent contract of a lifecycle-managed runtime component.
// It reuses the Core Lifecycle states and transition table (ARCHITECTURE.md,
// "Lifecycle transitions") and the Core Result / Error / Status / Health types.
// It introduces no second lifecycle or error abstraction and no CONFIGURED
// state.
//
// IDENTITY AND METADATA
//   - A component is constructed with a ComponentInfo (valid id, non-empty
//     name, version) and info() returns it. The id is immutable for the
//     component's lifetime: it is held by the base class in a const member.
//   - info() is non-virtual, noexcept, and the returned reference stays valid
//     (and unchanged) until the component is destroyed. It is unaffected by
//     lifecycle operations.
//
// OWNERSHIP AND LIFETIME
//   - Core never owns, copies or deletes components. A component is owned by
//     whoever constructed it. Core types that refer to a component (the
//     registry and runtime, KF-CORE-R03-002/004) hold a NON-OWNING reference;
//     the owner must keep the component alive and at a stable address for as
//     long as it is registered with or used by them.
//   - Components are neither copyable nor movable: they have identity, and the
//     address is part of that identity for non-owning holders.
//   - Destruction: the owner should shutdown() before destroying. Core makes
//     no call on the component after the owner unregisters it / destroys the
//     runtime.
//   - Values returned by status(), health() and capabilities() are snapshots
//     by value; they never refer into the component and cannot dangle.
//
// LIFECYCLE OPERATIONS
// All operations are synchronous: each returns only when it has completed, and
// the transient states INITIALIZING and STOPPING are therefore never observable
// after an operation returns. lifecycle_state() reports the state after the
// last completed operation; a newly constructed component is UNKNOWN.
//
//   operation     valid from            on success        Core transitions used
//   -----------   -------------------   ---------------   ----------------------------
//   configure()   UNKNOWN, STOPPED      state unchanged   none
//   initialize()  UNKNOWN, STOPPED      READY             -> INITIALIZING -> READY
//   start()       READY                 RUNNING           READY -> RUNNING
//   stop()        READY, RUNNING        STOPPED           [-> STOPPING] -> STOPPED
//   shutdown()    UNKNOWN, STOPPED      state unchanged   none (nothing to release)
//                 FAULT                 STOPPED           FAULT -> STOPPED
//
//   - configure() supplies configuration before (re)initialization; it does
//     not change the lifecycle state. A rejected configuration is reported
//     with its Error, is not partially applied, and leaves the state unchanged.
//   - shutdown() releases whatever initialize()/start() acquired and leaves the
//     component re-initializable (STOPPED -> INITIALIZING is a valid transition).
//     It is idempotent in UNKNOWN and STOPPED. It is not valid in READY or
//     RUNNING: stop() first.
//   - recovery: FAULT can only be left by shutdown(). RECOVERING is part of the
//     Core state set but no Component operation produces it; recovery and reset
//     semantics are defined by KF-CORE-R03-006.
//
// Invalid operations (any state/operation pair not listed as valid):
//   - return a failed Result with ErrorCode::INVALID_STATE,
//   - leave the lifecycle state unchanged and have no other effect.
//
// Operation failure (the operation was valid but could not be completed):
//   - initialize(), start(), stop(): the component moves to FAULT (the
//     transition INITIALIZING/READY/RUNNING/STOPPING -> FAULT is in the Core
//     table) and the cause is returned as the failed Result's Error.
//   - shutdown(): the state is unchanged (a FAULT component stays in FAULT)
//     and the Error is returned.
//   - There is no automatic retry or recovery.
//
// ERRORS
//   - Failures are reported only through Result<void>; operations do not throw
//     (they are not noexcept in the interface because implementations may
//     allocate, but a conforming implementation must not let exceptions escape).
//   - Every Error returned by a component carries source == info().id(), so a
//     failure is attributable and inspectable without extra context. Use
//     ErrorCode::INVALID_STATE for invalid operations, ErrorCode::INVALID_ARGUMENT
//     or ErrorCode::CONFIGURATION_ERROR for a rejected configuration, and the
//     most specific Core ErrorCode otherwise.
//
// THREAD SAFETY / REAL TIME
//   - Core imposes no universal thread-safety guarantee on components. Callers
//     must serialize all lifecycle operations on one component. info() is safe
//     to read concurrently because it is immutable. lifecycle_state(), status(),
//     health() and capabilities() are safe concurrently with other calls only if
//     the implementation documents it.
//   - All operations are control-plane: they may allocate and block and are not
//     for use in real-time paths. Core makes no hard-real-time claim.
//   - Core provides no scheduler, executor or threads for components.
//------------------------------------------------------------------------------
class Component {
public:
    virtual ~Component() = default;

    Component(const Component&) = delete;
    Component& operator=(const Component&) = delete;

    /// Immutable identity and metadata. See ComponentInfo.
    [[nodiscard]] const ComponentInfo& info() const noexcept { return info_; }

    virtual Result<void> configure(const Configuration&) = 0;
    virtual Result<void> initialize() = 0;
    virtual Result<void> start() = 0;
    virtual Result<void> stop() = 0;
    virtual Result<void> shutdown() = 0;
    [[nodiscard]] virtual LifecycleState lifecycle_state() const noexcept = 0;
    [[nodiscard]] virtual Status status() const = 0;
    [[nodiscard]] virtual Health health() const = 0;
    [[nodiscard]] virtual CapabilitySet capabilities() const = 0;

protected:
    explicit Component(ComponentInfo info) noexcept : info_(std::move(info)) {}

private:
    const ComponentInfo info_;
};
} // namespace kritva::core::runtime
