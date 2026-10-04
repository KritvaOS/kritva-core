# KF-CORE-R10 — API / Compatibility Review

## Status

**PASS / FROZEN** — freeze commit `55e57df`.

## Purpose

Freeze the approved Core 1.x compatibility, API classification, ABI posture, versioning and deprecation semantics before compatibility harness/package integration becomes the release implementation baseline.

## Entry Criteria

| Criterion | Result |
|---|---|
| R10-001 accepted | PASS (`1dba571`) |
| R10-002 accepted | PASS (`bbe10b6`, corrective `bebed6b`) |
| R10-003 accepted | PASS (`559d75f`, corrective `37e8489`) |
| R10-004 accepted | PASS (`a121c24`) |
| R10-005 accepted | PASS (`2882cd2`, acceptance record `97fd88c`) |
| Public API inventory reconciled with installed headers | PASS — 49 / 49 headers, audited mechanically |
| Compatibility policy documentation synchronized | PASS — five documents plus the register, cross-checked |
| Root requirements and traceability reconciled | PASS — CORE-COMPAT-001..008 defined; 116 requirements / 115 traced / 0 errors |
| Security-impact assessments recorded for all applicable tasks | PASS — NONE (R10-001), DOCUMENTATION ONLY (R10-002..005 and the corrective commit) |

## Production diff

`git diff kritva-core-r0.9 HEAD -- include src VERSION cmake` is **empty**. `CMakeLists.txt` differs by four added lines (the four audit CTests for the inventory and the compatibility policy; tooling only). No public API, symbol, attribute, deprecation, version or package-selection change. The frozen contract is policy text, the inventory and the audits that enforce them.

## Review Questions

| # | Question | Result | Evidence |
|---|---|---|---|
| 1 | Is every public installed API item classified? | PASS | `API_INVENTORY.md` classifies all 49 installed headers (all `stable`); item-level deprecation only through `DEPRECATIONS.md` (empty); `check_api_inventory.py` (self-test 16 defects). |
| 2 | Are source and semantic compatibility boundaries explicit? | PASS | `COMPATIBILITY_POLICY.md` sections 4–6: contract sources and precedence, classified source and semantic change tables, most-severe-class rule. |
| 3 | Is ABI scope explicit and conservative? | PASS | `ABI_POLICY.md`: no ABI promise, rationale from the real build, "not promised" matrix, future conditions, audit guard against ABI machinery in `CMakeLists.txt`. |
| 4 | Are enum / ErrorCode / virtual-interface / ownership changes classified? | PASS | COMPATIBILITY section 6; VERSIONING section 4. |
| 5 | Are SemVer rules unambiguous? | PASS | `VERSIONING_POLICY.md` section 3; the release-impact table is audited for consistency with the compatibility classes; tag naming audited. |
| 6 | Is deprecation / removal behavior explicit? | PASS | `DEPRECATION_POLICY.md`; removal only after a published deprecation and only in MAJOR; the register is audited against the headers. |
| 7 | Is CMake package / version-selection policy testable? | PASS | VERSIONING section 6 rule with audited examples; implementation and installed-consumer tests are R10-007. |
| 8 | Are migration obligations explicit for intentional incompatibilities? | PASS | DEPRECATION section 6. |
| 9 | Does the policy preserve Core platform independence? | PASS | No code change; no platform terms; production isolation audit clean. |
| 10 | Does the policy introduce any unapproved security guarantee? | PASS | None; each policy states compatibility is not security. |

## Cross-Document Findings (closed by the freeze commit)

The review read all five documents against each other and found four inconsistencies, closed by the single corrective commit `55e57df` (no accepted commit amended) and enforced by the compatibility audit:

| ID | Finding | Resolution |
|---|---|---|
| F1 | A change could match both a Review-required source row and an Incompatible semantic row (removing `noexcept`). | The most severe class applies; removing a documented `noexcept` is Incompatible, an undocumented one Review-required. |
| F2 | VERSIONING required removal to follow deprecation but DEPRECATION did not say so. | A stable item is removed only after it has been published as deprecated, except under the emergency exception. |
| F3 | Inventory class `deprecated` could be read at item level. | `deprecated` applies only to a whole header; item-level deprecation lives in `DEPRECATIONS.md`; the inventory preamble points to the accepted pages. |
| F4 | Tag naming for patch releases was undefined. | `MAJOR.MINOR.0` → `kritva-core-rMAJOR.MINOR`; patch releases → `kritva-core-rMAJOR.MINOR.PATCH`. |

## Contradiction Matrix

All pairs consistent after `55e57df`: COMPATIBILITY × ABI, × VERSIONING, × DEPRECATION, × INVENTORY; ABI × VERSIONING, × DEPRECATION, × INVENTORY; VERSIONING × DEPRECATION, × INVENTORY; DEPRECATION × INVENTORY. The specific risks checked: review-required is MINOR only after a recorded PASS (otherwise Incompatible / MAJOR); stable implies no ABI promise; a deprecated item is not removable in the series that deprecated it.

## Validation

Fresh clone of `55e57df`: Debug 69 / 69 and Release 69 / 69, 0 warnings; `make check` passes; compatibility-policy audit 4 pages / 0 errors with self-test 33 / 33; inventory audit 49 / 49 with self-test 16 / 16; traceability 116 / 115 / 0; `git diff --check` clean.

## Freeze

From `55e57df`, the following are frozen and may change only through an explicit architecture-review exception:

- source and semantic compatibility policy;
- the ABI posture (no ABI promise);
- the `MAJOR.MINOR.PATCH` versioning, release-impact and evolution classification, and tag naming;
- the deprecation, migration and removal policy;
- the public API inventory baseline;
- the installed-package version-selection rule for 1.x (implemented and validated by R10-007).

## Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (independent architecture review) |
| Decision | **PASS / FROZEN** |
| Freeze commit | `55e57df` |
| Evidence basis | Local evidence acceptance (commits unpushed; the connector cannot fetch them) |
| Date | 05-10-2026 |

Authorized next: KF-CORE-R10-006 — Compatibility & Boundary Validation Harness, which validates the frozen policy and must not redefine it or add a production compatibility framework.
