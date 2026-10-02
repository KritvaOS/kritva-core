# KF-CORE-R02-001 — Acceptance Criteria

## 1. Task Information

- Task: Result<T> Contract Hardening
- Milestone: `KF-CORE-R02`
- Dependency: `KF-CORE-R01`
- Expected commit:
  `fix(core): harden Result contract`

## 2. Objective

Make Result<T> and Result<void> success/failure state semantics explicit and testable, including safe preconditions for value/error access and correct move behavior.

## 3. Requirement Traceability

The implementer must identify the authoritative requirement IDs affected by this task.

| Requirement ID | Header/API | Implementation | Test | Evidence |
|---|---|---|---|---|
| CORE-ERR-004 | `include/kritva/core/error/result.hpp` | `Result<T>`, `Result<void>` (header-only) | `tests/unit/result_test.cpp` (`kritva_core_result`) | commit `df44d38`; 16/16 ctest |

**Acceptance:** No requirement referenced by the implementation may remain undefined.

## 4. Functional Acceptance

- [x] AC-001: Preserve the established Result design unless a concrete contract defect requires a minimal correction.
- [x] AC-002: `Result<T>` must represent exactly one logical outcome: success containing T or failure containing Error.
- [x] AC-003: `Result<void>` must represent success or failure without requiring a value object.
- [x] AC-004: `has_value()` should be available if the current API lacks an unambiguous success-state query.
- [x] AC-005: `value()` is valid only for a successful Result; `error()` is valid only for a failed Result. The chosen invalid-access behavior must be explicit and tested.
- [x] AC-006: Move construction/assignment must preserve a valid post-operation contract for the destination.
- [x] AC-007: Do not introduce exceptions, a new outcome framework, serialization, or runtime dependencies.

## 5. Test Acceptance

- [x] TEST-001: Result<T> success construction and state query
- [x] TEST-002: Result<T> failure construction and error retrieval
- [x] TEST-003: Result<void> success
- [x] TEST-004: Result<void> failure
- [x] TEST-005: Invalid value/error access according to the documented contract
- [x] TEST-006: Move construction
- [x] TEST-007: Move assignment
- [x] TEST-008: Copy behavior if supported by the current API

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
fix(core): harden Result contract
```

- [x] Commit hash recorded: `df44d38`.

## 12a. Implementation Evidence (Claude)

- Commit: `df44d38` `fix(core): harden Result contract` (planning baseline: `e6acfb6`).
- Files changed: `include/kritva/core/error/result.hpp`, `tests/unit/result_test.cpp`, `REQUIREMENTS.md`, `API.md`.
- Build: `rm -rf build && cmake -S . -B build && cmake --build build -j$(nproc)` — 0 warnings.
- Tests: `ctest --test-dir build` — 16/16 passed (includes `kritva_core_result`).
- Sanitizers: separate Debug build with `-fsanitize=address,undefined -fno-sanitize-recover=all` — 16/16 passed.
- Coverage: `make coverage` — 98% (154/157 lines); uncovered lines are in `configuration.cpp`/`lifecycle.cpp`, unrelated to this task.
- Header check passed; `git diff --check` clean.
- `make format-check` and `make lint`: not executed as checks — repository targets are TODO stubs (clang-format/clang-tidy/cppcheck not configured).
- Contract recorded: `failure()` requires an error code other than `ErrorCode::NONE`. Passing `NONE` violates the API precondition; debug builds diagnose it via `assert`, release builds (NDEBUG) do not abort and behavior is undefined. Tests run with asserts enabled (`-UNDEBUG`).
- Known limitations: invalid-access trap tests use `fork()` and run only on unix/apple; release-build misuse remains undefined behavior by design.

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
