# KF-CORE-R02-006 — Acceptance Criteria

## 1. Task Information

- Task: Requirements/API Traceability
- Milestone: `KF-CORE-R02`
- Dependency: `KF-CORE-R02-005`
- Expected commit:
  `docs(core): close requirements API traceability`

## 2. Objective

Create complete traceability from authoritative requirements to public headers, implementation, and tests.

## 3. Requirement Traceability

The implementer must identify the authoritative requirement IDs affected by this task.

| Requirement ID | Header/API | Implementation | Test | Evidence |
|---|---|---|---|---|
| TBD | TBD | TBD | TBD | TBD |

**Acceptance:** No requirement referenced by the implementation may remain undefined.

## 4. Functional Acceptance

- [ ] AC-001: Every requirement ID referenced by public headers, tests, or documentation must exist in REQUIREMENTS.md.
- [ ] AC-002: Every public Core contract must map to an authoritative requirement ID.
- [ ] AC-003: Traceability must cover Types, Status, Health, Error/Result, Event, Capability, Configuration, Lifecycle, Statistics, Time, Messaging, Runtime contracts, and Platform abstractions present in R0.2.
- [ ] AC-004: Where IDs such as CORE-TYP-001, CORE-STA-001, CORE-HEA-001, CORE-ERR-001, CORE-EVT-001, CORE-CAP-001, and CORE-CFG-001 are used, their definitions must be authoritative.
- [ ] AC-005: Traceability format must be consistent: Requirement → Public Header → Implementation → Test.
- [ ] AC-006: Do not invent implementation behavior merely to fill a traceability table.

## 5. Test Acceptance

- [ ] TEST-001: Traceability audit script/check if practical
- [ ] TEST-002: Manual verification that each requirement points to a real header/test
- [ ] TEST-003: Verify no stale requirement IDs remain

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
docs(core): close requirements API traceability
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
