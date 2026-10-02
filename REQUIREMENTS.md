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
- CORE-PLAT-001 Define scheduler platform contract: TaskConfig field semantics (priority is implementation-independent and relative with no Core-defined range, cpu_affinity is a bit mask where 0 means unconstrained, period 0 means aperiodic), non-owning context and entry lifetime, create_task registers inactive tasks while stopped with dynamic creation while running left to the adapter, idempotent start/stop, opaque TaskId with 0 invalid and unique among existing tasks, and atomic reported (not thrown) failures.
- CORE-PLAT-002 Define clock platform contract.
- CORE-PLAT-003 Define watchdog platform contract.
- CORE-RT-001 Define lifecycle-managed component contract.
- CORE-RT-002 Define runtime contract.
- CORE-MSG-001 Define platform-neutral message identity/header.

### Foundation types
- CORE-TYP-001 Provide a strong 64-bit identity type; value 0 is the invalid identity.
- CORE-TYP-002 Provide a Version type (major.minor.patch) with string formatting.
- CORE-TYP-003 Provide a nanosecond-resolution Duration type.
- CORE-TYP-004 Provide string key/value Metadata.
- CORE-TIME-001 Provide Timestamp (nanoseconds + clock domain) and the platform-neutral clock abstraction (time::IClock). time::IClock is the canonical clock contract; each clock instance has one fixed domain (MONOTONIC: non-decreasing, unspecified epoch; REALTIME: wall clock that may step). Timestamps from different domains are incomparable: equality includes the domain and Timestamp has no ordering or subtraction. platform::IClock is a compatibility alias of the same type, not a second abstraction. Thread-safety of now() is adapter-defined.
- CORE-TIME-002 Define platform-neutral timer abstraction.

### Lifecycle, status, health
- CORE-LIF-001 Define lifecycle states.
- CORE-STA-001 Define status codes and a Status (code + message) type.
- CORE-HEA-001 Define health states.
- CORE-HEA-002 Provide a Health type reporting a health state.

### Statistics
- CORE-STS-001 Provide a Counter (monotonic unsigned, wraps modulo 2^64 on overflow; not thread-safe, no allocation).
- CORE-STS-002 Provide a Gauge (signed instantaneous value stored as-is, no clamping; not thread-safe, no allocation).
- CORE-STS-003 Provide a Statistics aggregate of common operational counters and gauges with documented field meanings (utilization is a whole percent by convention); not thread-safe, not an atomic snapshot.

### Error and result
- CORE-ERR-001 Define error codes and severities.
- CORE-ERR-002 Define the Error record (code, severity, source, timestamp, message).
- CORE-ERR-003 Reserved (unused).
- CORE-ERR-004 Provide Result<T>/Result<void> with documented access preconditions: value() only on success, error() only on failure, and failure() only with a non-NONE error code. Precondition violations are asserted in debug builds and undefined behavior in release builds; moved-from Results keep their outcome with an unspecified payload.

### Events
- CORE-EVT-001 Define event types.
- CORE-EVT-002 Event carries event identity and source identity.
- CORE-EVT-003 Event carries a timestamp and severity.
- CORE-EVT-004 Event carries a correlation identity.

### Capability and configuration
- CORE-CAP-001 Define capability identity.
- CORE-CAP-002 Define a Capability record.
- CORE-CAP-003 Provide a CapabilitySet; adding an existing identity replaces it.
- CORE-CFG-001 Define typed configuration Parameters and a Configuration container.
- CORE-CFG-003 Define a configuration version (alias of Version).

### Repository, build and documentation
These are process/infrastructure requirements for the repository itself. They are verified by inspection or by the checks named in the process traceability table, not by unit tests.
- CORE-REQ-002 Maintain this requirements document and its traceability tables, audited by scripts/audit/check_traceability.py.
- CORE-BUILD-001 Provide a CMake build (and Make convenience targets) that builds Core and runs its tests.
- CORE-TEST-001 Provide CI that builds Core and runs the CTest suite on push and pull request.
- CORE-DOC-001 Provide a repository README describing Kritva Core.
- CORE-ARCH-001 Document the architectural dependency boundaries (what Core owns and does not own).
- CORE-ARCH-003 Record the R0.2 architecture audit status.
- CORE-DEV-001 Document the bounded work-package rules for contributors.

### API umbrella and messaging
- CORE-API-001 Provide the umbrella header kritva/core/core.hpp.
- CORE-MSG-002 Define platform-neutral topic identity.

## P1
- Rich typed configuration constraints.
- Publisher/subscriber transport abstraction.
- Runtime dependency graph and component manager.
- Lock-free/zero-copy messaging options.
- PTP integration outside Core.

## Traceability (Requirement -> Header -> Implementation -> Test)

Checked by `make traceability-check` (scripts/audit/check_traceability.py): every referenced requirement ID is defined exactly once, every defined ID has a row, every named file exists, every public header appears here and carries its row's ID in its `Requirements:` tag, and every test source is registered with CTest.

| Requirement | Public header | Implementation | Test |
|---|---|---|---|
| CORE-GEN-004 | include/kritva/core/ | - | tests/contract/core_contract_test.cpp |
| CORE-API-001 | core.hpp | - | tests/contract/core_contract_test.cpp |
| CORE-TYP-001 | types/id.hpp | header-only | tests/unit/types_test.cpp |
| CORE-TYP-002 | types/version.hpp | src/version.cpp | tests/unit/version_test.cpp, tests/unit/types_test.cpp |
| CORE-TYP-003 | types/duration.hpp | header-only | tests/unit/types_test.cpp |
| CORE-TYP-004 | types/metadata.hpp | src/metadata.cpp | tests/unit/types_test.cpp |
| CORE-TIME-001 | types/timestamp.hpp, time/clock.hpp | header-only | tests/unit/time_test.cpp |
| CORE-TIME-002 | time/timer.hpp | header-only | tests/unit/time_test.cpp |
| CORE-LIF-001 | lifecycle/lifecycle_state.hpp | - | tests/unit/lifecycle_test.cpp |
| CORE-LIF-002, CORE-LIF-003 | lifecycle/lifecycle.hpp | src/lifecycle.cpp | tests/unit/lifecycle_test.cpp |
| CORE-STA-001 | status/status_code.hpp, status/status.hpp | header-only | tests/unit/status_test.cpp |
| CORE-HEA-001, CORE-HEA-002 | health/health_state.hpp, health/health.hpp | header-only | tests/unit/health_test.cpp |
| CORE-STS-001 | statistics/counter.hpp | header-only | tests/unit/statistics_test.cpp |
| CORE-STS-002 | statistics/gauge.hpp | header-only | tests/unit/statistics_test.cpp |
| CORE-STS-003 | statistics/statistics.hpp | header-only | tests/unit/statistics_test.cpp |
| CORE-ERR-001 | error/error_code.hpp | header-only | tests/unit/error_test.cpp |
| CORE-ERR-002 | error/error.hpp | header-only | tests/unit/error_test.cpp |
| CORE-ERR-004 | error/result.hpp | header-only | tests/unit/result_test.cpp |
| CORE-EVT-001..004 | event/event_type.hpp, event/event.hpp | header-only | tests/unit/event_test.cpp |
| CORE-CAP-001 | capability/capability_id.hpp | header-only | tests/unit/capability_test.cpp |
| CORE-CAP-002 | capability/capability.hpp | header-only | tests/unit/capability_test.cpp |
| CORE-CAP-003 | capability/capability_set.hpp | src/capability_set.cpp | tests/unit/capability_test.cpp |
| CORE-CFG-001, CORE-CFG-002 | configuration/parameter.hpp, configuration/configuration.hpp | src/configuration.cpp | tests/unit/configuration_test.cpp |
| CORE-CFG-003 | configuration/configuration_version.hpp | header-only | tests/unit/configuration_test.cpp |
| CORE-RT-001 | runtime/component.hpp | header-only | tests/unit/runtime_test.cpp |
| CORE-RT-002 | runtime/runtime.hpp | header-only | tests/unit/runtime_test.cpp |
| CORE-MSG-001 | messaging/message.hpp | header-only | tests/unit/messaging_test.cpp |
| CORE-MSG-002 | messaging/topic.hpp | header-only | tests/unit/messaging_test.cpp |
| CORE-PLAT-001 | platform/scheduler.hpp | contract only | tests/unit/platform_test.cpp, tests/contract/scheduler_contract.hpp |
| CORE-PLAT-002 | platform/clock.hpp (alias of time/clock.hpp) | contract only | tests/unit/platform_test.cpp |
| CORE-PLAT-003 | platform/watchdog.hpp | contract only | tests/unit/platform_test.cpp |

## Process traceability (Requirement -> Artifact -> Verification)

Artifact and Verification hold file paths, or `inspection` where no automated check exists. The Note column is explanatory only. Nothing here implies behavior beyond what the artifact contains.

| Requirement | Artifact | Verification | Note |
|---|---|---|---|
| CORE-GEN-001 | docs/architecture/boundaries.md | inspection | Boundary review; GEN-003 audit covers the dependency part. |
| CORE-GEN-002 | CMakeLists.txt | inspection | Library target requires `cxx_std_20`. |
| CORE-GEN-003 | CMakeLists.txt | scripts/audit/check_traceability.py | Audit rejects find_package/FetchContent/ExternalProject/add_subdirectory and ROS2/DDS/EtherCAT includes. |
| CORE-GEN-005 | CMakeLists.txt | scripts/audit/check_traceability.py | Audit requires every test source to be registered with CTest; the tests use no hardware. |
| CORE-REQ-002 | REQUIREMENTS.md | scripts/audit/check_traceability.py | |
| CORE-BUILD-001 | CMakeLists.txt, Makefile | inspection | |
| CORE-TEST-001 | .github/workflows/kritva-core-ci.yml | inspection | |
| CORE-DOC-001 | README.md | inspection | |
| CORE-ARCH-001 | docs/architecture/boundaries.md | inspection | |
| CORE-ARCH-003 | docs/architecture/audit-status.md | inspection | |
| CORE-DEV-001 | docs/development/intern-work-package.md | inspection | |
