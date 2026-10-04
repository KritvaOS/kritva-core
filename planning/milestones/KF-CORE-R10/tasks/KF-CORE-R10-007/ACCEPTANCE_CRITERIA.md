# KF-CORE-R10-007 — Acceptance Criteria

Status: ACCEPTED

## Primary Commit

`test(core): validate R1.0 package compatibility`

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

Primary commit: `f212e46` `test(core): validate R1.0 package compatibility` (exact message; baseline `01a1768`), plus the review-required F5 corrective commit `30c461b` `docs(core): clarify EXACT package version matching`. `git diff 01a1768 HEAD -- include src VERSION` is empty; the project version stays `0.9.0`.

- Production-side change (the only one in R1.0): `CMakeLists.txt` package version file `SameMinorVersion` -> `SameMajorVersion`.
- Installed-consumer test: requests derived from the project version against the installed package (accepted: major, `major.0`, `major.minor`, full version; refused: newer patch, newer minor, next and previous major; `EXACT` only for the full installed version string).
- `tests/install/package_version_matrix.cmake`: real `find_package` over 5 installed versions x 17 requests x with/without `EXACT` (165 cases): 0 disagreements with the policy model; the same matrix with the old same-minor mode shows 16 disagreements (detected). The policy audit fails if the package mode is not `SameMajorVersion`.
- F5 (architecture-review exception to the frozen policy text): `EXACT` is accepted only when the requested version string equals the installed version string (CMake's `EXACT` is textual); policy text, example table, audit model and tests agree. Policy audit self-test 36/36.
- Fresh clone of `30c461b`: Debug and Release 74/74, 0 warnings; `make check` passes; traceability 118 / 117 / 0; `git diff --check` clean.
- Security impact: **SECURITY IMPACT: DOCUMENTATION ONLY** (package selection is documented as not an authenticity or provenance statement; no mechanism added).

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | First submission **CHANGES REQUIRED** (F5 only); after `30c461b` **PASS / ACCEPTED** |
| Accepted commits | `f212e46` and `30c461b` |
| Date | 05-10-2026 |

Reviewer notes (local evidence acceptance; commits unpushed): the `SameMajorVersion` implementation is the intended single production-side build change; F5 is an approved architecture-review exception for wording only (underlying R10-004 intent unchanged); policy text, audit, matrix and real CMake behavior now agree; the differential matrix proves the new contract is enforced and discriminated from the pre-1.0 behavior. The Integration Freeze must cite the approved F5 exception and classify the production diff as one intended build/package change plus tests, tooling and documentation.

**Reviewer Decision: PASS — KF-CORE-R10-007 is ACCEPTED.**
