# R07 Design Consult

## Decision

APPROVED — Component Operational Foundation direction confirmed.

## Core Decision

R0.7 should not introduce a new generic operational-state framework merely because `Status`, `Health`, `Statistics` and `Event` exist. Instead, R0.7 establishes controlled Component operational observation/reporting semantics around the existing typed Core concepts.

## Architecture Questions and Decisions

### D01 — Operational state

No new Component Operational State machine. Lifecycle, Status and Health remain distinct.

### D02 — Ownership

Component owns/report its operational information. Core does not maintain a second authoritative copy.

### D03 — Health

Health is Component-reported information independent of Runtime FAULT. Runtime does not interpret Health as recovery policy.

### D04 — Diagnostics

Do not introduce a generic `Diagnostic` container. Existing typed diagnostics remain authoritative: Error, fault_error(), Statistics, state(), Status and Health.

### D05 — Events

Component Events may be explicitly reported through an integrator-owned sink. No Core EventBus, queue or broker.

### D06 — Event semantics

Events describe occurrences. Events do not implicitly command start/stop/reset/retry/reconfigure/watchdog behavior.

### D07 — Statistics

Component operational statistics are Component-owned and separate from Runtime statistics. Statistics remain optional.

### D08 — Operational access

Do not broaden `ComponentContext` into a general Component management interface without a separate reviewed use case.

### D09 — Runtime observation

Runtime may expose read-only operational observation only where justified, but must not interpret it as lifecycle policy in R0.7.

### D10 — Out-of-scope

No Core logging backend, telemetry transport, event bus, background worker, platform-specific operational semantics, health-driven recovery, automatic retry/restart, ROS2/DDS, EtherCAT, or concrete platform implementation.
