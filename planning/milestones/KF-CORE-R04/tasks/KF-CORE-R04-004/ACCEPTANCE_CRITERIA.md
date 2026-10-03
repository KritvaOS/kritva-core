# KF-CORE-R04-004 — Acceptance Criteria

## Task Information

- **Task ID:** `KF-CORE-R04-004`
- **Title:** Watchdog Contract
- **Requirement:** `CORE-PLAT-007`
- **Status:** PLANNED
- **Primary commit message:** `feat(core): define watchdog platform contract`

## Acceptance Decision

Reviewer decision:
- [ ] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Reviewer: ChatGPT architecture/review gate  
Implementation agent: Codex/Claude

## Objective

Define a complete watchdog adapter contract without coupling watchdog expiration to Runtime recovery.

## Scope

['watchdog state', 'start/kick/stop', 'timeout validation', 'idempotency', 'failure propagation', 'hardware/software boundary', 'expiry semantics']

## Out of Scope

['hardware watchdog driver', 'automatic Runtime reset/recovery', 'background watchdog service']

## Requirement Traceability

The implementation must define/trace `CORE-PLAT-007` without duplicating or renumbering existing requirements. The authoritative `REQUIREMENTS.md` is updated as part of the task when the requirement is actually implemented.

## Detailed Acceptance Criteria

['Watchdog initial and running/stopped states are explicit.', 'start, kick and stop semantics are deterministic.', 'Zero and negative timeout behavior is explicitly defined.', 'Repeated start/stop behavior is defined.', 'Kick behavior outside the running state is defined.', 'Failures preserve existing Error/Result semantics.', 'Watchdog expiry behavior is explicitly bounded to the adapter contract.', 'Watchdog expiry does not implicitly trigger RuntimeManager::reset().', 'No Core background worker is required by the contract.']

## New Tests Required

['state tests', 'timeout validation', 'idempotency', 'kick semantics', 'failure propagation', 'no implicit Runtime recovery']

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
feat(core): define watchdog platform contract
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
