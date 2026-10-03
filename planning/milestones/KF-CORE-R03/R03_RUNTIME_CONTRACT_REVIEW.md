# KF-CORE-R03 — Runtime Contract Review

## Gate
**Timing:** After KF-CORE-R03-006 and before KF-CORE-R03-007.

## Purpose
Freeze the concrete Runtime Manager, lifecycle, failure and recovery contracts before integration-test work begins. This gate prevents integration testing from becoming a vehicle for changing accepted runtime semantics.

## Entry Criteria
- [ ] R03-004 accepted.
- [ ] R03-005 accepted.
- [ ] R03-006 accepted.
- [ ] CORE-RT-006, CORE-RT-007 and CORE-RT-008 are authoritative and traceable.
- [ ] Foundation API Review is PASS/FROZEN.

## Review Checklist
### Runtime Manager
- [ ] Existing CORE-RT-002 / `runtime::Runtime` remains authoritative.
- [ ] No competing Runtime abstraction.
- [ ] Registry/DependencyGraph composition frozen.
- [ ] Topology validation and freeze semantics frozen.
- [ ] Setup mutation after freeze rejected.

### Lifecycle
- [ ] Runtime state table frozen.
- [ ] Configure semantics frozen.
- [ ] Dependency-first forward operations frozen.
- [ ] Exact reverse-order teardown frozen.
- [ ] Fail-fast/no-rollback behavior frozen.
- [ ] Shutdown progress semantics frozen.

### Failure and Recovery
- [ ] Component-originated Error preservation frozen.
- [ ] FAULT semantics frozen.
- [ ] `fault_error()` lifetime frozen.
- [ ] `reset()` is explicit and is the only recovery mechanism.
- [ ] Reset ends in STOPPED; no RECOVERING state.
- [ ] No retry of failed operation.
- [ ] Two-pass reverse cleanup frozen.
- [ ] Failed cleanup/progress semantics frozen.
- [ ] Statistics semantics frozen.
- [ ] Health does not trigger recovery.
- [ ] No background recovery/thread/watchdog.

## Cross-Cutting Policy
- [ ] Error semantics consistent with R02.
- [ ] No dedicated Warning API.
- [ ] Health is distinct from warning/error.
- [ ] Diagnostics do not imply a logging backend.
- [ ] Events remain distinct from application messages.
- [ ] Runtime-owned Statistics follow R02 semantics.
- [ ] Core logging boundary remains unchanged.

## Freeze Rule
After PASS, R03-007 may begin. Any production API/semantic change required after this gate returns to architecture review.

## Sign-off
| Item | Result | Evidence | Reviewer | Date |
|---|---|---|---|---|
| Runtime Contract Review | PENDING | — | — | — |
