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
| TBD | TBD | TBD | TBD | TBD |

**Acceptance:** No requirement referenced by the implementation may remain undefined.

## 4. Functional Acceptance

- [ ] AC-001: Preserve the current simple counter/gauge model.
- [ ] AC-002: Do not automatically convert fields to atomics.
- [ ] AC-003: Document that current statistics objects are not thread-safe unless externally synchronized.
- [ ] AC-004: Document that they are not hard-real-time synchronization primitives.
- [ ] AC-005: Counter increment/read behavior must be deterministic.
- [ ] AC-006: Gauge set/read behavior must be deterministic.
- [ ] AC-007: Statistics aggregate fields must have clear meaning and units where applicable.
- [ ] AC-008: Do not add telemetry transport, Prometheus, OpenTelemetry, or serialization.

## 5. Test Acceptance

- [ ] TEST-001: Counter initial value
- [ ] TEST-002: Counter increment and read
- [ ] TEST-003: Gauge set and read
- [ ] TEST-004: Statistics default state
- [ ] TEST-005: Statistics field updates
- [ ] TEST-006: Boundary values appropriate to existing types

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
docs(core): clarify statistics contract
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
