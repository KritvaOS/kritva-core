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
    ├── KF-CORE-R03/
    │   ├── MILESTONE.md
    │   ├── IMPLEMENTATION_SEQUENCE.md
    │   ├── REQUIREMENTS_PROPOSAL.md
    │   ├── gate records and evidence
    │   └── tasks/
    └── KF-CORE-R04/
        ├── MILESTONE.md
        ├── IMPLEMENTATION_SEQUENCE.md
        ├── REQUIREMENTS_PROPOSAL.md
        ├── R04_PLATFORM_API_REVIEW.md
        ├── R04_PLATFORM_API_REVIEW_EVIDENCE.md
        ├── R04_PLATFORM_INTEGRATION_FREEZE.md
        ├── R04_PLATFORM_INTEGRATION_FREEZE_EVIDENCE.md
        ├── R04_RELEASE_GATE.md
        └── tasks/
            ├── KF-CORE-R04-001/
            ├── KF-CORE-R04-002/
            ├── KF-CORE-R04-003/
            ├── KF-CORE-R04-004/
            ├── KF-CORE-R04-005/
            ├── KF-CORE-R04-006/
            ├── KF-CORE-R04-007/
            └── KF-CORE-R04-008/
```

## Execution Flow

Milestone → Task → Acceptance Criteria → Implementation → Unit Tests → Integration/Regression Tests → Validation Evidence → Git Commit → Independent Review → Task Acceptance → API Freeze Gate → Next Task → Milestone Validation → Release Tag.

## R03 API Freeze Gates

- After R03-003: Foundation API Review.
- After R03-006: Runtime Contract Review.
- After R03-007: Integration Freeze.
- After R03-008: Release Gate.

Project-level `TASKS.md` and `REVIEW_CHECKLIST.md` are authoritative. Do not create duplicate milestone-level versions of those files.


## Current State

R0.5 is released and closed:

- `kritva-core-r0.5`
- version 0.5.0
- release-record commit `adf8ac2`

R0.4 (`kritva-core-r0.4`, `b31108d`), R0.3 (`kritva-core-r0.3`, `cc16ec9`) and earlier milestones are also released and closed.

R0.4 defines platform contracts and integration boundaries and R0.5 adds the platform runtime integration foundation (`PlatformContext`, requirements, explicit service consumption), while keeping `kritva-core` platform independent. Linux, RTOS, vendor BSP/HAL, EtherCAT, ROS2/DDS and hardware-specific implementations remain outside Core.
