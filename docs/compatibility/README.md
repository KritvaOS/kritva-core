# Kritva Core Compatibility Documentation (R1.0)

The Core 1.x compatibility and evolution contract.

| Document | Purpose |
|---|---|
| `API_INVENTORY.md` | Classification of every installed public header. |
| `COMPATIBILITY_POLICY.md` | Source and semantic compatibility classes. |
| `ABI_POLICY.md` | ABI posture: no ABI promise. |
| `VERSIONING_POLICY.md` | Release impact, evolution review, tag naming and package version selection. |
| `DEPRECATION_POLICY.md`, `DEPRECATIONS.md` | Deprecation lifecycle and the register of deprecated items. |

## How the policy is enforced

| Mechanism | What it catches |
|---|---|
| `scripts/audit/check_api_inventory.py` | an installed header missing from, or misclassified in, the inventory |
| `scripts/audit/check_compat_policy.py` | policy structure, references, class and release-impact consistency, deprecation register versus headers, ABI machinery in the build |
| `scripts/audit/check_api_surface.py` with `tests/compat/api_surface.snapshot` | any change to the public declaration surface, reported as a candidate class |
| `tests/unit/api_compat_boundary_test.cpp` | an enumerator value, type property or virtual-interface shape that changed |

## Changing a stable item deliberately

1. Classify the change and write the change record (`VERSIONING_POLICY.md`, section 5) and obtain the review decision.
2. Make the change together with its header contract text, owning documentation, inventory row, requirement traceability and changelog entry.
3. Regenerate the surface snapshot with `make api-surface-update`, update the boundary test where it pins the changed property, and include both in the same reviewed change.
4. A snapshot or boundary-test change without an evolution review is an accidental regression, not a deliberate change.
