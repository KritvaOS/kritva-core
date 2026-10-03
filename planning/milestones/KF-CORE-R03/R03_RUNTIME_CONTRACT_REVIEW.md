# KF-CORE-R03 — Runtime Contract Review

## Gate
**Timing:** After KF-CORE-R03-006 and before KF-CORE-R03-007.

## Purpose
Freeze the concrete Runtime Manager, lifecycle, failure and recovery contracts before integration-test work begins. This gate prevents integration testing from becoming a vehicle for changing accepted runtime semantics.

## Entry Criteria
- [x] R03-004 accepted.
- [x] R03-005 accepted.
- [x] R03-006 accepted.
- [x] CORE-RT-006, CORE-RT-007 and CORE-RT-008 are authoritative and traceable.
- [x] Foundation API Review is PASS/FROZEN.

## Review Checklist
### Runtime Manager
- [x] Existing CORE-RT-002 / `runtime::Runtime` remains authoritative.
- [x] No competing Runtime abstraction.
- [x] Registry/DependencyGraph composition frozen.
- [x] Topology validation and freeze semantics frozen.
- [x] Setup mutation after freeze rejected.

### Lifecycle
- [x] Runtime state table frozen.
- [x] Configure semantics frozen.
- [x] Dependency-first forward operations frozen.
- [x] Exact reverse-order teardown frozen.
- [x] Fail-fast/no-rollback behavior frozen.
- [x] Shutdown progress semantics frozen.

### Failure and Recovery
- [x] Component-originated Error preservation frozen.
- [x] FAULT semantics frozen.
- [x] `fault_error()` lifetime frozen.
- [x] `reset()` is explicit and is the only recovery mechanism.
- [x] Reset ends in STOPPED; no RECOVERING state.
- [x] No retry of failed operation.
- [x] Two-pass reverse cleanup frozen.
- [x] Failed cleanup/progress semantics frozen.
- [x] Statistics semantics frozen.
- [x] Health does not trigger recovery.
- [x] No background recovery/thread/watchdog.

## Cross-Cutting Policy
- [x] Error semantics consistent with R02.
- [x] No dedicated Warning API.
- [x] Health is distinct from warning/error.
- [x] Diagnostics do not imply a logging backend.
- [x] Events remain distinct from application messages.
- [x] Runtime-owned Statistics follow R02 semantics.
- [x] Core logging boundary remains unchanged.

## Freeze Rule
After PASS, R03-007 may begin. Any production API/semantic change required after this gate returns to architecture review.

## Sign-off
| Item | Result | Evidence | Reviewer | Date |
|---|---|---|---|---|
| Runtime Contract Review | **PASS / FROZEN** | `R03_RUNTIME_CONTRACT_REVIEW_EVIDENCE.md` (HEAD `2c06652`; production code last changed `ee3d55d`) | ChatGPT | 03-10-2026 |

## Recorded decisions

Frozen for the remainder of R03 (any production API or semantic change returns to architecture review): `RuntimeManager` implements the unchanged `CORE-RT-002` `Runtime`; its setup, topology freeze, lifecycle table, forward/reverse ordering, fail-fast with no rollback and no retry, FAULT on a failed initialize/start/stop, `reset()` as the sole FAULT exit ending in STOPPED with no RECOVERING state, two-pass reverse cleanup, preserved original `fault_error()` and progress on failed cleanup, runtime-owned per-component progress, explicit `initialize()` for a new attempt, statistics semantics, health never triggering recovery, no warning API, no logging backend, no threads or background supervision, and no thread-safety or real-time guarantee.

Open items: (1) no recovery directly to READY: frozen; (2) `fault_error()` raw pointer: approved (non-null in FAULT, same pointer valid after a failed reset, null after a successful reset or outside FAULT, invalid after destruction; documentation is the contract); (3) the same `Configuration` for every component: approved, no per-component routing in R03; (4) allocating control-plane operations: approved; (5) no thread-safety or real-time claim: frozen; (6) the 0.2.0 to 0.3.0 version bump is release work: **R03-007 must remain version-neutral**; R03-008 prepares and validates 0.3.0 (`VERSION`, CMake project version, package metadata, install-consumer expectations, `find_package(kritva_core 0.3 ...)` and the compatibility checks); the R03 Release Gate finalizes the release commit and tag `kritva-core-r0.3`.

Rule for R03-007: it tests the frozen contract and does not discover or redesign it; a genuine contract defect found by integration testing is reported and returned to architecture review, not fixed silently.
