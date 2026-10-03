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

## Release Record

Release record must be documentation-only and committed before the annotated `kritva-core-r0.7` tag is created. Remote main, tag object and peeled tag must be independently verified.
