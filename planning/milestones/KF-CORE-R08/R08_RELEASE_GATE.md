# R08 Release Gate

## Gate

PASS required before creating `kritva-core-r0.8`.

## Required Evidence

- R08-001 through R08-007 accepted.
- Configuration API Review PASS/FROZEN.
- Integration Freeze PASS/HONORED.
- Fresh-clone validation PASS.
- Debug and Release builds pass with no warnings.
- Full CTest regression passes.
- ASan/UBSan/TSan validation passes where configured.
- Strict warning build and GCC `-fanalyzer` pass.
- Coverage meets the R08 policy.
- Traceability audit passes.
- Public-header self-containment passes.
- Install-consumer validation passes.
- Production isolation and dependency audits pass.
- Version `0.8.0` metadata is internally consistent.
- Release record is documentation-only.
- Release tag is created only after PASS.
- Remote `main`, annotated tag object and peeled tag are independently verified.

Status: PASS (05-10-2026)

## Reviewer Decision

Only the independent architect/reviewer records the final gate decision.

| Gate | Decision | Release Commit | Tag | Reviewer | Date |
|---|---|---|---|---|---|
| R08 Release Gate | **PASS** | the commit carrying this record (release-record commit; resolve with `git show kritva-core-r0.8`) | `kritva-core-r0.8` (annotated, on the release-record commit; created locally, push pending) | ChatGPT | 05-10-2026 |

The reviewer qualified the decision: PASS authorizes release creation; the milestone is not RELEASED / SYNCHRONIZED / CLOSED until the owner pushes `main` and the annotated tag and the remote branch, tag object and peeled tag are independently verified.

## Recorded release

- Release: Kritva Core R0.8 — Component Configuration Foundation
- Version: `0.8.0` (`VERSION` and the CMake project version agree; enforced by the traceability audit)
- Release candidate: `1e7ba2b` (fresh-clone validation; validation record `1aa3611`; R08-007 accepted at `35b648c`)
- Scope: R0.8 hardens the existing configuration path into a normative contract (lifecycle eligibility, ownership and detachment, atomic application, validation boundary, `ConfigurationVersion` as schema/contract compatibility version, Runtime forwarding and failure isolation) with **no production type, signature or behavior change**; it adds no dynamic reconfiguration, parameter service, persistence, event or background activity and implements no concrete platform.
- Gates: R08 Configuration API Review PASS / FROZEN (`e8f8a16`, evidence `0a73b5a`, production freeze baseline `bdb4b93`); R08 Integration Freeze PASS / HONORED (`c358e76`, evidence `e1051a0`); final validation PASS (R08-007); Release Gate PASS.
- Tasks accepted: R08-001 `605516b`, R08-002 `6efeaac`, R08-003 `bdb4b93`, R08-004 `f4b6de4`, R08-005 `a5dfbc1`, R08-006 `94fad8e`, R08-007 `1aa3611`.
- Testing rule honored: unit/contract tests of the R0.8 contracts (R08-001..004, 006), the Runtime/component configuration integration test (R08-005; seeded differential of 200 seeds × 40 steps with per-step forwarding facts) and the complete regression suite (57 CTest groups, R0.1–R0.7 and R0.8) all pass; the release does not rest on the new tests alone.
- Decision: **PASS**
- Release tag: `kritva-core-r0.8`, annotated, on the documentation-only release-record commit that records this gate (not on `1e7ba2b`). That commit changes only release-state documentation (this record, `MILESTONE_STATUS.md`, the milestone files, the planning `CHANGELOG.md`, `MASTER_TRACKER.md`); no implementation, API, behavior, lint or formatting change.
- Push: to be performed by the user; after publication the remote branch, the annotated tag object and the peeled tag commit are verified and recorded.

Deferred as post-R0.8 work or documented caveats (none block the release): `make lint` and `make format-check` tooling (still TODO stubs), the 32-bit scheduler CPU affinity mask, the conformance suite level-2 mutation strictness gap, a `ComponentContext`, `ComponentEventReporter`, statistics provider or retained configuration pointer outliving what it refers to (documented undefined behavior; non-owning by design), the unused `<chrono>` include in `types/duration.hpp`, the stale root `implementation.md`, and the absence of any concrete platform adapter (outside Core by design).
