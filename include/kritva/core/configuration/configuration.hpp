//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : configuration.hpp
// Description : In-memory configuration container with basic validation.
//
// Component   : Kritva Core
// Module      : Configuration
// Layer       : Core Foundation
//
// Requirements: CORE-CFG-001; CORE-CFG-002; CORE-CFG-004; CORE-CFG-005; CORE-CFG-006; CORE-CFG-011
// API         : CORE-API-CONFIGURATION
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include "parameter.hpp"
#include "../error/result.hpp"
#include <cstddef>
#include <string>
#include <unordered_map>
namespace kritva::core {

//------------------------------------------------------------------------------
// COMPONENT CONFIGURATION CONTRACT: LIFECYCLE ELIGIBILITY (CORE-CFG-004, CORE-CFG-011)
//
// This block is the normative statement of when and how a Configuration is
// applied to a Component. It restates and hardens what runtime::Component
// (component.hpp) and runtime::RuntimeManager (runtime_manager.hpp) already say;
// those headers are unchanged and remain authoritative for the lifecycle table.
//
// WHAT CONFIGURATION IS
//   A Configuration is a caller-owned, detached CONTROL-PLANE VALUE handed
//   synchronously to Component::configure(const Configuration&). The Component
//   owns the meaning of what it accepts; Core owns only the generic
//   representation and this contract. Core keeps no second, authoritative copy
//   of any component's configuration and has no configuration registry, store,
//   server, broker, persistence, event or background worker.
//
// WHEN IT MAY BE APPLIED
//   configure() is valid ONLY from the lifecycle states UNKNOWN and STOPPED.
//     - From READY, RUNNING and FAULT (and from the transient INITIALIZING and
//       STOPPING and the never-produced RECOVERING, which are not observable
//       because every operation is synchronous) it fails with
//       ErrorCode::INVALID_STATE, source = the component's id, and has NO other
//       effect: the lifecycle state is unchanged and nothing is applied,
//       validated or consumed.
//     - A successful configure() leaves the lifecycle state UNCHANGED
//       (UNKNOWN stays UNKNOWN, STOPPED stays STOPPED); it does not initialize,
//       start or otherwise advance the component. A rejected configure() in a
//       valid state also leaves the state unchanged.
//     - The same rule applies to RuntimeManager::configure(): valid only while
//       the Runtime is UNKNOWN or STOPPED, INVALID_STATE otherwise with no
//       component invoked and no state or statistics change; success leaves the
//       Runtime state unchanged. See runtime_manager.hpp for forwarding and order.
//
// WHAT DOES NOT EXIST (no dynamic reconfiguration)
//   There is no CONFIGURED, RECONFIGURING or equivalent lifecycle state, and no
//   reconfigure(), set_parameter(), get_parameter(), apply or update operation
//   and no generic Component::configuration() accessor, on Component,
//   RuntimeManager or ComponentContext. A component that is READY or RUNNING
//   cannot be reconfigured through Core: stop it (and, if the integrator
//   wishes, shut it down), then configure() again from STOPPED. Live tuning,
//   parameter services and remote configuration are outside Core.
//
// SYNCHRONOUS CONTROL-PLANE BEHAVIOR (CORE-CFG-011)
//   configure() runs entirely on the caller's thread and returns only when it
//   has completed or failed. It may allocate and block (a Configuration is a
//   container of strings), uses no Core thread, executor, callback or
//   background activity, and Core makes NO real-time claim for it. It is not
//   for real-time paths. A Configuration, like Status and Statistics, is not
//   thread-safe: concurrent const reads are safe, any concurrent mutation needs
//   external synchronization.
//
// INPUT OWNERSHIP AND DETACHMENT (CORE-CFG-005)
//   The CALLER owns the Configuration it passes. configure() receives it by
//   const reference, so a component cannot modify it, and it is valid only for
//   the duration of the call. After configure() returns, successfully or not, the
//   caller may change, move or destroy its object and the component must be
//   unaffected:
//     - A conforming Component COPIES whatever it keeps. It does not retain the
//       address of, a reference to, a pointer into (including the pointer
//       returned by get()) or a view of the caller's Configuration or of any
//       Parameter, string or value inside it beyond the synchronous call.
//     - A Configuration is a plain copyable value: a copy is independent of its
//       source, and mutating either never changes the other. The pointer returned
//       by Configuration::get() is valid only until the next mutation of that
//       Configuration or its destruction.
//     - Core never copies, stores or caches a component's Configuration: the
//       Runtime forwards the caller's object by reference during the call and
//       keeps nothing; there is no Core configuration registry, global store or
//       second authoritative copy, and no generic Component::configuration()
//       accessor is required (or provided) to read back what a component applied.
//       The component owns the semantic state it accepts and decides whether and
//       how to expose it.
//
// ATOMIC, NON-PARTIAL APPLICATION (CORE-CFG-006)
//   A conforming Component applies a configuration all-or-nothing:
//     - On success, the configuration it accepted is its applied configuration.
//     - On ANY failure (invalid state, malformed input, semantic rejection,
//       resource failure, or an incompatible schema) NOTHING is applied: the
//       previously accepted configuration, or the component's initial state if it
//       never accepted one, stays exactly as it was. No parameter of the rejected
//       configuration may be visible in the component's applied state and no
//       partially updated state may result. The usual way to meet this is to
//       validate and stage the whole input first and commit it with a step that
//       cannot fail (for example moving or swapping a prepared value).
//     - Failure leaves the lifecycle state unchanged (CORE-CFG-004).
//   Core can state this rule but cannot enforce it on a component; it is the
//   contract a conforming component meets and the reference conformance checks
//   verify. There are no transactions across components: the Runtime does not
//   roll back components that were configured before another one failed.
//
// INDEPENDENCE
//   Configuration is independent of Status, Health and a Runtime FAULT: a
//   configuration failure never changes Status, Health or the Runtime state by
//   itself, and none of them is consulted to decide whether to configure.
//------------------------------------------------------------------------------
class Configuration {
public:
    [[nodiscard]] Result<void> validate() const;
    Result<void> set(Parameter parameter);
    [[nodiscard]] const Parameter* get(const std::string& name) const noexcept;
    [[nodiscard]] bool contains(const std::string& name) const noexcept;
    [[nodiscard]] std::size_t size() const noexcept { return parameters_.size(); }
private:
    std::unordered_map<std::string, Parameter> parameters_;
};
} // namespace kritva::core
