# KF-CORE-R04 — Release Gate

## Purpose

Authorize the Kritva Core R0.4 release only after all R0.4 tasks and gates have passed.

## Entry Criteria

- R04-001 through R04-008 accepted.
- Platform API Review PASS / FROZEN.
- Platform Integration Freeze PASS / HONORED.
- Final validation PASS.
- No unresolved production API changes.
- Requirements/API/documentation reconciled.

## Release Validation

- version is 0.4.0;
- `VERSION` and CMake project version agree;
- clean Debug build;
- clean Release build;
- full CTest;
- ASan/UBSan;
- TSan where configured;
- `-Werror`;
- GCC `-fanalyzer`;
- coverage reviewed;
- traceability zero errors;
- install-consumer green;
- prohibited dependency scan green;
- platform-independent production sources verified;
- working tree clean.

## Release Target

Annotated tag:

`kritva-core-r0.4`

Tag creation is authorized only after the reviewer records PASS.

## Decision

Status: PLANNED

Possible outcomes:
- PASS
- CHANGES REQUIRED
- BLOCKED

Reviewer: ChatGPT architecture/review gate.
