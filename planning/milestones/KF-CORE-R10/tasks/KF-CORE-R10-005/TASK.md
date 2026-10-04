# KF-CORE-R10-005 — Deprecation & Migration Policy

## Objective

Define deprecation markers, documentation, migration guidance, compatibility windows and removal rules.

## Requirement

CORE-COMPAT-008, defined by this task in root `REQUIREMENTS.md`.

## Concrete Scope (policy and tooling only; no manufactured deprecation)

- `docs/compatibility/DEPRECATION_POLICY.md`: lifecycle (stable, deprecated, removable, removed in MAJOR), deprecating an item (review-required, MINOR, standard `[[deprecated]]` attribute with replacement and earliest removal, same-change obligations), compatibility window and removal, migration guidance content and location, security considerations, relationship to other R1.0 policy.
- `docs/compatibility/DEPRECATIONS.md`: the register of deprecated items, empty at 1.0.0 (the inventory identifies no real candidate).
- `scripts/audit/check_compat_policy.py` extended: register vs public headers (every `[[deprecated]]` registered, every registered item deprecated, complete rows, valid versions, removal a later MAJOR); self-test demonstrates the policy with injected fixtures in a copy of the headers, never in the real tree.
- Requirement CORE-COMPAT-008 with a process row; links; `TESTING.md`; `CHANGELOG.md`.

## Exclusions

- No Core item is deprecated; no attribute is added to any header; no header, `src/` or behavior change; no new public API, ABI mechanism, package rule or version change.

## Dependencies

R10-004 (ACCEPTED).
