# KF-CORE-R04-008 — Acceptance Criteria

## Task Information

- **Task ID:** `KF-CORE-R04-008`
- **Title:** Full R0.4 Validation
- **Requirement:** `CORE-PLAT-011`
- **Status:** PLANNED
- **Primary commit message:** `test(core): complete R04 platform validation`

## Acceptance Decision

Reviewer decision:
- [ ] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Reviewer: ChatGPT architecture/review gate  
Implementation agent: Codex/Claude

## Objective

Validate the frozen R0.4 platform abstraction release candidate without introducing new functionality.

## Scope

['clean builds', 'CTest', 'sanitizers', 'strict diagnostics', 'static analysis', 'coverage', 'traceability', 'install consumer', 'dependency scan', 'freeze verification', 'documentation consistency']

## Out of Scope

['new production functionality', 'post-freeze API changes', 'platform implementations']

## Requirement Traceability

The implementation must define/trace `CORE-PLAT-011` without duplicating or renumbering existing requirements. The authoritative `REQUIREMENTS.md` is updated as part of the task when the requirement is actually implemented.

## Detailed Acceptance Criteria

['Fresh clone builds successfully in Debug and Release.', 'Complete CTest suite passes.', 'ASan and UBSan pass.', 'TSan passes where configured.', '-Werror passes.', 'GCC -fanalyzer passes.', 'Coverage is reviewed against the R0.3 baseline and changed contracts are adequately covered.', 'Requirements traceability reports zero errors.', 'Installed package is consumed by an external project.', 'VERSION and CMake project version agree.', 'Production Core sources contain no prohibited platform implementation/dependencies.', 'Production API/semantics remain unchanged after the Platform Integration Freeze unless explicitly reviewed.', 'Working tree is clean and evidence is reproducible.']

## New Tests Required

['full CTest', 'Debug/Release', 'ASan/UBSan', 'TSan where configured', '-Werror', '-fanalyzer', 'coverage', 'traceability', 'install consumer', 'dependency scan', 'freeze diff']

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
test(core): complete R04 platform validation
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
