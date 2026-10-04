# KF-CORE-R10-002 — Source & Semantic Compatibility Contract

## Objective

Define source and semantic compatibility guarantees, including ownership, lifetime, threading, enums/errors and observable behavior.

## Requirements

CORE-COMPAT-002 (source compatibility) and CORE-COMPAT-003 (semantic compatibility), defined by this task in root `REQUIREMENTS.md`.

## Concrete Scope

- `docs/compatibility/COMPATIBILITY_POLICY.md`: scope (stable inventory items only), compatibility dimensions, contract sources and precedence (behavior no contract states is not promised), source-compatibility change classification, semantic-compatibility protected properties and change classification, compatibility-is-not-security, relationship to later R1.0 policy, exclusions.
- Change classes `Compatible`, `Review-required`, `Incompatible`; review-required is treated as incompatible until the R10-004 evolution process exists. No mapping to major/minor/patch (R10-004).
- `scripts/audit/check_compat_policy.py` with `--self-test`, wired into `make check` and two CTests (structure and references only).
- Requirements CORE-COMPAT-002/003 with process-traceability rows; links from `docs/api/README.md` and `docs/api/API_GUIDELINES.md`; `TESTING.md`; `CHANGELOG.md`.

## Exclusions

- No header, `src/` or behavior change; no new public API; no ABI posture (R10-003); no version-number mapping, enum/ErrorCode/virtual-interface evolution procedure or change-review process (R10-004); no deprecation or migration policy (R10-005); no package rule (R10-007); no validation harness (R10-006).
- No stub-to-maintained conversion; no new API page for headers without one (decided at R10-008).

## Dependencies

R10-001 (ACCEPTED).
