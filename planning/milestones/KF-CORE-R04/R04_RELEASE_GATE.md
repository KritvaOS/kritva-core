# KF-CORE-R04 — Release Gate

## Purpose

Authorize the Kritva Core R0.4 release only after all R0.4 tasks and gates have passed.

> R0.4 establishes platform contracts and integration boundaries; it does not implement a concrete Linux, RTOS, MCU, vendor, Nexus, or Edge platform adapter.

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

Status: PASS (04-10-2026)

Possible outcomes:
- PASS
- CHANGES REQUIRED
- BLOCKED

Reviewer: ChatGPT architecture/review gate.

## Reviewer Decision

Only the independent architect/reviewer records the final gate decision.

| Gate | Decision | Release Commit | Tag | Reviewer | Date |
|---|---|---|---|---|---|
| R04 Release Gate | **PASS** | the commit carrying this record (release-record commit; resolve with `git show kritva-core-r0.4`) | `kritva-core-r0.4` (annotated, on the release-record commit; created locally, push pending) | ChatGPT | 04-10-2026 |

## Recorded release

- Release: Kritva Core R0.4 — Platform Abstraction
- Version: `0.4.0` (`VERSION` and the CMake project version agree; enforced by the traceability audit)
- Release candidate: `e7df87c` (fresh-clone validation; validation record `f0669eb`; R04-008 accepted at `37fdc1a`)
- Scope: R0.4 establishes platform contracts and integration boundaries; it does not implement a concrete Linux, RTOS, MCU, vendor, Nexus, or Edge platform adapter.
- Gates: R04 Platform API Review PASS / FROZEN (`380ade3`); R04 Platform Integration Freeze PASS / HONORED (`84046b0`, production freeze point `f7231c1`); final validation PASS (R04-008).
- Tasks accepted: R04-001 `d1c5f13`, R04-002 `eb06fa0`, R04-003 `67114bb`, R04-004 `4af4756`, R04-005 `f7231c1`, R04-006 `460de87`, R04-007 `36c5cb8`, R04-008 `f0669eb`.
- Decision: **PASS**
- Release tag: `kritva-core-r0.4`, annotated, on the documentation-only release-record commit that records this gate (not on `e7df87c`). That commit changes only release-state documentation (this record, `MILESTONE_STATUS.md`, the milestone files and the planning `CHANGELOG.md`); no implementation, API, behavior, lint or formatting change.
- Push: to be performed by the user; after publication the remote branch and the annotated tag object are verified and recorded.

Deferred as post-R0.4 work or documented caveats (none block the release): `make lint` and `make format-check` tooling (still TODO stubs), the 32-bit scheduler CPU affinity mask, the single-check mutation strictness gap of the conformance suite, the unused `<chrono>` include in `types/duration.hpp`, the stale root `implementation.md`, and the absence of any concrete platform adapter (outside Core by design).
