# Kritva Core — Master Task Register

R0.2, R0.3, R0.4, R0.5 and R0.6 are recorded as released historical milestones. R0.7 is the current planned architecture milestone.

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

## KF-CORE-R06 — Component Execution Context

| ID | Task | Primary Area | Status | Est. Effort |
|---|---|---|---|---:|
| KF-CORE-R06-001 | Component Execution Context & Ownership Model | context | ACCEPTED | 3–4 ED |
| KF-CORE-R06-002 | Operational Context Services & Access Policy | context/services | ACCEPTED | 3–4 ED |
| KF-CORE-R06-003 | Context Injection Without Runtime Lifecycle Change | runtime/component | ACCEPTED | 3–4 ED |
| KF-CORE-R06-004 | Context Requirements & Capability Binding | context/requirements | ACCEPTED | 2–3 ED |
| R06 Component API Review | Freeze public context/API | architecture | PASS / FROZEN | 1 ED |
| KF-CORE-R06-005 | Reference Context Harness & Contract Tests | tests/context | ACCEPTED | 3–4 ED |
| KF-CORE-R06-006 | Runtime/Component Context Integration Tests | tests/integration | ACCEPTED | 3–4 ED |
| R06 Integration Freeze | Freeze production behavior | architecture | PASS / HONORED | 0.5 ED |
| KF-CORE-R06-007 | Full R0.6 Validation | integration/validation | ACCEPTED | 2–3 ED |
| R06 Release Gate | Release 0.6.0 | release | PASS / RELEASED | 1 ED |

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


## KF-CORE-R07 — Component Operational Foundation (Architecture Confirmed)

| ID | Task | Primary Area | Status | Est. Effort |
|---|---|---|---|---:|
| R07 Design Consult | Operational model and architectural boundary | architecture | APPROVED | 2–3 ED |
| R07 Scope Confirmation | Confirm milestone scope and exclusions | architecture | APPROVED | 1 ED |
| KF-CORE-R07-001 | Component Operational Observation Contract | runtime/observation | ACCEPTED (6849a73) | 3–4 ED |
| KF-CORE-R07-002 | Component Status & Health Reporting Contract | status/health | ACCEPTED (61e0067) | 2–3 ED |
| KF-CORE-R07-003 | Component Operational Event Contract | event | ACCEPTED (56ff226) | 3–4 ED |
| KF-CORE-R07-004 | Component Statistics Ownership & Observation Contract | statistics | ACCEPTED (16654e9) | 2–3 ED |
| R07 Component Operational API Review | Freeze public operational API | architecture | PASS / FROZEN (`6b1296f`) | 1 ED |
| KF-CORE-R07-005 | Reference Operational Harness & Contract Tests | tests/operational | ACCEPTED (0b1bd1d) | 3–4 ED |
| KF-CORE-R07-006 | Runtime/Component Operational Integration | tests/integration | ACCEPTED (3f524cd) | 3–4 ED |
| R07 Integration Freeze | Freeze production operational behavior | architecture | PASS / HONORED (`142a32e`) | 0.5 ED |
| KF-CORE-R07-007 | Full R0.7 Validation | integration/validation | ACCEPTED `86dfcb9` (candidate d83e1ba) | 2–3 ED |
| R07 Release Gate | Release 0.7.0 | release | PLANNED | 1 ED |

## R07 Dependency Graph

```text
R07 Design Consult
        ↓
R07 Scope Confirmation
        ↓
R07-001 Component Operational Observation Contract
        ↓
R07-002 Component Status & Health Reporting Contract
        ↓
R07-003 Component Operational Event Contract
        ↓
R07-004 Component Statistics Ownership & Observation Contract
        ↓
R07 Component Operational API Review
        ↓
R07-005 Reference Operational Harness & Contract Tests
        ↓
R07-006 Runtime/Component Operational Integration
        ↓
R07 Integration Freeze
        ↓
R07-007 Full R0.7 Validation
        ↓
R07 Release Gate
```

### R0.7 Scope Decision

R0.7 establishes a narrow, platform-independent Component operational observation/reporting contract using existing Core concepts. It does not introduce a new operational state machine, Core EventBus, telemetry backend, logging backend, background worker, automatic recovery, or platform-specific operational framework. Component operational data remains Component-owned; Runtime remains lifecycle authority; integrator code owns observation sinks, telemetry and policy.

R0.7 does not add `statistics()` to the mandatory `runtime::Component` base interface by default. Component statistics are an optional operational contract defined by R07-004.

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
