# KF-CORE-R02-006 — Acceptance Criteria

## 1. Task Information

- Task: Requirements/API Traceability
- Milestone: `KF-CORE-R02`
- Dependency: `KF-CORE-R02-005`
- Expected commit:
  `docs(core): close requirements API traceability`

## 2. Objective

Create complete traceability from authoritative requirements to public headers, implementation, and tests.

## 3. Requirement Traceability

The implementer must identify the authoritative requirement IDs affected by this task.

| Requirement ID | Header/API | Implementation | Test | Evidence |
|---|---|---|---|---|
| All 49 defined IDs (CORE-GEN/API/TYP/TIME/LIF/STA/HEA/STS/ERR/EVT/CAP/CFG/RT/MSG/PLAT + repository IDs) | `REQUIREMENTS.md` tables | `scripts/audit/check_traceability.py` | `make traceability-check` | commit `00899f9`; 0 errors, 0 warnings |

**Acceptance:** No requirement referenced by the implementation may remain undefined.

## 4. Functional Acceptance

- [x] AC-001: Every requirement ID referenced by public headers, tests, or documentation must exist in REQUIREMENTS.md.
- [x] AC-002: Every public Core contract must map to an authoritative requirement ID.
- [x] AC-003: Traceability must cover Types, Status, Health, Error/Result, Event, Capability, Configuration, Lifecycle, Statistics, Time, Messaging, Runtime contracts, and Platform abstractions present in R0.2.
- [x] AC-004: Where IDs such as CORE-TYP-001, CORE-STA-001, CORE-HEA-001, CORE-ERR-001, CORE-EVT-001, CORE-CAP-001, and CORE-CFG-001 are used, their definitions must be authoritative.
- [x] AC-005: Traceability format must be consistent: Requirement → Public Header → Implementation → Test.
- [x] AC-006: Do not invent implementation behavior merely to fill a traceability table.

## 5. Test Acceptance

- [x] TEST-001: Traceability audit script/check if practical
- [x] TEST-002: Manual verification that each requirement points to a real header/test
- [x] TEST-003: Verify no stale requirement IDs remain

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
docs(core): close requirements API traceability
```

- [x] Commit hash recorded: `00899f9`.

## 12a. Implementation Evidence (Claude)

- Commit: `00899f9` `docs(core): close requirements API traceability` (R02-005 accepted at `b95548a`).
- Files changed: `REQUIREMENTS.md`, `API.md` (section 18), `TESTING.md`, `Makefile` (new `traceability-check`, wired into `make check`; header tag `BUILD-001` -> `CORE-BUILD-001`), `scripts/audit/check_traceability.py` (new), and comment-only `Requirements:` tag corrections in `tests/unit/lifecycle_test.cpp` (+CORE-LIF-001), `tests/unit/event_test.cpp` (+CORE-EVT-004), `tests/contract/core_contract_test.cpp` (+CORE-API-001). No production code or behavior changed.
- Audit findings before the fix (from the script): 7 requirement IDs referenced but never defined in `REQUIREMENTS.md` (`CORE-REQ-002`, `CORE-BUILD-001`, `CORE-TEST-001`, `CORE-DOC-001`, `CORE-ARCH-001`, `CORE-ARCH-003`, `CORE-DEV-001`); `CORE-TIME-001` defined twice; `CORE-GEN-001/002/003/005` defined but not traced; three tests that exercise a requirement without carrying its tag.
- Resolution:
  1. The seven IDs are defined in a new "Repository, build and documentation" section. **Reviewer attention:** their wording describes the existing artifact that already carries the ID (README, CMake/Make, CI workflow, boundaries/audit-status/work-package docs, the requirements document); they are labeled process/infrastructure requirements verified by inspection. No new behavior is implied. This is the one place new requirement text was written; please confirm the wording.
  2. The duplicate P0 `CORE-TIME-001` line was removed; the detailed definition (including R02-005's clock-domain rules) is kept.
  3. A second table, "Process traceability (Requirement -> Artifact -> Verification)", traces GEN-001/002/003/005 and the seven new IDs. Verification is a file or `inspection`; nothing was invented. GEN-003 and GEN-005 get real automated checks from the audit script (no dependency mechanisms/ROS2/DDS/EtherCAT includes; every test source is registered with CTest).
  4. The main table row for `CORE-PLAT-001` now lists `tests/contract/scheduler_contract.hpp`; a bare test name was replaced by a full path.
  5. `CORE-ERR-003` stays "Reserved (unused)": exempt from tracing, still defined.
- Audit script (`make traceability-check`): fails on an undefined referenced ID, duplicate definition, defined-but-untraced ID, row naming an undefined ID or missing file, public header absent from the table, header whose `Requirements:` tag lacks the row's IDs, unregistered test source, forbidden dependency mechanism or include. Warns when no test in a row carries the row's ID. Verified by mutation: renaming a test file, renaming an ID and deleting a definition each produce errors and exit status 1.
- Result on the repository: 49 requirements, 48 traced (1 reserved), 0 errors, 0 warnings.
- Not changed: `HEADER-001` (tooling rule ID used by `check_source_headers.py`, its workflow and hook) is outside the `CORE-` requirement namespace and is deliberately left alone.
- Build: `rm -rf build && cmake -S . -B build && cmake --build build -j$(nproc)` — 0 warnings.
- Tests: `ctest --test-dir build --output-on-failure` — 16/16 passed.
- Sanitizers: separate Debug build `-fsanitize=address,undefined` — 16/16 passed.
- Coverage: `make coverage` — 98% overall (unchanged).
- `make check` (header-check, traceability-check, format-check, lint) passed; `format-check`/`lint` are TODO stubs (no tooling run). `git diff --check` clean.
- Known limitations: the audit checks that the recorded chain exists and is tagged consistently; it cannot judge whether a test meaningfully verifies a requirement. Requirement numbering has gaps (for example no CORE-ARCH-002, CORE-REQ-001, CORE-TEST-002); they were not invented.

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

- [x] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Review notes:

KF-CORE-R02-006 implementation commit `00899f9` (evidence `7d2a36c`) was
reviewed against the task requirements and submitted validation evidence.

The seven newly defined requirement IDs (CORE-REQ-002, CORE-BUILD-001,
CORE-TEST-001, CORE-DOC-001, CORE-ARCH-001, CORE-ARCH-003, CORE-DEV-001)
are approved: they are recovered from existing project artifacts, not
invented as new product behavior. Numbering gaps are correctly preserved.
HEADER-001 correctly remains outside the CORE namespace.

The traceability audit script detects structural corruption (removed
definitions, changed IDs, renamed tests) and exits non-zero. Its limitation
is recorded: it establishes that the recorded chain exists and is tagged
consistently, not that a test genuinely verifies a requirement.

Result: 49 requirements identified, 48 traced, CORE-ERR-003 explicitly
reserved/exempt, 0 audit errors, 0 audit warnings.

Validation evidence is sufficient:
- Clean build: 0 warnings
- CTest: 16/16 passed
- ASan + UBSan: 16/16 passed
- Coverage: 98%
- make check: passed
- git diff --check: passed
- Production implementation unchanged

Task KF-CORE-R02-006 is ACCEPTED.

Reviewer Decision: PASS

## 15. Acceptance Rule

The task cannot be marked ACCEPTED solely because the code compiles. All applicable functional, test, regression, quality, scope, documentation, and Git criteria must have evidence.
