# KF-CORE-R02-007 — Acceptance Criteria

## 1. Task Information

- Task: Foundation Contract Tests
- Milestone: `KF-CORE-R02`
- Dependency: `KF-CORE-R02-006`
- Expected commit:
  `test(core): strengthen foundation contract coverage`

## 2. Objective

Strengthen automated tests so the R0.2 foundation contracts are objectively verified before Runtime Foundation work begins.

## 3. Requirement Traceability

The implementer must identify the authoritative requirement IDs affected by this task.

| Requirement ID | Header/API | Implementation | Test | Evidence |
|---|---|---|---|---|
| TBD | TBD | TBD | TBD | TBD |

**Acceptance:** No requirement referenced by the implementation may remain undefined.

## 4. Functional Acceptance

- [ ] AC-001: Tests must distinguish new/changed tests from existing regression tests.
- [ ] AC-002: Cover Lifecycle, Status, Health, Error, Result, Capability, Configuration, Event, Statistics, and Types.
- [ ] AC-003: Cover normal and negative/boundary behavior where the API defines it.
- [ ] AC-004: Tests must not depend on wall-clock timing, network, hardware, or vendor SDKs.
- [ ] AC-005: Tests should be deterministic and repeatable.
- [ ] AC-006: Do not add tests for unimplemented Runtime Manager behavior.

## 5. Test Acceptance

- [ ] TEST-001: Lifecycle transition validity/invalidity
- [ ] TEST-002: Status contract
- [ ] TEST-003: Health contract
- [ ] TEST-004: Error contract
- [ ] TEST-005: Result contract
- [ ] TEST-006: Capability set behavior
- [ ] TEST-007: Configuration validation/get/set
- [ ] TEST-008: Event envelope
- [ ] TEST-009: Statistics
- [ ] TEST-010: Types

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
test(core): strengthen foundation contract coverage
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
