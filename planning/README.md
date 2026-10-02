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
    └── KF-CORE-R02/
        └── tasks/
            └── <TASK_ID>/
                ├── TASK.md
                └── ACCEPTANCE_CRITERIA.md
```

## Execution Flow

Milestone → Task → Acceptance Criteria → Implementation → New Tests → Regression → Git Commit → Independent Review → Task Acceptance → Next Task → Milestone Validation → Release Tag.
