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
| TBD | TBD | TBD | TBD | TBD |

**Acceptance:** No requirement referenced by the implementation may remain undefined.

## 4. Functional Acceptance

- [ ] AC-001: Preserve the established Result design unless a concrete contract defect requires a minimal correction.
- [ ] AC-002: `Result<T>` must represent exactly one logical outcome: success containing T or failure containing Error.
- [ ] AC-003: `Result<void>` must represent success or failure without requiring a value object.
- [ ] AC-004: `has_value()` should be available if the current API lacks an unambiguous success-state query.
- [ ] AC-005: `value()` is valid only for a successful Result; `error()` is valid only for a failed Result. The chosen invalid-access behavior must be explicit and tested.
- [ ] AC-006: Move construction/assignment must preserve a valid post-operation contract for the destination.
- [ ] AC-007: Do not introduce exceptions, a new outcome framework, serialization, or runtime dependencies.

## 5. Test Acceptance

- [ ] TEST-001: Result<T> success construction and state query
- [ ] TEST-002: Result<T> failure construction and error retrieval
- [ ] TEST-003: Result<void> success
- [ ] TEST-004: Result<void> failure
- [ ] TEST-005: Invalid value/error access according to the documented contract
- [ ] TEST-006: Move construction
- [ ] TEST-007: Move assignment
- [ ] TEST-008: Copy behavior if supported by the current API

## 6. Regression Acceptance

- [ ] All known regression tests pass.
- [ ] `ctest --test-dir build --output-on-failure` passes.
- [ ] No previously passing test is removed or disabled without explicit review.

## 7. Build Acceptance

- [ ] Clean configure succeeds.
- [ ] Clean build succeeds.
- [ ] No new compiler errors.
- [ ] No new unexplained compiler warnings.

## 8. Coverage Acceptance

- [ ] Coverage is generated/reviewed if configured.
- [ ] New logic has appropriate test coverage.
- [ ] Any material uncovered branch is documented.

## 9. Sanitizer / Static Analysis Acceptance

- [ ] Required configured sanitizer runs pass.
- [ ] Required configured static analysis passes.
- [ ] Any existing unrelated finding is explicitly identified rather than hidden.

## 10. Scope Acceptance

- [ ] No Runtime Manager implementation added.
- [ ] No platform-specific implementation added to Core.
- [ ] No unrelated refactoring.
- [ ] Public API changes are limited to this task's contract needs.

## 11. Documentation Acceptance

- [ ] Relevant API/requirements documentation updated.
- [ ] Requirement IDs are traceable.
- [ ] No documentation contradicts the implementation.

## 12. Git Acceptance

- [ ] Working tree was clean before implementation.
- [ ] Diff reviewed.
- [ ] Commit contains only this task's logical changes.
- [ ] Exact commit message used:

```text
fix(core): harden Result contract
```

- [ ] Commit hash recorded.

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
