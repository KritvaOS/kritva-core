# KF-CORE-R02-008 — Acceptance Criteria

## 1. Task Information

- Task: Full R0.2 Validation
- Milestone: `KF-CORE-R02`
- Dependency: `KF-CORE-R02-007`
- Expected commit:
  `test(core): complete R0.2 validation`

## 2. Objective

Execute the final objective acceptance gate for KF-CORE-R02. This task is validation, not a feature-development task.

## 3. Requirement Traceability

The implementer must identify the authoritative requirement IDs affected by this task.

| Requirement ID | Header/API | Implementation | Test | Evidence |
|---|---|---|---|---|
| All 49 requirement IDs | `REQUIREMENTS.md` traceability tables | unchanged by this task | `make traceability-check`; all 17 CTest tests | validated commit `ea619f8`; 49 / 48 traced / 1 reserved; 0 errors, 0 warnings |

**Acceptance:** No requirement referenced by the implementation may remain undefined.

## 4. Functional Acceptance

- [x] AC-001: Start from a clean working tree.
- [x] AC-002: Perform a clean configure/build.
- [x] AC-003: Run all registered CTest tests.
- [x] AC-004: Run the established Core regression suite.
- [x] AC-005: Review test count and failures.
- [x] AC-006: Review coverage and explain material gaps.
- [x] AC-007: Run configured sanitizers/static analysis where available.
- [x] AC-008: Review public API for accidental platform dependencies.
- [x] AC-009: Review REQUIREMENTS.md traceability.
- [x] AC-010: Review documentation and changelog.
- [x] AC-011: Review Git history and task commits.
- [x] AC-012: Record exact evidence in the task result.
- [x] AC-013: Do not introduce unrelated feature work during validation.

## 5. Test Acceptance

- [x] TEST-001: All CTest tests
- [x] TEST-002: All 11 known regression tests
- [x] TEST-003: Coverage
- [x] TEST-004: Sanitizer/static analysis where configured
- [x] TEST-005: Clean rebuild from a fresh build directory

## 6. Regression Acceptance

- [x] All known regression tests pass.
- [x] `ctest --test-dir build --output-on-failure` passes.
- [x] No previously passing test is removed or disabled without explicit review.

## 7. Build Acceptance

- [x] Clean configure succeeds.
- [x] Clean build succeeds.
- [x] No new compiler errors.
- [x] No new unexplained compiler warnings.

## 8. Coverage Acceptance

- [x] Coverage is generated/reviewed if configured.
- [x] New logic has appropriate test coverage.
- [x] Any material uncovered branch is documented.

## 9. Sanitizer / Static Analysis Acceptance

- [x] Required configured sanitizer runs pass.
- [x] Required configured static analysis passes.
- [x] Any existing unrelated finding is explicitly identified rather than hidden.

## 10. Scope Acceptance

- [x] No Runtime Manager implementation added.
- [x] No platform-specific implementation added to Core.
- [x] No unrelated refactoring.
- [x] Public API changes are limited to this task's contract needs.

## 11. Documentation Acceptance

- [x] Relevant API/requirements documentation updated.
- [x] Requirement IDs are traceable.
- [x] No documentation contradicts the implementation.

## 12. Git Acceptance

- [x] Working tree was clean before implementation.
- [x] Diff reviewed.
- [x] Commit contains only this task's logical changes.
- [x] Exact commit message used:

```text
test(core): complete R0.2 validation
```

- [x] Commit hash recorded: `1472b79` (validation record); build fix `ea619f8`.

## 12a. Validation Evidence (Claude)

This task changes no source. Validated state: HEAD `ea619f8` (R02-001..007 accepted; one validation-found build fix, see F1). Validation was run from a **fresh `git clone`** of the repository (so only committed content is tested) plus the working tree.

### V1. Clean tree and clean build
- `git status --short` empty before and after.
- Fresh clone: `rm -rf build && cmake -S . -B build && cmake --build build -j$(nproc)` — exit 0, 0 warnings.
- GCC 11.4.0, CMake 3.30.2, Linux 6.8.

### V2. Tests
- `ctest --test-dir build --output-on-failure` — **17/17 passed** (16 R0.1/R0.2 regression tests + `kritva_core_foundation_contract` added by R02-007). No failures, none disabled or removed (`ctest -N`: 17 tests).
- The 11 named regression tests, each run by exact name: lifecycle, contract, status, health, error, result, capability, configuration, event, statistics, types — all Passed. (CTest names the last one `kritva_core_types`; the acceptance list says `kritva_core_types_test`, which is the executable name. Same test.)
- Determinism: `ctest --repeat until-fail:25 --schedule-random` — 25 randomized-order repetitions, all passed.
- Release build (`-DCMAKE_BUILD_TYPE=Release`): 17/17 passed, 0 warnings (tests keep asserts active via `-UNDEBUG`).

### V3. Sanitizers and static checks
- ASan + UBSan (`-fsanitize=address,undefined -fno-sanitize-recover=all`): 0 warnings, **17/17 passed**.
- TSan (`-fsanitize=thread`): first run failed every test with `FATAL: ThreadSanitizer: unexpected memory mapping` — an environment issue (this kernel's ASLR entropy versus GCC 11 TSan), not a code failure. Re-run with ASLR disabled (`setarch $(uname -m) -R ctest`): **17/17 passed**. The suite is single-threaded, so TSan has limited value here.
- Strict warnings: `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror` build of library and all tests: succeeds.
- GCC `-fanalyzer -Wall -Wextra` over every `src/*.cpp`: no diagnostics.
- `clang-tidy`, `cppcheck`, `clang++`, `valgrind`: **not installed**; `make lint` and `make format-check` are TODO stubs and were not counted as evidence. Static analysis therefore consists only of the strict-warning and `-fanalyzer` checks above. `clang-format` is installed but there is no `.clang-format` configuration to check against.
- `make check` (header-check, traceability-check, format-check, lint): passed (last two are stubs).
- `git diff --check`: clean.

### V4. Coverage
- `make coverage`: **98% — 160/163 lines.** Header-only contract types are 100%.
- Material gaps (3 lines, unchanged since R0.1, both defensive): `src/lifecycle.cpp:33` (`return false` after an exhaustive `switch`; unreachable for valid enum values) and `src/configuration.cpp:40,46` (failure branch in `validate()`; unreachable because `set()` rejects empty names, so no public sequence can produce an invalid stored parameter).

### V5. Public API platform-dependency review
- Public headers and sources include only the C++ standard library (`<cstdint>`, `<string>`, `<utility>`, `<unordered_map>`, `<vector>`, `<variant>`, `<optional>`, `<string_view>`, `<functional>`, `<compare>`, `<cassert>`, `<cstddef>`, `<chrono>`). No OS, threading, I/O, ROS2/DDS, EtherCAT, vendor or hardware headers (also enforced by `make traceability-check` for CORE-GEN-003: no `find_package`/`FetchContent`/`ExternalProject`/`add_subdirectory`).
- `platform/` headers are abstract contracts only (`IScheduler`, `IWatchdog`, alias `IClock`); no implementation.
- Behavior change in `include/` and `src/` across all of R0.2, from the R0.1 baseline `245d91e`: exactly one — the debug-build `assert(error.code != ErrorCode::NONE)` in `Result::failure()` (R02-001, accepted). Everything else is documentation comments and doc-only reformatting of `StatusCode`.

### V6. Requirements traceability
- `make traceability-check`: 49 requirements defined, 48 traced, 1 reserved/exempt (`CORE-ERR-003`), 0 errors, 0 warnings. Mutation-checked in R02-006.

### V7. Git history review
- R0.2 history from `e6acfb6`: each task has one primary implementation commit with the exact expected message (`df44d38`, `d6d939d`, `d7d29cb`, `0773857`+`0cdffdb`, `6ed8762`+`e1e6cfb`, `00899f9`, `bf2144a`) followed by evidence and acceptance documentation commits. Review follow-ups (004, 005) are separate commits; no commit was amended or rewritten. Two commits outside the task list: `c1ed767` (reserved-requirement note requested in R02-006 review) and `ea619f8` (F1 below).
- No `Co-Authored-By` trailers, per the stated preference.
- Nothing has been pushed: the branch is 29 commits ahead of `origin/main`.

### Findings (none block the technical gate; F2 and F3 need a decision before tagging)
- **F1 — fixed, separate commit `ea619f8` `fix(build): create coverage output directory`.** `make coverage` failed in a fresh clone (`gcovr` cannot create the gitignored `coverage/` directory; it had only ever existed in a long-lived working tree). One-line Makefile fix (`@mkdir -p coverage`), verified in the fresh clone.
- **F2 — decision needed: root `CHANGELOG.md` and `VERSION` are stale.** `CHANGELOG.md` has a single `0.2.0-proposed` entry from the R0.1 skeleton and says nothing about R0.2 hardening; `VERSION` is `0.2.0-proposed`. Only `planning/CHANGELOG.md` records R0.2. The milestone gate requires "Changelog updated". I did not change either file: choosing the release version string and wording is a release decision.
- **F3 — decision needed: no git tags exist locally**, including `kritva-core-r0.1`, which `MILESTONE_STATUS.md` cites as the R0.1 release. Please confirm whether it exists on `origin` (not checked: no network/SSH access used). I did not create `kritva-core-r0.2`; per the workflow it is created only after R02-008 and the milestone are accepted.
- **F4 — 29 unpushed local commits** on `main`. Not pushed; pushing needs your decision.
- **F5 — static analysis gap:** see V3. Recommend installing and configuring clang-tidy/cppcheck and wiring `make lint` in a later task.
- **F6 — cosmetic:** `types/duration.hpp` includes `<chrono>` but does not use it (standard header, no platform impact). Not changed (unrelated refactoring).
- **F7 — stale file:** repository-root `implementation.md` is an R0.1/R0.2 analysis document whose open questions (for example the platform-alias decision) have since been resolved. Not changed.
- **F8 — environment:** the TSan note in V3.

### Milestone gate status (for the reviewer)
- Tasks accepted: 7/8 (008 pending this review).
- New tests pass / full regression passes / coverage reviewed / sanitizers reviewed / API reviewed / traceability complete: evidence above.
- Documentation updated: yes for API/REQUIREMENTS/ARCHITECTURE/TESTING; root CHANGELOG/VERSION open (F2).
- Git history reviewed: V7. Release tag: not created (F3).

## 13. Evidence Required From Codex/Claude

Provide the following in the implementation response:

1. Summary of changes
2. Exact files changed
3. Requirement IDs addressed
4. New tests added/modified
5. Build command and output summary
6. Task-specific test command/output summary
7. Full regression command/output summary
8. Coverage result
9. Sanitizer/static-analysis result
10. Git commit hash
11. Known limitations
12. Any follow-up recommendation

## 14. Independent Reviewer Decision

Reviewer: ChatGPT

- [ ] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Review notes:

TBD

## 15. Acceptance Rule

The task cannot be marked ACCEPTED solely because the code compiles. All applicable functional, test, regression, quality, scope, documentation, and Git criteria must have evidence.
