# KF-CORE-R02-003 — Acceptance Criteria

## 1. Task Information

- Task: Statistics Contract Clarification
- Milestone: `KF-CORE-R02`
- Dependency: `KF-CORE-R02-002`
- Expected commit:
  `docs(core): clarify statistics contract`

## 2. Objective

Define the contract of Counter, Gauge, and Statistics clearly without turning Core into a telemetry framework.

## 3. Requirement Traceability

The implementer must identify the authoritative requirement IDs affected by this task.

| Requirement ID | Header/API | Implementation | Test | Evidence |
|---|---|---|---|---|
| CORE-STS-001..003 | `include/kritva/core/statistics/{counter,gauge,statistics}.hpp` | header-only | `tests/unit/statistics_test.cpp` (`kritva_core_statistics`) | commit `d7d29cb`; 16/16 ctest |

**Acceptance:** No requirement referenced by the implementation may remain undefined.

## 4. Functional Acceptance

- [x] AC-001: Preserve the current simple counter/gauge model.
- [x] AC-002: Do not automatically convert fields to atomics.
- [x] AC-003: Document that current statistics objects are not thread-safe unless externally synchronized.
- [x] AC-004: Document that they are not hard-real-time synchronization primitives.
- [x] AC-005: Counter increment/read behavior must be deterministic.
- [x] AC-006: Gauge set/read behavior must be deterministic.
- [x] AC-007: Statistics aggregate fields must have clear meaning and units where applicable.
- [x] AC-008: Do not add telemetry transport, Prometheus, OpenTelemetry, or serialization.

## 5. Test Acceptance

- [x] TEST-001: Counter initial value
- [x] TEST-002: Counter increment and read
- [x] TEST-003: Gauge set and read
- [x] TEST-004: Statistics default state
- [x] TEST-005: Statistics field updates
- [x] TEST-006: Boundary values appropriate to existing types

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
docs(core): clarify statistics contract
```

- [x] Commit hash recorded: `d7d29cb`.

## 12a. Implementation Evidence (Claude)

- Commit: `d7d29cb` `docs(core): clarify statistics contract` (R02-002 accepted at `5763a9e`).
- Files changed: `include/kritva/core/statistics/counter.hpp`, `gauge.hpp`, `statistics.hpp`, `tests/unit/statistics_test.cpp`, `REQUIREMENTS.md` (CORE-STS-001..003), `API.md` (section 15).
- Scope: no behavior or layout change; no atomics; no telemetry/serialization. Contract documented: Counter wraps modulo 2^64, `increment(0)` no-op; Gauge stores any int64 as-is; all types not thread-safe, not synchronization primitives, no allocation/blocking, not hard-real-time.
- **Decision for reviewer:** `Statistics::utilization` previously had no stated unit. Documented as whole percent, 0..100 by convention, not enforced (existing tests already use 75/80/90). `queue_depth` documented as item count, >= 0 by convention, not enforced. Please confirm or request a different unit.
- Tests added: increment(0) no-op, reset after wrap, no clamping of Gauge/Statistics values, constexpr value-initialized Statistics, trivially-copyable and value_type signedness static_asserts.
- Build: `rm -rf build && cmake -S . -B build && cmake --build build -j$(nproc)` — 0 warnings.
- Tests: `ctest --test-dir build` — 16/16 passed (includes `kritva_core_statistics`).
- Sanitizers: separate Debug build `-fsanitize=address,undefined` — 16/16 passed.
- Coverage: `make coverage` — 98% overall (unchanged).
- Each statistics header compiles standalone with `-Wall -Wextra`; header check passed; `git diff --check` clean.
- `make format-check`/`make lint`: TODO stubs, not executed.
- Known limitations: copy of `Statistics` is not an atomic snapshot (documented); units other than the two conventions above are owner-defined.

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
