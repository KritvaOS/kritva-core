# Kritva Core Planning

This directory is the authoritative planning and execution control for Kritva Core.

## Roles

- ChatGPT: architecture definition, independent review, acceptance gate.
- Codex/Claude: implementation, tests, evidence, Git commit.

## Hierarchy

```text
planning/
├── MILESTONE.md
├── MILESTONE_STATUS.md
├── TASKS.md
├── REVIEW_CHECKLIST.md
├── GIT_COMMIT_STEP.md
├── CHANGELOG.md
└── milestones/
    ├── KF-CORE-R02/
    │   └── tasks/
    └── KF-CORE-R03/
        ├── MILESTONE.md
        ├── IMPLEMENTATION_SEQUENCE.md
        ├── REQUIREMENTS_PROPOSAL.md
        ├── R03_FOUNDATION_API_REVIEW.md
        ├── R03_FOUNDATION_API_REVIEW_EVIDENCE.md
        ├── R03_RUNTIME_CONTRACT_REVIEW.md
        ├── R03_INTEGRATION_FREEZE.md
        ├── R03_RELEASE_GATE.md
        └── tasks/
            ├── KF-CORE-R03-001/
            │   ├── TASK.md
            │   └── ACCEPTANCE_CRITERIA.md
            ├── KF-CORE-R03-002/
            │   ├── TASK.md
            │   └── ACCEPTANCE_CRITERIA.md
            ├── KF-CORE-R03-003/
            ├── KF-CORE-R03-004/
            ├── KF-CORE-R03-005/
            ├── KF-CORE-R03-006/
            ├── KF-CORE-R03-007/
            │   ├── TASK.md
            │   └── ACCEPTANCE_CRITERIA.md
            └── KF-CORE-R03-008/
                ├── TASK.md
                └── ACCEPTANCE_CRITERIA.md
```

## Execution Flow

Milestone → Task → Acceptance Criteria → Implementation → Unit Tests → Integration/Regression Tests → Validation Evidence → Git Commit → Independent Review → Task Acceptance → API Freeze Gate → Next Task → Milestone Validation → Release Tag.

## R03 API Freeze Gates

- After R03-003: Foundation API Review.
- After R03-006: Runtime Contract Review.
- After R03-007: Integration Freeze.
- After R03-008: Release Gate.

Project-level `TASKS.md` and `REVIEW_CHECKLIST.md` are authoritative. Do not create duplicate milestone-level versions of those files.


## Current R03 State

The R03 Foundation API Review has passed. R03-001 through R03-006 are accepted. The next gate is the R03 Runtime Contract Review; R03-007 is blocked until that gate passes, and R03-008 is blocked by the Integration Freeze. See `milestones/KF-CORE-R03/` for the authoritative milestone planning package.
