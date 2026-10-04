# KF-CORE-R09 — Release Gate

## Status

PLANNED — release gate record to be completed after R09-007.

## Release Candidate

- Version: `0.9.0`
- Tag: `kritva-core-r0.9`
- Candidate commit: to be recorded at gate
- Release-record commit: documentation-only commit created after PASS

## Entry Criteria

- All R09 implementation tasks accepted.
- R09 Capability API Review PASS / FROZEN.
- R09 Integration Freeze PASS / HONORED.
- Full validation PASS.
- API documentation synchronized.
- Security architecture review complete.
- Traceability reconciled.
- VERSION and build metadata consistent.
- Release documentation reconciled.

## Required Validation

- Fresh-clone Debug build.
- Fresh-clone Release build.
- Full CTest regression.
- ASan + UBSan.
- TSan using documented ASLR-disabled environment where required.
- Strict warning build with `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`.
- GCC `-fanalyzer`.
- Public-header self-containment.
- Coverage review against R0.8 baseline.
- Traceability audit.
- Dependency/prohibited-header audit.
- Production isolation audit.
- Install-consumer test.
- API documentation consistency audit.
- Security boundary/assumption audit.
- `git diff --check`.
- Clean working tree.

## Release Procedure

1. Complete independent R09 Release Gate review.
2. Commit release documentation as a documentation-only release-record commit.
3. Create annotated `kritva-core-r0.9` tag on the release-record commit only after PASS.
4. Push `main` and tag.
5. Independently verify remote `main`, tag object and peeled tag.
6. Reconcile all planning documents to `RELEASED / SYNCHRONIZED / CLOSED`.

## Decision

To be completed by independent reviewer:

**PASS / RELEASED** or **CHANGES REQUIRED** or **BLOCKED**
