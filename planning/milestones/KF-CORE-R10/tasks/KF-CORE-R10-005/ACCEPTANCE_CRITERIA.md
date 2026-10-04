# KF-CORE-R10-005 — Acceptance Criteria

Status: ACCEPTED

## Primary Commit

`docs(core): define API deprecation and migration policy`

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

Primary commit: `2882cd2` `docs(core): define API deprecation and migration policy` (exact message; baseline `1c1ae54`). `git diff 1c1ae54 HEAD -- include src CMakeLists.txt VERSION` is empty; no version change; no `[[deprecated]]` exists in `include/`.

- `docs/compatibility/DEPRECATION_POLICY.md` (CORE-COMPAT-008): lifecycle stable, deprecated, removable, removed in MAJOR; deprecating an item is review-required (MINOR after evolution review, never PATCH); replacement or rationale; window (published in at least one release before the removing MAJOR; earliest removal the next MAJOR); migration guidance content and location; security considerations. `docs/compatibility/DEPRECATIONS.md`: the register, empty at 1.0.0 (no real candidate; none manufactured).
- `scripts/audit/check_compat_policy.py`: register audited against the headers; the self-test works on a copy of `include/` and injects fixtures; 26 deliberate defects detected in total.
- Fresh clone: Debug and Release 69/69, 0 warnings; `make check` passes; traceability 116 / 115 / 0; `git diff --check` clean.
- Security impact: **SECURITY IMPACT: DOCUMENTATION ONLY**.

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS / ACCEPTED** |
| Accepted commit | `2882cd2` |
| Date | 05-10-2026 |

Reviewer notes (local evidence acceptance; commits unpushed): the window is accepted (at least one released version before removal, earliest removal the next MAJOR, no fixed minor count; removal cannot occur in the major series that first deprecated it); the register is authoritative for item-level deprecations and the inventory class `deprecated` applies only when a whole header is deprecated; the `docs/compatibility/migration/` per-MAJOR convention is accepted without creating files; not manufacturing a deprecated API is explicitly approved and fixture-based verification is preferred. Next gate: R10 API / Compatibility Review, with a policy-to-policy contradiction matrix added to the evidence package.

**Reviewer Decision: PASS — KF-CORE-R10-005 is ACCEPTED.**
