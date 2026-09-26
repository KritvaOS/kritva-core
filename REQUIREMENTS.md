//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : REQUIREMENTS.md
// Description : Updated requirements for the API skeleton.
//
// Component   : Kritva Core
// Module      : Requirements
// Layer       : Core Foundation
//
// Requirements: CORE-REQ-002
// API         : CORE-API-REQUIREMENTS
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


# Kritva Core Requirements R0.2

## P0
- CORE-GEN-001 Platform-independent foundation.
- CORE-GEN-002 C++20.
- CORE-GEN-003 No ROS2/DDS/EtherCAT/vendor dependency.
- CORE-GEN-004 Public APIs under include/kritva/core.
- CORE-GEN-005 Independently testable without physical hardware.
- CORE-LIF-002 Define lifecycle transition behavior.
- CORE-LIF-003 Reject invalid lifecycle transitions.
- CORE-CFG-002 Provide a validation path for configuration.
- CORE-PLAT-001 Define scheduler platform contract.
- CORE-PLAT-002 Define clock platform contract.
- CORE-PLAT-003 Define watchdog platform contract.
- CORE-RT-001 Define lifecycle-managed component contract.
- CORE-RT-002 Define runtime contract.
- CORE-MSG-001 Define platform-neutral message identity/header.
- CORE-TIME-001 Define platform-neutral clock abstraction.

## P1
- Rich typed configuration constraints.
- Publisher/subscriber transport abstraction.
- Runtime dependency graph and component manager.
- Lock-free/zero-copy messaging options.
- PTP integration outside Core.
