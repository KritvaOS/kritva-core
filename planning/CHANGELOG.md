# Kritva Core — Changelog

## [Unreleased]

### Release record

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

Pending independent review:
- KF-CORE-R06-002 — Operational Context Services & Access Policy (`072b713`)

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
