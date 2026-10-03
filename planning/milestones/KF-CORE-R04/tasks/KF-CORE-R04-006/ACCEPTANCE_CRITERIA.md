# KF-CORE-R04-006 — Acceptance Criteria

## Task Information

- **Task ID:** `KF-CORE-R04-006`
- **Title:** Platform Conformance Tests
- **Requirement:** `CORE-PLAT-009`
- **Status:** PLANNED
- **Primary commit message:** `test(core): add platform conformance suite`

## Acceptance Decision

Reviewer decision:
- [ ] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Reviewer: ChatGPT architecture/review gate  
Implementation agent: Codex/Claude

## Objective

Provide reusable contract tests for future platform adapters using public APIs and no physical hardware.

## Scope

['scheduler conformance', 'clock conformance', 'timer conformance', 'watchdog conformance', 'adapter-defined behavior separation', 'test-only fakes', 'mutation testing']

## Out of Scope

['real Linux adapter', 'real RTOS adapter', 'hardware CI requirement', 'platform implementation']

## Requirement Traceability

The implementation must define/trace `CORE-PLAT-009` without duplicating or renumbering existing requirements. The authoritative `REQUIREMENTS.md` is updated as part of the task when the requirement is actually implemented.

## Detailed Acceptance Criteria

['Conformance tests use only public Core APIs.', 'Tests run without hardware, network or vendor SDK.', 'Mandatory Core semantics are tested independently from adapter policy.', 'Adapter-defined behavior is not incorrectly asserted as universal.', 'Negative and failure paths are covered.', 'Tests are reusable by future external adapters.', 'Mutation testing demonstrates that important contract violations are detected.', 'No production API is added solely to make tests easier.', 'Existing R0.3 regression remains green.']

## New Tests Required

['all platform contract suites', 'negative cases', 'permutation/ordering where applicable', 'mutation testing', 'full regression']

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
test(core): add platform conformance suite
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
