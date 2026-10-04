# KF-CORE-R09-007 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R09-007 |
| Status | ACCEPTED |
| Primary commit | `test(core): complete R0.9 capability validation` |
| Reviewer | ChatGPT — independent acceptance gate |
| Estimated effort | 2–3 ED |
| Dependency | R09-006 accepted |

## Objective

Run the complete R0.9 quality matrix, verify the release boundary and prepare the 0.9.0 release candidate.

## Proposed Requirements Traceability

CORE-CAP-011

The implementing task must update authoritative root `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- Fresh-clone Debug and Release builds pass with zero unexpected warnings.
- Full CTest regression passes.
- ASan/UBSan pass.
- TSan passes using documented environment.
- Strict Werror passes.
- GCC analyzer passes where configured.
- Header self-containment passes.
- Coverage meets milestone policy.
- Traceability is clean.
- Install consumer passes.
- API documentation and security records are consistent.
- Production diff after Integration Freeze contains no unreviewed semantic change.
- Release metadata is internally consistent for 0.9.0.

## New Tests

- Complete validation matrix.
- Release candidate smoke/consumer test.
- Ten randomized order repetitions where ordering/determinism is relevant.

## Regression Tests

- Full pre-R09 CTest suite remains green.
- Existing R0.8 Runtime, Component, Configuration, Platform and Context behavior remains green.

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

- Newly added executable production lines must be covered.
- No unexplained coverage exclusion.
- Overall R0.9 coverage remains at or above 98% and has no unexplained regression greater than 1 percentage point from the R0.8 98.9% baseline.
- Required sanitizer, strict-analysis and install-consumer evidence is supplied at milestone validation.

## API Documentation

- Any accepted public API or semantic contract change is documented in the corresponding `docs/api/` Markdown in the same task.
- Documentation must match accepted headers, semantics, ownership/lifetime, lifecycle interaction, errors, threading/real-time expectations and exclusions.
- API/documentation mismatch is an acceptance blocker.

## Security Impact

- The implementor records `SECURITY IMPACT: NONE`, `SECURITY IMPACT: DOCUMENTATION ONLY`, or `SECURITY IMPACT: ARCHITECTURE REVIEW REQUIRED`.
- No new security mechanism is introduced without explicit architecture approval.

## Expected Files Changed

- `Validation evidence record.`
- `Release candidate metadata.`
- `No unrelated production files.`

## Git Commit

```text
test(core): complete R0.9 capability validation
```

## Evidence Required from Implementor

- `git status` before and after implementation.
- `git diff --check`.
- Build/test output.
- Focused contract-test evidence.
- Mutation evidence for contract-sensitive behavior where applicable.
- API documentation diff.
- Security-impact assessment.
- Diff/stat summary.
- Commit SHA.
- Explicit mapping from every acceptance criterion to objective evidence.

## Implementor Evidence

Validated release candidate: HEAD `ef14e99` (`build(core): prepare 0.9.0 release metadata`), validated from a **fresh `git clone`** of the repository (clean tree, `git status` empty before building). R09 Integration Freeze PASS / HONORED (`1e68451`, evidence `553258b`).

### Candidate history since the Integration Freeze (production freeze commit `4c86b53`)
| Commit | Purpose | Production code? |
|---|---|---|
| `2608795` `test(core): add capability contract reference harness` | R09-004 (accepted) | no (tests only) |
| `0f6b6c3` `test(core): add capability readiness lifecycle integration tests` | R09-005 (accepted) | no (tests, docs, requirements) |
| `04f0859` `test(core): validate R0.9 documentation security and boundaries` | R09-006 (accepted): documentation audit, boundary snapshot, security record, install consumer, `CORE-CAP-011` | no |
| `20d2fde` `docs(planning): accept R09-006 and record the Security Architecture Review` | acceptance and the PASS security review record | no |
| `ef14e99` `build(core): prepare 0.9.0 release metadata` | `VERSION` and CMake project version 0.9.0; README `find_package(kritva_core 0.9)`; REQUIREMENTS/API titles R0.9; root CHANGELOG 0.9.0 section; install consumer default request 0.9 and `Version{0,9,0}` | no |

**Production-change audit:** `git diff 4c86b53 HEAD -- include src` is **empty**; the three capability headers are byte-identical to the freeze commit (checked per file in the fresh clone); against `kritva-core-r0.8` the only production differences are the reviewed R09 contract text (`capability.hpp` +62, `capability_id.hpp` +11, `capability_set.hpp` +118); `src/` and `platform/requirements.hpp`, `runtime/component.hpp`, `runtime_manager.hpp`, `dependency_graph.hpp`, `component_context.hpp` and `platform/adapter.hpp` are byte-identical to the released tag (checked per file).

### AC-01 Clean build
Fresh clone: Debug and Release, 0 warnings each (GCC 11.4.0, Linux 6.8). Public-header self-containment (`kritva_core_header_checks`) builds in every configuration.

### AC-02 Complete regression
`ctest`: **65/65 passed** in Debug and Release (unit, contract, conformance, platform, context, operational, configuration, capability, integration, isolation, documentation-audit and install tests); 10 randomized-order repetitions all pass.

### AC-03 Sanitizers and strict quality
ASan + UBSan: 0 warnings, 65/65. TSan (ASLR off via `setarch -R`): 65/65. Strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`: 0 warnings, 65/65. GCC `-fanalyzer` is clean over every `src/` file and over the contract, platform, context, operational, configuration, capability and integration tests.

### AC-04 Coverage
`make coverage` from the fresh clone: **618/625 lines (98.9%)**, identical to the R0.8 result; R0.9 adds no production code, so no new lines and no new exclusions (the 7 uncovered lines are the established baseline).

### AC-05 Traceability and documentation audit
`make check`: header-check passed; traceability **108 requirements, 107 traced (CORE-ERR-003 reserved), 0 errors, 0 warnings**; `CORE-CAP-004` to `CORE-CAP-011` are defined once (`CORE-CAP-011` at R09-006, covering R09-006 and R09-007); earlier IDs are not renumbered. The API documentation audit reports **15 documents indexed, 6 maintained, 9 stubs, 0 errors**, and its self-test detects all ten deliberate defects.

### AC-06 Freeze verification
See the production-change audit: no production change after the freeze.

### AC-07 Install consumer and package version
The install test installs to a scratch prefix and builds and runs an external consumer against the installed package only, exercising R0.3 to R0.8 and, new, the R0.9 capability contract (replace-in-place and first-insertion order, identity-only lookup, independent snapshots, a storable invalid-identity entry, requirement declaration and atomic rejection, identity-only evaluation with one snapshot, `UNSUPPORTED` from `check_required`; return codes 84–93). The installed config reports `PACKAGE_VERSION 0.9.0` and `find_package` accepts 0.9 and 0.9.0; wrong expectations (0.9.1 and 0.8.0) make the install test fail (checked manually, both exit 1; the expected 0.9.0 passes).

### AC-08 Isolation and dependency audits
`kritva_core_test_isolation` and the audit (`CORE-PLAT-016`) pass: no production file includes test-only support, every production TU and the umbrella header compile with only `include/` on the path, no `install()` rule ships test support. Dependency scan: no `<thread>`, `<mutex>`, `<atomic>`, `<future>`, `<condition_variable>`, `<queue>`, `<deque>`, `<iostream>`, `<fstream>` or `<cstdio>` in `include/` or `src/`, no POSIX/Linux/Windows include, no ROS2/DDS/EtherCAT/RTOS identifier (the only hit is a comment in `boundary.hpp` naming them as forbidden); no `find_package`/`FetchContent`/`ExternalProject`; `git diff --check` clean. No service registry, locator, resolver, discovery, readiness state or capability credential exists (compile-time boundary snapshot).

### AC-09 Documentation and security
REQUIREMENTS (R0.9, `CORE-CAP-001..011`), API sections 48–51, ARCHITECTURE, README, TESTING, root CHANGELOG, `docs/api` (six maintained pages), `docs/security` (SD-R09-01..07), `VERSION` and CMake (both 0.9.0, enforced by the audit) agree with the frozen contract. The Security Architecture Review is PASS with **SECURITY IMPACT: DOCUMENTATION ONLY**.

### AC-10 Clean candidate
Working tree clean after every commit; the candidate commit is `ef14e99`; this evidence is committed as `test(core): complete R0.9 capability validation`. Candidate suitable for the R09 Release Gate.

### Findings / follow-ups (none block)
- `make lint` and `make format-check` remain deferred stubs (static analysis is strict warnings and GCC `-fanalyzer`), as in R0.3 to R0.8.
- 32-bit scheduler affinity mask (documented, frozen); conformance suite level-2 mutation strictness gap (documented, accepted at R04-006); a context, reporter, provider, retained configuration pointer or `CapabilitySet` reference used after what it refers to is destroyed is documented undefined behavior (non-owning by design).
- Nine `docs/api` pages for unchanged domains are labelled stubs (policy confirmed at R09-006) and are replaced when their contract next changes.
- The unused `<chrono>` include in `types/duration.hpp` and the stale root `implementation.md` remain (cosmetic, unchanged).
- Process notes recorded during R0.9: the baseline reconciliation `c250c54` had nested the API domain directories and was corrected by `7b6b0f4` (disclosed at R09-003); my mutation experiments leave the incremental build directory stale, so every frozen-state check above was run after a full rebuild or from a fresh clone.
- The release commit and tag `kritva-core-r0.9` are decided at the R09 Release Gate; nothing is tagged or pushed.

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS** |
| Accepted commit | `683a3ce` (release candidate `ef14e99`) |
| Evidence reference | Implementor Evidence above |
| Date | 05-10-2026 |

Reviewer notes: fresh-clone validation of the clean candidate, the full quality matrix, traceability 108/107/0, the API documentation audit (0 errors), coverage 618/625 unchanged, the installed-package consumer including the R0.9 capability contract and version enforcement, isolation and dependency scans, the Security Architecture Review PASS and an empty production diff since the freeze commit `4c86b53` are accepted. The reviewer noted its connector could not resolve the local SHAs, so the verdict relies on the supplied evidence. `CORE-CAP-011` is authoritative.

**Reviewer Decision: PASS — KF-CORE-R09-007 is ACCEPTED.**
