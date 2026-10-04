# KF-CORE-R10-002 — Acceptance Criteria

Status: ACCEPTED

## Primary Commit

`docs(core): define source and semantic compatibility contract`

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

Primary commit: `bbe10b6` `docs(core): define source and semantic compatibility contract` (exact message; baseline `260a5ec`). `git diff 260a5ec HEAD -- include src` is empty; no version change.

- `docs/compatibility/COMPATIBILITY_POLICY.md` (CORE-COMPAT-002/003): stable-inventory scope; independent source/semantic/ABI dimensions; contract sources and precedence (unstated behavior not promised); classified source and semantic change tables (Compatible / Review-required / Incompatible; review-required treated as incompatible until R10-004); compatibility is not security; exclusions.
- `scripts/audit/check_compat_policy.py`: structure and references only; `--self-test` detects 8 deliberate defects; wired into `make check` and two CTests.
- Fresh clone: Debug and Release 69/69 (67 prior + 2 new), 0 warnings; `make check` passes; traceability 111 / 110 / 0; `git diff --check` clean.
- Security impact: **SECURITY IMPACT: DOCUMENTATION ONLY**.

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS / ACCEPTED** |
| Accepted commit | `bbe10b6` |
| Date | 05-10-2026 |

Reviewer notes (local evidence acceptance; commits unpushed): unstated behavior is not promised; error message text is not contractual; weakening a precondition is review-required and strengthening it incompatible; removing `noexcept` is review-required as a weakened guarantee. **Required qualification (c):** a contract-restoring fix is Compatible only when it restores the already-authoritative contract, is recorded in the changelog, is covered by a regression test and is not used to reinterpret an ambiguous contract; an ambiguous contract resolved one way is an evolution decision. Applied in a focused corrective commit (the accepted commit is not amended). ABI remains R10-003's decision.

**Reviewer Decision: PASS — KF-CORE-R10-002 is ACCEPTED.**
