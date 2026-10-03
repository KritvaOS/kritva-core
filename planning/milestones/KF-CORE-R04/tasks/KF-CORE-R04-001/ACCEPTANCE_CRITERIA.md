# KF-CORE-R04-001 — Acceptance Criteria

## Task Information

- **Task ID:** `KF-CORE-R04-001`
- **Title:** Platform Adapter Boundary & Context
- **Requirement:** `CORE-PLAT-004`
- **Status:** PLANNED
- **Primary commit message:** `feat(core): define platform adapter boundary`

## Acceptance Decision

Reviewer decision:
- [ ] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Reviewer: ChatGPT architecture/review gate  
Implementation agent: Codex/Claude

## Objective

Define the stable boundary between platform-independent Core contracts and external platform adapter implementations.

## Scope

['adapter interface/boundary semantics', 'ownership and lifetime rules', 'opaque context semantics', 'error propagation', 'platform-independence constraints', 'public-header contract tests']

## Out of Scope

['Linux/POSIX implementation', 'RTOS implementation', 'vendor BSP/HAL', 'hardware drivers', 'platform singleton', 'background execution']

## Requirement Traceability

The implementation must define/trace `CORE-PLAT-004` without duplicating or renumbering existing requirements. The authoritative `REQUIREMENTS.md` is updated as part of the task when the requirement is actually implemented.

## Detailed Acceptance Criteria

['Core-facing platform contracts are implementable outside kritva-core.', 'No production Core source includes Linux/POSIX/RTOS/vendor SDK/hardware headers.', 'Ownership and lifetime of adapter objects and opaque contexts are explicit.', 'Core does not create, own or destroy platform-global services unless a later contract explicitly requires it.', 'Adapter failures use existing Result/Error semantics.', 'Thread-safety is explicitly documented rather than implied.', 'No hard-real-time guarantee is introduced.', 'Fake adapters can exercise the boundary without hardware.']

## New Tests Required

['public-header compile test', 'adapter-boundary contract test', 'ownership/lifetime test', 'negative/error propagation test', 'prohibited dependency scan']

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
feat(core): define platform adapter boundary
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
