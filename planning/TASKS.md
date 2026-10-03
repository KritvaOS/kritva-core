# Kritva Core — Master Task Register

## KF-CORE-R02 — Core Contract Hardening

| ID | Task | Primary Area | Status | Commit |
|---|---|---|---|---|
| KF-CORE-R02-001 | Result<T> contract hardening | error/result | ACCEPTED | `df44d38` |
| KF-CORE-R02-002 | Status API/header cleanup | status | ACCEPTED | `d6d939d` |
| KF-CORE-R02-003 | Statistics contract clarification | statistics | ACCEPTED | `d7d29cb` |
| KF-CORE-R02-004 | Scheduler contract review | platform/scheduler | ACCEPTED | `0773857`, `0cdffdb` |
| KF-CORE-R02-005 | Clock abstraction cleanup | time/platform | ACCEPTED | `6ed8762`, `e1e6cfb` |
| KF-CORE-R02-006 | Requirements/API traceability | requirements/docs | ACCEPTED | `00899f9` |
| KF-CORE-R02-007 | Foundation contract tests | tests | ACCEPTED | `bf2144a` |
| KF-CORE-R02-008 | Full R0.2 validation | integration/validation | ACCEPTED | `1472b79` |

## KF-CORE-R03-PREP — Pre-R0.3 Preparation

| ID | Task | Primary Area | Status | Commit |
|---|---|---|---|---|
| KF-CORE-R03-PREP-001 | Install and Package Core Library (`CORE-BUILD-002`) | build/install | ACCEPTED | `29255d5` |

## KF-CORE-R03 — Runtime Foundation

| ID | Task | Primary Area | Status | Primary Commit |
|---|---|---|---|---|
| KF-CORE-R03-001 | Component Contract & Identity | runtime/component | ACCEPTED (`655c1dd`) | `feat(core): define component runtime contract` |
| KF-CORE-R03-002 | Component Registry | runtime/registry | REVIEW (`7ae9a32`) | `feat(core): add component registry` |
| KF-CORE-R03-003 | Dependency Management | runtime/dependency | PLANNED | `feat(core): add runtime dependency management` |
| KF-CORE-R03-004 | Runtime Manager | runtime/manager | PLANNED | `feat(core): add runtime manager` |
| KF-CORE-R03-005 | Runtime Lifecycle | runtime/lifecycle | PLANNED | `feat(core): implement runtime lifecycle orchestration` |
| KF-CORE-R03-006 | Runtime Failure & Recovery | runtime/error-recovery | PLANNED | `feat(core): define runtime failure handling` |
| KF-CORE-R03-007 | Runtime Integration Tests | tests/integration | PLANNED | `test(core): add runtime integration contracts` |
| KF-CORE-R03-008 | Full R03 Validation | integration/validation | PLANNED | `test(core): complete R03 runtime validation` |

## R03 Dependency Graph

```text
R03-001 Component Contract & Identity
        ↓
R03-002 Component Registry
        ↓
R03-003 Dependency Management
        ↓
R03 Foundation API Review
        ↓
R03-004 Runtime Manager
        ↓
R03-005 Runtime Lifecycle
        ↓
R03-006 Runtime Failure & Recovery
        ↓
R03 Runtime Contract Review
        ↓
R03-007 Runtime Integration Tests
        ↓
R03 Integration Freeze
        ↓
R03-008 Full R03 Validation
        ↓
R03 Release Gate
```

## Acceptance Rule

A task moves to ACCEPTED only after:
1. implementation is complete,
2. required new unit tests pass,
3. required integration/regression tests pass,
4. required quality checks pass,
5. evidence is supplied,
6. independent review passes.

API freeze gates are mandatory and are not bypassed by task compilation or test success.
