# KF-CORE-R02-008 — Acceptance Criteria

## 1. Task Information

- Task: Full R0.2 Validation
- Milestone: `KF-CORE-R02`
- Dependency: `KF-CORE-R02-007`
- Expected commit:
  `test(core): complete R0.2 validation`

## 2. Objective

Execute the final objective acceptance gate for KF-CORE-R02. This task is validation, not a feature-development task.

## 3. Requirement Traceability

The implementer must identify the authoritative requirement IDs affected by this task.

| Requirement ID | Header/API | Implementation | Test | Evidence |
|---|---|---|---|---|
| TBD | TBD | TBD | TBD | TBD |

**Acceptance:** No requirement referenced by the implementation may remain undefined.

## 4. Functional Acceptance

- [ ] AC-001: Start from a clean working tree.
- [ ] AC-002: Perform a clean configure/build.
- [ ] AC-003: Run all registered CTest tests.
- [ ] AC-004: Run the established Core regression suite.
- [ ] AC-005: Review test count and failures.
- [ ] AC-006: Review coverage and explain material gaps.
- [ ] AC-007: Run configured sanitizers/static analysis where available.
- [ ] AC-008: Review public API for accidental platform dependencies.
- [ ] AC-009: Review REQUIREMENTS.md traceability.
- [ ] AC-010: Review documentation and changelog.
- [ ] AC-011: Review Git history and task commits.
- [ ] AC-012: Record exact evidence in the task result.
- [ ] AC-013: Do not introduce unrelated feature work during validation.

## 5. Test Acceptance

- [ ] TEST-001: All CTest tests
- [ ] TEST-002: All 11 known regression tests
- [ ] TEST-003: Coverage
- [ ] TEST-004: Sanitizer/static analysis where configured
- [ ] TEST-005: Clean rebuild from a fresh build directory

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
test(core): complete R0.2 validation
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
