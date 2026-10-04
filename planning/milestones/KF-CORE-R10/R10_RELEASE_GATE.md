# KF-CORE-R10 — Release Gate

## Status

**PASS / RELEASE AUTHORIZED** — release candidate `af16847`; tag `kritva-core-r1.0` on the release-record commit.

## Release Target

- Version: `1.0.0`
- Tag: `kritva-core-r1.0`
- Release candidate: to be recorded at gate
- Release-record commit: documentation-only commit created after PASS

## Entry Criteria

- All R10 implementation tasks accepted.
- R10 API / Compatibility Review PASS / FROZEN.
- R10 Integration Freeze PASS / HONORED.
- Full validation PASS.
- Public API inventory and classification reconciled.
- Compatibility policy reconciled into canonical documentation.
- Version/package policy verified by install-consumer tests.
- Security review complete.
- Requirements traceability clean.
- Release documentation reconciled.

## Required Validation

- Fresh-clone Debug build.
- Fresh-clone Release build.
- Full CTest regression.
- ASan + UBSan.
- TSan where configured.
- Strict warning build with `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`.
- GCC `-fanalyzer`.
- Public-header self-containment.
- Coverage review against the R0.9 baseline.
- Requirements traceability audit.
- API documentation audit.
- Dependency/prohibited-header audit.
- Production isolation audit.
- Install-consumer/package compatibility test.
- Compatibility/boundary test suite.
- Security boundary/assumption audit.
- `git diff --check`.
- Clean working tree.

## Release Procedure

1. Complete independent R10 Release Gate review.
2. Commit release documentation as a documentation-only release-record commit.
3. Create annotated `kritva-core-r1.0` tag on the release-record commit only after PASS.
4. Push `main` and tag.
5. Independently verify remote `main`, tag object and peeled tag.
6. Reconcile all planning documents to `RELEASED / SYNCHRONIZED / CLOSED`.

## Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (independent architecture review) |
| Decision | **PASS / RELEASE AUTHORIZED** |
| Release candidate | `af16847` `build(core): prepare 1.0.0 release candidate` (R10-009 accepted; acceptance record `85d3dbe`) |
| Version | 1.0.0 (`VERSION` = CMake project version, traceability-enforced) |
| API / Compatibility Review freeze | `55e57df` (record `f08b666`) |
| R10 Integration Freeze | `660e4f4` (record `f36fda1`) |
| R10-008 acceptance / Security Architecture Review | `c0edf1f` / `285580d` (PASS, SECURITY IMPACT: DOCUMENTATION ONLY) |
| Production API / source diff vs `kritva-core-r0.9` | **EMPTY** (`git diff kritva-core-r0.9 af16847 -- include src`) |
| Intended production packaging change | `CMakeLists.txt` package version file `SameMinorVersion` -> `SameMajorVersion` (the only production-side change in R1.0) |
| Approved frozen-policy exception | F5 (`30c461b`): `EXACT` accepted only when the requested version string equals the installed version string |
| Open blockers | 0 |
| Evidence basis | Local evidence acceptance (R1.0 commits unpushed at the gate); remote verification follows the owner push |
| Date | 05-10-2026 |

**Reviewer Decision: PASS — Kritva Core 1.0.0 is authorized for release.**

## Validation Summary (fresh clone of `af16847`)

Debug, Release, strict `-Werror`, ASan+UBSan and TSan (ASLR off) 76 / 76; Release random order x10 all 76 / 76; installed-consumer test on the real 1.0.0 package; package version matrix 165 cases / 0 disagreements (old same-minor mode detected); traceability 120 requirements / 119 traced / 0 errors; API documentation 15 documents / 10 maintained / 5 stubs / 0 errors; API inventory 49 / 49; compatibility-policy audit 0 errors; API surface 49 headers / 335 declarations / 0 differences; security documentation 8 records / 0 errors; coverage 619 / 626 lines (R0.9 baseline 618 / 625: one extra covered line in the unchanged `error/result.hpp`, the same seven uncovered production lines, no new production line, no new exclusion).

## Known Tooling Note (GCC analyzer; not a blocker)

GCC 11.4 `-fanalyzer` produces pre-existing `-Wanalyzer-null-dereference` diagnostics through libstdc++ `std::vector` internals in unchanged test code. The same diagnostics reproduce against the R0.9 baseline. No R1.0 production source or new compatibility test is affected.

| Scope | Result |
|---|---|
| Production source (`src/`) | clean (0 warnings at `-O0`) |
| R1.0-added compatibility test | clean |
| Existing unchanged tests | pre-existing warnings reproduced from R0.9 |

The analyzer is not claimed to be globally clean for R1.0. Recorded as a known tooling note and added to the post-1.0 backlog (scope the analyzer sweep).

## Release Record

| Item | Value |
|---|---|
| Release commit | `4acce3b` `docs(release): record Kritva Core R1.0 release gate` |
| Tag | `kritva-core-r1.0` (annotated, on the release-record commit, not on the candidate) |
| Candidate-to-release diff | `git diff af16847 HEAD -- include src tests CMakeLists.txt VERSION` empty |
| Remote verification | PASS / RELEASED / SYNCHRONIZED / CLOSED — `git ls-remote`: `refs/heads/main` = `4acce3b` at release verification; tag object `fc9b51f`; peeled tag `kritva-core-r1.0^{}` = `4acce3b`; remote `VERSION` = 1.0.0 and CMake project version 1.0.0 |
