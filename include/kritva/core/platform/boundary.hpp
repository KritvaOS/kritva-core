//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : boundary.hpp
// Description : Platform adapter boundary: ownership, lifetime, context and error rules.
//
// Component   : Kritva Core
// Module      : Platform Contract
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-004
// API         : CORE-API-PLATFORM
//
// Author      : KritvaOS Core Team
// Created     : 04-10-2026
//==============================================================================

#pragma once
#include "../error/error.hpp"
#include "../types/callback.hpp"
#include <string>
#include <utility>
namespace kritva::core::platform {

//------------------------------------------------------------------------------
// Platform adapter boundary (CORE-PLAT-004)
//
// Kritva Core defines CONTRACTS for platform services (scheduler, clock, timer,
// watchdog, platform identity); platform-specific repositories IMPLEMENT them.
// This file states the rules that every Core-facing platform contract shares.
// It deliberately contains no platform behavior.
//
//   R0.4 establishes platform contracts and integration boundaries; it does not
//   implement a concrete Linux, RTOS, MCU, vendor, Nexus, or Edge platform
//   adapter.
//
// PLATFORM INDEPENDENCE
//   - No Core public header or source includes or links an operating-system,
//     RTOS, vendor SDK/BSP/HAL, hardware-driver, ROS 2/DDS or EtherCAT
//     interface. Core compiles and its tests run on any conforming C++20
//     toolchain without physical hardware. The repository audit enforces this.
//   - Adapters are separate code that includes Core headers and implements the
//     abstract contracts. Core never includes an adapter.
//
// OWNERSHIP AND LIFETIME
//   - The integrator (the application or platform repository) owns every adapter
//     and every service it exposes. Core owns none of them, never creates or
//     destroys a platform object, and never keeps a global or singleton
//     platform; there is no service locator. Core types that refer to a platform
//     object hold a NON-OWNING reference supplied by the integrator.
//   - A platform object must outlive every Core object that holds a reference to
//     it, and keep a stable address while referenced. Using a Core object after
//     the platform object it references was destroyed is undefined behavior
//     that Core cannot detect.
//   - Contracts never transfer ownership through a pointer or reference. There
//     is no shared_ptr, unique_ptr or custom deleter in a platform contract.
//
// CALLBACKS AND OPAQUE CONTEXT
//   - Platform contracts that deliver an event use kritva::core::Callback: a
//     function pointer and an opaque, non-owning context pointer that is passed
//     unchanged. Core never inspects, copies or frees the context.
//   - Every callback-bearing service states: who invokes the callback, where it
//     may execute, whether it may block, allocate or re-enter the service, what
//     lifetime the context must have, and what happens at stop and destruction.
//     Callback functions shall not throw.
//   - A callback does NOT run on a Core-owned thread: Core has none. Where the
//     callback runs (an adapter thread, a timer interrupt, a tick hook) is
//     adapter-defined and must be documented by the adapter.
//
// ERRORS
//   - Adapters report failure only through Result<T> / Result<void> with the
//     existing Error type; contract operations do not throw. The error codes
//     have these meanings across all platform contracts:
//       INVALID_ARGUMENT      the request itself is invalid (null callback,
//                             non-positive period, negative period, ...).
//       INVALID_STATE         the request is valid but not allowed in the
//                             service's current state (start while running).
//       UNSUPPORTED           the request is valid but this platform cannot
//                             provide it.
//       RESOURCE_UNAVAILABLE  the platform ran out of a resource.
//       TIMEOUT               a bounded wait expired.
//     A failed request has no effect (it is atomic) unless the contract says
//     otherwise.
//
// THREAD SAFETY AND REAL TIME
//   - Core imposes no universal thread-safety guarantee on a platform contract:
//     whether a service may be called concurrently is adapter-defined and must
//     be documented by the adapter. Callers serialize calls unless told
//     otherwise.
//   - Core makes no hard-real-time, latency or jitter guarantee through any
//     platform contract. Adapters document what they provide.
//
// make_error() is the only code in this header: a convenience for adapters that
// builds an Error with a code and message (no source or timestamp).
//------------------------------------------------------------------------------
[[nodiscard]] inline Error make_error(ErrorCode code, std::string message,
                                      ErrorSeverity severity = ErrorSeverity::ERROR) {
    return Error{code, severity, {}, {}, std::move(message)};
}

} // namespace kritva::core::platform
