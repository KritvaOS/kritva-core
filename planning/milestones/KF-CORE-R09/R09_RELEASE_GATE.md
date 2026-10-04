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

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS / RELEASE AUTHORIZED** |
| Version | 0.9.0 |
| Release candidate | `ef14e99` (R09-007 accepted at `683a3ce`, acceptance record `64ff580`) |
| Production freeze | `4c86b53`; `git diff 4c86b53 HEAD -- include src` empty |
| Entry criteria | R09-001..R09-007 ACCEPTED; Capability API Review PASS / FROZEN; Integration Freeze PASS / HONORED; Security Architecture Review PASS (DOCUMENTATION ONLY) |
| Validation | Fresh-clone validation of `ef14e99`: 65/65 in Debug, Release, ASan+UBSan, TSan and strict `-Werror`; `-fanalyzer` clean; traceability 108 / 107 / 0; API documentation audit 0 errors (15 indexed, 6 maintained, 9 stubs); install consumer PASS |
| Open blockers | 0 |
| Date | 05-10-2026 |

**Reviewer Decision: PASS — R0.9 is authorized for release.**

## Release Record

| Item | Value |
|---|---|
| Release commit | the docs-only release-record commit `docs(release): record Kritva Core R0.9 release gate` |
| Tag | `kritva-core-r0.9` (annotated, on the release-record commit) |
| Candidate-to-release diff | `git diff ef14e99 HEAD -- include src tests CMakeLists.txt VERSION` empty |
| Remote verification | PENDING — owner push, then independent remote audit; final state recorded in a follow-up documentation-only commit |
