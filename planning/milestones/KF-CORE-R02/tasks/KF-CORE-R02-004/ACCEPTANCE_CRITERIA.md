# KF-CORE-R02-004 — Acceptance Criteria

## 1. Task Information

- Task: Scheduler Contract Review
- Milestone: `KF-CORE-R02`
- Dependency: `KF-CORE-R02-003`
- Expected commit:
  `docs(core): clarify scheduler contract`

## 2. Objective

Make the scheduler abstraction precise enough for later Linux/RTOS implementations while keeping Core platform neutral.

## 3. Requirement Traceability

The implementer must identify the authoritative requirement IDs affected by this task.

| Requirement ID | Header/API | Implementation | Test | Evidence |
|---|---|---|---|---|
| CORE-PLAT-001 | `include/kritva/core/platform/scheduler.hpp` | contract only | `tests/unit/platform_test.cpp` (`kritva_core_platform`), `tests/contract/scheduler_contract.hpp` | commits `0773857`, `0cdffdb`; 16/16 ctest |

**Acceptance:** No requirement referenced by the implementation may remain undefined.

## 4. Functional Acceptance

- [x] AC-001: Document TaskConfig fields and their semantics.
- [x] AC-002: Clarify task entry function lifetime requirements.
- [x] AC-003: Clarify ownership of task context.
- [x] AC-004: Clarify whether create_task creates a stopped task or starts it.
- [x] AC-005: Clarify start/stop idempotency and invalid-state behavior.
- [x] AC-006: Clarify priority interpretation without assigning OS-specific numeric meaning.
- [x] AC-007: Clarify CPU-affinity semantics; `cpu_affinity=0` must not remain ambiguous.
- [x] AC-008: Clarify TaskId lifetime and uniqueness expectations.
- [x] AC-009: Document resource-failure behavior.
- [x] AC-010: Do not implement Linux pthreads, RTOS tasks, EtherCAT, or vendor SDK behavior.

## 5. Test Acceptance

- [x] TEST-001: Header/API compile validation
- [x] TEST-002: Contract-level tests if an interface mock exists
- [x] TEST-003: Task configuration boundary tests where types permit

## 6. Regression Acceptance

- [x] All known regression tests pass.
- [x] `ctest --test-dir build --output-on-failure` passes.
- [x] No previously passing test is removed or disabled without explicit review.

## 7. Build Acceptance

- [x] Clean configure succeeds.
- [x] Clean build succeeds.
- [x] No new compiler errors.
- [x] No new unexplained compiler warnings.

## 8. Coverage Acceptance

- [x] Coverage is generated/reviewed if configured.
- [x] New logic has appropriate test coverage.
- [x] Any material uncovered branch is documented.

## 9. Sanitizer / Static Analysis Acceptance

- [x] Required configured sanitizer runs pass.
- [x] Required configured static analysis passes.
- [x] Any existing unrelated finding is explicitly identified rather than hidden.

## 10. Scope Acceptance

- [x] No Runtime Manager implementation added.
- [x] No platform-specific implementation added to Core.
- [x] No unrelated refactoring.
- [x] Public API changes are limited to this task's contract needs.

## 11. Documentation Acceptance

- [x] Relevant API/requirements documentation updated.
- [x] Requirement IDs are traceable.
- [x] No documentation contradicts the implementation.

## 12. Git Acceptance

- [x] Working tree was clean before implementation.
- [x] Diff reviewed.
- [x] Commit contains only this task's logical changes.
- [x] Exact commit message used:

```text
docs(core): clarify scheduler contract
```

- [x] Commit hash recorded: `0773857` (initial), `0cdffdb` (review round 1 follow-up).

## 12a. Implementation Evidence (Claude)

- Commit: `0773857` `docs(core): clarify scheduler contract` (R02-003 accepted at `213efcd`).
- Files changed: `include/kritva/core/platform/scheduler.hpp` (documentation only; declarations unchanged), `tests/unit/platform_test.cpp`, `tests/contract/scheduler_contract.hpp` (new, header-only), `REQUIREMENTS.md` (CORE-PLAT-001), `API.md` (section 16).
- No scheduler implementation, no pthread/RTOS/vendor code, no public signature change.
- Contract decisions (superseded where noted by review round 2 below; see round 2 for the final text):
  1. TaskConfig fields: documented per field. `name` non-null, copied by the scheduler, need only live through `create_task`.
  2. Entry lifetime: non-null (`INVALID_ARGUMENT`), valid for the scheduler's lifetime, must not throw, must not call `stop()` of its own scheduler.
  3. Context: opaque, nullable, caller-owned, never dereferenced/freed by the scheduler; must outlive running tasks (until `stop()` returned).
  4. `create_task()` while STOPPED only registers an inactive task. While RUNNING: adapter policy (see round 2).
  5. `start()`/`stop()` are scheduler-wide and idempotent; `start()` is all-or-nothing; `stop()` returns only when no entry is running.
  6. Priority: see round 2.
  7. CPU affinity: bit mask of logical CPUs 0..31 (R0.2 limitation); non-zero is honoured or refused, never silently ignored.
  8. `cpu_affinity == 0` means "no constraint", not "CPU 0"; pin to CPU 0 with `0x1`.
  9. `TaskId`: see round 2.
  10. Resource exhaustion: `RESOURCE_UNAVAILABLE`, reported through `Result`, never abort/exception; failed `create_task` creates nothing and consumes no id; failed `start` leaves the scheduler STOPPED.
  Also: `period` 0 = aperiodic, positive = periodic, negative invalid; overrun behavior is adapter-defined.
- Tests: `FakeScheduler` in `platform_test.cpp` replaced by a conforming reference double (previous assertions retained and passing); new tests for zero-value semantics, create-does-not-start, affinity, priority rejection, exhaustion, all-or-nothing start, id rules; reusable `check_scheduler_contract()` for future adapters.
- Build: `rm -rf build && cmake -S . -B build && cmake --build build -j$(nproc)` — 0 warnings.
- Tests: `ctest --test-dir build` — 16/16 passed (includes `kritva_core_platform`).
- Sanitizers: separate Debug build `-fsanitize=address,undefined` — 16/16 passed.
- Coverage: `make coverage` — 98% overall (unchanged).
- `scheduler.hpp` compiles standalone with `-Wall -Wextra`; header check passed; `git diff --check` clean.
- `make format-check`/`make lint`: TODO stubs, not executed.
- Known limitations: 32-bit affinity mask limits addressable CPUs to 32 in R0.2; no task-destroy/removal API; periodic overrun policy and timing accuracy are adapter-defined; the reference double is a test double and proves the contract is self-consistent, not that any real adapter conforms.
- Human review: this clarifies public API semantics (AGENTS.md section 12); round 1 review applied (below).

### Review round 1 — CHANGES REQUIRED (ChatGPT, on `0773857`)

Required: (1) do not freeze `INVALID_STATE` for create-while-running; (2) remove Core-defined priority range/maximum; (3) do not promise TaskId never-reused; (4) distinguish INVALID_ARGUMENT vs UNSUPPORTED for affinity; (5) explain why `stop()` from an entry is prohibited; (6) do not specify destructor semantics; (7) document the 32-bit mask as an R0.2 limitation. Not to be added: `destroy_task()`, adapters, thread pool, executor, real-time policy, CPU topology.

### Review round 2 — follow-up commit `0cdffdb` `fix(core): refine scheduler contract semantics`

`0773857` is unchanged (not amended). Changes, one per required item:

1. **create_task while RUNNING:** now an adapter policy. (a) dynamic creation supported, the task joins the running set and the adapter documents when it first runs; or (b) not supported, fails with `INVALID_STATE` atomically (no task, no id, no state change). Core does not mandate a static task set. Conformance checker accepts both; the reference double implements both via `allow_dynamic_creation`.
2. **Priority:** implementation-independent relative value; "higher = more urgent *within one scheduler instance*"; no Core-defined range, maximum or OS mapping; adapter documents its mapping and its handling of unrepresentable values. The test double's rejection above 255 is labeled adapter policy.
3. **TaskId:** 0 invalid; unique among currently existing tasks of one instance; valid while the task exists; reuse after a future destroy operation is implementation-defined and not promised. The consequence (no destroy operation means ids/tasks grow for the scheduler lifetime, so adapters document capacity) is stated in the header.
4. **Affinity errors:** `INVALID_ARGUMENT` = mask invalid for the platform (selects no existing CPU; extra non-existent bits are adapter-defined); `UNSUPPORTED` = well-formed request the platform cannot provide. Test comments distinguish the two.
5. **stop() from entry:** reason documented (synchronous scheduler-wide stop would wait on the calling task itself, deadlock); implementations return `INVALID_STATE`.
6. **Destruction:** "destroying implies stop()" removed. Now: adapters must not release scheduler resources while any entry executes and must reach an orderly stop first; callers should stop() before destroying and before releasing any context. Core does not specify destructor behavior.
7. **32-bit mask:** documented as an R0.2 limitation of the current `TaskConfig` type, not a long-term architectural limit; a wider form may come with KF-CORE-R04; no redesign here.

Files changed in round 2: `include/kritva/core/platform/scheduler.hpp` (documentation only), `tests/contract/scheduler_contract.hpp`, `tests/unit/platform_test.cpp`, `REQUIREMENTS.md`, `API.md`. No `destroy_task()`, adapter, thread pool, executor or real-time policy added.

Round 2 validation: clean-tree build 0 warnings; `ctest` 16/16; ASan+UBSan build 16/16; coverage 98%; `scheduler.hpp` standalone compile OK; header check passed; `git diff --check` clean; `format-check`/`lint` TODO stubs (not executed).

## 13. Evidence Required From Codex/Claude

Provide the following in the implementation response:

1. Summary of changes
2. Exact files changed
3. Requirement IDs addressed
4. New tests added/modified
5. Build command and output summary
6. Task-specific test command/output summary
7. Full regression command/output summary
8. Coverage result
9. Sanitizer/static-analysis result
10. Git commit hash
11. Known limitations
12. Any follow-up recommendation

## 14. Independent Reviewer Decision

Reviewer: ChatGPT

- [x] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Review notes:

Round 1 review identified seven scheduler-contract issues. The follow-up
commit `0cdffdb` addresses all seven items while leaving the original
implementation commit `0773857` unchanged.

The revised contract correctly keeps scheduler policy platform-independent.
Dynamic task creation is adapter-defined, with atomic INVALID_STATE failure
when unsupported while running. Priority is defined as an implementation-
independent relative value without imposing an OS-specific numeric range.

TaskId semantics no longer promise permanent non-reuse. CPU-affinity errors
distinguish INVALID_ARGUMENT from UNSUPPORTED. The prohibition on calling
synchronous scheduler-wide stop() from a running task is documented with
its deadlock rationale.

Scheduler destruction is intentionally outside the Core contract, while
adapters are required to stop orderly before releasing resources. The
current 32-bit affinity mask is documented as an R0.2 limitation rather
than a long-term architecture restriction.

No scheduler implementation, executor, thread pool, destroy_task(), or
platform-specific code was introduced.

Validation evidence is sufficient:
- Clean build: 0 warnings
- CTest: 16/16 passed
- ASan + UBSan: 16/16 passed
- Coverage: 98%
- scheduler.hpp standalone compilation: passed
- Header check: passed
- git diff --check: passed
- format-check/lint: repository TODO stubs, therefore not applicable

The round-1 CHANGES REQUIRED findings are resolved.

Task KF-CORE-R02-004 is ACCEPTED.

Reviewer Decision: PASS

## 15. Acceptance Rule

The task cannot be marked ACCEPTED solely because the code compiles. All applicable functional, test, regression, quality, scope, documentation, and Git criteria must have evidence.
