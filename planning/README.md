# Kritva Core Planning

This directory is the authoritative planning and execution control for Kritva Core.

## Roles

- ChatGPT: architecture definition, independent review, acceptance gate.
- Codex/Claude: implementation, tests, evidence, Git commit.

## Hierarchy

```text
planning/
├── MASTER_TRACKER.md
├── MILESTONE.md
├── MILESTONE_STATUS.md
├── TASKS.md
├── REVIEW_CHECKLIST.md
├── GIT_COMMIT_STEP.md
├── CHANGELOG.md
└── milestones/
    ├── KF-CORE-R02/
    │   ├── IMPLEMENTATION_SEQUENCE.md
    │   └── tasks/
    ├── KF-CORE-R03/
    │   ├── MILESTONE.md
    │   ├── IMPLEMENTATION_SEQUENCE.md
    │   ├── REQUIREMENTS_PROPOSAL.md
    │   ├── gate records and evidence
    │   └── tasks/
    ├── KF-CORE-R04/
    │   ├── MILESTONE.md
    │   ├── IMPLEMENTATION_SEQUENCE.md
    │   ├── REQUIREMENTS_PROPOSAL.md
    │   ├── gate records and evidence
    │   └── tasks/
    ├── KF-CORE-R05/
    │   ├── MILESTONE.md
    │   ├── IMPLEMENTATION_SEQUENCE.md
    │   ├── REQUIREMENTS_PROPOSAL.md
    │   ├── design/API/gate records
    │   └── tasks/
    └── KF-CORE-R06/
        ├── MILESTONE.md
        ├── IMPLEMENTATION_SEQUENCE.md
        ├── REQUIREMENTS_PROPOSAL.md
        ├── R06_DESIGN_DECISIONS.md
        ├── R06_COMPONENT_API_REVIEW.md
        ├── R06_INTEGRATION_FREEZE.md
        ├── R06_RELEASE_GATE.md
        ├── R06_PLANNING_NOTES.md
        ├── R06_TESTING_AND_COMMIT_POLICY.md
        └── tasks/
```

## Authority

`MASTER_TRACKER.md` is the consolidated program/effort/status view.

Project-level `TASKS.md` and `REVIEW_CHECKLIST.md` define common execution and quality policy.

Task-level `TASK.md` and `ACCEPTANCE_CRITERIA.md` define the task scope and acceptance evidence.

Gate records define milestone-level API freeze, integration freeze and release decisions.

Do not create duplicate milestone-level `TASKS.md` or `REVIEW_CHECKLIST.md` files.

## Execution Flow

Milestone → Task → Acceptance Criteria → Implementation → Unit Tests → Integration/Regression Tests → Validation Evidence → Git Commit → Independent Review → Task Acceptance → API Freeze Gate → Next Task → Milestone Validation → Release Gate → Release Tag → Remote Verification.

## Current State

R0.5 is released and closed:

- `kritva-core-r0.5`
- version `0.5.0`
- release-record commit `adf8ac2`
- remote verification: PASS / RELEASED / SYNCHRONIZED / CLOSED

R0.2, R0.3 and R0.4 are also released and closed.

R0.4 established platform contracts and integration boundaries. R0.5 added the platform runtime integration foundation (`PlatformContext`, requirements and explicit service consumption), while keeping `kritva-core` platform independent.

## R0.6 Planning State

R0.6 — Component Execution Context is currently an **architecture proposal**.

- Proposed version: `0.6.0`
- Implementation status: NOT STARTED
- Estimated effort: `20–27` engineering-days
- Next gate: R06 architecture/design review
- No R0.6 production API is authorized yet.
- R0.5 API and Runtime/platform boundaries remain authoritative.

See `MASTER_TRACKER.md` and `milestones/KF-CORE-R06/`.
