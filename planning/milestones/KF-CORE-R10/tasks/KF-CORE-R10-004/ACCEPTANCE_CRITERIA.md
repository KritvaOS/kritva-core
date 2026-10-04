# KF-CORE-R10-004 — Acceptance Criteria

Status: ACCEPTED

## Primary Commit

`docs(core): define versioning and API evolution policy`

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

Primary commit: `a121c24` `docs(core): define versioning and API evolution policy` (exact message; baseline `32ec62a`). `git diff 32ec62a HEAD -- include src CMakeLists.txt VERSION` is empty; no version change.

- `docs/compatibility/VERSIONING_POLICY.md` (CORE-COMPAT-005..007): MAJOR.MINOR.PATCH identity; release-impact table over the R10-002 classes; enumeration, `ErrorCode`, virtual-interface and constant evolution rules; API evolution review; installed-package version-selection rule for 1.x (same MAJOR, installed >= requested; implemented and validated by R10-007).
- `scripts/audit/check_compat_policy.py`: release-impact table consistency with the compatibility classes, package examples evaluated against a model of the stated rule; `--self-test` detects 17 deliberate defects.
- Consequential two-line edit of `COMPATIBILITY_POLICY.md` (review-required: "a change that does not pass that review is treated as `Incompatible`").
- Fresh clone: Debug and Release 69/69, 0 warnings; `make check` passes; traceability 115 / 114 / 0; `git diff --check` clean.
- Security impact: **SECURITY IMPACT: DOCUMENTATION ONLY**.

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS / ACCEPTED** |
| Accepted commit | `a121c24` |
| Date | 05-10-2026 |

Reviewer notes (local evidence acceptance; commits unpushed): review-required is MINOR only after a recorded successful evolution review, otherwise Incompatible / MAJOR; the R10-002 wording synchronization is valid; experimental-to-stable is MINOR provided the promotion itself permits no semantic break; the emergency removal exception is accepted as written (explicit architecture review, documented justification, recorded release note); the 1.x package rule is accepted as the direction, with exact CMake behavior implemented and validated under R10-007. R10-005 must not manufacture a deprecation: no deprecated Core API is introduced unless the inventory identifies a real candidate; demonstrate with fixtures or tooling.

**Reviewer Decision: PASS — KF-CORE-R10-004 is ACCEPTED.**
