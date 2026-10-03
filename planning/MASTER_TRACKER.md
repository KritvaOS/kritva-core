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
| Latest released milestone | **KF-CORE-R07** |
| Latest released version | **0.7.0** |
| Latest release tag | `kritva-core-r0.7` |
| Latest release commit | `424984f` |
| Remote verification | **PASS / RELEASED / SYNCHRONIZED / CLOSED** |
| Completed milestones | R0.2, R0.3, R0.4, R0.5, R0.6, R0.7 |
| Current active milestone | None — R0.7 released and closed; next milestone not yet planned |
| R07 implementation status | RELEASED / CLOSED |
| Open release blockers | 0 |
| API freeze active | No |
| Concrete platform implementation in `kritva-core` | No |

## Consolidated Milestone Tracker

| Phase | Milestone | Objective | Tasks | Status | Progress | Est. Effort | Dependencies | Quality Gates | Release |
|---|---|---|---:|---|---:|---:|---|---|---|
| Foundation | R0.2 | Core contract hardening | 8 | RELEASED | 8/8 | Historical | R0.1 | Validation + Release Gate | 0.2.0 |
| Runtime | R0.3 | Runtime foundation | 8 | RELEASED | 8/8 | Historical | R0.2 | Foundation API Review + Runtime Contract Review + Integration Freeze + Release Gate | 0.3.0 |
| Platform | R0.4 | Platform abstraction | 8 | RELEASED | 8/8 | Historical | R0.3 | Platform API Review + Integration Freeze + Release Gate | 0.4.0 |
| Platform Runtime | R0.5 | Platform runtime integration foundation | 7 | RELEASED | 7/7 | Historical | R0.4 | Platform API Review + Integration Freeze + Release Gate | 0.5.0 |
| Component Context | R0.6 | Controlled component execution context without changing Runtime lifecycle semantics | 7 | RELEASED | 7/7 | 20–27 ED | R0.5 | Component API Review + Integration Freeze + Validation + Release Gate | 0.6.0 |
| Component Operations | **R0.7** | Controlled Component operational observation/reporting without changing Runtime lifecycle semantics | **7 + 2 architecture gates + API/freeze gates** | **PLANNED** | **0/7 implementation tasks** | **24–32 ED** | R0.6 | Design Consult + Scope Confirmation + API Review + Integration Freeze + Validation + Release Gate | 0.7.0 (target) |

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

R0.6 is fully released and closed.

R0.7 architecture is confirmed through the Design Consult and Scope Confirmation. Implementation authorization begins with R07-001 only after its task package and acceptance criteria are issued. R0.7 is therefore **PLANNED — ARCHITECTURE CONFIRMED**, not yet in implementation.

## Deferred Known Issues

These remain deferred unless explicitly brought into scope:

- `make lint` / `make format-check` tooling stubs
- 32-bit scheduler affinity mask
- conformance level-2 mutation strictness gap
- `PlatformContext` used after adapter destruction is undefined behavior by non-owning design
- unused `<chrono>` include in `types/duration.hpp`
- stale root `implementation.md`

Deferred issues must not silently enter R0.7 implementation scope.
