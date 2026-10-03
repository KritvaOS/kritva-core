# KF-CORE-R04-007 — Acceptance Criteria

## Task Information

- **Task ID:** `KF-CORE-R04-007`
- **Title:** Runtime–Platform Integration Boundary
- **Requirement:** `CORE-PLAT-010`
- **Status:** PLANNED
- **Primary commit message:** `feat(core): define runtime platform integration boundary`

## Acceptance Decision

Reviewer decision:
- [ ] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Reviewer: ChatGPT architecture/review gate  
Implementation agent: Codex/Claude

## Objective

Define how future Runtime integration may consume platform services without changing accepted R0.3 Runtime semantics or adding platform implementation to Core.

## Scope

['Runtime/platform dependency boundary', 'optional scheduler integration', 'clock use', 'watchdog boundary', 'error propagation', 'lifetime/ownership', 'configuration boundary']

## Out of Scope

['thread creation in Runtime', 'Linux/RTOS implementation', 'automatic watchdog recovery', 'global platform services', 'background execution']

## Requirement Traceability

The implementation must define/trace `CORE-PLAT-010` without duplicating or renumbering existing requirements. The authoritative `REQUIREMENTS.md` is updated as part of the task when the requirement is actually implemented.

## Detailed Acceptance Criteria

['RuntimeManager remains platform independent.', 'Scheduler integration is explicit and optional.', 'Clock integration preserves accepted R0.3 lifecycle semantics.', 'Watchdog use cannot silently trigger Runtime recovery.', 'Platform errors preserve Result/Error semantics.', 'Platform service ownership/lifetime is explicit.', 'No Core background thread or executor is introduced.', 'No platform-specific header enters Runtime production code.', 'No R0.3 Runtime API or semantic change occurs without architecture review.']

## New Tests Required

['compile/boundary tests', 'Runtime regression', 'platform failure propagation', 'no-thread/prohibited-dependency scan', 'public API compatibility check']

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
feat(core): define runtime platform integration boundary
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
