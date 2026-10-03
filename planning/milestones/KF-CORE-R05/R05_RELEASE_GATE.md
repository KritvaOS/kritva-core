# KF-CORE-R05 — Release Gate

## Purpose

Authorize the Kritva Core R0.5 release only after all R0.5 tasks and gates pass.

> R0.5 establishes the platform runtime integration foundation. It does not implement a concrete Linux, RTOS, MCU, vendor, Nexus or Edge platform.

## Entry Criteria

- R05-001 through R05-007 accepted.
- Platform API Review PASS / FROZEN.
- Platform Integration Freeze PASS / HONORED.
- No unresolved production API changes.
- Requirements, API and planning documentation reconciled.

## Release Validation

- version is `0.5.0`;
- `VERSION` and CMake project version agree;
- clean Debug build;
- clean Release build;
- full CTest regression;
- ASan/UBSan;
- TSan where configured;
- `-Werror`;
- GCC `-fanalyzer` where configured;
- coverage reviewed;
- traceability zero errors;
- install-consumer green;
- prohibited dependency scan green;
- platform-independent production sources verified;
- no Core-owned background execution;
- no implicit platform-service lifecycle;
- working tree clean.

## Mandatory Testing Rule

The release gate must distinguish:

1. **Unit tests** — focused contract correctness.
2. **Integration tests** — Runtime/platform behavior using public APIs and reference/fake services.
3. **Regression tests** — the complete existing suite, demonstrating that R0.3/R0.4 behavior remains intact.

A release cannot PASS when only the new R0.5 tests pass.

## Release Target

Annotated tag:

`kritva-core-r0.5`

Tag creation is authorized only after the independent reviewer records PASS.

## Decision

Possible outcomes:

- PASS
- CHANGES REQUIRED
- BLOCKED

Reviewer: ChatGPT architecture/review gate.

## Release Procedure

1. Confirm all tasks ACCEPTED.
2. Confirm both R0.5 gates PASS.
3. Update milestone/status/changelog records.
4. Run clean release validation from the intended release commit.
5. Verify `git status` is clean.
6. Create annotated tag.
7. Verify tag and peeled commit.
8. Push branch and tag.
9. Verify remote refs with `git ls-remote`.
10. Reconcile release documentation with the actual remote state.

No release tag may be created before the independent Release Gate PASS.
