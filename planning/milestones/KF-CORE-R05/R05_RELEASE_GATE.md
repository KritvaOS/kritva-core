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

Status: PASS (05-10-2026)

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

## Reviewer Decision

Only the independent architect/reviewer records the final gate decision.

| Gate | Decision | Release Commit | Tag | Reviewer | Date |
|---|---|---|---|---|---|
| R05 Release Gate | **PASS** | the commit carrying this record (release-record commit; resolve with `git show kritva-core-r0.5`) | `kritva-core-r0.5` (annotated, on the release-record commit; created locally, push pending) | ChatGPT | 05-10-2026 |

## Recorded release

- Release: Kritva Core R0.5 — Platform Runtime Integration Foundation
- Version: `0.5.0` (`VERSION` and the CMake project version agree; enforced by the traceability audit)
- Release candidate: `5fb5e69` (fresh-clone validation; validation record `8384f5d`; R05-007 accepted at `5b68140`)
- Scope: R0.5 establishes the platform runtime integration foundation; it does not implement a concrete Linux, RTOS, MCU, vendor, Nexus or Edge platform.
- Gates: R05 Platform API Review PASS / FROZEN (`05d981e`); R05 Platform Integration Freeze PASS / HONORED (`cf6e617`, production freeze point `f23777b`); final validation PASS (R05-007); Release Gate PASS.
- Tasks accepted: R05-001 `c5910e7`, R05-002 `f9d6007`, R05-003 `f8cd523`, R05-004 `f23777b`, R05-005 `7f30626`, R05-006 `fe04d35`, R05-007 `8384f5d`.
- Testing rule honored: unit/contract tests of the R0.5 contracts, integration tests through public APIs with reference/fake services, and the complete existing regression suite (32 of 39 CTest groups predate R0.5) all pass; the release does not rest on the new tests alone.
- Decision: **PASS**
- Release tag: `kritva-core-r0.5`, annotated, on the documentation-only release-record commit that records this gate (not on `5fb5e69`). That commit changes only release-state documentation (this record, `MILESTONE_STATUS.md`, the milestone files and the planning `CHANGELOG.md`); no implementation, API, behavior, lint or formatting change.
- Push: to be performed by the user; after publication the remote branch, the annotated tag object and the peeled tag commit are verified and recorded.

Deferred as post-R0.5 work or documented caveats (none block the release): `make lint` and `make format-check` tooling (still TODO stubs), the 32-bit scheduler CPU affinity mask, the single-check mutation strictness gap of the conformance suite, a `PlatformContext` outliving its adapter (documented undefined behavior; non-owning by design), the unused `<chrono>` include in `types/duration.hpp`, the stale root `implementation.md`, and the absence of any concrete platform adapter (outside Core by design).
