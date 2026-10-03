# Kritva Core Planning Package — R0.7 Architecture-Confirmed

This archive preserves the supplied planning history through R0.6 and adds the R0.7 architecture-confirmed planning package.

## Common planning files

- `MASTER_TRACKER.md` — consolidated milestone/task/effort/status view
- `MILESTONE.md` — master milestone roadmap
- `MILESTONE_STATUS.md` — consolidated milestone and release status
- `TASKS.md` — single authoritative project-level task register
- `REVIEW_CHECKLIST.md` — common and R0.7 quality/review gates
- `GIT_COMMIT_STEP.md` — project workflow, release process and R0.7 commit messages
- `CHANGELOG.md` — milestone planning/release history
- `README.md` — planning hierarchy and current state

## R0.7 planning state

R0.6 is RELEASED/CLOSED at version 0.6.0.

R0.7 — `KF-CORE-R07 Component Operational Foundation` — is **PLANNED / ARCHITECTURE CONFIRMED / IMPLEMENTATION NOT STARTED**.

The R0.7 Design Consult and Scope Confirmation are approved. Implementation begins with `KF-CORE-R07-001` after its task-specific `TASK.md` and `ACCEPTANCE_CRITERIA.md` are reviewed and issued.

## R0.7 architecture boundary

R0.7 establishes a narrow Component operational observation/reporting contract using existing Core concepts. It does not introduce a new operational state machine, Core EventBus, telemetry/logging backend, background worker, automatic recovery, automatic restart/retry, or platform-specific operational framework. Runtime remains lifecycle authority; Components remain authoritative for their operational information; integrators own observation sinks, telemetry and policy.

R0.7 does not make Component statistics mandatory on the base Component interface.
