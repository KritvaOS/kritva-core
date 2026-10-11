# Core status report

`status.json` is this repository's machine-readable status report for the kritva-program consolidator (schema 2.0, vendored as `status.schema.json`; SHA-256 in `status.schema.json.sha256`, verified in CI).

It is generated, never hand-edited:

```
python3 scripts/status/gen_status.py            # write status/status.json
python3 scripts/status/gen_status.py --check    # local: full comparison
python3 scripts/status/gen_status.py --check-facts   # CI: planning-derived facts only
python3 -m unittest scripts/status/test_gen_status.py
```

Scope is the nine KF-CORE-R10 tasks (Core R1.0). Earlier milestones are not reported; including them in the program baseline is a central decision.

Authority: this repo reports facts only. `execution=COMPLETE` means the planning records list the task as ACCEPTED/RELEASED; it is not central acceptance. Evidence entries are criteria references with null commit, CI run and hash, so they earn no progress credit. Review, acceptance, weights and approvals live in kritva-program (`v2/tracking/`). `source.commit` is the inspected revision (not the commit containing the report) and is expected to trail HEAD after later commits. The canonical schema is `schema/status.schema.json` in kritva-program (canonical commit: record here once chosen).
