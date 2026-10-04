# KF-CORE-R10-001 — Public API Inventory & Compatibility Classification

## Objective

Inventory the actual R0.9 installed/public Core API and classify every public surface needed to form the R1.0 compatibility baseline.

## Requirement

CORE-COMPAT-001 (defined by this task in root `REQUIREMENTS.md`).

## Concrete Scope (approved at plan alignment)

- `docs/compatibility/API_INVENTORY.md`: one row per header installed from `include/kritva/core/` (49 at the R0.9 baseline) with domain, stability class, sensitivity flags (ownership/lifetime, thread safety, enum/error values, virtual interface), owning API documentation page and its `docs/api/API_INDEX.md` status.
- Every R0.9 header is `stable`; no `experimental`, `deprecated`, `internal` or `test-only` class is manufactured.
- Explicit documentation decisions for the nine stub pages, headers with no owning page, the umbrella header, the CMake package and the absence of any ABI claim.
- `scripts/audit/check_api_inventory.py` with `--self-test`, run by `make check` and two CTests (`find_program`, tooling only).
- Requirement CORE-COMPAT-001 in `REQUIREMENTS.md` (definition and process-traceability row), link from `docs/api/README.md`, `TESTING.md` and `CHANGELOG.md`.

## Exclusions

- No API redesign, no header or `src/` change, no functional behavior change.
- No ABI promise by inventory alone; no source/semantic, versioning, deprecation or package policy (R10-002..R10-005, R10-007).
- No conversion of stub pages to maintained pages; no new page for headers without one.
- No version change (1.0.0 only at R10-009).

## Dependencies

R10 Scope Confirmation (APPROVED).
