# KF-CORE-R05-002 — Acceptance Criteria

    ## Task Information

    - **Task ID:** `KF-CORE-R05-002`
    - **Title:** Platform Service Requirement Model
    - **Requirement:** `CORE-PLAT-013`
    - **Status:** PLANNED
    - **Primary commit message:** `feat(core): define platform service requirements`

    ## Acceptance Decision

    Reviewer decision:
    - [ ] PASS
    - [ ] CHANGES REQUIRED
    - [ ] BLOCKED

    Reviewer: ChatGPT architecture/review gate  
    Implementation agent: Claude/Codex

    ## Detailed Acceptance Criteria

    - [ ] Required and optional services are represented explicitly.
- [ ] Capability identity is authoritative; platform name/version is never used for inference.
- [ ] Missing required services produce documented deterministic results.
- [ ] Requirement evaluation has no side effects.
- [ ] Existing CapabilitySet semantics are preserved.

    ## New Unit / Contract Tests Required

    - [ ] required service present/absent
- [ ] optional service absent
- [ ] capability matching
- [ ] unsupported requirement
- [ ] no side effects

    ## Integration Testing Rule

    Integration coverage must prove requirement evaluation does not mutate Runtime or platform service state.

    - [ ] Integration tests use only public APIs.
    - [ ] No physical hardware is required.
    - [ ] Test-only reference/fake services are used where needed.
    - [ ] No production test hook or private API is introduced solely to make integration testing possible.

    ## Regression Testing Rule

    Every task must run the complete existing regression suite, not only newly added tests.

    ```bash
    cmake -S . -B build
    cmake --build build -j$(nproc)
    ctest --test-dir build --output-on-failure
    ```

    Required where supported by the repository:

    ```bash
    make check
    make traceability-check
    ```

    - [ ] New/focused tests pass.
    - [ ] Full CTest regression passes.
    - [ ] No previously passing test regresses.
    - [ ] Regression output is supplied as evidence.

    ## Quality Requirements

    - [ ] No compiler warnings in the supported clean build.
    - [ ] `-Werror` passes where configured.
    - [ ] ASan/UBSan passes where configured.
    - [ ] TSan passes where configured for concurrency-sensitive changes.
    - [ ] GCC `-fanalyzer` passes where configured.
    - [ ] Coverage is reviewed.
    - [ ] Traceability audit reports zero errors.
    - [ ] Prohibited dependency audit reports zero production violations.
    - [ ] Public API changes are documented.
    - [ ] No unrelated generated files or changes are present.

    ## Expected Files Changed

    Actual changed files must be listed by the implementation agent. Do not pre-authorize unrelated files.

    Expected categories may include:

    - public headers;
    - production source where required;
    - focused unit/contract tests;
    - integration tests;
    - CMake/test registration;
    - `REQUIREMENTS.md` when the proposed requirement becomes authoritative;
    - directly relevant architecture/API documentation.

    ## Commit

    Use exactly:

    ```text
    feat(core): define platform service requirements
    ```

    One logical task = one primary implementation commit.

    ## Evidence Required From Implementation Agent

    1. [ ] Primary commit SHA.
    2. [ ] Exact changed files.
    3. [ ] `git diff --check` result.
    4. [ ] Build command/result.
    5. [ ] Focused unit-test command/result.
    6. [ ] Full regression command/result.
    7. [ ] Integration-test command/result, where applicable.
    8. [ ] Sanitizer/static-analysis result, where applicable.
    9. [ ] Coverage result, where applicable.
    10. [ ] Traceability result.
    11. [ ] Prohibited-dependency scan result.
    12. [ ] Public API diff/summary.
    13. [ ] Explicit out-of-scope confirmation.
    14. [ ] Final working-tree status.

    ## Reviewer Sign-Off

    | Item | Result |
    |---|---|
    | Reviewer | ChatGPT architecture/review gate |
    | Decision | PENDING |
    | Accepted commit | PENDING |
    | Evidence reference | PENDING |
    | Date | PENDING |

    **Reviewer Decision:** PENDING
