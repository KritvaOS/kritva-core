# Kritva Core Requirements

## 1. Purpose

Initial requirements for Kritva Core R0.1.

## 2. Conventions

Requirement IDs use `CORE-<DOMAIN>-<NUMBER>`.

Priority: P0 required for R0.1; P1 important but may follow initial implementation; P2 future.

## 3. General

| ID | Priority | Requirement |
|---|---|---|
| CORE-GEN-001 | P0 | Core shall provide platform-independent foundational contracts. |
| CORE-GEN-002 | P0 | Core shall target C++20. |
| CORE-GEN-003 | P0 | Core shall not require ROS2, DDS, EtherCAT, or vendor-specific drivers. |
| CORE-GEN-004 | P0 | Public APIs shall be exposed under `include/kritva/core/`. |
| CORE-GEN-005 | P0 | Core shall be testable independently of physical hardware. |

## 4. Identity and Types

| ID | Priority | Requirement |
|---|---|---|
| CORE-TYP-001 | P0 | Provide stable identity abstraction. |
| CORE-TYP-002 | P0 | Provide version representation. |
| CORE-TYP-003 | P0 | Provide timestamp and duration abstractions. |
| CORE-TYP-004 | P0 | Provide metadata representation. |

## 5. Lifecycle

| ID | Priority | Requirement |
|---|---|---|
| CORE-LIF-001 | P0 | Represent lifecycle state. |
| CORE-LIF-002 | P0 | Define lifecycle transition behavior. |
| CORE-LIF-003 | P0 | Detect invalid lifecycle transitions. |

Initial states: `UNKNOWN`, `INITIALIZING`, `READY`, `RUNNING`, `STOPPING`, `STOPPED`, `FAULT`, `RECOVERING`.

## 6. Status and Health

| ID | Priority | Requirement |
|---|---|---|
| CORE-STA-001 | P0 | Provide operational status. |
| CORE-HEA-001 | P0 | Provide health state independently from status. |
| CORE-HEA-002 | P0 | Represent status and health independently. |

Example: `Status = RUNNING`, `Health = DEGRADED`.

## 7. Statistics

| ID | Priority | Requirement |
|---|---|---|
| CORE-STS-001 | P0 | Support counters. |
| CORE-STS-002 | P0 | Support gauges. |
| CORE-STS-003 | P0 | Support operational diagnostics. |

Initial counter examples: `sample_count`, `error_count`, `retry_count`, `drop_count`.

## 8. Error and Result

| ID | Priority | Requirement |
|---|---|---|
| CORE-ERR-001 | P0 | Provide structured error representation. |
| CORE-ERR-002 | P0 | Support severity. |
| CORE-ERR-003 | P0 | Identify error source. |
| CORE-ERR-004 | P0 | Provide `Result<T>` for operational failures. |

## 9. Events

| ID | Priority | Requirement |
|---|---|---|
| CORE-EVT-001 | P0 | Provide a common event envelope. |
| CORE-EVT-002 | P0 | Events contain source identity. |
| CORE-EVT-003 | P0 | Events contain timestamp information. |
| CORE-EVT-004 | P0 | Events support correlation where required. |

## 10. Capability

| ID | Priority | Requirement |
|---|---|---|
| CORE-CAP-001 | P0 | Represent capability identity. |
| CORE-CAP-002 | P0 | Represent capability version. |
| CORE-CAP-003 | P0 | Support capability sets. |

## 11. Configuration

| ID | Priority | Requirement |
|---|---|---|
| CORE-CFG-001 | P0 | Represent typed configuration parameters. |
| CORE-CFG-002 | P0 | Support parameter constraints. |
| CORE-CFG-003 | P0 | Represent configuration version/schema information. |
| CORE-CFG-004 | P0 | Not mandate serialization or persistence technology. |

## 12. Traceability

Each implemented requirement should map to:

`Requirement → API → Source → Test → Verification Result`.
