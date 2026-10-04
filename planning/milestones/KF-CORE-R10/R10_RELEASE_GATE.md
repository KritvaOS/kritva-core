# KF-CORE-R10 — Release Gate

## Status

PLANNED — release gate record to be completed after R10-009.

## Release Target

- Version: `1.0.0`
- Tag: `kritva-core-r1.0`
- Release candidate: to be recorded at gate
- Release-record commit: documentation-only commit created after PASS

## Entry Criteria

- All R10 implementation tasks accepted.
- R10 API / Compatibility Review PASS / FROZEN.
- R10 Integration Freeze PASS / HONORED.
- Full validation PASS.
- Public API inventory and classification reconciled.
- Compatibility policy reconciled into canonical documentation.
- Version/package policy verified by install-consumer tests.
- Security review complete.
- Requirements traceability clean.
- Release documentation reconciled.

## Required Validation

- Fresh-clone Debug build.
- Fresh-clone Release build.
- Full CTest regression.
- ASan + UBSan.
- TSan where configured.
- Strict warning build with `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`.
- GCC `-fanalyzer`.
- Public-header self-containment.
- Coverage review against the R0.9 baseline.
- Requirements traceability audit.
- API documentation audit.
- Dependency/prohibited-header audit.
- Production isolation audit.
- Install-consumer/package compatibility test.
- Compatibility/boundary test suite.
- Security boundary/assumption audit.
- `git diff --check`.
- Clean working tree.

## Release Procedure

1. Complete independent R10 Release Gate review.
2. Commit release documentation as a documentation-only release-record commit.
3. Create annotated `kritva-core-r1.0` tag on the release-record commit only after PASS.
4. Push `main` and tag.
5. Independently verify remote `main`, tag object and peeled tag.
6. Reconcile all planning documents to `RELEASED / SYNCHRONIZED / CLOSED`.

## Decision

Pending final R1.0 evidence and independent review.
