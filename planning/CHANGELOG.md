# Kritva Core — Changelog

## [Unreleased]

### KF-CORE-R03 — Runtime Foundation Planning

Planning activated for the R03 Runtime Foundation milestone.

Foundation tasks defined:
- KF-CORE-R03-001 — Component Contract & Identity
- KF-CORE-R03-002 — Component Registry
- KF-CORE-R03-003 — Dependency Management

Foundation gate:
- R03 Foundation API Review after R03-003.

- KF-CORE-R03-004 — Runtime Manager (`e4d3a9b`, follow-ups `40e33e9`, `25eb914`)

Runtime gates:
- R03 Runtime Contract Review after R03-006.
- R03 Integration Freeze after R03-007.
- R03 Release Gate after R03-008.

Accepted:
- KF-CORE-R03-001 — Component Contract & Identity (`655c1dd`, `35efee1`, `9a98ab3`)
- KF-CORE-R03-002 — Component Registry (`7ae9a32`)
- KF-CORE-R03-003 — Dependency Management (`795fb94`)
- R03 Foundation API Review — PASS / FROZEN (Component, Registry, DependencyGraph contracts frozen; `CORE-RT-002` Runtime interface remains authoritative for R03-004)
- R03-004–006 acceptance criteria prepared: Runtime Manager, Runtime Lifecycle, Runtime Failure & Recovery.
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

## [kritva-core-r0.1]

### KF-CORE-R01 — Core Foundation

Initial Kritva Core foundation and public API contracts.

Release tag: `kritva-core-r0.1`
