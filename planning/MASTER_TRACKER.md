# Kritva Core — Master Program Tracker

## Purpose

`MASTER_TRACKER.md` is the consolidated management view for Kritva Core.

It tracks milestone/phase status, task progress, estimated effort, actual/observed effort when evidenced, dependencies, quality gates, and release state.

This file is **not** an acceptance authority. Task acceptance remains governed by the task-specific `ACCEPTANCE_CRITERIA.md` and independent reviewer decision.

## Program Roles

- **ChatGPT** — architecture definition, independent review, acceptance gate.
- **Claude/Codex** — implementation, tests, objective evidence, Git commits.
- **Git history / repository** — authoritative implementation and release evidence.

## Status Definitions

| Status | Meaning |
|---|---|
| NOT DEFINED | Future milestone/phase has not yet been formally architected |
| PLANNED | Scope and task package approved for implementation |
| IN PROGRESS | Implementation underway |
| REVIEW | Evidence submitted for independent review |
| ACCEPTED | Independent review passed |
| FROZEN | API/production boundary frozen at a formal gate |
| RELEASED | Milestone accepted, tagged and remotely verified |
| BLOCKED | Specific blocker prevents progress |

## Effort Rules

Effort is tracked in **engineering-days (ED)**.

For future milestones:

`Total Estimate = Architecture + Implementation + Tests + Review/Fix + Validation/Release`

Estimated effort is established before implementation. Actual/observed effort is recorded only when supported by repository evidence or explicit engineering records; commit count is not treated as effort.

## Current Dashboard

| Metric | Current state |
|---|---|
| Latest released milestone | **KF-CORE-R08** |
| Latest released version | **0.8.0** |
| Latest release tag | `kritva-core-r0.8` |
| Latest release commit | `cbbec81` |
| Remote verification | **PASS / RELEASED / SYNCHRONIZED / CLOSED** |
| Completed milestones | R0.2, R0.3, R0.4, R0.5, R0.6, R0.7, R0.8 |
| Current active milestone | **KF-CORE-R09 — ACCEPTED / RELEASE GATE PASS / PENDING PUSH AND REMOTE VERIFICATION** |
| R08 implementation status | RELEASED / CLOSED |
| R09 implementation status | IMPLEMENTED / ACCEPTED (7 / 7); Release Gate PASS; not yet pushed |
| Open release blockers | 0 |
| API freeze active | No |
| Concrete platform implementation in `kritva-core` | No |
| R09 production API freeze | Yes — PASS / FROZEN at `4c86b53`; Integration Freeze HONORED |

## Consolidated Milestone Tracker

| Phase | Milestone | Objective | Tasks | Status | Progress | Est. Effort | Dependencies | Quality Gates | Release |
|---|---|---|---:|---|---:|---:|---|---|---|
| Foundation | R0.2 | Core contract hardening | 8 | RELEASED | 8/8 | Historical | R0.1 | Validation + Release Gate | 0.2.0 |
| Runtime | R0.3 | Runtime foundation | 8 | RELEASED | 8/8 | Historical | R0.2 | Foundation API Review + Runtime Contract Review + Integration Freeze + Release Gate | 0.3.0 |
| Platform | R0.4 | Platform abstraction | 8 | RELEASED | 8/8 | Historical | R0.3 | Platform API Review + Integration Freeze + Release Gate | 0.4.0 |
| Platform Runtime | R0.5 | Platform runtime integration foundation | 7 | RELEASED | 7/7 | Historical | R0.4 | Platform API Review + Integration Freeze + Release Gate | 0.5.0 |
| Component Context | R0.6 | Controlled component execution context without changing Runtime lifecycle semantics | 7 | RELEASED | 7/7 | 20–27 ED | R0.5 | Component API Review + Integration Freeze + Validation + Release Gate | 0.6.0 |
| Component Operations | **R0.7** | Controlled Component operational observation/reporting without changing Runtime lifecycle semantics | 7 | RELEASED | 7/7 | 24–32 ED (estimate; actual not recorded) | R0.6 | Design Consult + Scope Confirmation + API Review + Integration Freeze + Validation + Release Gate | 0.7.0 (target) |
| Component Configuration | **R0.8** | Component Configuration foundation | 7 | RELEASED | 7/7 | 22–31 ED (estimate; actual not recorded) | R0.7 released | Design Consult + Scope Confirmation + Configuration API Review + Integration Freeze + Release Gate | 0.8.0 |
| Capability / Readiness Boundary | **R0.9** | Capability Contract & Readiness Boundary | 7 | ACCEPTED (Release Gate PASS; pending push) | 7/7 | 22–30 ED | R0.8 released | Design Consult + Scope Confirmation + Capability API Review + Security Review + Integration Freeze + Validation + Release Gate | 0.9.0 |

## R0.7 Task Tracker

| ID | Task | Est. | Dependency | Status | Gate |
|---|---|---:|---|---|---|
| R07 Design Consult | Operational model and architectural boundary | 2–3 ED | R0.6 released | APPROVED | Scope input |
| R07 Scope Confirmation | Confirm scope and exclusions | 1 ED | Design Consult | APPROVED | Implementation authorization input |
| KF-CORE-R07-001 | Component Operational Observation Contract | 3–4 ED | Scope Confirmation | ACCEPTED (6849a73) | Operational contract |
| KF-CORE-R07-002 | Component Status & Health Reporting Contract | 2–3 ED | R07-001 | ACCEPTED (61e0067) | Reporting contract |
| KF-CORE-R07-003 | Component Operational Event Contract | 3–4 ED | R07-002 | ACCEPTED (56ff226) | Event contract |
| KF-CORE-R07-004 | Component Statistics Ownership & Observation Contract | 2–3 ED | R07-003 | ACCEPTED (16654e9) | API review input |
| R07 Component Operational API Review | Freeze public operational API | 1 ED | R07-001..004 | PASS / FROZEN (`6b1296f`) | **PASS / FROZEN** |
| KF-CORE-R07-005 | Reference Operational Harness & Contract Tests | 3–4 ED | API Review PASS/FROZEN | ACCEPTED (0b1bd1d) | Contract tests |
| KF-CORE-R07-006 | Runtime/Component Operational Integration | 3–4 ED | R07-005 | ACCEPTED (3f524cd) | Integration Freeze |
| R07 Integration Freeze | Freeze production operational behavior | 0.5 ED | R07-006 | PASS / HONORED (`142a32e`) | **PASS / HONORED** |
| KF-CORE-R07-007 | Full R0.7 Validation | 2–3 ED | Integration Freeze | ACCEPTED `86dfcb9` (candidate d83e1ba) | Release Gate input |
| R07 Release Gate | Release 0.7.0 | 1 ED | R07-007 | PASS / RELEASED (`424984f`) | **PASS** |

**R07 working estimate: 20–28 ED**, including architecture and release gates; actual effort remains unrecorded until supported by evidence.

## R0.7 Architecture Decisions

1. Existing `Status`, `Health`, `Statistics`, and `Event` concepts are preferred over parallel abstractions.
2. No new Component Operational State machine is introduced.
3. Component is authoritative for its operational information; Core does not maintain mirrored operational truth.
4. Observation is read-only and side-effect free.
5. `status()` and `health()` remain value/snapshot queries; no mutable internal references are exposed.
6. Cross-property atomic observation is not guaranteed by Core.
7. Component statistics are optional; R0.7 does not force `statistics()` onto every Component.
8. Component events are explicitly reported to integrator-owned sinks; no Core EventBus, queue, broker or dispatcher is introduced.
9. Events do not cause lifecycle or recovery actions.
10. Runtime remains lifecycle authority and does not poll, interpret or act on Component operational data automatically.
11. No Core-owned background execution, telemetry transport, logging backend, health-driven recovery, automatic retry or restart is introduced.
12. Concrete platform implementations remain outside `kritva-core`.

## R0.7 Dependency Graph

```text
R07 Design Consult
        ↓
R07 Scope Confirmation
        ↓
R07-001 Observation
        ↓
R07-002 Status & Health
        ↓
R07-003 Events
        ↓
R07-004 Statistics
        ↓
R07 Operational API Review
        ↓
R07-005 Reference Harness
        ↓
R07-006 Runtime Integration
        ↓
R07 Integration Freeze
        ↓
R07-007 Full Validation
        ↓
R07 Release Gate
```


## R0.8 Task Tracker

| ID | Task | Est. | Dependency | Status | Gate |
|---|---|---:|---|---|---|
| R08 Design Consult | Configuration model and architectural boundary | 2–3 ED | R0.7 released | APPROVED | Scope input |
| R08 Scope Confirmation | Confirm milestone scope and exclusions | 1 ED | Design Consult | APPROVED | Implementation authorization input |
| KF-CORE-R08-001 | Component Configuration Contract & Lifecycle Semantics | 2–3 ED | Scope Confirmation | ACCEPTED (605516b) | Configuration contract |
| KF-CORE-R08-002 | Configuration Ownership & Atomic Application | 2–3 ED | R08-001 | ACCEPTED (6efeaac) | Ownership/atomicity contract |
| KF-CORE-R08-003 | Configuration Version & Validation Contract | 2–3 ED | R08-002 | ACCEPTED (bdb4b93) | API review input |
| R08 Configuration API Review | Freeze public configuration semantics | 1 ED | R08-001..003 | PASS / FROZEN (`0a73b5a`) | API freeze |
| KF-CORE-R08-004 | Reference Configuration Harness & Contract Tests | 3–4 ED | API Review PASS/FROZEN | ACCEPTED (f4b6de4) | Contract tests |
| KF-CORE-R08-005 | Runtime/Component Configuration Integration | 3–4 ED | R08-004 | ACCEPTED (a5dfbc1) | Integration Freeze input |
| R08 Integration Freeze | Freeze production configuration behavior | 0.5 ED | R08-005 | PASS / HONORED (`e1051a0`) | Production freeze |
| KF-CORE-R08-006 | Configuration Boundary & Regression Validation | 2–3 ED | Integration Freeze PASS/HONORED | ACCEPTED (94fad8e) | Validation |
| KF-CORE-R08-007 | Full R0.8 Validation & Release Candidate | 2–3 ED | R08-006 | ACCEPTED `1aa3611` (candidate 1e7ba2b) | Release Gate input |
| R08 Release Gate | Release 0.8.0 | 1 ED | R08-007 | PASS / RELEASED (`cbbec81`) | **PASS** |

**R08 working estimate: 22–31 ED**, including architecture and release gates; actual effort remains unrecorded until supported by evidence.

## R0.8 Architecture Decisions

1. Configuration is a detached control-plane input to the existing `Component::configure()` operation.
2. Configuration is valid only from `UNKNOWN` and `STOPPED` in R0.8; no dynamic reconfiguration API is introduced.
3. Caller owns the input value; Component owns accepted/applied semantic state; Core owns the generic contract only.
4. Failed configuration cannot partially apply and leaves lifecycle state unchanged.
5. Core structural validation and Component semantic validation remain distinct.
6. `ConfigurationVersion` is a schema/contract compatibility version, not a runtime revision/history counter.
7. Runtime forwards configuration in dependency order without interpreting, persisting, retrying or rolling back.
8. Configuration failure is independent of Runtime FAULT, Status and Health.
9. ComponentContext remains unchanged and configuration-neutral.
10. Dynamic control, parameter services, persistence, remote configuration and robotics-specific semantics remain deferred.

## R0.8 Dependency Graph

```text
R08 Design Consult
        ↓
R08 Scope Confirmation
        ↓
R08-001 Configuration Contract
        ↓
R08-002 Ownership & Atomic Application
        ↓
R08-003 Version & Validation
        ↓
R08 Configuration API Review
        ↓
R08-004 Reference Harness
        ↓
R08-005 Runtime Integration
        ↓
R08 Integration Freeze
        ↓
R08-006 Boundary & Regression Validation
        ↓
R08-007 Full Validation / Release Candidate
        ↓
R08 Release Gate
```

## Permanent Documentation Policy

Beginning with R0.9, documentation synchronization is a standing acceptance requirement for every milestone. Every task explicitly assesses documentation impact; affected API, architecture, requirements, security and guide documents are updated in the same logical task unless a documented exception is approved. Every milestone must complete full documentation reconciliation before Release Gate PASS.

Canonical maintained API documentation is Markdown under `docs/api/`. Generated HTML is a publication artifact and is not the authoritative specification.

## Mandatory Quality Gates

Every future milestone continues to use the following minimum quality gates:

### Per task

```bash
git status
git diff --check
cmake -S . -B build
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
git diff --stat
git diff
git status
```

Where applicable:

```bash
make check
make traceability-check
```

And for milestone validation:

- Debug build
- Release build
- full CTest regression
- ASan + UBSan
- TSan using the documented ASLR-disabled environment where required
- strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`
- GCC `-fanalyzer`
- public-header self-containment
- coverage review against accepted baseline
- traceability audit
- dependency/prohibited-header audit
- production isolation audit
- install-consumer test
- `git diff --check`
- clean working tree

## Testing Model

1. **Unit/Contract** — every changed production contract gets focused tests, negative paths and mutation testing where behavior is contract-sensitive.
2. **Integration** — public APIs only; Component/Runtime operational boundary, event sink and ownership are exercised.
3. **Regression** — full historical suite remains green; randomized/differential tests are used where deterministic behavior is part of the contract.

## Commit Policy

- One logical task = one primary implementation commit.
- Exact commit message is defined in each task's `ACCEPTANCE_CRITERIA.md`.
- No amend of accepted task commits.
- Focused follow-up commit is permitted when review finds a specific corrective issue.
- Do not mix unrelated changes.
- A later task must not silently change an earlier accepted public API.
- Any breaking/semantic change after an API Freeze must return to architecture review.

## Current Decision

R0.8 is fully released and closed (`kritva-core-r0.8`, version 0.8.0, release-record commit `cbbec81`, remote verification PASS / RELEASED / SYNCHRONIZED / CLOSED).

No next milestone is planned yet.

## Deferred Known Issues

These remain deferred unless explicitly brought into scope:

- `make lint` / `make format-check` tooling stubs
- 32-bit scheduler affinity mask
- conformance level-2 mutation strictness gap
- `PlatformContext` used after adapter destruction is undefined behavior by non-owning design
- unused `<chrono>` include in `types/duration.hpp`
- stale root `implementation.md`

Deferred issues must not silently enter R0.8 implementation scope.


## R0.9 Task Tracker

| ID | Task | Est. | Dependency | Status | Gate |
|---|---|---:|---|---|---|
| R09 Design Consult | Capability/readiness model and architectural boundary | 2–3 ED | R0.8 released | APPROVED | Scope input |
| R09 Scope Confirmation | Confirm milestone scope and exclusions | 1 ED | Design Consult | APPROVED | Implementation authorization input |
| KF-CORE-R09-001 | Capability Contract & Provider Semantics | 2–3 ED | Scope Confirmation | ACCEPTED (4081dc0) | Capability contract |
| KF-CORE-R09-002 | CapabilitySet Invariants & Version Semantics | 2–3 ED | R09-001 | ACCEPTED (4bd241e) | Capability semantics |
| KF-CORE-R09-003 | Requirement / Capability Matching Boundary | 2–3 ED | R09-002 | ACCEPTED (e91a51f) | API review input |
| R09 Capability API Review | Freeze public capability semantics | 1 ED | R09-001..003 | PASS / FROZEN (`c84bb9c`) | API freeze |
| KF-CORE-R09-004 | Reference Capability & Requirement Harness | 3–4 ED | API Review PASS/FROZEN | ACCEPTED (2608795) | Contract tests |
| KF-CORE-R09-005 | Component Readiness / Lifecycle Boundary Integration | 3–4 ED | R09-004 | ACCEPTED (0f6b6c3) | Integration Freeze input |
| R09 Integration Freeze | Freeze production capability behavior | 0.5 ED | R09-005 | PASS / HONORED (`553258b`) | Production freeze |
| KF-CORE-R09-006 | API Documentation, Security & Boundary Validation | 2–3 ED | Integration Freeze PASS/HONORED | ACCEPTED (04f0859) | Validation |
| KF-CORE-R09-007 | Full R0.9 Validation & Release Candidate | 2–3 ED | R09-006 | ACCEPTED `683a3ce` (candidate ef14e99) | Release Gate input |
| R09 Release Gate | Release 0.9.0 | 1 ED | R09-007 | PASS (release authorized) | PASS |

**R09 working estimate: 22–30 ED**, including architecture and release gates; actual effort remains unrecorded until supported by evidence.

## R0.9 Architecture Decisions

1. R0.9 is API-neutral by default; existing capability, requirement, context, Runtime and lifecycle mechanisms are preferred.
2. Capability identity is authoritative through `CapabilityId`; Capability metadata is descriptive, not security evidence.
3. Capability version describes the provided capability contract and is not runtime, configuration or authorization state.
4. Capability requirement and Component DependencyGraph semantics remain separate.
5. Capability matching remains identity-based by default; no generic version-range solver is introduced.
6. Core does not automatically calculate readiness or add a new readiness lifecycle state.
7. Health remains independent of readiness/lifecycle decisions.
8. No ServiceRegistry, service locator, dependency resolver, dependency-injection framework or dynamic discovery mechanism is introduced.
9. API documentation is maintained as canonical Markdown under `docs/api/`.
10. Security planning begins in R0.9; security mechanisms remain out of scope unless explicitly approved.
11. Nexus/Edge and platform-specific semantics remain outside `kritva-core`.

## R0.9 Dependency Graph

```text
R09 Design Consult
        ↓
R09 Scope Confirmation
        ↓
R09-001 Capability Contract
        ↓
R09-002 CapabilitySet & Version
        ↓
R09-003 Requirement / Matching Boundary
        ↓
R09 Capability API Review
        ↓
R09-004 Reference Harness
        ↓
R09-005 Readiness / Lifecycle Integration
        ↓
R09 Integration Freeze
        ↓
R09-006 Documentation / Security / Boundary Validation
        ↓
R09-007 Full Validation / Release Candidate
        ↓
R09 Release Gate
```

## Current Decision

R0.8 remains RELEASED / SYNCHRONIZED / CLOSED. R0.9 (Capability Contract & Readiness Boundary, version 0.9.0) has all seven tasks and all gates accepted and the Release Gate PASSED; the annotated tag `kritva-core-r0.9` is created locally on the release-record commit and awaits the owner push and independent remote verification.
