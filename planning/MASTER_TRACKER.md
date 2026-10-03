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
| Latest released milestone | **KF-CORE-R05** |
| Latest released version | **0.5.0** |
| Latest release tag | `kritva-core-r0.5` |
| Latest release commit | `adf8ac2` |
| Remote verification | **PASS / RELEASED / SYNCHRONIZED / CLOSED** |
| Completed milestones | R0.2, R0.3, R0.4, R0.5 |
| Current active milestone | **KF-CORE-R06 — PLANNED ARCHITECTURE PROPOSAL** |
| R06 implementation status | NOT STARTED |
| Open release blockers | 0 |
| API freeze active | No — R06 architecture phase |
| Concrete platform implementation in `kritva-core` | No |

## Consolidated Milestone Tracker

| Phase | Milestone | Objective | Tasks | Status | Progress | Est. Effort | Dependencies | Quality Gates | Release |
|---|---|---|---:|---|---:|---:|---|---|---|
| Foundation | R0.2 | Core contract hardening | 8 | RELEASED | 8/8 | Historical | R0.1 | Validation + Release Gate | 0.2.0 |
| Runtime | R0.3 | Runtime foundation | 8 | RELEASED | 8/8 | Historical | R0.2 | Foundation API Review + Runtime Contract Review + Integration Freeze + Release Gate | 0.3.0 |
| Platform | R0.4 | Platform abstraction | 8 | RELEASED | 8/8 | Historical | R0.3 | Platform API Review + Integration Freeze + Release Gate | 0.4.0 |
| Platform Runtime | R0.5 | Platform runtime integration foundation | 7 | RELEASED | 7/7 | Historical | R0.4 | Platform API Review + Integration Freeze + Release Gate | 0.5.0 |
| Component Context | **R0.6** | Controlled component execution/operational context without changing Runtime lifecycle semantics | **7** | **PLANNED ARCHITECTURE PROPOSAL** | 0/7 | **20–27 ED** | R0.5 | Architecture Review + API Freeze + Integration Freeze + Validation + Release Gate | 0.6.0 (target) |

### Historical effort note

R0.2–R0.5 are retained as historical milestones. The current repository does not contain reliable day-level effort records, so this tracker deliberately does **not** invent historical actual effort.

## R0.6 Task Tracker — Architecture Proposal

| ID | Task | Est. | Dependency | Status | Gate |
|---|---|---:|---|---|---|
| KF-CORE-R06-001 | Component Execution Context & Ownership Model | 3–4 ED | R0.5 released | ACCEPTED (8031c47) | API scope review |
| KF-CORE-R06-002 | Operational Context Services & Access Policy | 3–4 ED | R06-001 | ACCEPTED (072b713) | Contract review |
| KF-CORE-R06-003 | Context Injection Without Runtime Lifecycle Change | 3–4 ED | R06-002 | ACCEPTED (adb0e08) | Integration contract |
| KF-CORE-R06-004 | Context Requirements & Capability Binding | 2–3 ED | R06-002 | ACCEPTED (5b755af) | API review input |
| R06 Component API Review | Freeze public context/API | 1 ED | R06-001..004 | PASS / FROZEN (`06207c7`) | **PASS / FROZEN** |
| KF-CORE-R06-005 | Reference Context Harness & Contract Tests | 3–4 ED | API Freeze | ACCEPTED (c7f7b46) | Contract tests |
| KF-CORE-R06-006 | Runtime/Component Context Integration Tests | 3–4 ED | R06-005 | ACCEPTED (2fd5424) | Integration Freeze |
| R06 Integration Freeze | Freeze production behavior | 0.5 ED | R06-006 | PASS / HONORED | **PASS / HONORED** |
| KF-CORE-R06-007 | Full R0.6 Validation | 2–3 ED | Integration Freeze | ACCEPTED (7351db6; candidate b473e5d) | Release Gate input |
| R06 Release Gate | Release 0.6.0 | 1 ED | R06-007 | PLANNED | **PASS** |

**R06 working estimate: 20–27 ED**, excluding unresolved architecture changes returned by review.

## R0.6 Dependency Graph

```text
R06-001 Component Execution Context & Ownership
        ↓
R06-002 Operational Context Services & Access Policy
        ↓
R06-003 Context Injection Without Runtime Lifecycle Change
        ↓
R06-004 Context Requirements & Capability Binding
        ↓
R06 Component API Review
        ↓
R06-005 Reference Context Harness & Contract Tests
        ↓
R06-006 Runtime/Component Context Integration Tests
        ↓
R06 Integration Freeze
        ↓
R06-007 Full R0.6 Validation
        ↓
R06 Release Gate
```

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
- install-consumer test
- `git diff --check`
- clean working tree

## Mandatory Testing Model

Testing is explicitly divided into three layers:

1. **Unit/Contract**
   - every new/changed production contract gets focused tests;
   - negative/error/invalid-state paths are mandatory;
   - mutation testing is used for contract-sensitive behavior.

2. **Integration**
   - public APIs only;
   - integrator-facing flows across Component/Context/Runtime boundaries;
   - failure propagation, ownership and lifecycle isolation are exercised.

3. **Regression**
   - complete pre-milestone test suite remains green;
   - new milestone tests do not replace historical coverage;
   - randomized-order and differential tests are used where determinism is part of the contract.

## Commit Policy

- One logical task = one primary implementation commit.
- Exact commit message is defined in each task's acceptance criteria.
- No amend of accepted task commits.
- Focused follow-up commit is permitted when review finds a specific corrective issue.
- Do not mix unrelated changes.
- A later task must not silently change an earlier accepted public API.
- Any breaking/semantic change after an API Freeze must return to architecture review.

## Current Decision

R0.5 is fully released and closed.

R0.6 is currently an **architecture proposal**, not yet an implementation authorization. The R06 task package below is therefore intended for architecture review before the first implementation task is started.

## Deferred Known Issues

These are carried forward unless explicitly brought into scope by a later milestone:

- `make lint` / `make format-check` tooling stubs
- 32-bit scheduler affinity mask
- conformance level-2 mutation strictness gap
- `PlatformContext` used after adapter destruction is undefined behavior by non-owning design
- unused `<chrono>` include in `types/duration.hpp`
- stale root `implementation.md`

Deferred issues must not silently enter R06 implementation scope.
