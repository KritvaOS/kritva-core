# R06 Component Integration Freeze

## Purpose

Freeze production API and Runtime/Component semantics before final R0.6 validation.

## Entry criteria

- R06-005 accepted
- R06-006 accepted
- component/context integration tests pass
- all pre-R0.6 regression tests remain green
- public APIs only in integration tests
- no unresolved production API change

## Freeze requirements

- no Runtime lifecycle redesign;
- no Component lifecycle signature drift;
- no hidden context ownership;
- no automatic platform-service lifecycle;
- no background Core execution;
- no test-only production hooks;
- production diff from API freeze point is clean unless explicitly approved.

## Evidence (submitted 05-10-2026)

Freeze candidate: production sources are final as of `5b755af` (R06-004, the last production change). R06-005 `c7f7b46` and R06-006 `2fd5424` changed tests only. Head: `d0794b5`. Component API Review baseline: PASS / FROZEN at `6136cab` (evidence `06207c7`).

### Entry criteria

R06-005 and R06-006 are ACCEPTED (`0c419ab`, `d0794b5`). The component/context integration tests pass and are deterministic (seeded; 5 randomized-order repetitions of the 11 context, platform and isolation test groups all pass). All pre-R0.6 regression tests remain green (45/45 in total). The integration tests use public APIs only. No unresolved production API change exists.

### Production diff

`git diff 06207c7 HEAD -- include src` is **empty**: production code and API are byte-identical to the Component API Review baseline. Against `kritva-core-r0.5` the production change is exactly the reviewed R06 set: `runtime/component_context.hpp` (new) and one include in `core.hpp`; `src/` is unchanged and every R0.3, R0.4 and R0.5 header is byte-identical to the tag.

### Freeze requirements

| Requirement | Evidence |
|---|---|
| No Runtime lifecycle redesign | `src/` and `runtime_manager.hpp` byte-identical to `kritva-core-r0.5`; a 40 x 40 seeded differential with components that use their context, over every service combination with every platform method failing, gives results, states, faults, statistics, retries and invocation traces identical to the plain baseline; a 72-case equivalence shows a platform failure through a context is exactly an ordinary component failure (complete invocation traces included) |
| No Component lifecycle signature drift | `runtime/component.hpp` byte-identical; `static_assert`s on the five lifecycle signatures and `info()` |
| No hidden context ownership | the context owns, creates and destroys nothing; lifetime probes show the Runtime, the component and its context never destroy the platform, a service or an identity; assignment is deleted and a temporary identity or component is refused at compile time |
| No automatic platform-service lifecycle | the Runtime made zero adapter queries and zero service calls in every operation and state (FAULT and reset included) for components that hold a context; every context path is a query; services advance only when a test says so |
| No background Core execution | no `<thread>`, `<mutex>`, `<atomic>`, `<condition_variable>`, `<future>`, `<iostream>` or `<cstdio>` in `include/` or `src/` (enforced by the audit) |
| No test-only production hooks | the harness lives only under `tests/` (`tests/runtime/`, `tests/platform/`); the audit (`CORE-PLAT-016`) and `kritva_core_test_isolation` forbid any production include of it (verified by a planted violation); the Core headers the new tests include are `core.hpp` only |
| Production diff from the API freeze point is clean | empty (above) |

### Validation (head `d0794b5`; production unchanged since `5b755af`)

`ctest` 45/45 in Debug, Release, ASan+UBSan, strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror` and TSan (ASLR off via `setarch -R`); 0 warnings; GCC `-fanalyzer` clean; public-header self-containment passes; `make traceability-check`: 79 requirements, 78 traced (CORE-ERR-003 reserved), 0 errors, `CORE-CTX-001` to `006` defined once; coverage 99% (595/601, unchanged); no `find_package` or `FetchContent`; `git diff --check` clean.

### Open issues (none blocking)

Carried forward: the 32-bit scheduler affinity mask; the conformance suite's level-2 mutation strictness gap; the `make lint` / `make format-check` stubs; a context used after what it refers to is destroyed is documented undefined behavior (non-owning by design). The only remaining work is R06-007 (validation, release metadata 0.6.0) and the Release Gate; neither may change production API.

Reviewer decision: **PASS / HONORED** (05-10-2026)

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Date | 05-10-2026 |
| Decision | **PASS / HONORED** |

**Production freeze point: `5b755af`** (R06-004, the last production change). R06-005 `c7f7b46` and R06-006 `2fd5424` changed tests only; R06-006 accepted at `d0794b5`; evidence `769ac8d`; Component API Review baseline `06207c7`. `git diff 06207c7 HEAD -- include src` is empty: production code and API are unchanged after the freeze.

Frozen: `runtime::ComponentContext` (as frozen at the Component API Review) together with the Runtime/Component semantics it must not change, and the test-only harness and isolation guards. R06-007 is validation and release metadata only; the production API must not be redesigned or expanded.

Non-blocking open issues retained: the 32-bit scheduler affinity mask; the conformance suite's level-2 mutation strictness gap; the `make lint` / `make format-check` stubs; the documented non-owning lifetime rule.

**Reviewer Decision: PASS / HONORED — KF-CORE-R06 Integration Freeze is ACCEPTED.**
