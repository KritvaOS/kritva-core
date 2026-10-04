# KF-CORE-R09 — Integration Freeze

## Status

PLANNED — review record to be completed after R09-005.

## Purpose

Freeze R0.9 production capability/readiness behavior before boundary/regression validation.

## Entry Criteria

- R09 Capability API Review PASS / FROZEN.
- R09-004 accepted.
- R09-005 accepted.
- Full regression is green.
- Seeded/differential integration behavior demonstrates no unintended Runtime lifecycle changes.

## Required Checks

- [ ] No automatic dependency resolution.
- [ ] No automatic readiness transition/state machine.
- [ ] No Health-to-lifecycle coupling.
- [ ] DependencyGraph behavior remains unchanged.
- [ ] PlatformRequirements behavior remains compatible.
- [ ] No ComponentContext or Runtime lifecycle expansion outside approved scope.
- [ ] Capability snapshots remain caller-owned/non-owning as specified.
- [ ] No hidden background execution, retry or recovery.
- [ ] API documentation remains synchronized.
- [ ] Security boundary remains unchanged or explicitly reviewed.
- [ ] Production diff contains only approved R0.9 changes.

## Evidence (submitted 05-10-2026)

Freeze candidate: production sources are final as of `4c86b53` (Capability API Review freeze commit; the last production change is R09-003 `e91a51f`). R09-004 `2608795` and R09-005 `0f6b6c3` changed tests, documentation and requirements only. Head: `1679463`. Capability API Review: PASS / FROZEN (evidence `c84bb9c`, recorded `7e44049`).

### Entry criteria

R09-004 and R09-005 are ACCEPTED (`d90e4e9`, `1679463`). The capability, lifecycle, dependency and isolation test groups pass and are deterministic (5 randomized-order repetitions of 13 groups all pass after a full rebuild). All pre-R0.9 regression tests remain green (62/62 in total). The seeded model (200 seeds, vacuity-guarded, replayed twice per seed) and the order-independence property test (60 seeds × 3 provisions) show no unintended Runtime lifecycle change. The integration tests use public APIs only.

### Production diff

`git diff 4c86b53 HEAD -- include src` is **empty**. Against `kritva-core-r0.8` the production change is exactly the reviewed R09 contract text: `capability.hpp` (+62), `capability_id.hpp` (+11), `capability_set.hpp` (+118); `src/` and every other header (`requirements.hpp`, `context.hpp`, `adapter.hpp`, `component.hpp`, `runtime_manager.hpp`, `dependency_graph.hpp`, `component_context.hpp`) are byte-identical to the released tag. Since the freeze the only changes are: the test harness and tests, `CMakeLists.txt` registrations, `REQUIREMENTS.md`, `API.md`, `TESTING.md` and the two `docs/api` pages (`LIFECYCLE.md`, `DEPENDENCY_GRAPH.md`) that document the unchanged headers.

### Required checks

| Check | Evidence |
|---|---|
| No automatic dependency resolution | by-name absence of resolve/bind/inject/satisfy/discover/provider_of/require_capability/capability-dependency on seven surfaces (R09-003); the Runtime performs no capability query (zero snapshots) and no dependency is ever derived from a capability |
| No automatic readiness transition/state machine | the lifecycle keeps its eight states (R09-005 observes only UNKNOWN/READY/RUNNING/STOPPED/FAULT); no readiness state or query exists; the boundary text and `LIFECYCLE.md` |
| No Health-to-lifecycle coupling | all four health values with the provision removed and restored leave the lifecycle unchanged and the capability set never touches Health; a Runtime-reading mutant is detected (R0.7 suites) |
| DependencyGraph behavior unchanged | `dependency_graph.hpp` byte-identical; its one edge operation takes ComponentIds only; order independent of provision over 60 × 3 random cases; reversed-order mutant detected |
| PlatformRequirements behavior compatible | `requirements.hpp` byte-identical; the R0.5/R0.6 suites unchanged and green; identity-only matching tested against an independent oracle |
| No ComponentContext or Runtime lifecycle expansion | `component_context.hpp`, `runtime_manager.hpp`, `component.hpp` byte-identical; the integrator-built context is unchanged across a full lifecycle |
| Capability snapshots caller-owned/non-owning as specified | by-value provider snapshots; independent copies in both directions (ASan); reference validity rules tested; one snapshot per evaluation |
| No hidden background execution, retry or recovery | no capability snapshot or call after a failure while the capability appears and 50 observations follow; only explicit `reset()` then an explicit new `initialize()`; `retry_count` 0; word-bounded scan: no `<thread>`, `<mutex>`, `<atomic>`, `<future>`, `<condition_variable>`, `<queue>`, `<deque>`, `<iostream>`, `<fstream>` or `<cstdio>` in `include/` or `src/`, no OS, RTOS, ROS2, DDS or EtherCAT identifier and no Nexus identifier except the comment in `platform/boundary.hpp` that names Nexus and Edge platforms as excluded (the dependency-graph "edge" is the graph term, not a product) |
| API documentation remains synchronized | `docs/api/capability/{CAPABILITY,CAPABILITY_SET,CAPABILITY_REQUIREMENTS}.md`, `docs/api/platform/PLATFORM_REQUIREMENTS.md`, `docs/api/lifecycle/LIFECYCLE.md`, `docs/api/runtime/DEPENDENCY_GRAPH.md` authored with their contract text (15 required sections each); every `API_INDEX.md` path exists; the mechanical audit is R09-006 |
| Security boundary unchanged or reviewed | no new API, authority boundary, persistence, discovery, background execution or credential; per task NONE or DOCUMENTATION ONLY; the formal review record is R09-006 |
| Production diff contains only approved R0.9 changes | yes (above) |

### Validation (head `1679463`; production unchanged since `4c86b53`)

`ctest` 62/62 in Debug, Release, ASan+UBSan, strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror` and TSan (ASLR off); 0 warnings; GCC `-fanalyzer` clean; header self-containment passes; `make check` and traceability: 107 requirements, 106 traced, 0 errors, `CORE-CAP-004` to `010` defined once; coverage 618/625 unchanged; `git diff --check` clean.

### Open issues (none blocking)

Carried forward: 32-bit scheduler affinity mask; conformance level-2 mutation gap; `make lint`/`make format-check` stubs; documented non-owning lifetime UB; the invalid-identity entry remains storable by design. R09-006 (documentation audit tooling, security review record, boundary snapshot, install-consumer exercise) and R09-007 (release candidate 0.9.0) remain; neither may change production API or behavior.

## Decision

Reviewer decision: **PASS / HONORED** (05-10-2026), ChatGPT (independent reviewer). Evidence commit `553258b`.

**Production freeze commit: `4c86b53`** (Capability API Review freeze; last production change R09-003 `e91a51f`); validation head `1679463`. R09-004 `2608795` and R09-005 `0f6b6c3` changed tests, documentation and requirements only. `git diff 4c86b53 HEAD -- include src` is empty: production code, API and behavior are unchanged after the freeze. Documentation drift: none. Security boundary change: none. Open blockers: 0.

Allowed after the freeze: tests, documentation, requirements/traceability, validation tooling, security records and release metadata. Not allowed without an explicit architecture-review exception: production API changes, production semantic or behavior changes, and new readiness or dependency infrastructure. R09-006 must preserve the same rule while adding the documentation audit tooling, the boundary snapshot, the security review and the install-consumer validation.

Non-blocking open issues retained: the 32-bit scheduler affinity mask; the conformance level-2 mutation gap; the `make lint` / `make format-check` stubs; the documented non-owning lifetime rule; the storable invalid-identity entry (documented, `add()` unchanged).

## Freeze Rule

After PASS/HONORED, production API/semantic changes require an explicit architecture-review return.
