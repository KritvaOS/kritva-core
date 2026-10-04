//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : BOUNDARIES.md
// Description : Architectural dependency boundaries and ownership.
//
// Component   : Kritva Core
// Module      : Architecture
// Layer       : Core Foundation
//
// Requirements: CORE-ARCH-001
// API         : CORE-API-BOUNDARY
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


# Kritva Core Boundaries

## Core owns

- Foundational types
- Lifecycle and Component contracts
- Status and Health
- Error and Result
- Events and Statistics
- Capability contracts
- Configuration contracts
- Runtime and dependency-ordering contracts
- Messaging contracts
- Time abstractions
- Context and platform contracts

## Core does not own

- Linux/PREEMPT_RT implementations
- RTOS/MCU implementations
- STM32/TI/NXP/vendor BSPs or SDKs
- ROS 2/DDS
- EtherCAT implementations
- Hardware drivers
- Sensor or motor algorithms
- AI, perception, planning, and skills
- Concrete Nexus or Edge platform implementations

## Dependency direction

```text
Application / Integration
          |
          v
Concrete Platform
          |
          v
     Kritva Core
```

Core must not depend upward on application/platform implementations.

## R0.9 genericity rule

Capability, Requirement, Dependency, Readiness, Health, and Lifecycle are separate concepts. Core must not infer platform-specific architecture from these concepts.
