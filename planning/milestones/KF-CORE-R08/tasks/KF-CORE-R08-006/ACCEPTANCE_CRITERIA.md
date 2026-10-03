# KF-CORE-R08-006 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R08-006 |
| Status | PLANNED |
| Primary commit | `test(core): validate configuration boundary and regression` |
| Reviewer | ChatGPT — independent acceptance gate |
| Estimated effort | 2–3 ED |
| Dependency | R08 Integration Freeze PASS/HONORED |

## Objective

Validate the frozen R08 configuration boundary and preserve the repository-level quality baseline after Integration Freeze.

## Proposed Requirements Traceability

CORE-CFG-013

The implementing task must update authoritative root `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- R08 Integration Freeze is PASS/HONORED before validation begins.
- Fresh-clone Debug and Release builds pass.
- Full regression passes.
- Public-header self-containment passes.
- Traceability reports zero errors.
- Prohibited dependency/isolation audit remains clean.
- Install-consumer validation covers the accepted R08 configuration contract.
- Coverage policy is satisfied.
- Production diff from the Integration Freeze contains no unapproved semantic/API changes.
- `git diff --check` is clean and the working tree is clean.
- Deferred known issues are not silently pulled into R08 scope.

## New Tests

- Add only boundary/regression tests required to prove the freeze and install-consumer contract.
- No new public API may be invented by this validation task.

## Regression Tests

- Complete R0.7 baseline plus all accepted R08 tests; the final count may increase as tests are added.

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

## Coverage / Quality

- Newly added executable lines must be covered.
- No unexplained coverage exclusion.
- Overall R08 coverage remains at least 98% and has no unexplained regression greater than 1 percentage point from the R0.7 98.9% baseline.
- Required sanitizer and strict-analysis evidence is supplied at milestone validation.

## Expected Files Changed

Only files directly required for this task, its tests, and traceability/documentation reconciliation. No unrelated files.

## Git Commit

```text
test(core): validate configuration boundary and regression
```

## Evidence Required from Implementor

- `git status` before and after implementation
- `git diff --check`
- build and test output
- focused test evidence
- mutation evidence for contract-sensitive behavior
- diff/stat summary
- commit SHA
- explicit mapping from every acceptance criterion to objective evidence

## Implementor Evidence

Primary commit: `94fad8e` `test(core): validate configuration boundary and regression`. **Test-only:** `git diff bdb4b93 HEAD -- include src` is empty (production freeze point `bdb4b93`; Integration Freeze PASS / HONORED, evidence `e1051a0`, recorded at `c358e76`). Changes: `tests/unit/configuration_boundary_test.cpp` (new), `tests/install/consumer/main.cpp` (configuration exercise, return codes 73–83), `CMakeLists.txt`, `REQUIREMENTS.md` (`CORE-CFG-013` + row), `TESTING.md`. Requirement note: `CORE-CFG-013` ("validate the R0.8 foundation, regression boundary and release candidate") is defined once here with its full text and covers both R08-006 and R08-007, as the proposal assigns it to both; R08-007 adds no further requirement.

- **Frozen-boundary snapshot (`kritva_core_configuration_boundary`, compile-time):** pins every signature, member type, enumerator value and shape the configuration contract governs — `Configuration` (validate/set/get/contains/size, copy/move), `Parameter` and the exact `ParameterValue` variant, `ConfigurationVersion` == `Version` (three `uint32_t`), the nine `Component` members and its non-copyable/non-movable abstract shape, `RuntimeManager`'s configure/lifecycle/state/fault/statistics signatures, `ComponentContext` (two pointers, immutable, no configuration construction), and the values of `ErrorCode` (12), `ErrorSeverity`, `LifecycleState` (8), `StatusCode`, `HealthState`; plus exhaustive no-default switches so an **added enumerator fails the strict (`-Wall -Werror`) build**, and a by-name member detector (checked against itself) that finds `reconfigure`, `set_parameter`, `get_parameter`, `configuration`, `apply`, `update_configuration` and `revision`/`version`/`generation`/`history`/`transaction_id`/`schema` on `Component`, `RuntimeManager`, `ComponentContext` and `Configuration` for any signature and overload (final classes probed directly).
- **Sensitivity of the snapshot (mutants of the production headers, compile-failure = detected):** 20 mutants run — configure by value; configure `noexcept`; validate non-const; get returning by value; set by const reference; an extra `ParameterValue` alternative; Runtime configure by value; an inserted and an appended `ErrorCode`; an added `LifecycleState`; and added `reconfigure`, `set_parameter`, `revision`, overloaded `history` and `configuration()` members on Component, Configuration, RuntimeManager and ComponentContext. All 20 detected; the first run left 6 survivors (a name-based member the original call-style detector could not see, and two appended enumerators that only fail under `-Wswitch -Werror`); they were closed by the by-name detector and by verifying the appended enumerators under the strict flags (both now detected).
- **Install consumer (R0.8 contract through the installed package):** a component that validates structurally, rejects semantically with `CONFIGURATION_ERROR` and applies all-or-nothing; atomic structural rejection at `set()`; a Runtime forwarding the caller's configuration in dependency order with the state unchanged; `INVALID_STATE` when READY; stop at the first failure from STOPPED with no fault, no rollback and the earlier component keeping its accepted value; `ConfigurationVersion` is `Version`. The install test passes.
- **Fresh-clone validation of this commit** (clean tree, `git status` empty): Debug and Release 0 warnings, ctest **57/57** in both; `make check`: header-check passed, traceability **100 requirements, 99 traced, 0 errors, 0 warnings**; dependency scan clean (word-bounded, no thread/queue/OS/RTOS/ROS2/DDS/EtherCAT, no `find_package`/`FetchContent`); `git diff --check` clean; production diff since the freeze empty and, against `kritva-core-r0.7`, exactly the two reviewed configuration headers (+127, +34 lines). The same commit passes in the working tree under ASan+UBSan, strict `-Werror`, TSan (ASLR off), GCC `-fanalyzer` and coverage 618/625 (unchanged).
- Deferred known issues were not pulled into scope (32-bit affinity mask, conformance level-2 gap, lint/format stubs, lifetime UB documentation, cosmetic `<chrono>` include and root `implementation.md`).
- Out of scope confirmed: no production change; version metadata (`VERSION`, CMake, README, CHANGELOG) is R08-007.

## Reviewer Decision

`PASS / CHANGES REQUIRED / BLOCKED` — to be completed by ChatGPT only.
