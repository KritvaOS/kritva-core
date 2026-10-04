# Kritva Core — Master Milestone Roadmap

## Milestone Lifecycle

PLANNED → IN PROGRESS → IMPLEMENTATION COMPLETE → REVIEW → ACCEPTED → RELEASED

## KF-CORE-R02 — Core Contract Hardening

Status: RELEASED (`kritva-core-r0.2`)

## KF-CORE-R03 — Runtime Foundation

Status: RELEASED (`kritva-core-r0.3`)

## KF-CORE-R04 — Platform Abstraction

Status: RELEASED (`kritva-core-r0.4`)

R0.4 established stable platform contracts and the Runtime/platform integration boundary without implementing concrete platforms.

## KF-CORE-R05 — Platform Runtime Integration Foundation

Status: RELEASED (`kritva-core-r0.5` -> `adf8ac2`, published to origin)

### Objective

Establish a controlled, platform-independent mechanism by which Kritva Core functionality can explicitly consume externally owned platform services while preserving Runtime determinism, platform ownership and the R0.4 contracts.

### Task Order

```text
R05-001 Platform Context & Service Access Model
        ↓
R05-002 Platform Service Requirement Model
        ↓
R05-003 Explicit Platform Service Consumption
        ↓
R05-004 Runtime–Platform Lifecycle Boundary
        ↓
R05 Platform API Review
        ↓
R05-005 Reference Platform Integration
        ↓
R05-006 Platform Integration & Runtime Tests
        ↓
R05 Platform Integration Freeze
        ↓
R05-007 Full R0.5 Validation
        ↓
R05 Release Gate
```

### Architectural Rules

- `IPlatformAdapter` remains the authoritative platform boundary.
- `PlatformContext` is a non-owning view, not a service registry.
- Platform service ownership/lifecycle remains external.
- Service use is explicit.
- Capability identity is authoritative.
- Runtime lifecycle semantics remain unchanged.
- No Core-owned background execution.
- No concrete platform implementation in `kritva-core`.
- Breaking API/semantic changes require architecture review.

See `planning/milestones/KF-CORE-R05/` for the complete proposal and task acceptance package.

## KF-CORE-R06 — Component Execution Context

Status: RELEASED (`kritva-core-r0.6` -> `a4c41aa`, published to origin)

### Objective

Provide integrator-written Components with one explicit, deterministic, non-owning context for accessing approved operational services while preserving the R0.3 Runtime lifecycle semantics and the R0.5 platform ownership boundary.

### R0.6 Scope Status

R0.6 is implemented and accepted (R06-001..007, Component API Review PASS / FROZEN, Integration Freeze PASS / HONORED, Release Gate PASS). Version 0.6.0; release candidate `b473e5d`.

### Task Order

```text
R06-001 Component Execution Context & Ownership Model
        ↓
R06-002 Operational Context Services & Access Policy
        ↓
R06-003 Context Injection Without Runtime Lifecycle Change
        ↓
R06-004 Context Requirements & Capability Binding
        ↓
R06 Component API Review
        ↓
R06-005 Reference Context Harness & Contract Tests
        ↓
R06-006 Runtime/Component Context Integration Tests
        ↓
R06 Integration Freeze
        ↓
R06-007 Full R0.6 Validation
        ↓
R06 Release Gate
```

### R0.6 Architectural Rules

- R0.5 remains authoritative.
- `IPlatformAdapter` and `PlatformContext` are not replaced by R0.6.
- Context is not a service registry or locator.
- Core does not acquire ownership of platform services or integrator resources.
- Context access is explicit and has no hidden lifecycle side effects.
- Runtime lifecycle semantics remain unchanged unless an explicit architecture review approves a change.
- Integration tests use public APIs only.
- No concrete Linux/RTOS/MCU/vendor/Nexus/Edge platform enters `kritva-core`.
- No Core-owned background execution or automatic recovery is introduced.
- Any breaking or semantic API change returns to architecture review before implementation continues.

See `planning/milestones/KF-CORE-R06/` for the R0.6 architecture, task package, validation evidence and release records.


## KF-CORE-R07 — Component Operational Foundation

Status: RELEASED (`kritva-core-r0.7` -> `424984f`, published to origin)

### Objective

Establish a narrow, platform-independent Component operational observation/reporting contract using existing Core concepts (`Status`, `Health`, `Statistics`, and `Event`) without changing Runtime lifecycle semantics or introducing a Core operational framework.

### Architecture Confirmation

- Component remains authoritative for its operational information.
- Runtime remains authoritative for lifecycle orchestration and Runtime-owned statistics.
- Existing typed operational concepts are preferred over parallel abstractions.
- Observation is read-only and side-effect free.
- No new Component Operational State machine is introduced.
- Component statistics are optional; they are not mandatory on the base `runtime::Component` interface.
- Events are explicitly reported to integrator-owned sinks; Core does not provide an EventBus, queue, broker or dispatcher.
- Operational information does not automatically drive lifecycle, recovery, retry or restart.
- No Core-owned background execution, telemetry backend, logging backend or platform-specific implementation is introduced.

### Task Order

```text
R07 Design Consult
        ↓
R07 Scope Confirmation
        ↓
R07-001 Component Operational Observation Contract
        ↓
R07-002 Component Status & Health Reporting Contract
        ↓
R07-003 Component Operational Event Contract
        ↓
R07-004 Component Statistics Ownership & Observation Contract
        ↓
R07 Component Operational API Review
        ↓
R07-005 Reference Operational Harness & Contract Tests
        ↓
R07-006 Runtime/Component Operational Integration
        ↓
R07 Integration Freeze
        ↓
R07-007 Full R0.7 Validation
        ↓
R07 Release Gate
```

See `planning/milestones/KF-CORE-R07/` for the complete architecture, scope, task acceptance and gate package.


## KF-CORE-R08 — Component Configuration Foundation

Status: RELEASED (`kritva-core-r0.8` -> `cbbec81`, published to origin)

### Objective

Establish a precise, platform-independent Component Configuration Contract around the existing `Configuration`, `ConfigurationVersion`, `Component::configure()` and `RuntimeManager::configure()` path while preserving the R0.3 Runtime lifecycle model, R0.6 ComponentContext boundary and R0.7 operational separation.

### Task Order

```text
R08 Design Consult
        ↓
R08 Scope Confirmation
        ↓
R08-001 Component Configuration Contract & Lifecycle Semantics
        ↓
R08-002 Configuration Ownership & Atomic Application
        ↓
R08-003 Configuration Version & Validation Contract
        ↓
R08 Configuration API Review
        ↓
R08-004 Reference Configuration Harness & Contract Tests
        ↓
R08-005 Runtime/Component Configuration Integration
        ↓
R08 Integration Freeze
        ↓
R08-006 Configuration Boundary & Regression Validation
        ↓
R08-007 Full R0.8 Validation & Release Candidate
        ↓
R08 Release Gate
```

### Architectural Rules

- Configuration is a detached control-plane value.
- `configure()` is valid only in `UNKNOWN` and `STOPPED` for R0.8.
- Configuration failure is state-preserving and non-partial.
- Component owns applied semantic configuration; Core owns generic contract only.
- `ConfigurationVersion` means schema/contract compatibility version.
- Runtime forwards configuration and does not interpret, retry, rollback or persist it.
- Configuration failure is independent of Runtime FAULT, Status and Health.
- `ComponentContext` remains unchanged.
- No dynamic reconfiguration, parameter server, persistence, remote configuration, event infrastructure or robotics-specific parameter framework.
- No concrete platform implementation enters `kritva-core`.

### Planned Effort

22–31 ED estimate; actual effort not yet recorded.

See `planning/milestones/KF-CORE-R08/` for the complete architecture, scope, task acceptance package and gate records.


## KF-CORE-R09 — Capability Contract & Readiness Boundary

Status: RELEASED / SYNCHRONIZED / CLOSED — `kritva-core-r0.9` (0.9.0, release-record commit `d72343a`).

### Objective

Harden and document the existing Capability, CapabilitySet, requirement and lifecycle/readiness boundaries without introducing a generic dependency-management framework or coupling Core to future Nexus, Edge, Linux, MCU, EtherCAT, ROS2/DDS, vendor or hardware architecture.

### Architectural Rules

- Capability, Requirement, Component Dependency, Lifecycle, Readiness and Health remain distinct concepts.
- Existing Core mechanisms are preferred; R0.9 is API-neutral by default.
- `CapabilityId` is the authoritative capability identity.
- Capability metadata is descriptive and not security evidence.
- Runtime remains lifecycle authority.
- DependencyGraph remains ComponentId-based and separate from capability matching.
- Core does not calculate generic readiness automatically and does not add a new readiness lifecycle state.
- No ServiceRegistry, locator, resolver, dependency-injection framework or dynamic discovery is introduced.
- API documentation is canonical Markdown under `docs/api/`; documentation changes accompany accepted API/semantic changes.
- Security impact is assessed explicitly; security mechanisms remain out of scope unless separately approved.
- Concrete platform and Nexus/Edge implementations remain outside Core.

### Task Order

```text
R09 Design Consult
        ↓
R09 Scope Confirmation
        ↓
R09-001 Capability Contract & Provider Semantics
        ↓
R09-002 CapabilitySet Invariants & Version Semantics
        ↓
R09-003 Requirement / Capability Matching Boundary
        ↓
R09 Capability API Review
        ↓
R09-004 Reference Capability & Requirement Harness
        ↓
R09-005 Component Readiness / Lifecycle Boundary Integration
        ↓
R09 Integration Freeze
        ↓
R09-006 API Documentation, Security & Boundary Validation
        ↓
R09-007 Full R0.9 Validation & Release Candidate
        ↓
R09 Release Gate
```

### Planned Effort

22–30 ED estimate; actual effort remains unrecorded until supported by evidence.

See `planning/milestones/KF-CORE-R09/` for the complete architecture, task acceptance and gate package.


## KF-CORE-R10 — Core 1.0 API Maturity & Compatibility Foundation

Status: PLANNED — Design Consult APPROVED; Scope Confirmation pending; implementation not authorized.

### Objective

Establish Kritva Core as a stable 1.x platform-independent foundation by defining and validating the long-term API evolution, source/semantic compatibility, ABI policy, versioning, deprecation, package/install compatibility, migration, documentation and release-contract rules required for `1.0.0` and subsequent 1.x releases.

### Task Order

```text
R10 Design Consult
        ↓
R10 Scope Confirmation
        ↓
R10-001 Public API Inventory & Compatibility Classification
        ↓
R10-002 Source & Semantic Compatibility Contract
        ↓
R10-003 ABI / Binary Compatibility Policy
        ↓
R10-004 Versioning & API Evolution Policy
        ↓
R10-005 Deprecation & Migration Policy
        ↓
R10 API / Compatibility Review
        ↓
R10-006 Compatibility & Boundary Validation Harness
        ↓
R10-007 Package / Install Compatibility
        ↓
R10 Integration Freeze
        ↓
R10-008 Documentation / Security / Traceability Validation
        ↓
R10-009 Full Validation & Release Candidate
        ↓
R10 Release Gate
```

### Architectural Rules

- R0.9 remains the normative pre-1.0 functional/API baseline.
- Source compatibility, semantic compatibility and ABI/binary compatibility are distinct dimensions.
- No universal ABI guarantee is implied; any ABI commitment requires an explicit support matrix.
- Stable API removal normally requires a major release; deprecation and migration are explicit processes.
- Public enum/ErrorCode values, virtual interfaces, ownership/lifetime/threading guarantees and package behavior are compatibility-sensitive.
- CMake package/version-selection semantics are part of the supported installation contract.
- No Nexus/Edge/Linux/MCU/EtherCAT/ROS2/DDS/vendor-specific implementation enters `kritva-core`.
- No new Runtime, dependency, discovery, readiness or security enforcement framework is introduced by R1.0.
- R0.9 release history is preserved and not retroactively rewritten.

### Planned Effort

25–35 ED estimate including architecture, implementation, testing, documentation, security review, validation and release audit. Actual effort is recorded only when supported by evidence.

### Release Target

Version `1.0.0`, annotated tag `kritva-core-r1.0`, subject to the R10 Release Gate.

See `planning/milestones/KF-CORE-R10/` for the complete architecture, requirements proposal, task acceptance package and gate records.
