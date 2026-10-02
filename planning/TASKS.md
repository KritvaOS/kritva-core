# Kritva Core — Master Task Register

## KF-CORE-R02 — Core Contract Hardening

| ID | Task | Primary Area | Status | Commit |
|---|---|---|---|---|
| KF-CORE-R02-001 | Result<T> contract hardening | error/result | ACCEPTED | `df44d38` |
| KF-CORE-R02-002 | Status API/header cleanup | status | PLANNED | — |
| KF-CORE-R02-003 | Statistics contract clarification | statistics | PLANNED | — |
| KF-CORE-R02-004 | Scheduler contract review | platform/scheduler | PLANNED | — |
| KF-CORE-R02-005 | Clock abstraction cleanup | time/platform | PLANNED | — |
| KF-CORE-R02-006 | Requirements/API traceability | requirements/docs | PLANNED | — |
| KF-CORE-R02-007 | Foundation contract tests | tests | PLANNED | — |
| KF-CORE-R02-008 | Full R0.2 validation | integration/validation | PLANNED | — |

## Acceptance Rule

A task moves to ACCEPTED only after:
1. implementation is complete,
2. required new tests pass,
3. regression passes,
4. required quality checks pass,
5. evidence is supplied,
6. independent review passes.

## R02 Dependency Graph

```text
001 Result
   ↓
002 Status
   ↓
003 Statistics
   ↓
004 Scheduler
   ↓
005 Clock
   ↓
006 Traceability
   ↓
007 Contract Tests
   ↓
008 Full Validation
```
