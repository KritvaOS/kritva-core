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
| CORE-PLAT-001 | `include/kritva/core/platform/scheduler.hpp` | contract only | `tests/unit/platform_test.cpp` (`kritva_core_platform`), `tests/contract/scheduler_contract.hpp` | commit `0773857`; 16/16 ctest |

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

- [x] Commit hash recorded: `0773857`.

## 12a. Implementation Evidence (Claude)

- Commit: `0773857` `docs(core): clarify scheduler contract` (R02-003 accepted at `213efcd`).
- Files changed: `include/kritva/core/platform/scheduler.hpp` (documentation only; declarations unchanged), `tests/unit/platform_test.cpp`, `tests/contract/scheduler_contract.hpp` (new, header-only), `REQUIREMENTS.md` (CORE-PLAT-001), `API.md` (section 16).
- No scheduler implementation, no pthread/RTOS/vendor code, no public signature change.
- Contract decisions (all for reviewer confirmation; items marked * are choices, not facts of the existing code):
  1. TaskConfig fields: documented per field. `name` non-null, copied by the scheduler, need only live through `create_task`.
  2. Entry lifetime: non-null (`INVALID_ARGUMENT`), valid for the scheduler's lifetime, must not throw, must not call `stop()` of its own scheduler.
  3. Context: opaque, nullable, caller-owned, never dereferenced/freed by the scheduler; must outlive running tasks (until `stop()` returned).
  4. `create_task()` only registers a STOPPED task, never starts it or calls `entry`.* Valid only while the scheduler is STOPPED; while RUNNING it returns `INVALID_STATE`.*
  5. `start()`/`stop()` are scheduler-wide and idempotent;* `start()` is all-or-nothing;* `stop()` returns only when no entry is running;* destruction implies `stop()`.*
  6. Priority: relative, larger = more urgent,* 0 = least urgent default; no OS mapping; above-adapter-maximum is `INVALID_ARGUMENT`, never clamped.*
  7. CPU affinity: bit mask of logical CPUs 0..31 (32-bit limit noted); non-zero is honoured or refused (`UNSUPPORTED` / `INVALID_ARGUMENT`), never silently ignored.*
  8. `cpu_affinity == 0` means "no constraint", not "CPU 0"; pin to CPU 0 with `0x1`.*
  9. `TaskId`: opaque, non-zero (0 reserved),* unique per scheduler instance, never reused, valid until the scheduler is destroyed. There is no task-destroy operation (follow-up candidate, not added).
  10. Resource exhaustion: `RESOURCE_UNAVAILABLE`, reported through `Result`, never abort/exception; failed `create_task` creates nothing and consumes no id; failed `start` leaves the scheduler STOPPED.
  Also: `period` 0 = aperiodic, positive = periodic, negative invalid; overrun behavior is adapter-defined.*
- Tests: `FakeScheduler` in `platform_test.cpp` replaced by a conforming reference double (previous assertions retained and passing); new tests for zero-value semantics, create-does-not-start, affinity, priority rejection, exhaustion, all-or-nothing start, id rules; reusable `check_scheduler_contract()` for future adapters.
- Build: `rm -rf build && cmake -S . -B build && cmake --build build -j$(nproc)` — 0 warnings.
- Tests: `ctest --test-dir build` — 16/16 passed (includes `kritva_core_platform`).
- Sanitizers: separate Debug build `-fsanitize=address,undefined` — 16/16 passed.
- Coverage: `make coverage` — 98% overall (unchanged).
- `scheduler.hpp` compiles standalone with `-Wall -Wextra`; header check passed; `git diff --check` clean.
- `make format-check`/`make lint`: TODO stubs, not executed.
- Known limitations: 32-bit affinity mask limits addressable CPUs to 32; no task-destroy/removal API; periodic overrun policy and timing accuracy are adapter-defined; the reference double is a test double and proves the contract is self-consistent, not that any real adapter conforms.
- Human review: this clarifies public API semantics (AGENTS.md section 12); items marked * need explicit reviewer confirmation.

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

- [ ] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Review notes:

TBD

## 15. Acceptance Rule

The task cannot be marked ACCEPTED solely because the code compiles. All applicable functional, test, regression, quality, scope, documentation, and Git criteria must have evidence.
