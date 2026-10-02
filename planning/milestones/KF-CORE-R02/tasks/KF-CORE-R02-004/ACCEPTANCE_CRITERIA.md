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
| TBD | TBD | TBD | TBD | TBD |

**Acceptance:** No requirement referenced by the implementation may remain undefined.

## 4. Functional Acceptance

- [ ] AC-001: Document TaskConfig fields and their semantics.
- [ ] AC-002: Clarify task entry function lifetime requirements.
- [ ] AC-003: Clarify ownership of task context.
- [ ] AC-004: Clarify whether create_task creates a stopped task or starts it.
- [ ] AC-005: Clarify start/stop idempotency and invalid-state behavior.
- [ ] AC-006: Clarify priority interpretation without assigning OS-specific numeric meaning.
- [ ] AC-007: Clarify CPU-affinity semantics; `cpu_affinity=0` must not remain ambiguous.
- [ ] AC-008: Clarify TaskId lifetime and uniqueness expectations.
- [ ] AC-009: Document resource-failure behavior.
- [ ] AC-010: Do not implement Linux pthreads, RTOS tasks, EtherCAT, or vendor SDK behavior.

## 5. Test Acceptance

- [ ] TEST-001: Header/API compile validation
- [ ] TEST-002: Contract-level tests if an interface mock exists
- [ ] TEST-003: Task configuration boundary tests where types permit

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
docs(core): clarify scheduler contract
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
