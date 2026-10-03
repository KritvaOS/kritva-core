# KF-CORE-R04-003 — Acceptance Criteria

## Task Information

- **Task ID:** `KF-CORE-R04-003`
- **Title:** Clock & Timer Contract
- **Requirement:** `CORE-PLAT-006`
- **Status:** PLANNED
- **Primary commit message:** `feat(core): define clock and timer platform contracts`

## Acceptance Decision

Reviewer decision:
- [ ] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Reviewer: ChatGPT architecture/review gate  
Implementation agent: Codex/Claude

## Objective

Complete the clock/timer platform boundary while preserving time::IClock as the canonical clock contract.

## Scope

['clock canonicalization', 'clock domains', 'timer lifecycle', 'one-shot/periodic behavior', 'cancellation', 'callback/context lifetime', 'execution context', 'ownership', 'failure semantics']

## Out of Scope

['hardware clock implementation', 'OS timer implementation', 'Core-owned worker thread', 'automatic Runtime recovery']

## Requirement Traceability

The implementation must define/trace `CORE-PLAT-006` without duplicating or renumbering existing requirements. The authoritative `REQUIREMENTS.md` is updated as part of the task when the requirement is actually implemented.

## Detailed Acceptance Criteria

['time::IClock remains the canonical clock abstraction.', 'platform::IClock, if retained, remains an alias and not a competing contract.', 'Clock-domain semantics remain explicit and cross-domain timestamps remain incomparable.', 'Timer lifecycle and state transitions are explicitly defined.', 'One-shot and periodic timer behavior is unambiguous.', 'Cancellation semantics and race boundaries are documented.', 'Timer callback and context lifetime are explicit.', 'Timer callback execution context does not imply a Core-owned thread.', 'Invalid duration/error behavior is explicit.', 'Timer ownership and destruction semantics are explicit.']

## New Tests Required

['clock alias/domain tests', 'timer lifecycle tests', 'one-shot/periodic tests', 'cancellation tests', 'callback/context lifetime tests', 'negative duration/error tests']

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
feat(core): define clock and timer platform contracts
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
