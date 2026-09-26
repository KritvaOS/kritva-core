//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : audit-status.md
// Description : Audit decision summary.
//
// Component   : Kritva Core
// Module      : Architecture
// Layer       : Core Foundation
//
// Requirements: CORE-ARCH-003
// API         : CORE-API-AUDIT
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


# Kritva Core R0.2 Audit Status

## KEEP
- capability
- configuration
- error
- event
- health
- lifecycle
- statistics
- status
- types
- existing engineering documentation and header policy

## MODIFY
- lifecycle: replace unrestricted set_state with validated transitions
- configuration: implement basic validation and reserve constraint framework
- capability_set: add lookup/duplicate handling
- timestamp: remove platform clock acquisition from the Core value type
- statistics: keep primitives generic; robot-specific metrics belong above Core
- CMake: switch from header-only INTERFACE target to compiled library because APIs have implementations

## ADD
- runtime
- messaging
- time
- platform contracts
- initial unit/contract tests
- file manifest
- architecture boundary document

## DO NOT ADD
- Linux/STM32/TI code
- ROS 2/DDS
- EtherCAT
- PREEMPT_RT source
- hardware drivers
