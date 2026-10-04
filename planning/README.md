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
    ├── KF-CORE-R02/ … KF-CORE-R09/
    └── KF-CORE-R10/
        ├── MILESTONE.md
        ├── IMPLEMENTATION_SEQUENCE.md
        ├── REQUIREMENTS_PROPOSAL.md
        ├── R10_DESIGN_CONSULT.md
        ├── R10_SCOPE_CONFIRMATION.md
        ├── R10_DESIGN_DECISIONS.md
        ├── R10_BASELINE_REVIEW.md
        ├── R10_API_COMPATIBILITY_REVIEW.md
        ├── R10_SECURITY_ARCHITECTURE.md
        ├── R10_INTEGRATION_FREEZE.md
        ├── R10_RELEASE_GATE.md
        ├── R10_TESTING_AND_COMMIT_POLICY.md
        ├── TASK_MANIFEST.md
        └── tasks/
            ├── KF-CORE-R10-001/
            ├── …
            └── KF-CORE-R10-009/
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

R0.8 is released and closed (`kritva-core-r0.8`, version `0.8.0`, release-record commit `cbbec81`, remote verification PASS / RELEASED / SYNCHRONIZED / CLOSED). R0.9 is also released and closed:

- `kritva-core-r0.9` (annotated tag on release-record commit `d72343a`; tag object `f77fecb`)
- version `0.9.0`
- release candidate `ef14e99`
- remote verification: PASS / RELEASED / SYNCHRONIZED / CLOSED

R0.2 to R0.8 are also released and closed. R0.8 hardened the existing configuration path into a normative contract (lifecycle eligibility, ownership, atomic application, validation boundary, `ConfigurationVersion` as schema compatibility version, Runtime forwarding and failure isolation) with no production type, signature or behavior change, and `kritva-core` still platform independent.

See `MASTER_TRACKER.md`, `milestones/KF-CORE-R09/` and `milestones/KF-CORE-R10/`.


## R1.0 State

KF-CORE-R10 — Core 1.0 API Maturity & Compatibility Foundation is released and closed:

- `kritva-core-r1.0` (annotated tag on release-record commit `4acce3b`; tag object `fc9b51f`)
- version `1.0.0`
- release candidate `af16847`
- remote verification: PASS / RELEASED / SYNCHRONIZED / CLOSED

R1.0 establishes the source, semantic and ABI compatibility posture (no ABI promise), the public API inventory and classification, versioning, evolution and deprecation policy, package/install version selection, compatibility validation and release governance. It introduces no platform-specific code and no new runtime framework.
