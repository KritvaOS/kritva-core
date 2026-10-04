# KF-CORE-R10-001 — Acceptance Criteria

Status: ACCEPTED

## Primary Commit

`docs(core): inventory and classify R1.0 public API`

## Acceptance Criteria

1. Every installed/public header under `include/kritva/core/` is inventoried.
2. Every externally visible API element relevant to compatibility is classified.
3. Stable/experimental/deprecated/internal/test-only distinctions are explicit.
4. Ownership/lifetime, thread-safety and semantic compatibility sensitivity are recorded where applicable.
5. API documentation/stub status conflicts are identified and reconciled through explicit decisions.
6. No production behavior or API signature is changed by this task unless separately approved.
7. Traceability requirements are defined/reconciled for the inventory artifact.
8. Documentation reflects the accepted inventory.
9. Security impact is classified.
10. Full task validation passes and no unexplained regression is introduced.

## Concrete Acceptance Gate (approved at plan alignment)

- 49/49 current public headers inventoried; 0 duplicate, 0 missing, 0 invalid classifications, 0 broken documentation references.
- `check_api_inventory.py --self-test` passes (every deliberate defect detected); the CTests and `make check` pass.
- Traceability passes with CORE-COMPAT-001 defined and traced.
- `git diff 27934bf HEAD -- include src` is empty; no version change.

## Security Impact

Record exactly one approved classification.

## Documentation Acceptance

Inventory/classification documentation is canonical and contradicts no public header or accepted contract.

## Git Acceptance

Exact primary commit message above; accepted task commit is not amended.

## Implementor Evidence

Primary commit: `1dba571` `docs(core): inventory and classify R1.0 public API` (exact message; baseline `27934bf`). `git diff 27934bf HEAD -- include src` is empty; no version change.

- `docs/compatibility/API_INVENTORY.md`: 49/49 installed headers, each exactly once, all `stable`; flags Own/Thr/Enum/Virt (Enum, Virt, Thr audited against header text; Own is reviewed judgment); owning documentation page and `API_INDEX.md` status; documentation decisions D-INV-1..5 (nine stubs not promoted; 29 headers without an owning page named, not silently documented; umbrella header; package in scope; no ABI claim).
- `scripts/audit/check_api_inventory.py`: 49 inventoried / 49 installed / 0 errors; `--self-test` detects 16 deliberate defects; wired into `make check` and CTests `kritva_core_api_inventory_audit` and `..._self_test`.
- Requirement CORE-COMPAT-001 defined with a process-traceability row. The traceability parser already recognizes the COMPAT prefix, so no tooling change was needed.
- Fresh clone: Debug and Release 67/67 (65 prior + 2 new), 0 warnings; `make check` passes; traceability 109 / 108 / 0; `git diff --check` clean.
- Security impact: **SECURITY IMPACT: NONE**.

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS / ACCEPTED** |
| Accepted commit | `1dba571` |
| Date | 05-10-2026 |

Reviewer notes: evidence-based local acceptance (commits unpushed). Decisions confirmed: 29 headers without an owning page is discovery data, not a failure (public header is not the same as an independent documented contract; R10-002 / R10-008 decide documentation obligations); the inventory is the authoritative header-to-document mapping and must stay mechanically resolvable and unambiguous (no two pages claim one header without an explicit reason); sensitivity flags are inventory metadata, not compatibility judgments; no traceability tooling change required.

**Reviewer Decision: PASS — KF-CORE-R10-001 is ACCEPTED.**
