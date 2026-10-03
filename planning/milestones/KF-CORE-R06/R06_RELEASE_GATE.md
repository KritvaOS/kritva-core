# KF-CORE-R06 Release Gate

## Purpose

Independent final release decision for Kritva Core R0.6 / version 0.6.0.

## Entry criteria

- R06-001..R06-007 accepted
- Component API Review PASS/FROZEN
- Integration Freeze PASS/HONORED
- fresh-clone validation passed
- version and CMake metadata agree
- full test/quality matrix passes
- traceability is clean
- install consumer is green
- dependency/isolation audits are clean
- documentation reconciled
- working tree clean

## Release procedure

1. Independent reviewer records PASS.
2. Documentation-only release-record commit is created.
3. Annotated `kritva-core-r0.6` tag is created on the release-record commit, not the validation candidate.
4. Main and tag are pushed.
5. Remote branch, tag object and peeled tag are independently verified.
6. Post-release documentation records RELEASED / SYNCHRONIZED / CLOSED.

## Decision

PASS / CHANGES REQUIRED / BLOCKED
