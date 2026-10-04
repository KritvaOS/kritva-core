# KF-CORE-R10-004 — Versioning & API Evolution Policy

## Objective

Define SemVer release-impact classification and change-review rules for public API, semantics, package behavior and compatibility-sensitive values.

## Requirements

CORE-COMPAT-005 (release impact), CORE-COMPAT-006 (enum / ErrorCode / virtual-interface / constant evolution), CORE-COMPAT-007 (evolution review and package version-selection rule), defined by this task in root `REQUIREMENTS.md`.

## Concrete Scope (policy-first; no version bump)

- `docs/compatibility/VERSIONING_POLICY.md`: version identity; release-impact table mapping the R10-002 classes to MAJOR/MINOR/PATCH; evolution rules for enumerations and `ErrorCode`, virtual interfaces, constants; the API evolution review (change record, independent decision, same-change obligations, record); the installed-package version-selection rule for 1.x (same MAJOR, installed >= requested, newer or different-MAJOR request rejected, EXACT only when equal) with examples; compatibility-is-not-security; exclusions.
- `scripts/audit/check_compat_policy.py` extended: release-impact table consistency with the compatibility classes, package examples checked against the stated rule, required sections and references; self-test extended.
- Requirements CORE-COMPAT-005..007 with process rows; links from `docs/api/README.md` and `API_GUIDELINES.md`; `COMPATIBILITY_POLICY.md` cross-reference now names the evolution review; `TESTING.md`; `CHANGELOG.md`.

## Exclusions

- No header, `src/`, CMake build or version change (the package rule is implemented by R10-007, `1.0.0` is set by R10-009); no new public API; no deprecation or migration procedure (R10-005); no ABI mechanism.

## Dependencies

R10-003 (ACCEPTED).
