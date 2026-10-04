# KF-CORE-R09-007 — Full R0.9 Validation & Release Candidate

## Status

PLANNED — implementation not started.

## Objective

Run the complete R0.9 quality matrix, verify the release boundary and prepare the 0.9.0 release candidate.

## Scope

- Fresh-clone validation.
- Debug/Release builds.
- Full CTest.
- ASan/UBSan.
- TSan.
- Strict Werror.
- GCC analyzer.
- Header self-containment.
- Coverage and traceability.
- Install consumer.
- Documentation/security reconciliation.
- Production freeze and release-candidate audit.

## Out of Scope
- New feature implementation outside approved R09 scope.

## Dependencies

R09-006 accepted

## Requirement Traceability

CORE-CAP-011

## Implementation Guidance

Use the smallest public surface necessary. Reuse accepted R0.8 capability, platform, Runtime and lifecycle contracts. Any proposed public API addition must be explicitly identified, documented, security-reviewed and routed through the R09 Capability API Review.
