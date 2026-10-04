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
    ├── KF-CORE-R06/
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

R0.8 is released and closed:

- `kritva-core-r0.8`
- version `0.8.0`
- release-record commit `cbbec81`
- remote verification: PASS / RELEASED / SYNCHRONIZED / CLOSED

R0.2 to R0.7 are also released and closed. R0.8 hardened the existing configuration path into a normative contract (lifecycle eligibility, ownership, atomic application, validation boundary, `ConfigurationVersion` as schema compatibility version, Runtime forwarding and failure isolation) with no production type, signature or behavior change, and `kritva-core` still platform independent.

See `MASTER_TRACKER.md` and `milestones/KF-CORE-R08/`.

