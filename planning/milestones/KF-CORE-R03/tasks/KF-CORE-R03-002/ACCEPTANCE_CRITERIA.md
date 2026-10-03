# KF-CORE-R03-002 — Component Registry
## Acceptance Criteria

### 1. Objective

Provide a deterministic, platform-independent registry for registering and locating runtime components using `ComponentId`.

### 2. Requirement Traceability

| ID | Requirement |
|---|---|
| CORE-RT-003 | Component registry with deterministic registration and lookup semantics |

### 3. Scope

In scope:
- registration
- duplicate identity handling
- lookup
- contains
- deterministic enumeration
- ownership/lifetime contract

Out of scope:
- lifecycle orchestration
- dependency resolution
- scheduler/executor
- threads
- runtime start/stop
- automatic recovery
- hardware/platform implementation

### 4. Acceptance Criteria

#### AC-002-01 — Registration
- [x] Valid component registers successfully.
- [x] Invalid/empty identity is rejected.
- [x] Null component/reference is rejected where applicable.
- [x] Registration result is deterministic.
- [x] Successfully registered component is discoverable.

#### AC-002-02 — Duplicate Identity
- [x] Duplicate identity registration fails deterministically.
- [x] Existing component is not implicitly replaced.
- [x] Failed registration leaves registry unchanged.
- [x] Existing Core error/result semantics are used.

#### AC-002-03 — Lookup
- [x] Existing component can be found by `ComponentId`.
- [x] Missing component has deterministic not-found behavior.
- [x] Lookup has no side effects.

#### AC-002-04 — Contains
- [x] Registered component returns true.
- [x] Missing component returns false.
- [x] Operation has no side effects.

#### AC-002-05 — Enumeration
- [x] Enumeration is deterministic.
- [x] Each registered component appears exactly once.
- [x] Container implementation details are not exposed.
- [x] Ordering rule is documented and tested.
- [x] Ordering does not depend on accidental `unordered_map` iteration.

#### AC-002-06 — Ownership/Lifetime
- [x] Ownership model is explicit.
- [x] Component lifetime is explicit.
- [x] The registry is non-owning: it holds non-owning references to components owned by the application/runtime owner (R03-001 ownership model preserved).
- [x] The registry never owns, copies, moves or deletes a component; components are non-copyable and non-movable (R03-001), and the registry must not require otherwise.
- [x] The registry does not silently become an owning container.
- [x] Behavior when a component is destroyed is defined.
- [x] Validity of returned references/pointers is defined.
- [x] No dangling reference is permitted.

#### AC-002-07 — Unregister
- [x] `unregister()` is not required unless a concrete R03 need is demonstrated.
- [x] If deferred, it is explicitly documented as out of scope.
- [x] No incomplete public placeholder API is added.

#### AC-002-08 — Runtime Separation
- [x] Registry does not initialize components.
- [x] Registry does not start/stop components.
- [x] Registry does not perform dependency ordering.
- [x] Registry does not create threads or invoke a scheduler.

### 5. Required Unit Tests

- [x] register valid component
- [x] reject invalid component
- [x] duplicate registration
- [x] find existing component
- [x] find missing component
- [x] contains existing component
- [x] contains missing component
- [x] deterministic enumeration
- [x] registry state after failed registration
- [x] ownership/lifetime behavior where testable

### 6. Required Integration / Regression Tests

- [x] R03-001 component contract tests pass.
- [x] Minimal reference components can be registered and found.
- [x] Multiple components can coexist.
- [x] Registry can be consumed through the public API.
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

- Commit: `7ae9a32` `feat(core): add component registry` (R03-001 accepted at `4140e15`; the accepted `ComponentId` / `ComponentInfo` / `Component` contracts are unchanged).
- Files changed: `include/kritva/core/runtime/component_registry.hpp` and `src/component_registry.cpp` (new), `include/kritva/core/core.hpp` (umbrella include), `CMakeLists.txt` (library source and test registration), `tests/unit/component_registry_test.cpp` (new), `tests/install/consumer/main.cpp` (consumer now also links the registry), `REQUIREMENTS.md` (`CORE-RT-003` defined, traceability row), `ARCHITECTURE.md`, `API.md` (section 21).
- Public API: `ComponentRegistry` with `register_component(Component&) -> Result<void>`, `find(ComponentId) const noexcept -> Component*`, `contains`, `size`, `empty`, `components() const -> std::vector<Component*>`; copy and move deleted. No change to any accepted API.
- Design decisions (please confirm; none are forced by existing code):
  1. **Non-owning** (R03-001 model carried forward): stores `Component*` in a private `std::map<ComponentId, Component*>`; never owns, copies, moves, deletes or calls a component. The registry itself is neither copyable nor movable.
  2. **Duplicate id:** `ErrorCode::INVALID_ARGUMENT`, `Error::source` = the id, message names the id; no replacement; registry unchanged. There is no `ALREADY_EXISTS` code in the frozen R02 `ErrorCode`; adding one would be an R02 contract extension, so I reused `INVALID_ARGUMENT`.
  3. **Invalid identity / null:** not representable. `ComponentInfo` always holds a valid id and a `Component&` cannot be null, so there is no rejection path to implement or test; documented rather than adding dead code.
  4. **Enumeration order:** ascending `ComponentId::value()` (unsigned), independent of registration order and of container iteration order (`std::map`, not `unordered_map`). Chosen over registration order so the order is a pure function of the set of ids (usable later as the R03-003 tie-break). `components()` returns a snapshot vector of non-owning pointers; the container is not exposed.
  5. **Lookup:** `nullptr` / `false` when missing, including the invalid id. A `const` registry returns mutable `Component*` (shallow-const handle).
  6. **Lifetime:** a registered component must outlive any *use* of the registry; using the registry after a registered component was destroyed is undefined behavior (not detectable). Destroying the registry never touches a component. Returned pointers are the registered objects and stay valid as long as they live.
  7. **Unregister:** not added; documented as out of scope. No placeholder API.
  8. Registration is allowed in any lifecycle state and never changes it. Allocation failure throws `std::bad_alloc` with the registry unchanged (`try_emplace` strong guarantee), consistent with other Core containers.
  9. Not thread-safe; `register_component()`/`components()` allocate (control-plane); `find`/`contains` are O(log n) without allocation.
- Tests: new CTest `kritva_core_component_registry` (12 test functions): register and discover (same object, not a copy); no invalid-identity path; registration never drives the component and works in UNKNOWN/RUNNING/FAULT; duplicate fails deterministically without replacement (different object and same object); failed registration leaves the registry unchanged and later registration still works; find/contains for present, missing, invalid id, empty and `const` registries with no side effects; enumeration in ascending order over all 120 registration permutations of 5 ids, exactly once for 1000 shuffled ids (fixed seed), and for extreme unsigned ids; snapshot independence; non-owning shape (not copyable/movable) and destruction order in both directions; registry never calls a lifecycle operation; reference components coexist and are driven by their owner, and the R03-001 component contract checker still passes for a registered component.
- **Mutation evidence** (each temporary edit reverted; files verified identical): duplicate replaces the existing entry; unordered/registration-dependent enumeration; registry calls `initialize()`; `contains()` true for the invalid id; duplicate error code changed; duplicate error without source; descending order (all aborted the test).
- Build: `rm -rf build && cmake -S . -B build && cmake --build build -j$(nproc)` — 0 warnings.
- Tests: `ctest --test-dir build --output-on-failure` — **20/20** (19 prior + `kritva_core_component_registry`). Release 20/20; ASan+UBSan 20/20; TSan (ASLR disabled) 20/20; strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror` 20/20; GCC `-fanalyzer` over `src/*.cpp` clean.
- Install-consumer regression: green, and the consumer now also constructs a `ComponentRegistry` from the installed library.
- Standalone compile of `component_registry.hpp`: OK. Includes: `component.hpp`, `component_id.hpp`, `result.hpp`, `<map>`, `<vector>`, `<cstddef>`, `<string>`, `<utility>`; no OS, threading, ROS2/DDS, EtherCAT or vendor headers.
- `make check` passed (traceability 51 requirements, 50 traced, 0 errors; format-check/lint remain stubs). `git diff --check` clean.
- Coverage: `make coverage` — 98% (208/212). New `src/component_registry.cpp` is 18/19; the single uncovered line is the closing brace of `components()`, i.e. the exception-unwinding path when `push_back` would throw (not reachable without an allocation failure; the vector is `reserve`d first). The other three uncovered lines are the pre-existing defensive lines in `lifecycle.cpp`/`configuration.cpp`.
- Final `git status --short`: clean after the commit.
- Known limitations / out of scope by design: no unregister, no dependency ordering, no lifecycle orchestration, no thread safety; `components()` allocates; registry-after-component-destruction cannot be detected.

### 9. Git Commit

Implementation commit:

`feat(core): add component registry`

Focused post-review corrections must use a separate `fix(core): ...` commit.

### 10. Reviewer Sign-off

| Item | Result |
|---|---|
| Implementation complete | ☐ |
| Unit tests complete | ☐ |
| Integration/regression tests complete | ☐ |
| Validation gates complete | ☐ |
| Registry API reviewed | ☐ |
| Ownership/lifetime reviewed | ☐ |
| Architecture constraints satisfied | ☐ |
| Reviewer decision | **PASS / CHANGES REQUIRED / BLOCKED** |
| Reviewer | __________________ |
| Date | __________________ |

Final acceptance remains subject to the combined R03 Foundation API Review after R03-003.
