# KF-CORE-R02-007 — Acceptance Criteria

## 1. Task Information

- Task: Foundation Contract Tests
- Milestone: `KF-CORE-R02`
- Dependency: `KF-CORE-R02-006`
- Expected commit:
  `test(core): strengthen foundation contract coverage`

## 2. Objective

Strengthen automated tests so the R0.2 foundation contracts are objectively verified before Runtime Foundation work begins.

## 3. Requirement Traceability

The implementer must identify the authoritative requirement IDs affected by this task.

| Requirement ID | Header/API | Implementation | Test | Evidence |
|---|---|---|---|---|
| CORE-LIF-001..003, CORE-STA-001, CORE-HEA-001/002, CORE-ERR-001/002/004, CORE-CAP-001..003, CORE-CFG-001..003, CORE-EVT-001..004, CORE-STS-001..003, CORE-TYP-001..004 | public headers via `core.hpp` | existing implementation (unchanged) | `tests/contract/foundation_contract_test.cpp` (`kritva_core_foundation_contract`) | commit `bf2144a`; 17/17 ctest |

**Acceptance:** No requirement referenced by the implementation may remain undefined.

## 4. Functional Acceptance

- [x] AC-001: Tests must distinguish new/changed tests from existing regression tests.
- [x] AC-002: Cover Lifecycle, Status, Health, Error, Result, Capability, Configuration, Event, Statistics, and Types.
- [x] AC-003: Cover normal and negative/boundary behavior where the API defines it.
- [x] AC-004: Tests must not depend on wall-clock timing, network, hardware, or vendor SDKs.
- [x] AC-005: Tests should be deterministic and repeatable.
- [x] AC-006: Do not add tests for unimplemented Runtime Manager behavior.

## 5. Test Acceptance

- [x] TEST-001: Lifecycle transition validity/invalidity
- [x] TEST-002: Status contract
- [x] TEST-003: Health contract
- [x] TEST-004: Error contract
- [x] TEST-005: Result contract
- [x] TEST-006: Capability set behavior
- [x] TEST-007: Configuration validation/get/set
- [x] TEST-008: Event envelope
- [x] TEST-009: Statistics
- [x] TEST-010: Types

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
test(core): strengthen foundation contract coverage
```

- [x] Commit hash recorded: `bf2144a`.

## 12a. Implementation Evidence (Claude)

- Commit: `bf2144a` `test(core): strengthen foundation contract coverage` (R02-006 accepted at `63cb14e`; reserved-requirement note `c1ed767`).
- **New vs existing tests:** one NEW CTest target, `kritva_core_foundation_contract` (`tests/contract/foundation_contract_test.cpp`). The 16 pre-existing tests are unmodified in this task and form the regression set. Suite is now 17 tests.
- Files changed: `tests/contract/foundation_contract_test.cpp` (new), `CMakeLists.txt` (registration), `ARCHITECTURE.md`, `REQUIREMENTS.md` (new test added to traceability rows), `API.md` (section 11 wording). No production code changed.
- What the new test does (public `core.hpp` only; deterministic; no timing, network, hardware or threads):
  1. **Lifecycle:** all 64 (from,to) pairs, each starting from a Lifecycle driven into `from` via documented transitions, checked against an independent expected table written in the test. Valid: state becomes target. Invalid: `INVALID_STATE` Error with `ERROR` severity and non-empty message, and state unchanged. Also: 15 valid edges, no self-transitions, `is_running()` true only in RUNNING, FAULT recovery path, rejected transition does not poison later ones, STOPPED can re-initialize, `noexcept` guarantees.
  2. **Status/Health:** defaults, independent fields, explicit construction, distinct states, UNKNOWN is zero, noexcept traits.
  3. **Error/Result:** NONE is zero, all 12 codes distinct, severity ordering, Error carried unchanged through Result, real failures from `Lifecycle` and `Configuration` are well-formed.
  4. **CapabilitySet:** empty behavior, replace-by-identity keeps size and position and leaves others untouched, insertion order.
  5. **Configuration:** all four value types round-trip, replace changes the type, empty name rejected leaving state unchanged, case-sensitive names, validate on empty and populated.
  6. **Event:** defaults, independent event/source/correlation ids, timestamp domain survives, distinct event types.
  7. **Statistics, Types:** Counter wrap via the umbrella; Id validity/ordering/hash; `Version::to_string` exact format; Duration conversions incl. negative; Metadata replace, empty value present, case sensitivity.
- **Mutation evidence that the new test can fail** (each temporary edit made the test abort, then was reverted with `git checkout`; tree verified clean): allowing READY->STOPPING; capability replace turned into append; configuration accepting an empty name; `Version::to_string` separator change; Metadata `set` no longer replacing. Baseline passed before and after.
- **Documentation defect found and fixed:** `API.md` section 11 stated the lifecycle transition table "is documented in `ARCHITECTURE.md`", but it was not there. The table now exists in `ARCHITECTURE.md` ("Lifecycle transitions") and matches the implementation. Reviewer: this documents existing behavior; no behavior was changed.
- Not tested, by design: Runtime Manager behavior (out of scope); `Result` invalid-access traps (covered in `result_test.cpp`); scheduler/clock contracts (covered in `platform_test.cpp`, `time_test.cpp`).
- Build: `rm -rf build && cmake -S . -B build && cmake --build build -j$(nproc)` — 0 warnings.
- Tests: `ctest --test-dir build --output-on-failure` — 17/17 passed. Also a Release build (`-DCMAKE_BUILD_TYPE=Release`) — 17/17 passed (tests keep asserts active via `-UNDEBUG`).
- Sanitizers: separate Debug build `-fsanitize=address,undefined` — 17/17 passed.
- Coverage: `make coverage` — 98% (160/163 lines, was 158/161). Material uncovered lines, unchanged by this task: `src/lifecycle.cpp:33` (the unreachable `return false` after an exhaustive `switch`) and `src/configuration.cpp:40,46` (the defensive failure branch inside `validate()`, unreachable because `set()` already rejects empty names).
- `make check` passed (header-check, traceability-check 49/48/0/0; format-check and lint are TODO stubs); `git diff --check` clean.
- Known limitations: expected tables are maintained by hand in the test, so an intentional contract change must update the test (that is the point); the test is single-threaded and does not examine thread-safety claims.

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
