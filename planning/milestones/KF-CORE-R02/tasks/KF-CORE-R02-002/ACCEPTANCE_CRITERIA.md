# KF-CORE-R02-002 — Acceptance Criteria

## 1. Task Information

- Task: Status API/Header Cleanup
- Milestone: `KF-CORE-R02`
- Dependency: `KF-CORE-R02-001`
- Expected commit:
  `fix(core): clean up Status API contract`

## 2. Objective

Make Status and StatusCode headers self-contained, semantically clear, and consistent with Core conventions.

## 3. Requirement Traceability

The implementer must identify the authoritative requirement IDs affected by this task.

| Requirement ID | Header/API | Implementation | Test | Evidence |
|---|---|---|---|---|
| CORE-STA-001 | `include/kritva/core/status/status_code.hpp`, `status.hpp` | header-only | `tests/unit/status_test.cpp` (`kritva_core_status`) | commit `d6d939d`; 16/16 ctest |

**Acceptance:** No requirement referenced by the implementation may remain undefined.

## 4. Functional Acceptance

- [x] AC-001: `status_code.hpp` remains the authoritative status-code enumeration.
- [x] AC-002: `status.hpp` must explicitly include every standard header needed by its declarations/implementation.
- [x] AC-003: Status construction and mutation semantics must be clear.
- [x] AC-004: Thread-safety/mutability assumptions must be documented.
- [x] AC-005: Do not expand the status taxonomy unless required to fix an identified contract gap.
- [x] AC-006: Do not redesign Status around exceptions.

## 5. Test Acceptance

- [x] TEST-001: Default status
- [x] TEST-002: Each defined StatusCode
- [x] TEST-003: Construction with code/detail where supported
- [x] TEST-004: Mutation/accessor behavior
- [x] TEST-005: Header self-containment compile test if the repository uses such tests

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
fix(core): clean up Status API contract
```

- [x] Commit hash recorded: `d6d939d`.

## 12a. Implementation Evidence (Claude)

- Commit: `d6d939d` `fix(core): clean up Status API contract` (R02-001 accepted at `ce8a21c`).
- Files changed: `include/kritva/core/status/status_code.hpp`, `include/kritva/core/status/status.hpp`, `tests/unit/status_test.cpp`, `API.md`.
- Scope decisions: no taxonomy change, no new members, no behavior change. Existing includes were already complete (`<cstdint>`, `<string>`, `<utility>`); verified by compiling each header standalone with `-Wall -Wextra`. Changes are contract documentation (code semantics, construction, mutability, allocation, thread-safety) and tests.
- Build: `rm -rf build && cmake -S . -B build && cmake --build build -j$(nproc)` — 0 warnings.
- Tests: `ctest --test-dir build` — 16/16 passed (includes `kritva_core_status`).
- Sanitizers: separate Debug build `-fsanitize=address,undefined` — 16/16 passed.
- Coverage: `make coverage` — 98% overall (unchanged); `status.hpp` 100%.
- Header check passed; `git diff --check` clean. `make format-check`/`make lint` are TODO stubs (not executed).
- Known limitations: `StatusCode` numeric values are documented as not stable for persistence; `Status` retains independent code/message (no OK-implies-empty-message rule) by design.

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

KF-CORE-R02-002 implementation commit `d6d939d` was reviewed against
the task requirements and submitted validation evidence.

The Status API contract has been clarified without changing the existing
StatusCode taxonomy or introducing unnecessary API redesign.

StatusCode semantics, construction, mutability, message lifetime,
allocation behavior, and thread-safety expectations are documented.
Standalone header compilation and Status copy/move/API behavior are
covered by tests.

Validation evidence is sufficient:
- Clean build: 0 warnings
- CTest: 16/16 passed
- ASan + UBSan: 16/16 passed
- Coverage: 98%
- `status.hpp`: 100%
- Standalone header compilation: passed
- Header check: passed
- `git diff --check`: passed
- format-check/lint: repository TODO stubs, therefore not applicable

Requirement traceability for `CORE-STA-001` is recorded.

Task KF-CORE-R02-002 is ACCEPTED.

Reviewer Decision: PASS

## 15. Acceptance Rule

The task cannot be marked ACCEPTED solely because the code compiles. All applicable functional, test, regression, quality, scope, documentation, and Git criteria must have evidence.
