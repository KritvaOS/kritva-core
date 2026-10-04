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

## Direction

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

## Install and use as a library

```bash
cmake -S . -B build
cmake --build build
cmake --install build --prefix <prefix>      # or: make install PREFIX=<prefix>
```

Downstream CMake projects consume the installed package:

```cmake
find_package(kritva_core 0.9 CONFIG REQUIRED)   # add <prefix> to CMAKE_PREFIX_PATH
target_link_libraries(my_target PRIVATE kritva_core::kritva_core)
```

The installed package provides the `kritva_core` library and the public headers under `include/kritva/core/`. It has no third-party dependencies. Pre-1.0, a requested version must match the installed major and minor version. In a source tree that embeds Core with `add_subdirectory`, the same `kritva_core::kritva_core` target is available.
