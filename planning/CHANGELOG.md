# Kritva Core — Changelog

## [Unreleased]

## KF-CORE-R09 — Capability Contract & Readiness Boundary Planning

Architecture direction and scope confirmed. R0.9 planning package prepared; implementation not started.

R0.9 scope:
- Capability identity/provider semantics
- CapabilitySet invariants and version semantics
- Existing capability requirement/matching boundary
- Separation of capability matching from Component dependency ordering
- Explicit readiness/lifecycle boundary without a new readiness state
- API documentation synchronization in canonical Markdown
- Security trust/authority planning without a security subsystem
- Reference capability/requirement harness and integration validation
- Integration Freeze, full validation and Release Gate

R0.9 exclusions:
- Generic dependency injection / ServiceRegistry / locator / resolver
- Dynamic discovery or capability event broker
- Automatic readiness calculation or new lifecycle state
- Health-driven lifecycle/recovery
- Authentication, authorization, cryptography or key management in Core
- Nexus/Edge/Linux/MCU/EtherCAT/ROS2/DDS/vendor-specific implementation

Proposed requirement domain: `CORE-CAP-004..011`.
Planned effort: 22–30 ED; actual effort not yet recorded.


### KF-CORE-R09 — Capability Contract Implementation

Accepted:
- KF-CORE-R09-001 — Capability Contract & Provider Semantics (`4081dc0`; evidence `7a727e8`; baseline `c250c54`)
- KF-CORE-R09-002 — CapabilitySet Invariants & Version Semantics (`4bd241e`; evidence `7bcd90d`)
- KF-CORE-R09-003 — Requirement / Capability Matching Boundary (`e91a51f`; evidence `e53464a`; correction `7b6b0f4`)
- R09 Capability API Review — PASS / FROZEN (production freeze commit `4c86b53`; evidence `c84bb9c`)
- KF-CORE-R09-004 — Reference Capability & Requirement Harness (`2608795`; evidence `9d0a81d`)
- KF-CORE-R09-005 — Component Readiness / Lifecycle Boundary Integration (`0f6b6c3`; evidence `9a086ae`)
- R09 Integration Freeze — PASS / HONORED (production freeze commit `4c86b53`; evidence `553258b`)
- KF-CORE-R09-006 — API Documentation, Security & Boundary Validation (`04f0859`; evidence `87a2960`)

Pending independent review:
- KF-CORE-R09-007 — Full R0.9 Validation & Release Candidate (candidate `ef14e99`)

### KF-CORE-R07 — Component Operational Foundation Planning

Architecture and scope confirmed. R0.8 planning package prepared; implementation not started.

R0.8 scope:
- Component configuration lifecycle eligibility and state-preservation semantics
- Configuration ownership and detached-value behavior
- Atomic/non-partial configuration application
- Core structural versus Component semantic validation boundary
- `ConfigurationVersion` schema/contract compatibility semantics
- Runtime configuration forwarding and failure isolation
- Reference configuration harness and Runtime integration tests
- Integration Freeze, full validation and Release Gate

R0.8 exclusions:
- Dynamic reconfiguration / `reconfigure()` / parameter server
- Configuration persistence, remote configuration or configuration broker
- Configuration transactions/rollback and configuration event infrastructure
- ComponentContext expansion
- Runtime lifecycle redesign or automatic recovery
- Platform/ROS2/DDS/EtherCAT/vendor-specific configuration implementation

Proposed requirement domain: `CORE-CFG-004..013`.
Planned effort: 22–31 ED; actual effort not yet recorded.


### KF-CORE-R08 — Component Configuration Implementation

Accepted:
- KF-CORE-R08-001 — Component Configuration Contract & Lifecycle Semantics (`605516b`; evidence `bb3752a`)
- KF-CORE-R08-002 — Configuration Ownership & Atomic Application (`6efeaac`; evidence `6aab91b`)
- KF-CORE-R08-003 — Configuration Version & Validation Contract (`bdb4b93`; evidence `0ea3ea3`)
- R08 Configuration API Review — PASS / FROZEN (production freeze baseline `bdb4b93`; evidence `0a73b5a`)
- KF-CORE-R08-004 — Reference Configuration Harness & Contract Tests (`f4b6de4`; evidence `2b93ad6`)
- KF-CORE-R08-005 — Runtime/Component Configuration Integration (`a5dfbc1`; evidence `159b1bc`)
- R08 Integration Freeze — PASS / HONORED (production freeze point `bdb4b93`; evidence `e1051a0`)
- KF-CORE-R08-006 — Configuration Boundary & Regression Validation (`94fad8e`; evidence `644bdf8`)
- KF-CORE-R08-007 — Full R0.8 Validation & Release Candidate (`1aa3611`; candidate `1e7ba2b`)

Pending independent review:

### KF-CORE-R07 — Component Operational Foundation Planning

Architecture confirmed after R0.7 Design Consult and Scope Confirmation.

R0.7 scope:
- Component operational observation contract
- Status and Health reporting semantics
- Explicit operational Event reporting to integrator-owned sinks
- Optional Component-owned Statistics observation contract
- Reference operational harness and contract tests
- Runtime/Component operational integration tests preserving R0.3 lifecycle semantics
- Integration Freeze
- Full R0.7 validation
- Release Gate

Explicit exclusions:
- no new operational state machine
- no Core EventBus or event queue
- no telemetry or logging backend
- no Core-owned worker/thread/background execution
- no automatic retry, restart or health-driven recovery
- no platform-specific operational implementation

R0.7 implementation has not started. `KF-CORE-R07-001` is the next implementation task after task-specific acceptance criteria are issued.


### KF-CORE-R07 — Component Operational Implementation

Accepted:
- KF-CORE-R07-001 — Component Operational Observation Contract (`6849a73`; evidence `6518240`)
- KF-CORE-R07-002 — Component Status & Health Reporting Contract (`61e0067`; evidence `8d9e4b0`)
- KF-CORE-R07-003 — Component Operational Event Contract (`56ff226`; evidence `871b878`)
- KF-CORE-R07-004 — Component Statistics Ownership & Observation Contract (`16654e9`; evidence `e354dac`)
- R07 Component Operational API Review — PASS / FROZEN (evidence `6b1296f`)
- KF-CORE-R07-005 — Reference Operational Harness & Contract Tests (`0b1bd1d`; evidence `2d60432`)
- KF-CORE-R07-006 — Runtime/Component Operational Integration (`3f524cd`; evidence `ceffe92`)
- R07 Integration Freeze — PASS / HONORED (production freeze point `16654e9`; evidence `142a32e`)
- KF-CORE-R07-007 — Full R0.7 Validation (`86dfcb9`; candidate `d83e1ba`)

Pending independent review:

### Release record

- R08 Release Gate — PASS. Kritva Core R0.8 / version 0.8.0; release candidate `1e7ba2b`; tag `kritva-core-r0.8` on the documentation-only release-record commit (annotated, pushed to origin; tag object `2d576d7`).
- R07 Release Gate — PASS. Kritva Core R0.7 / version 0.7.0; release candidate `d83e1ba`; tag `kritva-core-r0.7` on the documentation-only release-record commit (annotated, pushed to origin; tag object `4aa3fab`).
- R06 Release Gate — PASS. Kritva Core R0.6 / version 0.6.0; release candidate `b473e5d`; tag `kritva-core-r0.6` on the documentation-only release-record commit (annotated, pushed to origin; tag object `fb9d631`).
- R05 Release Gate — PASS. Kritva Core R0.5 / version 0.5.0; release candidate `5fb5e69`; tag `kritva-core-r0.5` on the documentation-only release-record commit `adf8ac2` (annotated, pushed to origin).
- R04 Release Gate — PASS. Kritva Core R0.4 / version 0.4.0; release candidate `e7df87c`; tag `kritva-core-r0.4` on the documentation-only release-record commit `b31108d` (annotated, pushed to origin).
- R03 Release Gate — PASS. Kritva Core R0.3 / version 0.3.0; release candidate `f598fef`; tag `kritva-core-r0.3` on the documentation-only release-record commit `cc16ec9` (annotated, pushed to origin).

### KF-CORE-R03 — Runtime Foundation Planning

Planning activated for the R03 Runtime Foundation milestone.

Foundation tasks defined:
- KF-CORE-R03-001 — Component Contract & Identity
- KF-CORE-R03-002 — Component Registry
- KF-CORE-R03-003 — Dependency Management

Foundation gate:
- R03 Foundation API Review after R03-003.

Runtime gates:
- R03 Runtime Contract Review after R03-006.
- R03 Integration Freeze after R03-007.
- R03 Release Gate after R03-008.

Accepted:
- KF-CORE-R03-001 — Component Contract & Identity (`655c1dd`, `35efee1`, `9a98ab3`)
- KF-CORE-R03-002 — Component Registry (`7ae9a32`)
- KF-CORE-R03-003 — Dependency Management (`795fb94`)
- KF-CORE-R03-004 — Runtime Manager (`e4d3a9b`, follow-ups `40e33e9`, `25eb914`)
- KF-CORE-R03-005 — Runtime Lifecycle (`e2b660d`, follow-ups `a4f2a65`, `eac011f`)
- KF-CORE-R03-006 — Runtime Failure & Recovery (`ee3d55d`, follow-up `d651677`)
- R03 Runtime Contract Review — PASS / FROZEN (RuntimeManager lifecycle and failure/recovery contracts frozen)
- KF-CORE-R03-007 — Runtime Integration Tests (`9d1c7d1`)
- R03 Integration Freeze — PASS / ACTIVE
- KF-CORE-R03-008 — Full R03 Validation (`407df6b`; candidate `f598fef`; `CORE-RT-009`/`CORE-RT-010` defined, 0.3.0 release metadata)
- R03 Foundation API Review — PASS / FROZEN (Component, Registry, DependencyGraph contracts frozen; `CORE-RT-002` Runtime interface remains authoritative for R03-004)
- R03-004–006 acceptance criteria prepared: Runtime Manager, Runtime Lifecycle, Runtime Failure & Recovery.
- R03-007 acceptance package prepared: Runtime Integration Tests.
- R03-008 acceptance package prepared: Full R03 Validation.
- R03 Runtime Contract Review, Integration Freeze and R03 Release Gate criteria documented.
- R03-004 is READY; R03-005 and R03-006 remain blocked by task dependencies.
- Runtime Contract Review remains mandatory after R03-006.

Cross-cutting R03 policy review includes Error, Warning, Info/diagnostic messaging, Event versus message, Statistics update semantics, and logging boundary.

### KF-CORE-R02 — Core Contract Hardening

Accepted:
- Result<T> contract hardening (`df44d38`)
- Status API/header cleanup (`d6d939d`)
- Statistics contract clarification (`d7d29cb`)
- Scheduler contract review (`0773857`, refined in `0cdffdb` after review round 1)
- Clock abstraction cleanup (`6ed8762`, refined in `e1e6cfb` after review round 1)
- Requirements/API traceability (`00899f9`)
- Foundation contract tests (`bf2144a`)
- Full R0.2 validation (`1472b79`); build fix `ea619f8` (`make coverage` in a fresh clone)

### Post-R0.2 preparation

Accepted:
- KF-CORE-R03-PREP-001 — Install and package Core library (`29255d5`)



### KF-CORE-R06 — Component Execution Context Planning

Planning package prepared for the next proposed milestone:
- Component Execution Context and ownership model
- Operational context service access policy
- Context injection without Runtime lifecycle change
- Context requirement/capability binding
- Component API Review / Freeze
- Reference context contract harness
- Runtime/Component integration tests
- Integration Freeze
- Full R0.6 Validation
- Release Gate

Scope and API design confirmed by the independent reviewer on 05-10-2026 (`R06_DESIGN_DECISIONS.md` D09–D15); implementation in progress.

Accepted:
- KF-CORE-R06-001 — Component Execution Context & Ownership Model (`8031c47`)
- KF-CORE-R06-002 — Operational Context Services & Access Policy (`072b713`)
- KF-CORE-R06-003 — Context Injection Without Runtime Lifecycle Change (`adb0e08`)
- KF-CORE-R06-004 — Context Requirements & Capability Binding (`5b755af`)
- R06 Component API Review — PASS / FROZEN (evidence `06207c7`)
- KF-CORE-R06-005 — Reference Context Harness & Contract Tests (`c7f7b46`)
- KF-CORE-R06-006 — Runtime/Component Context Integration Tests (`2fd5424`)
- R06 Integration Freeze — PASS / HONORED (production freeze point `5b755af`; evidence `769ac8d`)

- KF-CORE-R06-007 — Full R0.6 Validation (`7351db6`; candidate `b473e5d`)

### KF-CORE-R05 — Platform Runtime Integration Foundation

Accepted:
- KF-CORE-R05-001 — Platform Context & Service Access Model (`c5910e7`)
- KF-CORE-R05-002 — Platform Service Requirement Model (`f9d6007`)
- KF-CORE-R05-003 — Explicit Platform Service Consumption (`f8cd523`)
- KF-CORE-R05-004 — Runtime–Platform Lifecycle Boundary (`f23777b`)
- R05 Platform API Review — PASS / FROZEN (evidence `05d981e`)
- KF-CORE-R05-005 — Reference Platform Integration (`7f30626`)
- KF-CORE-R05-006 — Platform Integration & Runtime Tests (`fe04d35`)
- R05 Platform Integration Freeze — PASS / HONORED (production freeze point `f23777b`; evidence `cf6e617`)
- KF-CORE-R05-007 — Full R0.5 Validation (`8384f5d`; candidate `5fb5e69`)

### KF-CORE-R04 — Platform Abstraction Planning

Accepted:
- KF-CORE-R04-001 — Platform Adapter Boundary & Context (`d1c5f13`)
- KF-CORE-R04-002 — Scheduler Contract Hardening (`eb06fa0`)
- KF-CORE-R04-003 — Clock & Timer Contract (`67114bb`)
- KF-CORE-R04-004 — Watchdog Contract (`4af4756`)
- R04 Platform API Review — PASS / FROZEN (evidence `380ade3`)
- KF-CORE-R04-005 — Platform Capability & Adapter Contract (`f7231c1`)
- KF-CORE-R04-006 — Platform Conformance Tests (`460de87`)
- R04 Platform Integration Freeze — PASS / HONORED (platform API frozen at `f7231c1`; evidence `84046b0`)
- KF-CORE-R04-007 — Runtime–Platform Integration Boundary (`36c5cb8`)
- KF-CORE-R04-008 — Full R0.4 Validation (`f0669eb`; candidate `e7df87c`)


R0.4 planning activated after the R0.3 release.

Tasks defined:
- KF-CORE-R04-001 — Platform Adapter Boundary & Context
- KF-CORE-R04-002 — Scheduler Contract Hardening
- KF-CORE-R04-003 — Clock & Timer Contract
- KF-CORE-R04-004 — Watchdog Contract
- KF-CORE-R04-005 — Platform Capability & Adapter Contract
- KF-CORE-R04-006 — Platform Conformance Tests
- KF-CORE-R04-007 — Runtime–Platform Integration Boundary
- KF-CORE-R04-008 — Full R0.4 Validation

Gates defined:
- R04 Platform API Review after R04-004.
- R04 Platform Integration Freeze after R04-006.
- R04 Release Gate after R04-008.

R0.4 explicitly keeps platform implementations outside `kritva-core`.

## [kritva-core-r0.1]

### KF-CORE-R01 — Core Foundation

Initial Kritva Core foundation and public API contracts.

Release tag: `kritva-core-r0.1`

