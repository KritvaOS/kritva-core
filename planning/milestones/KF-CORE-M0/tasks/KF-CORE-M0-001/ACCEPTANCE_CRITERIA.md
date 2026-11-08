# KF-CORE-M0-001 — Acceptance Criteria

Status: ACCEPTED (owner accepted 2026-11-08 under the interim reviewer roles of D-047, not independent review; criteria drafted from kritva-program `v2/planning/M0_TASK_DEFINITIONS.md`; central approval pending)

## Acceptance Criteria

1. The Core engineering contract `docs/development/ENGINEERING_CONTRACT.md` is drafted and approved by the owner (kritva-program D-113).
2. The pin file `docs/development/ENGINEERING_CONTRACT.pin.json` records the contract's exact commit SHA (`71a82e9b6e2ec3c6155270391adaa30e7751c1b9`) and SHA-256 (`b57e24ba9771b0d156a352ed88aeb0462123af3bf14e0b23b9ecb18a07127021`).
3. The workflow `contract-pin-check` fails when the contract and the pin differ, and it passes on the approved commit.
4. The program readiness check `contract_pin` passes.
5. Evidence is a merged pull request in this repository and a passing CI run on the merged commit, recorded in `status/evidence.json` once the task is complete.
6. No other repository's records are changed by this task.

## Authority

This file states criteria only. It does not accept the task. Acceptance, approval and credit are decided in kritva-program.
