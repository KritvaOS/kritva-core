# KF-CORE-R10-009 — Acceptance Criteria

Status: ACCEPTED

## Primary Commit

`build(core): prepare 1.0.0 release candidate`

## Acceptance Criteria

1. The task objective and dependency boundary are fully satisfied.
2. Evidence is tied to the accepted R1.0 compatibility policy.
3. No unapproved public API or semantic expansion occurs.
4. Relevant documentation is updated in the same logical task.
5. Requirements traceability is complete for all new/affected requirement IDs.
6. Applicable compatibility, negative and regression tests pass.
7. Security impact is explicitly classified.
8. Full applicable build/test/analysis checks pass.
9. Production isolation remains clean.
10. `git diff --check` is clean and the working tree is clean at acceptance.
11. Primary commit uses the exact commit message above and is not amended after acceptance.

## Security Impact

Record exactly one approved classification:

- `SECURITY IMPACT: NONE`
- `SECURITY IMPACT: DOCUMENTATION ONLY`
- `SECURITY IMPACT: ARCHITECTURE REVIEW REQUIRED`

## Documentation Acceptance

Documentation is canonical, complete for the changed contract, and does not contradict headers or accepted evidence.

## Implementor Evidence

Release candidate: `af16847` `build(core): prepare 1.0.0 release candidate` (exact message), validated from a **fresh clone** (clean tree). History since the Integration Freeze `660e4f4`: `f36fda1` (freeze record), `c0edf1f` (R10-008, accepted), `285580d` (acceptance record), `af16847` (release metadata).

- Release metadata only: `VERSION` and CMake project version 1.0.0; README package example and rule; install-consumer default request 1.0; API and requirements titles R1.0; root `CHANGELOG.md` 1.0.0 section.
- Production audit: `git diff kritva-core-r0.9 HEAD -- include src` and `git diff 660e4f4 HEAD -- include src` are empty; `CMakeLists.txt` differs from the freeze by the project version and two security-documentation audit CTests only.
- Validation: Debug, Release, strict `-Werror`, ASan+UBSan and TSan (ASLR off) 76/76; Release random order x10 all 76/76; installed-consumer test on the real 1.0.0 package; package version matrix 165 cases / 0 disagreements; traceability 120 / 119 / 0; API docs 15 / 10 / 5 / 0; inventory 49 / 49; compatibility-policy 0 errors; API surface 49 / 335 / 0 differences; security docs 8 records / 0 errors; `VERSION` = CMake project version.
- Coverage: 619 / 626 lines against 618 / 625 at `kritva-core-r0.9` (same command): the only difference is one extra covered line in the unchanged `error/result.hpp`; the same seven uncovered production lines; no new production line and no new exclusion.
- GCC 11.4 `-fanalyzer`: production sources clean (0 warnings at `-O0`); the R1.0-added test clean; existing unchanged tests produce `-Wanalyzer-null-dereference` diagnostics inside libstdc++ `std::vector` internals, reproduced on the `r0.9` tag (not clean globally).
- Security impact: **SECURITY IMPACT: DOCUMENTATION ONLY**.

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS / ACCEPTED — RELEASE CANDIDATE** |
| Accepted commit | `af16847` |
| Date | 05-10-2026 |

Reviewer notes (local evidence acceptance; commits unpushed): the coverage delta is explained (no new production lines); the analyzer finding is accepted as a documented pre-existing tooling issue and **must be recorded explicitly in the Release Gate record** (production source clean; R1.0-added test clean; existing-test sweep shows pre-existing warnings reproduced from R0.9; the analyzer is not to be called globally clean) and added to the post-1.0 backlog; the release procedure is approved: one docs-only release-record commit, the annotated tag `kritva-core-r1.0` on that commit only, owner push of `main` and the tag, independent remote audit, then a docs-only synchronization commit reconciling every R1.0 row.

**Reviewer Decision: PASS — KF-CORE-R10-009 is ACCEPTED; the R10 Release Gate is authorized.**
