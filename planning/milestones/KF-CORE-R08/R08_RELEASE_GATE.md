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

## Release Record

Release version: `0.8.0`

Release candidate: `<commit>`

Release-record commit: `<commit>`

Tag: `kritva-core-r0.8`

Tag object: `<annotated-tag-object>`

Remote `main`: `<commit>`

Remote tag: `<annotated-tag-object>`

Peeled tag: `<release-record-commit>`

## Decision

`PASS / RELEASED / SYNCHRONIZED / CLOSED` only after all evidence is verified.
