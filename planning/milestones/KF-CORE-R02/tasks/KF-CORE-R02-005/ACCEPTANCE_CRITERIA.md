# KF-CORE-R02-005 — Acceptance Criteria

## 1. Task Information

- Task: Clock Abstraction Cleanup
- Milestone: `KF-CORE-R02`
- Dependency: `KF-CORE-R02-004`
- Expected commit:
  `refactor(core): canonicalize clock abstraction`

## 2. Objective

Establish one canonical platform-neutral clock abstraction and eliminate conceptual duplication without breaking compatibility unnecessarily.

## 3. Requirement Traceability

The implementer must identify the authoritative requirement IDs affected by this task.

| Requirement ID | Header/API | Implementation | Test | Evidence |
|---|---|---|---|---|
| TBD | TBD | TBD | TBD | TBD |

**Acceptance:** No requirement referenced by the implementation may remain undefined.

## 4. Functional Acceptance

- [ ] AC-001: `time/clock.hpp` is the canonical IClock abstraction.
- [ ] AC-002: Clock domain semantics must distinguish monotonic and realtime clocks.
- [ ] AC-003: Callers must not silently compare timestamps from incompatible clock domains.
- [ ] AC-004: `platform/clock.hpp` may remain only as a compatibility alias if needed.
- [ ] AC-005: If a compatibility alias remains, it must not become a second independent contract.
- [ ] AC-006: Do not implement OS clocks in Core.
- [ ] AC-007: Do not add timers, scheduling, or callbacks beyond the existing contract.

## 5. Test Acceptance

- [ ] TEST-001: Clock interface compile test
- [ ] TEST-002: Timestamp/clock domain contract test if supported
- [ ] TEST-003: Compatibility include test if alias retained
- [ ] TEST-004: Timer interaction compile/API regression

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
refactor(core): canonicalize clock abstraction
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
