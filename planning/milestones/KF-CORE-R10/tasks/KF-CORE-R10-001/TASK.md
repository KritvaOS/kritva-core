# KF-CORE-R10-001 — Public API Inventory & Compatibility Classification

## Objective

Inventory the actual R0.9 installed/public Core API and classify every public surface needed to form the R1.0 compatibility baseline.

## Scope

- Enumerate public headers under `include/kritva/core/`.
- Identify public classes, structs, enums, constants, typedefs, functions, templates and CMake package targets that form the supported surface.
- Classify items as stable, experimental, deprecated, internal or test-only where applicable.
- Record ownership/lifetime, thread-safety, semantic and package compatibility sensitivity.
- Reconcile classification with current API docs and R0.9 stub/maintained status.
- Produce machine-checkable or mechanically auditable inventory data where practical.

## Exclusions

- No API redesign.
- No functional behavior change.
- No ABI promise by inventory alone.
- No conversion of all stub pages to maintained pages.

## Dependencies

R10 Scope Confirmation.
