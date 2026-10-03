# KF-CORE-R03-003 — Dependency Management
## Acceptance Criteria

### 1. Objective

Define and validate the dependency graph between registered components and produce a deterministic dependency-respecting order for runtime orchestration.

### 2. Requirement Traceability

| ID | Requirement |
|---|---|
| CORE-RT-004 | Represent dependencies using stable component identity |
| CORE-RT-005 | Detect dependency cycles and provide deterministic dependency ordering |

### 3. Scope

In scope:
- dependency representation
- dependency validation
- missing/self/duplicate dependency handling
- cycle detection
- deterministic dependency ordering

Out of scope:
- component lifecycle execution
- Runtime Manager
- scheduler/executor
- threads
- recovery
- hardware/platform implementation

### 4. Acceptance Criteria

#### AC-003-01 — Dependency Representation
- [x] Dependencies are represented using `ComponentId`.
- [x] Raw pointers are not used as dependency identity.
- [x] Platform-specific handles are not used.
- [x] Representation is deterministic.

#### AC-003-02 — Missing Dependency
For `A -> B` when B is not registered:
- [x] validation fails deterministically;
- [x] error identifies the missing dependency;
- [x] graph/registry is not partially modified;
- [x] no invalid order is returned.

#### AC-003-03 — Self Dependency
For `A -> A`:
- [x] dependency is rejected;
- [x] deterministic error is returned;
- [x] graph remains valid/unchanged.

#### AC-003-04 — Duplicate Dependency
For `A -> B` repeated:
- [x] behavior is explicitly defined;
- [x] recommended behavior is rejection;
- [x] no duplicate accepted edge exists.

Any alternative such as silent deduplication requires explicit reviewer approval.

#### AC-003-05 — Cycle Detection
For `A -> B -> C -> A`:
- [x] cycle is detected;
- [x] operation fails deterministically;
- [x] cycle members are identifiable where supported by the contract;
- [x] no invalid execution order is returned.

#### AC-003-06 — Multi-Level Dependencies
For `A -> B`, `B -> C`, `C -> D`:
- [x] D appears before C;
- [x] C appears before B;
- [x] B appears before A;
- [x] dependency-order semantics are documented.

#### AC-003-07 — Independent Components
For `A -> C` and `B -> C`:
- [x] C precedes A and B;
- [x] A/B ordering is deterministic;
- [x] recommended tie-break is deterministic registration order;
- [x] chosen tie-break rule is documented and tested.

#### AC-003-08 — Empty / Simple Graphs
- [x] empty graph returns empty order;
- [x] one independent component returns one-element order;
- [x] multiple independent components return deterministic order.

#### AC-003-09 — No Runtime Orchestration
- [x] no component initialization;
- [x] no component start/stop;
- [x] no scheduler invocation;
- [x] no thread creation;
- [x] no recovery behavior.

### 5. Required Unit Tests

- [x] no dependencies
- [x] single dependency
- [x] multi-level dependency
- [x] multiple dependencies
- [x] missing dependency
- [x] self dependency
- [x] duplicate dependency
- [x] simple cycle
- [x] multi-node cycle
- [x] independent components
- [x] deterministic ordering
- [x] invalid graph does not produce an execution order

### 6. Required Integration / Regression Tests

- [x] R03-001 tests pass.
- [x] R03-002 registry tests pass.
- [x] Dependency graph consumes registered components through public APIs.
- [x] Valid registry topology produces deterministic dependency order.
- [x] Invalid topology produces deterministic errors.
- [x] Existing R0.2 regression suite passes.
- [x] No platform-specific dependency is introduced.

### 7. Validation / Sign-off Tests

Required evidence:
- [x] clean Debug build
- [x] Release build
- [x] `-Werror`
- [x] CTest
- [x] ASan
- [x] UBSan
- [x] coverage
- [x] `make check`
- [x] traceability audit
- [x] public-header/API checks
- [x] forbidden-dependency checks
- [x] diff/status check

### 8. Expected Evidence

Codex/Claude must report:
- implementation commit SHA
- changed files
- unit-test command/output
- integration/regression-test command/output
- Debug/Release results
- sanitizer results
- coverage
- traceability/dependency checks
- final Git status

### 8a. Implementation Evidence (Claude)

- Commit: `795fb94` `feat(core): add runtime dependency management` (R03-002 accepted at `0760c93`; the frozen `ComponentId` / `ComponentInfo` / `Component` / `ComponentRegistry` contracts are unchanged).
- Files changed: `include/kritva/core/runtime/dependency_graph.hpp` and `src/dependency_graph.cpp` (new), `include/kritva/core/core.hpp` (umbrella include), `CMakeLists.txt` (library source, test registration), `tests/unit/dependency_graph_test.cpp` (new), `tests/install/consumer/main.cpp` (consumer also links the graph), `REQUIREMENTS.md` (`CORE-RT-004`, `CORE-RT-005` defined; traceability row), `ARCHITECTURE.md`, `API.md` (section 22).
- Public API: `DependencyGraph` with `add_dependency(dependent, dependency) -> Result<void>`, `dependencies_of(id)`, `size()`, `empty()`, `order(const ComponentRegistry&) -> Result<std::vector<ComponentId>>`. Copyable value type holding only `ComponentId`s. No change to any frozen API.
- Semantics ("dependent -> dependency" = the dependent depends on the dependency, which must come first):
  1. **Representation:** ids only (no pointers, handles, components or registry); ordered containers, so every observable result is deterministic.
  2. **Self, duplicate, invalid id:** `INVALID_ARGUMENT`, `Error::source` = the dependent, graph unchanged. Duplicates are rejected, not merged (the recommended behavior; no reviewer exception needed).
  3. **Cycle:** an edge that would close a cycle is rejected at `add_dependency()` with `INVALID_ARGUMENT`; the message lists the cycle (`a -> b -> ... -> a`, starting at the dependent, smallest id first), so the graph is always acyclic and `order()` can never meet a cycle. Cycle membership is identifiable in the message only.
  4. **Missing dependency:** `order(registry)` fails with `CONFIGURATION_ERROR`, `source` = the dependent, message naming the unregistered id (and whether it is the dependent or the dependency); the first offending edge in ascending (dependent, dependency) order; no order is returned and neither graph nor registry is modified. An empty registry with edges is an error, not an empty order.
  5. **Order:** every registered component exactly once, dependencies first; components without edges are included; an empty graph with an empty registry gives an empty order.
  6. **Tie-break (decision, deviates from the acceptance text):** the AC suggests "deterministic registration order". The accepted R03-002 contract makes registration order unobservable, so the tie-break is the **lowest `ComponentId`** among simultaneously ready components (greedy Kahn with an ordered ready set), as you indicated at the R03-002 review. The result is the lexicographically smallest valid order by id. Example: `A=1 -> C=3`, `B=2 -> C` gives `C, A, B`.
  7. Reverse the result for a dependents-first (for example stop) order; no separate reverse API was added.
- Design choices needing your confirmation: (a) the graph is standalone and takes the registry only in `order()`, rather than being bound to a registry at construction (keeps the registry contract frozen and avoids lifetime coupling); (b) cycles are rejected when the edge is added rather than at ordering time; (c) a missing component uses `CONFIGURATION_ERROR` (no new error code); (d) the order is returned as `ComponentId`s, not `Component*`, since dependency identity is the id (callers use `registry.find()`).
- Tests: new CTest `kritva_core_dependency_graph` (14 test functions): representation and value semantics; invalid ids; self dependency; duplicate rejected not merged; simple, two-node, multi-node and branching cycles with the exact reported path and an unchanged graph afterwards; empty, one-element and independent sets; single, multi-level and multi-dependency orders; the tie-break (including ids reversed relative to the edges); independence from registration order and from every one of the 24 edge insertion orders; a randomized brute-force oracle (300 random acyclic graphs of up to 6 nodes compared with the lexicographically smallest valid permutation); missing dependency, unregistered dependent, empty registry, deterministic first-offender, and recovery after registering the missing component; the graph never calls any lifecycle operation, an owner drives `ReferenceComponent`s in the computed order with every dependency started first, and the graph holds no lifetime dependency on components.
- **Mutation evidence** (each temporary edit reverted; file verified identical): highest-id tie-break; duplicate accepted; self dependency accepted; cycle check removed; missing dependency ignored; unregistered dependent ignored; order reversed; edge count not updated; a failed `add_dependency` leaving a partial edge — all nine aborted the test.
- Build: `rm -rf build && cmake -S . -B build && cmake --build build -j$(nproc)` — 0 warnings.
- Tests: `ctest --test-dir build --output-on-failure` — **21/21** (20 prior + `kritva_core_dependency_graph`). Release 21/21; ASan+UBSan 21/21; TSan (ASLR disabled) 21/21; strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror` 21/21; GCC `-fanalyzer` over `src/*.cpp` clean.
- Install-consumer regression: green; the consumer now also uses the installed `DependencyGraph`.
- Standalone compile of `dependency_graph.hpp`: OK. Includes only Core headers and standard `<map>`, `<set>`, `<vector>`, `<string>`, `<utility>`, `<cstddef>`, `<cassert>`; no OS, threading, ROS2/DDS, EtherCAT or vendor headers.
- `make check` passed (traceability 53 requirements, 52 traced, 0 errors; format-check/lint remain stubs). `git diff --check` clean.
- Coverage: `make coverage` — 98% (304/308). `src/dependency_graph.cpp` is 84/84 lines. The four uncovered lines are pre-existing or exception-unwind lines (`lifecycle.cpp`, `configuration.cpp`, the closing brace of `ComponentRegistry::components()`).
- Final `git status --short`: clean after the commit.
- Known limitations / out of scope by design: no lifecycle orchestration, no runtime manager, no reverse-order API, no edge removal, no thread safety; `add_dependency()` is O(V + E); cycle members are reported only in the message text.
- Gate: all three foundation tasks have commits. The **R03 Foundation API Review** (see `R03_FOUNDATION_API_REVIEW.md`) must PASS before KF-CORE-R03-004 begins; I have not started it.

### 9. Git Commit

Implementation commit:

`feat(core): add runtime dependency management`

Focused post-review corrections must use a separate `fix(core): ...` commit.

### 10. Reviewer Sign-off

| Item | Result |
|---|---|
| Implementation complete | ☑ |
| Unit tests complete | ☑ |
| Integration/regression tests complete | ☑ |
| Validation gates complete | ☑ |
| Dependency API reviewed | ☑ |
| Ordering semantics reviewed | ☑ |
| Architecture constraints satisfied | ☑ |
| Reviewer decision | **PASS** |
| Reviewer | ChatGPT |
| Date | 03-10-2026 |

Accepted commits: `795fb94` `feat(core): add runtime dependency management` (evidence `7e2a53b`). No corrective implementation commit was required.

Approved decisions (now frozen going into the R03 Foundation API Review):
- The tie-break among simultaneously eligible components is the lowest `ComponentId` (an approved deviation from the original "registration order" wording, because the accepted registry makes registration order unobservable). R03-004 must not introduce another implicit ordering rule.
- The graph is standalone: it uses the registry only in `order()` and is never bound to one.
- Cycles are rejected when the edge is added, so a successfully built graph is always acyclic; failed mutations are atomic.
- Missing registered components are detected in `order()` with `CONFIGURATION_ERROR` (source = the dependent); no new error code.
- `order()` returns `ComponentId`s, never `Component*`; the graph stores ids only and never owns or calls components.

**Reviewer Decision: PASS — KF-CORE-R03-003 is ACCEPTED.** The R03 Foundation API Review is the next gate; R03-004 is blocked until it passes.
