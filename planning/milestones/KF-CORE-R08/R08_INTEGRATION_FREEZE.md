# R08 Integration Freeze

## Gate

PASS required before R08-006 and R08-007 implementation/validation work.

## Preconditions

- R08-004 accepted.
- R08-005 accepted.
- Configuration API Review PASS/FROZEN.

## Freeze Requirements

- [ ] Runtime configuration forwarding is validated using public APIs.
- [ ] Component configuration failure does not alter Runtime lifecycle state.
- [ ] Configuration does not alter Status/Health automatically.
- [ ] Configuration is not available in READY/RUNNING through the R08 API.
- [ ] No retry, rollback or automatic recovery exists.
- [ ] No ComponentContext expansion exists.
- [ ] No Core configuration service/event/persistence infrastructure exists.
- [ ] Production dependency boundary remains clean.
- [ ] Randomized or permutation testing confirms established Runtime ordering remains deterministic.
- [ ] Production diff from the API Review freeze point contains no unapproved semantic changes.

## Evidence (submitted 05-10-2026)

Freeze candidate: production sources are final as of `bdb4b93` (R08-003, the last production change; contract text only). R08-004 `f4b6de4` and R08-005 `a5dfbc1` changed tests and documentation only. Head: `3d63fa9`. Configuration API Review baseline: PASS / FROZEN at `e8f8a16` (evidence `0a73b5a`).

### Entry criteria

R08-004 and R08-005 are ACCEPTED (`4e7fc20`, `3d63fa9`). The configuration tests pass and are deterministic (seeded; 5 randomized-order repetitions of the 7 configuration and isolation test groups all pass). All pre-R0.8 regression tests remain green (56/56 in total). The integration tests use public APIs only.

### Production diff

`git diff bdb4b93 HEAD -- include src` is **empty**: production code is byte-identical to the Configuration API Review baseline. Against `kritva-core-r0.7` the production change is exactly the reviewed R08 set: contract text in `configuration/configuration.hpp` (+127) and `configuration/configuration_version.hpp` (+34); `src/` and every other header (including `runtime/component.hpp`, `runtime_manager.hpp`, `component_context.hpp`, `parameter.hpp`) are byte-identical to the released tag.

### Freeze requirements

| Requirement | Evidence |
|---|---|
| Runtime configuration forwarding is validated using public APIs | the seeded differential (200 seeds × 40 steps, vacuity-guarded) plus the reusable `check_runtime_forwarding` on every configure step (R08-004/005) |
| Component configuration failure does not alter Runtime lifecycle state | the Runtime state and fault are asserted unchanged around every configure call; a failing component leaves the Runtime UNKNOWN/STOPPED with no fault; mutant entering FAULT detected |
| Configuration does not alter Status/Health automatically | spies read zero times over every seeded scenario; consequential mutants reading Status or Health during configure detected |
| Configuration is not available in READY/RUNNING through the R08 API | refused with INVALID_STATE (component and Runtime) before any component is invoked, and in FAULT; member-detection shows no reconfigure/set_parameter/get_parameter/apply/update/`configuration()` on Component, RuntimeManager or ComponentContext |
| No retry, rollback or automatic recovery exists | first failure stops the sequence, no component called twice, earlier acceptances kept, `retry_count` 0 at every step, a FAULT ends only by explicit `reset()`; retry, rollback and continue mutants detected |
| No ComponentContext expansion exists | `component_context.hpp` byte-identical to the tag; a context built by the integrator is unchanged across configuration; no configuration member (concepts) |
| No Core configuration service/event/persistence infrastructure exists | no store, registry, server, broker, persistence, event or worker; word-bounded dependency scan below |
| Production dependency boundary remains clean | no `<thread>`, `<mutex>`, `<atomic>`, `<future>`, `<condition_variable>`, `<queue>`, `<deque>`, `<iostream>`, `<fstream>` or `<cstdio>` in `include/` or `src/` and no OS, RTOS, ROS2, DDS or EtherCAT identifier (only a comment in `boundary.hpp`); audit and `kritva_core_test_isolation` pass |
| Randomized or permutation testing confirms Runtime ordering remains deterministic | permuted registration orders and random acyclic topologies, with the call order equal to `component_order()` at every step, before and after the topology is fixed; reversed-order mutants detected |
| Production diff from the API Review freeze point contains no unapproved semantic changes | empty (above) |

### Validation (head `3d63fa9`; production unchanged since `bdb4b93`)

`ctest` 56/56 in Debug, Release, ASan+UBSan, strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror` and TSan (ASLR off via `setarch -R`); 0 warnings; GCC `-fanalyzer` clean; public-header self-containment passes; `make traceability-check`: 99 requirements, 98 traced (CORE-ERR-003 reserved), 0 errors, `CORE-CFG-004` to `012` defined once; coverage 618/625; no `find_package` or `FetchContent`; `git diff --check` clean.

### Open issues (none blocking)

Carried forward: 32-bit scheduler affinity mask; conformance level-2 mutation gap; `make lint`/`make format-check` stubs; documented non-owning lifetime UB (context, reporter, provider). The remaining work is R08-006 (boundary and regression validation) and R08-007 (release candidate 0.8.0) and the Release Gate; none may change production API or behavior.

## Decision

`PASS / HONORED / CHANGES REQUIRED / BLOCKED`

## Evidence

Record the production freeze point, evidence commit and final diff audit here after the gate is executed.
