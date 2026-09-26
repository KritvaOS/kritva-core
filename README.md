//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : README.md
// Description : Updated project overview.
//
// Component   : Kritva Core
// Module      : Documentation
// Layer       : Core Foundation
//
// Requirements: CORE-DOC-001
// API         : CORE-API-README
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


# Kritva Core

Platform-independent runtime foundation for the Kritva Open Robotic Computing Platform.

## V0.2 direction

Core is an API-first, platform-independent library. Platform implementations are supplied by the parent `kritvaos-community` repository.

### Core owns
Types, lifecycle, status, health, errors, events, capabilities, configuration, runtime contracts, messaging contracts, time abstractions and platform contracts.

### Core does not own
Linux/PREEMPT_RT, FreeRTOS, STM32/TI BSPs, ROS 2/DDS, EtherCAT, vendor drivers or robot algorithms.

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```
