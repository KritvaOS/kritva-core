# R07 Release Gate

## Gate

PASS required before creating `kritva-core-r0.7`.

## Required Evidence

- R07-001 through R07-007 accepted.
- API Review PASS/FROZEN.
- Integration Freeze PASS/HONORED.
- Fresh-clone validation PASS.
- Debug and Release builds pass.
- Full CTest passes.
- ASan/UBSan/TSan validation passes where configured.
- Strict warnings and GCC analyzer pass.
- Coverage and traceability pass.
- Install-consumer passes.
- Production dependency/isolation audit passes.
- Working tree clean.
- Documentation reconciled.

Status: PASS (05-10-2026)

## Reviewer Decision

Only the independent architect/reviewer records the final gate decision.

| Gate | Decision | Release Commit | Tag | Reviewer | Date |
|---|---|---|---|---|---|
| R07 Release Gate | **PASS** | the commit carrying this record (release-record commit; resolve with `git show kritva-core-r0.7`) | `kritva-core-r0.7` (annotated, on the release-record commit; created locally, push pending) | ChatGPT | 05-10-2026 |

The reviewer qualified the decision: PASS authorizes release creation; the milestone is not RELEASED / SYNCHRONIZED / CLOSED until the owner pushes `main` and the annotated tag and the remote branch, tag object and peeled tag are independently verified.

## Recorded release

- Release: Kritva Core R0.7 — Component Operational Foundation
- Version: `0.7.0` (`VERSION` and the CMake project version agree; enforced by the traceability audit)
- Release candidate: `d83e1ba` (fresh-clone validation; validation record `86dfcb9`; R07-007 accepted at `0dae055`)
- Scope: R0.7 establishes read-only Component operational observation (`observe()`), Component-owned Status/Health/optional statistics semantics and explicit Event reporting to an integrator-owned sink, without a new state machine, event bus, telemetry or logging backend, Runtime polling or health-driven recovery; it does not implement a concrete platform.
- Gates: R07 Component Operational API Review PASS / FROZEN (`5021172`, evidence `6b1296f`); R07 Integration Freeze PASS / HONORED (`2cdaad9`, evidence `142a32e`, production freeze point `16654e9`); final validation PASS (R07-007); Release Gate PASS.
- Tasks accepted: R07-001 `6849a73`, R07-002 `61e0067`, R07-003 `56ff226`, R07-004 `16654e9`, R07-005 `0b1bd1d`, R07-006 `3f524cd`, R07-007 `86dfcb9`.
- Testing rule honored: unit/contract tests of the R0.7 contracts (R07-001..005), the Runtime/component operational integration test (R07-006; seeded differential of 150 seeds × 40 steps × six operational variants) and the complete regression suite (51 CTest groups, R0.1–R0.6 and R0.7) all pass; the release does not rest on the new tests alone.
- Decision: **PASS**
- Release tag: `kritva-core-r0.7`, annotated, on the documentation-only release-record commit that records this gate (not on `d83e1ba`). That commit changes only release-state documentation (this record, `MILESTONE_STATUS.md`, the milestone files, the planning `CHANGELOG.md`, `MASTER_TRACKER.md`); no implementation, API, behavior, lint or formatting change.
- Push: to be performed by the user; after publication the remote branch, the annotated tag object and the peeled tag commit are verified and recorded.

Deferred as post-R0.7 work or documented caveats (none block the release): `make lint` and `make format-check` tooling (still TODO stubs), the 32-bit scheduler CPU affinity mask, the conformance suite level-2 mutation strictness gap, a `ComponentContext`, `ComponentEventReporter` or statistics provider outliving what it refers to (documented undefined behavior; non-owning by design), the unused `<chrono>` include in `types/duration.hpp`, the stale root `implementation.md`, and the absence of any concrete platform adapter (outside Core by design).
