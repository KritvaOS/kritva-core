# Kritva Core — Master Task Register

R0.2, R0.3, R0.4 and R0.5 remain recorded as released historical milestones.

## KF-CORE-R05 — Platform Runtime Integration Foundation

| ID | Task | Primary Area | Status | Primary Commit |
|---|---|---|---|---|
| KF-CORE-R05-001 | Platform Context & Service Access Model | platform/context | ACCEPTED | `feat(core): add platform context` |
| KF-CORE-R05-002 | Platform Service Requirement Model | platform/requirements | ACCEPTED | `feat(core): define platform service requirements` |
| KF-CORE-R05-003 | Explicit Platform Service Consumption | platform/services | ACCEPTED | `feat(core): define explicit platform service consumption` |
| KF-CORE-R05-004 | Runtime–Platform Lifecycle Boundary | runtime/platform | ACCEPTED | `feat(core): preserve runtime platform lifecycle boundary` |
| KF-CORE-R05-005 | Reference Platform Integration | tests/platform | ACCEPTED | `test(core): add platform integration reference harness` |
| KF-CORE-R05-006 | Platform Integration & Runtime Tests | tests/integration | ACCEPTED | `test(core): add platform runtime integration tests` |
| KF-CORE-R05-007 | Full R0.5 Validation | integration/validation | ACCEPTED | `test(core): complete R0.5 validation` |

## R05 Dependency Graph

```text
R05-001
   ↓
R05-002
   ↓
R05-003
   ↓
R05-004
   ↓
R05 Platform API Review
   ↓
R05-005
   ↓
R05-006
   ↓
R05 Platform Integration Freeze
   ↓
R05-007
   ↓
R05 Release Gate
```

## KF-CORE-R06 — Component Execution Context (Architecture Proposal)

| ID | Task | Primary Area | Status | Est. Effort |
|---|---|---|---|---:|
| KF-CORE-R06-001 | Component Execution Context & Ownership Model | context | ACCEPTED | 3–4 ED |
| KF-CORE-R06-002 | Operational Context Services & Access Policy | context/services | ACCEPTED | 3–4 ED |
| KF-CORE-R06-003 | Context Injection Without Runtime Lifecycle Change | runtime/component | ACCEPTED | 3–4 ED |
| KF-CORE-R06-004 | Context Requirements & Capability Binding | context/requirements | ACCEPTED | 2–3 ED |
| R06 Component API Review | Freeze public context/API | architecture | PASS / FROZEN | 1 ED |
| KF-CORE-R06-005 | Reference Context Harness & Contract Tests | tests/context | ACCEPTED | 3–4 ED |
| KF-CORE-R06-006 | Runtime/Component Context Integration Tests | tests/integration | REVIEW | 3–4 ED |
| R06 Integration Freeze | Freeze production behavior | architecture | PLANNED | 0.5 ED |
| KF-CORE-R06-007 | Full R0.6 Validation | integration/validation | PLANNED | 2–3 ED |
| R06 Release Gate | Release 0.6.0 | release | PLANNED | 1 ED |

## R06 Dependency Graph

```text
R06-001
   ↓
R06-002
   ↓
R06-003
   ↓
R06-004
   ↓
R06 Component API Review
   ↓
R06-005
   ↓
R06-006
   ↓
R06 Integration Freeze
   ↓
R06-007
   ↓
R06 Release Gate
```

## Acceptance Rule

A task moves to ACCEPTED only after:

1. implementation is complete;
2. required focused unit/contract tests pass;
3. required integration tests pass;
4. the complete existing regression suite passes;
5. required quality checks pass;
6. objective evidence is supplied;
7. independent review passes.

Passing new tests alone is not sufficient.

API freeze gates are mandatory.
