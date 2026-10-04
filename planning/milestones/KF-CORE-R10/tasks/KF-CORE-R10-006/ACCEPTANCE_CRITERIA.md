# KF-CORE-R10-006 — Acceptance Criteria

Status: ACCEPTED

## Primary Commit

`test(core): add R1.0 compatibility boundary harness`

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

Primary commit: `b44b14b` `test(core): add R1.0 compatibility boundary harness` (exact message; baseline `f08b666`). `git diff f08b666 HEAD -- include src VERSION` is empty; no policy document changed; no production code, public API or compatibility framework added.

- `scripts/audit/check_api_surface.py` and `tests/compat/api_surface.snapshot` (335 declarations, 49 headers): differences reported as Incompatible / Review-required / Compatible candidates; self-test detects 10 changes and ignores 4 non-changes (comment, function body, private member, whitespace).
- `tests/unit/api_compat_boundary_test.cpp`: underlying type and value of every public enumerator (exhaustive switches), properties of 35 stable value types, pure-virtual set and signatures of 9 client-implemented interfaces. 13 mutants applied to a copy of the headers, all killed.
- Fresh clone: Debug, Release and strict `-Werror` 72/72, 0 warnings; `make check` passes; traceability 117 / 116 / 0; `git diff --check` clean.
- Security impact: **SECURITY IMPACT: NONE**.

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS / ACCEPTED** |
| Accepted commit | `b44b14b` |
| Date | 05-10-2026 |

Reviewer notes (local evidence acceptance; commits unpushed): the heuristic scanner is accepted as a textual boundary audit (it is not a C++ parser; the compile-time test is the independent compiler-enforced check) and no AST implementation is required in R1.0; nothrow-move is correctly not pinned; behavioral regression stays in the existing contract and integration tests; the snapshot is a review boundary, not a classification: a snapshot diff yields a candidate classification, then the R10-004 evolution policy and review where required, preserving the distinction between detecting a change and approving it.

**Reviewer Decision: PASS — KF-CORE-R10-006 is ACCEPTED.**
