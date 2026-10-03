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
// Requirements: CORE-CFG-001; CORE-CFG-002; CORE-CFG-004; CORE-CFG-011
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
