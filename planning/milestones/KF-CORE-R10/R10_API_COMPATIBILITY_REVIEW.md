# KF-CORE-R10 — API / Compatibility Review

## Status

PLANNED — gate record; decision pending R10-001 through R10-005 evidence.

## Purpose

Freeze the approved Core 1.x compatibility, API classification, ABI posture, versioning and deprecation semantics before compatibility harness/package integration becomes the release implementation baseline.

## Entry Criteria

- R10-001 accepted.
- R10-002 accepted.
- R10-003 accepted.
- R10-004 accepted.
- R10-005 accepted.
- Public API inventory reconciled with installed headers.
- Compatibility policy documentation synchronized.
- Root requirements and traceability reconciled.
- Security-impact assessments recorded for all applicable tasks.

## Review Questions

1. Is every public installed API item classified?
2. Are source and semantic compatibility boundaries explicit?
3. Is ABI scope explicit and conservative enough for supported C++ toolchains?
4. Are enum/ErrorCode/virtual-interface/ownership changes classified?
5. Are SemVer rules unambiguous?
6. Is deprecation/removal behavior explicit?
7. Is CMake package/version-selection policy testable?
8. Are migration obligations explicit for intentional incompatibilities?
9. Does the policy preserve Core platform independence?
10. Does the policy introduce any unapproved security guarantee?

## Decision

To be completed after independent architecture review.
