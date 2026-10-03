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

Status: PASS (05-10-2026)

## Reviewer Decision

Only the independent architect/reviewer records the final gate decision.

| Gate | Decision | Release Commit | Tag | Reviewer | Date |
|---|---|---|---|---|---|
| R06 Release Gate | **PASS** | the commit carrying this record (release-record commit; resolve with `git show kritva-core-r0.6`) | `kritva-core-r0.6` (annotated, on the release-record commit; created locally, push pending) | ChatGPT | 05-10-2026 |

## Recorded release

- Release: Kritva Core R0.6 — Component Execution Context
- Version: `0.6.0` (`VERSION` and the CMake project version agree; enforced by the traceability audit)
- Release candidate: `b473e5d` (fresh-clone validation; validation record `7351db6`; R06-007 accepted at `4e7bd60`)
- Scope: R0.6 provides the non-owning `runtime::ComponentContext`; it does not change the Runtime lifecycle, replace `PlatformContext`, add a service registry/locator, or implement a concrete platform.
- Gates: R06 Component API Review PASS / FROZEN (`06207c7`); R06 Integration Freeze PASS / HONORED (`769ac8d`, production freeze point `5b755af`); final validation PASS (R06-007); Release Gate PASS.
- Tasks accepted: R06-001 `8031c47`, R06-002 `072b713`, R06-003 `adb0e08`, R06-004 `5b755af`, R06-005 `c7f7b46`, R06-006 `2fd5424`, R06-007 `7351db6`.
- Testing rule honored: unit/contract tests of the R0.6 contracts (R06-001..005), integration tests through public APIs (R06-006), and the complete regression suite (45 CTest groups, R0.1–R0.5 and R0.6) all pass; the release does not rest on the new tests alone.
- Decision: **PASS**
- Release tag: `kritva-core-r0.6`, annotated, on the documentation-only release-record commit that records this gate (not on `b473e5d`). That commit changes only release-state documentation (this record, `MILESTONE_STATUS.md`, the milestone files and the planning `CHANGELOG.md`); no implementation, API, behavior, lint or formatting change.
- Push: to be performed by the user; after publication the remote branch, the annotated tag object and the peeled tag commit are verified and recorded.

Deferred as post-R0.6 work or documented caveats (none block the release): `make lint` and `make format-check` tooling (still TODO stubs), the 32-bit scheduler CPU affinity mask, the conformance suite level-2 mutation strictness gap, a `ComponentContext` outliving what it refers to (documented undefined behavior; non-owning by design), the unused `<chrono>` include in `types/duration.hpp`, the stale root `implementation.md`, and the absence of any concrete platform adapter (outside Core by design).
