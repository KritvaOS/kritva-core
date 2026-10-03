# KF-CORE-R05-002 — Platform Service Requirement Model

    ## Task Information

    - **Task ID:** `KF-CORE-R05-002`
    - **Title:** Platform Service Requirement Model
    - **Primary Area:** `Required/optional platform service and capability requirements`
    - **Requirement:** `CORE-PLAT-013`
    - **Status:** PLANNED
    - **Dependency:** R0.4 released; follows the R05 sequence
    - **Exact primary commit:** `feat(core): define platform service requirements`

    ## Objective

    Establish the R0.5 contract for platform service requirement model while preserving the released R0.4 platform boundary and R0.3 Runtime semantics.

    ## Scope

    - service requirement model
- capability-based requirement evaluation
- required/optional service semantics
- negative cases

    ## Out of Scope

    - Platform-specific capability inference
- Service locator
- Automatic service startup/shutdown
- Hardware capability inventory


## Architecture Rules

1. R0.4 released contracts are authoritative.
2. Core remains platform independent.
3. Platform objects remain externally owned and non-owning from Core.
4. No generic ServiceRegistry/service locator is introduced.
5. No concrete platform implementation is added to production Core.
6. No Core-owned background execution is introduced.
7. Breaking API/semantic changes require architecture review.
8. Unit tests validate focused task contracts; integration tests validate cross-component behavior.
9. The full existing regression suite must run for every task.
10. A task is not accepted solely because its new tests pass.


    ## Deliverables

    - Production implementation/documentation required by the task.
    - Focused unit tests for every changed production contract.
    - Integration tests where the task crosses Runtime/platform boundaries.
    - Updated CMake/test registration and traceability as required.
    - Evidence suitable for independent review.
    - One primary Git commit using the exact commit message in the acceptance criteria.

    ## Testing Rule

    **Unit testing:** Every production contract introduced or changed by this task must have focused unit/contract tests. Tests must exercise success, invalid input/state and relevant failure paths.

    **Integration testing:** Where the task affects interactions between Runtime, PlatformContext, IPlatformAdapter or platform services, add integration tests using only public APIs and test-only reference/fake services. Integration tests must not depend on physical hardware.

    **Regression running:** Every implementation attempt must run the complete existing CTest suite after the focused tests are added. Passing only the new tests is insufficient for acceptance.

    Minimum:

    ```bash
    cmake -S . -B build
    cmake --build build -j$(nproc)
    ctest --test-dir build --output-on-failure
    ```

    Also run applicable repository checks:

    ```bash
    make check
    make traceability-check
    ```

    ## Evidence Required

    The implementation agent must provide:

    1. primary commit SHA;
    2. exact changed-file list;
    3. `git diff --check`;
    4. clean build result;
    5. focused unit-test result;
    6. full regression result;
    7. integration-test result where applicable;
    8. sanitizer/static-analysis result where applicable;
    9. coverage result where applicable;
    10. traceability result;
    11. prohibited-dependency result;
    12. public API summary;
    13. explicit confirmation of out-of-scope exclusions;
    14. final `git status`.

    ## Commit Discipline

    One logical task = one primary implementation commit.

    Do not mix unrelated changes. Do not amend an accepted implementation commit. If review finds a defect after acceptance, use a focused follow-up commit and record it.

    ## Reviewer Authority

    Claude/Codex may report objective evidence and mark implementation evidence complete. Only the independent architecture reviewer records PASS / CHANGES REQUIRED / BLOCKED.
