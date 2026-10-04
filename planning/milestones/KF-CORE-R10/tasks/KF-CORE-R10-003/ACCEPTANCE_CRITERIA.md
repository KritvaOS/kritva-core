# KF-CORE-R10-003 — Acceptance Criteria

Status: ACCEPTED

## Primary Commit

`docs(core): define R1.0 ABI compatibility policy`

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

Primary commit: `559d75f` `docs(core): define R1.0 ABI compatibility policy` (exact message; baseline `c35047e`). `git diff c35047e HEAD -- include src` is empty; no CMake build change; no version change.

- `docs/compatibility/ABI_POLICY.md` (CORE-COMPAT-004): Core 1.x promises no ABI/binary compatibility; rationale grounded in the real tree (header-based inline/templated C++20 interface, exposed layouts, virtual interfaces, standard-library types, compiler-dependent mangling, default static library with no export control, SOVERSION or ABI tooling); bounded "not promised" matrix; what clients may rely on; conditions for any future ABI promise; no ABI mechanism introduced.
- `scripts/audit/check_compat_policy.py` audits the page and fails if `CMakeLists.txt` gains ABI machinery (SOVERSION, visibility settings, export-header generator, shared library target); `--self-test` detects 12 deliberate defects.
- Fresh clone: Debug and Release 69/69, 0 warnings; `make check` passes; traceability 112 / 111 / 0; `git diff --check` clean.
- Security impact: **SECURITY IMPACT: DOCUMENTATION ONLY**.

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS / ACCEPTED** |
| Accepted commit | `559d75f` |
| Date | 05-10-2026 |

Reviewer notes (local evidence acceptance; commits unpushed): the CMake guard is audit tooling, not an ABI mechanism, and must detect only the documented ABI-enabling mechanisms, not ordinary build changes. The "verified configuration" wording is accepted only with the precise interpretation (source and semantic use in the tested producer/consumer environment; no ABI guarantee, no arbitrary-binary linking, no cross-toolchain claim) and the frozen distinction *compiled library is not an ABI guarantee*; both are made explicit in `ABI_POLICY.md` by a focused corrective commit (the accepted commit is not amended).

**Reviewer Decision: PASS — KF-CORE-R10-003 is ACCEPTED.**
