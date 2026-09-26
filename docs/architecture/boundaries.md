//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : boundaries.md
// Description : Architectural dependency boundaries.
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
- Lifecycle and component contracts
- Status and health
- Error and Result
- Events
- Capabilities
- Configuration
- Runtime and messaging contracts
- Time abstractions
- Platform contracts

## Core does not own
- Linux/PREEMPT_RT implementation
- FreeRTOS/RTOS implementation
- STM32/TI/NXP BSPs
- ROS 2/DDS
- EtherCAT implementation
- Hardware drivers
- Sensor/motor algorithms
- AI, perception, planning, skills

## Dependency direction

project -> integration/platform -> kritva-core

Core never depends upward or on a vendor/OS implementation.
