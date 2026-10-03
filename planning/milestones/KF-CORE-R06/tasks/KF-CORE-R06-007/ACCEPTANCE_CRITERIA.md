# KF-CORE-R06-007 — Acceptance Criteria

## 1. Task information

- Task: `KF-CORE-R06-007`
- Title: Full R0.6 Validation
- Milestone: `KF-CORE-R06`
- Estimated effort: 2–3 ED
- Dependency: R06 Integration Freeze PASS/HONORED
- Requirement proposed: `CORE-CTX-007`
- Exact primary commit message: `test(core): complete R0.6 validation`

## 2. Objective


Perform fresh-clone final validation and release-candidate checks for version 0.6.0 without changing the frozen production API.


## 3. Required acceptance criteria

### Contract correctness

- [ ] The implementation matches the approved R06 architecture.
- [ ] Ownership and lifetime semantics are explicit.
- [ ] No hidden lifecycle side effects are introduced.
- [ ] Existing R0.5 behavior remains intact.
- [ ] Error behavior is deterministic and documented.
- [ ] Invalid/unavailable paths are explicitly tested where applicable.

### API discipline

- [ ] No unrelated public API changes.
- [ ] No breaking change hidden inside the task.
- [ ] No generic registry/locator.
- [ ] No platform name/version inference.
- [ ] No concrete OS/vendor/platform dependency.

### Tests

- [ ] Focused unit/contract tests added or updated.
- [ ] Negative/error/invalid-state tests added where applicable.
- [ ] Mutation testing performed for contract-sensitive behavior.
- [ ] Existing regression suite remains green.
- [ ] Integration tests added when task changes an integration boundary.


## Mandatory quality checks

- `git diff --check`
- Debug build
- Release build
- full CTest regression
- strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`
- ASan + UBSan
- TSan with documented ASLR-disabled environment where required
- GCC `-fanalyzer`
- public-header self-containment
- traceability audit
- prohibited dependency/header audit
- install-consumer test
- coverage review
- clean working tree

Passing new tests alone is not acceptance evidence.

## Regression rule

The complete existing regression suite must remain green. R06 tests supplement; they do not replace R0.2–R0.5 coverage.

## Acceptance decision

Only the independent reviewer records:

- PASS
- CHANGES REQUIRED
- BLOCKED

Claude/Codex may provide objective evidence and mark evidence checkboxes but does not record the reviewer verdict.


## 4. Requirement traceability

- [ ] `CORE-CTX-007` is defined in authoritative `REQUIREMENTS.md` only after task acceptance.
- [ ] Artifact and verification references are recorded.
- [ ] `make traceability-check` passes or equivalent repository audit passes.
- [ ] Previous requirement IDs are not renumbered.

## 5. Build commands

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
```

## 6. Test commands

```bash
ctest --test-dir build --output-on-failure
make check
make traceability-check
```

Apply configured sanitizer, strict-warning and analyzer commands required by the repository.

## 7. Coverage

- [ ] Coverage baseline reviewed.
- [ ] No unexplained regression in coverage.
- [ ] Any uncovered new behavior is justified in evidence.

## 8. Static / sanitizer analysis

- [ ] ASan/UBSan pass.
- [ ] TSan pass where configured.
- [ ] Strict `-Werror` pass.
- [ ] GCC `-fanalyzer` pass.
- [ ] Public-header self-containment pass.
- [ ] Dependency/prohibited-header audit pass.

## 9. Expected files

Task-specific paths may include:

- `include/kritva/core/...`
- `src/...`
- `tests/unit/...`
- `tests/integration/...`
- `tests/contract/...`
- `tests/audit/...`
- `REQUIREMENTS.md`
- `API.md`
- `ARCHITECTURE.md`
- corresponding planning/evidence files

Only files relevant to the approved task may change.

## 10. Git commit

Primary implementation commit must be exactly:

```text
test(core): complete R0.6 validation
```

Do not amend an accepted commit. Use a focused follow-up commit if independent review identifies a correction before acceptance.

## 11. Evidence supplied by implementer

- [ ] Commit SHA
- [ ] Test output
- [ ] Regression output
- [ ] Quality checks
- [ ] Traceability result
- [ ] Coverage result
- [ ] Mutation results
- [ ] Diff summary
- [ ] Out-of-scope confirmation

## 11a. Validation evidence (Claude)

Validated release candidate: HEAD `b473e5d` (`build(core): prepare 0.6.0 release metadata`), validated from a **fresh `git clone`** of the repository (clean tree, `git status` empty before building). The validation record itself is committed afterwards as `test(core): complete R0.6 validation` (documentation only). No new production functionality: the task validates the frozen release candidate.

### Candidate history since the Integration Freeze (production freeze point `5b755af`)
| Commit | Purpose | Production code? |
|---|---|---|
| `c7f7b46` `test(core): add component context reference harness` | R06-005 (accepted) | no (tests only) |
| `2fd5424` `test(core): add component context integration tests` | R06-006 (accepted) | no (tests only) |
| `5a4aee7` `docs(core): define R06 release validation requirement` | defines `CORE-CTX-007` and its traceability row | no |
| `b473e5d` `build(core): prepare 0.6.0 release metadata` | `VERSION` and CMake project version 0.6.0; README `find_package(kritva_core 0.6)`; REQUIREMENTS/API titles R0.6; TESTING; root CHANGELOG 0.6.0 section; install consumer default request 0.6 and a `ComponentContext` built for a component against the installed package | no |

**Production-change audit:** `git diff 5b755af HEAD -- include src` is **empty**: production code is byte-identical to the Integration Freeze point. Against the released `kritva-core-r0.5` tag, `src/` is unchanged and the only production header differences are the reviewed R06 set: `runtime/component_context.hpp` (new) and one include in `core.hpp`; every R0.3, R0.4 and R0.5 header is byte-identical to the tag.

### AC-01 Clean build
Fresh clone: Debug exit 0, 0 warnings; Release (`-DCMAKE_BUILD_TYPE=Release`) 0 warnings. GCC 11.4.0, CMake 3.30.2, Linux 6.8. The public-header self-containment check (`kritva_core_header_checks`, one translation unit per public header) is part of the build and passed. No build artifacts are committed.

### AC-02 Complete regression
`ctest`: **45/45 passed** in Debug and in Release (unit, contract, conformance, platform, component context, integration, isolation and install tests). 10 randomized-order repetitions (`--repeat until-fail:10 --schedule-random`): all passed. The complete R0.3 runtime group, the R0.4 platform, conformance and runtime-platform tests, the R0.5 context, requirements, reference platform and integration tests, and every R0.6 test pass; no existing test was modified in R0.6 except test-support (`reference_component.hpp` no longer `final`, `runtime_scenarios.hpp` gained an optional component factory). No test disabled, skipped or weakened.

### AC-03 Sanitizers and strict quality
ASan + UBSan (`-fsanitize=address,undefined -fno-sanitize-recover=all`): 0 warnings, 45/45. TSan (`-fsanitize=thread`, ASLR disabled with `setarch -R`, the documented environment workaround for this kernel; the suite is single-threaded): 45/45. Strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`: builds, 45/45. GCC `-fanalyzer` over every `src/*.cpp` and the platform, context, contract and integration tests: no diagnostics.

### AC-04 Coverage
`make coverage` from the fresh clone: **99% (595/601 lines)**, equal to the R0.5 result. `component_context.hpp`, `context.hpp` fully covered; `requirements.hpp` 50/51 (the one uncovered line is the exception-unwind closing brace of `evaluate()`, of the same kind as `component_registry.cpp:55`); `runtime_manager.cpp` 125/126, `dependency_graph.cpp` 84/84. The five baseline uncovered lines are unchanged and justified: `runtime_manager.cpp:98` and `lifecycle.cpp:33` (unreachable returns after exhaustive `switch`es), `component_registry.cpp:55` (exception-unwind brace), `configuration.cpp:40,46` (defensive branches unreachable because `set()` rejects empty names). No coverage exclusion was introduced.

### AC-05 Traceability
`make check`: header-check passed; `make traceability-check`: **80 requirements, 79 traced (CORE-ERR-003 is the reserved/exempt one), 0 errors, 0 warnings**. `CORE-CTX-001` to `CORE-CTX-007` are defined exactly once in `REQUIREMENTS.md` and traced; no earlier requirement was renumbered.

### AC-06 Freeze verification
See the production-change audit above: no production change after the freeze.

### AC-07 Install consumer and version validation
`kritva_core_install_consumer` passed in the fresh clone: it installs to a scratch prefix, checks the layout, then configures, builds and runs an external consumer against the installed package only. The consumer exercises the R0.3 runtime, the R0.4 platform boundary, the R0.5 `PlatformContext` and requirements and, new in this release, the R0.6 `ComponentContext` through the installed headers and library: an unbound context (no identity, unattached platform), a context built for a component (identity pointer, id, platform view), `require_scheduler()` failing with `UNSUPPORTED` whose **source is the component** (and sourceless for an unbound context), `supports()` and `has_capability()`, `attribute()` replacing only the source (code, severity and message unchanged; an unbound context returns the error unchanged), and `check_required()`/`evaluate()` with the same attribution rule. Version validation: the installed `kritva_coreConfigVersion.cmake` reports `PACKAGE_VERSION "0.6.0"`; the consumer checks `kritva_core_VERSION` against the project version (derived, not hard-coded); `find_package(kritva_core 0.6)` accepts `0.6` and `0.6.0` and refuses `0.7`, `0.5` and `9.0`. A wrong expectation (`0.6.1`, `0.5.0`) makes the install test fail (checked manually, both exit code 1).

### AC-08 Dependency / prohibited-header checks
`grep` over `include/` and `src/`: no `<thread> <mutex> <shared_mutex> <condition_variable> <future> <atomic> <semaphore> <iostream> <fstream> <cstdio>`, POSIX, Linux or Windows include; no ROS 2, DDS, EtherCAT, SOEM, FreeRTOS, Zephyr or VxWorks identifier except the sentence in a comment of `platform/boundary.hpp` naming them as forbidden. The traceability audit enforces the threading/logging scan (CORE-RT-010), CORE-GEN-003, the platform scan (CORE-PLAT-004) and the test-only isolation checks (CORE-PLAT-016); the CTest `kritva_core_test_isolation` compiles every production translation unit and the umbrella header with only `include/` on the include path. `CMakeLists.txt` has no `find_package` of third-party packages, `FetchContent` or `ExternalProject`; no `install()` rule installs test support. The test harness (`tests/runtime/`, `tests/platform/`) is never included by production code. `git diff --check` clean.

### AC-09 Documentation consistency
`REQUIREMENTS.md` (title R0.6, `CORE-CTX-001..007`), `API.md` (title R0.6; sections 25-38), `ARCHITECTURE.md`, `README.md` (`find_package(kritva_core 0.6)`), `TESTING.md` (component context harness and integration tests), root `CHANGELOG.md` (0.6.0 section with the known follow-ups), `VERSION` and the CMake project version (both 0.6.0, enforced by the audit) and the R06 planning documents agree with the frozen implementation. The remaining "R0.5" strings are historical.

### AC-10 Clean candidate
Working tree clean after every commit; the candidate commit is `b473e5d`; this evidence is committed as `test(core): complete R0.6 validation`. Candidate suitable for the R06 Release Gate.

### Findings / follow-ups (none block)
- `make lint` and `make format-check` remain deferred stubs (static analysis is strict warnings and GCC `-fanalyzer`), as in R0.3 to R0.5.
- 32-bit scheduler affinity mask (documented, frozen); conformance suite level-2 mutation strictness gap (documented, accepted at R04-006); a context (like `PlatformContext`) used after what it refers to is destroyed is documented undefined behavior (non-owning by design).
- The unused `<chrono>` include in `types/duration.hpp` and the stale root `implementation.md` remain (cosmetic, unchanged).
- The release commit and tag `kritva-core-r0.6` are decided at the R06 Release Gate; nothing is tagged or pushed.

## 12. Reviewer decision

**Reviewer only:**

- PASS
- CHANGES REQUIRED
- BLOCKED

Reviewer: ____________________  
Date: ____________________
