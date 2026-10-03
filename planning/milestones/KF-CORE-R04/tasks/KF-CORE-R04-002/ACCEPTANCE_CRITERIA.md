# KF-CORE-R04-002 — Acceptance Criteria

## Task Information

- **Task ID:** `KF-CORE-R04-002`
- **Title:** Scheduler Contract Hardening
- **Requirement:** `CORE-PLAT-005`
- **Status:** PLANNED
- **Primary commit message:** `feat(core): harden scheduler platform contract`

## Acceptance Decision

Reviewer decision:
- [ ] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Reviewer: ChatGPT architecture/review gate  
Implementation agent: Codex/Claude

## Objective

Harden CORE-PLAT-001 into a precise scheduler adapter contract without implementing a scheduler.

## Scope

['TaskId', 'TaskConfig', 'create_task semantics', 'start/stop', 'dynamic creation', 'affinity/priority policy', 'period semantics', 'context/entry lifetime', 'resource failure', 'teardown', 'thread-safety and RT boundary']

## Out of Scope

['Linux scheduler', 'RTOS scheduler', 'thread pool', 'hard RT guarantee', 'scheduler-owned Runtime recovery']

## Requirement Traceability

The implementation must define/trace `CORE-PLAT-005` without duplicating or renumbering existing requirements. The authoritative `REQUIREMENTS.md` is updated as part of the task when the requirement is actually implemented.

## Detailed Acceptance Criteria

['TaskId zero is invalid and successful IDs are unique within a scheduler instance.', 'Failed task creation is atomic and consumes no ID.', 'Task entry and context lifetime requirements are explicit.', 'Priority remains relative and platform mapping remains adapter-defined.', 'CPU affinity semantics distinguish invalid masks from unsupported affinity.', 'Zero affinity means unconstrained; zero period means aperiodic; negative period is invalid.', 'Scheduler start/stop semantics, idempotency and failure atomicity are explicit.', 'Dynamic creation while running is explicitly adapter policy.', 'Stopping from a task entry is explicitly prohibited where synchronous stop would self-wait.', 'No hard-real-time, latency or jitter guarantee is made by Core.', 'Scheduler operations report failures through Result and do not throw.']

## New Tests Required

['scheduler contract shape', 'ID uniqueness/atomicity', 'configuration semantics', 'start/stop idempotency', 'dynamic creation policies', 'resource exhaustion', 'failure atomicity', 'entry/context lifetime']

## Regression Tests

At minimum:

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
```

All existing R0.3 tests must remain green.

## Quality Requirements

- No compiler warnings in the supported clean build.
- `-Werror` must pass where configured.
- ASan/UBSan must pass where configured.
- TSan must pass where configured for concurrency-sensitive changes.
- GCC `-fanalyzer` must pass where configured.
- `make traceability-check` must report zero errors.
- Prohibited platform dependency scan must report zero production violations.
- Public API changes must be documented.
- No unrelated generated files or changes.

## Evidence Required From Implementation Agent

Provide:

1. implementation commit SHA;
2. exact files changed;
3. `git diff --check` result;
4. build commands and results;
5. relevant CTest output;
6. sanitizer/static-analysis results where applicable;
7. coverage result where applicable;
8. traceability result;
9. prohibited-dependency scan result;
10. public API diff/summary;
11. explicit confirmation that out-of-scope platform implementations were not added;
12. working-tree status.

## Expected Files Changed

The implementation agent must list the actual files. Do not pre-authorize unrelated files. Public headers, implementation files, tests, CMake registration, requirements and directly relevant documentation may change.

## Commit

Use exactly:

```text
feat(core): harden scheduler platform contract
```

One logical task = one primary implementation commit. Review fixes after review use a separate focused commit.

## Reviewer Sign-off

- [ ] Scope satisfied
- [ ] Requirement traceability satisfied
- [ ] Tests satisfied
- [ ] Quality checks satisfied
- [ ] Evidence reproducible
- [ ] Architecture boundary preserved
- [ ] No unresolved blocker

Final reviewer decision is made independently after evidence review.
