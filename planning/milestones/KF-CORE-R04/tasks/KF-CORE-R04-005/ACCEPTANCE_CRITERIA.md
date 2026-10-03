# KF-CORE-R04-005 — Acceptance Criteria

## Task Information

- **Task ID:** `KF-CORE-R04-005`
- **Title:** Platform Capability & Adapter Contract
- **Requirement:** `CORE-PLAT-008`
- **Status:** PLANNED
- **Primary commit message:** `feat(core): define platform capability contract`

## Acceptance Decision

Reviewer decision:
- [ ] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Reviewer: ChatGPT architecture/review gate  
Implementation agent: Codex/Claude

## Objective

Define minimal platform identity/capability reporting so higher layers can discover adapter capabilities without embedding platform implementation into Core.

## Scope

['platform identity', 'version', 'capability identities', 'supported/unsupported reporting', 'adapter metadata', 'non-owning reporting']

## Out of Scope

['large hardware inventory API', 'driver enumeration', 'automatic feature activation', 'platform singleton', 'vendor-specific capability types']

## Requirement Traceability

The implementation must define/trace `CORE-PLAT-008` without duplicating or renumbering existing requirements. The authoritative `REQUIREMENTS.md` is updated as part of the task when the requirement is actually implemented.

## Detailed Acceptance Criteria

['Platform identity is stable and represented with existing Core identity/version primitives where appropriate.', 'Capability identities are explicit and deterministic.', 'Unsupported capabilities are represented without pretending support exists.', 'Capability reporting does not instantiate hardware.', 'Capability reporting does not silently change Runtime behavior.', 'Adapter metadata does not expose vendor-specific implementation types through Core.', 'Ownership/lifetime of returned capability data is explicit.', 'No global singleton service registry is introduced.']

## New Tests Required

['identity tests', 'capability presence/absence tests', 'determinism tests', 'ownership/lifetime tests', 'unsupported capability tests']

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
feat(core): define platform capability contract
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
