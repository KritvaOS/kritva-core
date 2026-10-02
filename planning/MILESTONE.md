# Kritva Core — Master Milestone Roadmap

## Milestone Lifecycle

PLANNED → IN PROGRESS → IMPLEMENTATION COMPLETE → REVIEW → ACCEPTED → RELEASED

## KF-CORE-R01 — Core Foundation

Status: COMPLETE

Established the initial platform-independent Kritva Core API contracts and foundation implementation.

## KF-CORE-R02 — Core Contract Hardening

Status: RELEASED (`kritva-core-r0.2` → `46af52c`, published to origin)

### Objective

Harden the existing public contracts, remove ambiguity in API behavior, close requirements/API traceability gaps, strengthen foundation contract tests, and complete an objective R0.2 validation gate.

### Exit Conditions

- All R02 tasks accepted.
- All new tests pass.
- Full regression passes.
- Public API reviewed.
- Requirements traceability complete.
- Coverage reviewed.
- Sanitizer/static analysis reviewed where configured.
- Documentation and changelog updated.
- Git history reviewed.
- Release tag created.

### Task Order

001 → 002 → 003 → 004 → 005 → 006 → 007 → 008

Some tasks may be developed in parallel only if their dependency conditions are satisfied. Final validation is always last.

## KF-CORE-R03 — Runtime Foundation

Status: PLANNED

Implement the first concrete runtime foundation after R02 contracts are frozen.

## KF-CORE-R04 — Platform Abstraction

Status: PLANNED

Define validated platform integration boundaries for Linux, MCU/RTOS, Nexus and Edge implementations.
