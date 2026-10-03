# KF-CORE-R07-007 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R07-007 |
| Status | PLANNED |
| Primary commit | `test(core): complete R0.7 operational validation` |
| Reviewer | ChatGPT — independent acceptance gate |

## Objective

Perform the complete final validation of R0.7 after Integration Freeze and establish release-candidate evidence.

## Scope

Fresh-clone validation, Debug/Release, full CTest, sanitizers, strict warnings, analyzer, coverage, traceability, self-containment, install consumer and isolation/dependency audits.

## Proposed Requirements Traceability

R0.7 requirement domain: `CORE-OPS-*`. The implementing task must update authoritative `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- Integration Freeze is PASS/HONORED.
- Fresh-clone Debug and Release builds pass.
- Full regression and required R0.7 tests pass.
- ASan/UBSan/TSan pass where configured.
- Strict warnings and GCC analyzer pass.
- Coverage and traceability pass.
- Install consumer and production-isolation audits pass.
- No production semantic change remains after freeze.
- Release candidate evidence is complete and reproducible.

## New Tests

- Add focused unit/contract tests for each accepted semantic rule introduced by this task.
- Add negative/failure tests where applicable.
- Add mutation testing for contract-sensitive behavior.

## Regression Tests

- Complete existing CTest suite remains green.
- Existing R0.2–R0.6 contracts remain unchanged unless explicitly approved.

## Build

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
```

## Test

```bash
ctest --test-dir build --output-on-failure
```

Where applicable:

```bash
make check
make traceability-check
```

## Quality

- No unexplained regression from the accepted baseline.
- Public headers self-contained.
- Strict warnings/static analysis clean for affected code.
- Required sanitizer/coverage evidence supplied at milestone validation.

## Expected Files Changed

Implementation and tests only for this task, plus directly required traceability/documentation updates. No unrelated files.

## Git Commit

```text
test(core): complete R0.7 operational validation
```

## Evidence Required from Implementor

- `git status`
- `git diff --check`
- build output
- test output
- focused test evidence
- mutation evidence where required
- diff/stat summary
- commit SHA
- explicit mapping from each acceptance criterion to objective evidence

## Implementor Evidence

Validated release candidate: HEAD `d83e1ba` (`build(core): prepare 0.7.0 release metadata`), validated from a **fresh `git clone`** of the repository (clean tree, `git status` empty before building). R07 Integration Freeze PASS / HONORED (recorded at `2cdaad9`, evidence `142a32e`).

### Candidate history since the Integration Freeze (production freeze point `16654e9`)
| Commit | Purpose | Production code? |
|---|---|---|
| `0b1bd1d` `test(core): add component operational reference harness` | R07-005 (accepted) | no (tests only) |
| `3f524cd` `test(core): add component operational integration tests` | R07-006 (accepted) | no (tests only) |
| `d83e1ba` `build(core): prepare 0.7.0 release metadata` | `VERSION` and CMake project version 0.7.0; README `find_package(kritva_core 0.7)`; REQUIREMENTS/API titles R0.7; root CHANGELOG 0.7.0 section; defines `CORE-OPS-010` and its traceability row; install consumer default request 0.7 plus an operational exercise | no |

**Production-change audit:** `git diff 16654e9 HEAD -- include src` is **empty**; the three R0.7 headers are byte-identical to the API Review baseline `6b1296f` (checked per file in the fresh clone); against `kritva-core-r0.6`, `src/` is unchanged and the only production header differences are the reviewed R07 set (`component_observation.hpp`, `component_statistics.hpp`, `component_events.hpp` new, three includes in `core.hpp`).

### AC-01 Clean build
Fresh clone: Debug and Release, 0 warnings each (GCC 11.4.0, Linux 6.8). Public-header self-containment (`kritva_core_header_checks`, one TU per public header) builds in every configuration.

### AC-02 Complete regression
`ctest`: **51/51 passed** in Debug and Release (unit, contract, conformance, platform, component context, operational, integration, isolation and install tests); 10 randomized-order repetitions all pass.

### AC-03 Sanitizers and strict quality
ASan + UBSan: 0 warnings, 51/51. TSan (ASLR off via `setarch -R`): 51/51. Strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`: 0 warnings, 51/51. GCC `-fanalyzer` is clean over every `src/` file and over the contract, platform, context, operational and integration tests.

### AC-04 Coverage
`make coverage` from the fresh clone: **618/625 lines (98.9%)**, versus 595/601 (99%) at R0.6. The 7 uncovered lines are the established baseline (five) plus the exception-unwind closing braces of `evaluate()` and `observe()`; `component_statistics.hpp` and `component_events.hpp` are fully covered; no coverage exclusions were added.

### AC-05 Traceability
`make check`: header-check passed; **90 requirements, 89 traced (CORE-ERR-003 reserved), 0 errors, 0 warnings**; `CORE-OPS-001` to `CORE-OPS-010` are defined once (007 Runtime boundary, 008 integrator policy, 009 harness, 010 release validation); earlier IDs are not renumbered. The three new public headers are in the traceability table.

### AC-06 Freeze verification
See the production-change audit: no production change after the freeze.

### AC-07 Install consumer and package version
The install test installs to a scratch prefix and builds and runs an external consumer against the installed package only, exercising the R0.3 runtime, the R0.4 boundary, the R0.5 `PlatformContext`/requirements, the R0.6 `ComponentContext` and, new, R0.7: `observe()` of a component with no provider (statistics `nullopt`) and with one (engaged, values exact), `Status`/`Health` independent of lifecycle, a `ComponentEventReporter` that stamps a zero source, rejects a mismatching source with `INVALID_ARGUMENT` without calling the sink, returns the sink's failure unchanged (foreign source and code), and an unbound reporter giving `INVALID_STATE` (return codes 63–72). The installed config reports `PACKAGE_VERSION 0.7.0` and `find_package` accepts 0.7 and 0.7.0; wrong expectations (0.7.1 and 0.6.0) make the install test fail (checked manually, both exit 1; the expected 0.7.0 passes).

### AC-08 Isolation and dependency audits
`kritva_core_test_isolation` and the audit (`CORE-PLAT-016`) pass: no production file includes test-only support, every production TU and the umbrella header compile with only `include/` on the path, and no `install()` rule ships test support. Dependency scan: no `<thread>`, `<mutex>`, `<atomic>`, `<future>`, `<condition_variable>`, `<queue>`, `<deque>`, `<iostream>`, `<fstream>` or `<cstdio>` in `include/` or `src/`, no POSIX/Linux/Windows include and no ROS2/DDS/EtherCAT/RTOS identifier (the only hit is a comment in `boundary.hpp` naming them as forbidden); no `find_package`/`FetchContent`/`ExternalProject`; `git diff --check` clean.

### AC-09 Documentation
REQUIREMENTS (R0.7, `CORE-OPS-001..010`), API sections 39–43, ARCHITECTURE, README, TESTING, root CHANGELOG, `VERSION` and CMake (both 0.7.0, enforced by the audit) agree with the frozen implementation.

### AC-10 Clean candidate
Working tree clean after every commit; the candidate commit is `d83e1ba`; this evidence is committed as `test(core): complete R0.7 operational validation`. Candidate suitable for the R07 Release Gate.

### Findings / follow-ups (none block)
- `make lint` and `make format-check` remain deferred stubs (static analysis is strict warnings and GCC `-fanalyzer`), as in R0.3 to R0.6.
- 32-bit scheduler affinity mask (documented, frozen); conformance suite level-2 mutation strictness gap (documented, accepted at R04-006); a context, reporter or statistics provider used after what it refers to is destroyed is documented undefined behavior (non-owning by design).
- The unused `<chrono>` include in `types/duration.hpp` and the stale root `implementation.md` remain (cosmetic, unchanged).
- The release commit and tag `kritva-core-r0.7` are decided at the R07 Release Gate; nothing is tagged or pushed.

## Reviewer Decision

- [ ] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Reviewer decision is independent of implementor checkboxes.
