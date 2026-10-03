# KF-CORE-R03 — Foundation API Review: Evidence Pack

Prepared by Claude for the architecture reviewer. This file supplies objective evidence and a snapshot of the API surface to be frozen. It contains **no gate decision**: the PASS / CHANGES REQUIRED / BLOCKED decision, the "frozen" checkboxes and the sign-off table in `R03_FOUNDATION_API_REVIEW.md` belong to the reviewer. Where this file proposes policy, it is labeled **PROPOSAL** and needs an explicit decision.

> **Gate outcome: PASS / FROZEN** (03-10-2026). The proposals below were decided as recorded in `R03_FOUNDATION_API_REVIEW.md` ("Recorded decisions"); where they differ, that file prevails.

## 1. State under review

- Repository HEAD for this evidence: `6f9db3d` (`docs(planning): accept R03-003`). Production code last changed in `795fb94`.
- Accepted tasks and commits:

| Task | Primary commit | Follow-ups | Acceptance |
|---|---|---|---|
| KF-CORE-R03-001 Component Contract & Identity | `655c1dd` | `35efee1` (error-source wording), `9a98ab3` (planning traceability) | `4140e15` |
| KF-CORE-R03-002 Component Registry | `7ae9a32` | evidence `4649910` | `0760c93` |
| KF-CORE-R03-003 Dependency Management | `795fb94` | evidence `7e2a53b` | `6f9db3d` |

## 2. Entry-criteria evidence

Fresh `git clone` of the working repository at `6f9db3d`, clean tree:

| Check | Result |
|---|---|
| `rm -rf build && cmake -S . -B build && cmake --build build -j4` | exit 0, 0 warnings |
| `ctest --test-dir build --output-on-failure` | 21/21 passed |
| 10 randomized-order repetitions (`--repeat until-fail:10 --schedule-random`) | all passed |
| Release build | 0 warnings, 21/21 passed |
| ASan + UBSan (`-fsanitize=address,undefined -fno-sanitize-recover=all`) | 0 warnings, 21/21 passed |
| Strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror` | builds, 21/21 passed |
| `make check` (header-check, traceability-check) | passed; 53 requirements, 52 traced (1 reserved), 0 errors |
| `make coverage` | 98% (304/308 lines); `dependency_graph.cpp` 100%, `component_registry.cpp` 94%, remaining lines are pre-existing defensive code or exception-unwind braces |
| Install-consumer test (`kritva_core_install_consumer`) | passed; the installed package exposes `Component`, `ComponentRegistry`, `DependencyGraph` |
| TSan (ASLR disabled), GCC `-fanalyzer` | passed at the task commits (21/21, no analyzer diagnostics); not re-run at this docs-only HEAD |

Registered tests (21): version, lifecycle, foundation_contract, contract, status, health, error, result, capability, configuration, event, statistics, types, time, runtime, component, component_registry, dependency_graph, messaging, platform, install_consumer.

## 3. API surface proposed for freeze

Namespace `kritva::core::runtime`; headers under `include/kritva/core/runtime/`.

```cpp
// component_id.hpp
using ComponentId = kritva::core::Id;                       // valid iff value() != 0

// component_info.hpp
class ComponentInfo {                                       // immutable; only via create()
public:
    static Result<ComponentInfo> create(ComponentId id, std::string name, Version version = {});
    ComponentId id() const noexcept;
    const std::string& name() const noexcept;
    const Version& version() const noexcept;
};

// component.hpp
class Component {
public:
    virtual ~Component();
    Component(const Component&) = delete;  Component& operator=(const Component&) = delete;
    const ComponentInfo& info() const noexcept;             // non-virtual
    virtual Result<void> configure(const Configuration&) = 0;
    virtual Result<void> initialize() = 0;
    virtual Result<void> start() = 0;
    virtual Result<void> stop() = 0;
    virtual Result<void> shutdown() = 0;
    virtual LifecycleState lifecycle_state() const noexcept = 0;
    virtual Status status() const = 0;
    virtual Health health() const = 0;
    virtual CapabilitySet capabilities() const = 0;
protected:
    explicit Component(ComponentInfo info) noexcept;
};

// component_registry.hpp
class ComponentRegistry {                                   // non-owning; non-copyable, non-movable
public:
    Result<void> register_component(Component&);            // duplicate id -> INVALID_ARGUMENT
    Component* find(ComponentId) const noexcept;
    bool contains(ComponentId) const noexcept;
    std::size_t size() const noexcept;  bool empty() const noexcept;
    std::vector<Component*> components() const;             // ascending ComponentId
};

// dependency_graph.hpp
class DependencyGraph {                                     // ids only; copyable value type
public:
    Result<void> add_dependency(ComponentId dependent, ComponentId dependency);
    std::vector<ComponentId> dependencies_of(ComponentId) const;   // ascending
    std::size_t size() const noexcept;  bool empty() const noexcept;
    Result<std::vector<ComponentId>> order(const ComponentRegistry&) const;
};
```

`runtime/runtime.hpp` (`Runtime` interface, `CORE-RT-002`) is unchanged from R0.2 and is **not** part of this freeze; see section 6, item 1.

## 4. Checklist evidence (reviewer ticks the boxes)

### Component contract
| Item | Evidence |
|---|---|
| `ComponentId` semantics | alias of `Id`, valid iff non-zero; `component_id.hpp`; `component_test.cpp::test_component_id`; approved at R03-001 |
| Lifecycle contract | operation table in `component.hpp` and `ARCHITECTURE.md` ("Runtime component contract"); full matrix in `tests/contract/component_contract.hpp` run by `component_test.cpp` against the reference component; FAULT/shutdown paths in the same test |
| Metadata | `ComponentInfo` (id, non-empty name, `Version`); `component_test.cpp::test_component_info_*` |
| Ownership / lifetime | non-owning; `Component` non-copyable/non-movable; `info()` stable; `test_component_ownership_shape` |
| Result / Status / Error interaction | every `Error` from an instantiated component has `source == info().id()`; `ComponentInfo::create()` errors exempt; R02 `Result`/`Error`/`Status` reused with no new type |

### Registry contract
| Item | Evidence |
|---|---|
| Registration semantics | `register_component(Component&)`, any lifecycle state, no lifecycle effect; `component_registry_test.cpp` |
| Duplicate handling | `INVALID_ARGUMENT`, `source` = id, no replacement, registry unchanged |
| Lookup / contains | `find` -> `Component*`/`nullptr`, `contains`; side-effect free; shallow-const |
| Enumeration and deterministic ordering | `components()` snapshot, ascending `ComponentId`; all 120 registration permutations of 5 ids plus 1000 shuffled ids |
| Ownership / lifetime | strictly non-owning; registered components must outlive registry use (undetectable if violated); registry never touches components |
| Unregister scope | **explicitly decided: none in R03** (R03-002 acceptance) |

### Dependency contract
| Item | Evidence |
|---|---|
| Representation | `ComponentId` edges only, no pointers; `dependency_graph_test.cpp::test_representation_*` |
| Missing dependency | `order()` -> `CONFIGURATION_ERROR`, `source` = dependent, no order, nothing modified |
| Self dependency | `INVALID_ARGUMENT`, graph unchanged |
| Duplicate dependency | `INVALID_ARGUMENT`; rejected, never merged |
| Cycle detection | rejected at insertion with the cycle listed; graph always acyclic; failed adds atomic |
| Topological ordering | dependencies first; every registered component exactly once |
| Deterministic tie-break | lowest `ComponentId`; independent of registration and insertion order (24 insertion orders; 300-graph brute-force oracle) |

### Architecture constraints
| Constraint | Evidence |
|---|---|
| No ROS2/DDS, EtherCAT, Linux/vendor APIs, hardware | new headers/sources include only Core headers and standard `<map> <set> <vector> <string> <utility> <cstddef> <cassert>`; `make traceability-check` enforces CORE-GEN-003 |
| No scheduler/executor, threads | none present; registry/graph are single-threaded control-plane code |
| No runtime lifecycle orchestration | registry and graph never call a component (tests assert zero lifecycle calls) |
| No automatic recovery | `FAULT` is left only by `shutdown()`; recovery is R03-006 |

## 5. Cross-cutting policy review

These are reviewed at this gate and not implemented by R03-001..003. For each item: the **current facts** in the repository, and a **PROPOSAL** for the reviewer to accept, change or reject. No code or contract was changed to support them.

### 5.1 Error policy
- Facts: operational failures use `Result<T>` with `Error {code, severity, source, timestamp, message}`; `ErrorSeverity` is `INFO, WARNING, ERROR, CRITICAL`; no exceptions except `std::bad_alloc` from allocating containers; components set `source` to their id; registry/graph errors set `source` to the offending id.
- PROPOSAL: keep this model for R03-004..006. Runtime failures are returned as `Error` values that preserve the originating component's `Error` (code, source) rather than wrapping them in a second error type; the runtime may add context only in `message`. Failures of component operations that move the component to FAULT remain observable through `lifecycle_state()` and `health()`.

### 5.2 Warning policy
- Facts: `ErrorSeverity::WARNING` exists in `Error` and `Event`; `Health` has `DEGRADED`; there is no non-failing-warning return channel (an operation either succeeds or fails).
- PROPOSAL: R03 introduces no warning API; no `Result` carries a warning alongside success. (Refined at the gate: `HealthState::DEGRADED` is NOT equated with a warning; see the recorded decisions in `R03_FOUNDATION_API_REVIEW.md`.)

### 5.3 Info / diagnostic policy
- Facts: `Status::message()`, `Health::detail()` and `Event` severity `INFO` exist; Core contains no logging backend, no `<iostream>`, `<cstdio>` or syslog.
- PROPOSAL: diagnostics are values (status message, health detail, event) that callers may export; the runtime does not print or log. Messages are control-plane strings and may allocate.

### 5.3a Event versus message
- Facts: `Event` (`event/event.hpp`) is a fixed envelope (event id, source id, type, timestamp, severity, correlation id) for things that happened inside Core (types: lifecycle, status, health, error, configuration, capability). `messaging::MessageHeader` (id, source id, timestamp) and `Topic` (name) are transport-neutral addressing for application data. Neither has a delivery mechanism in Core.
- PROPOSAL: keep the distinction (events describe Core-internal state changes; messages carry application data) and add no publisher/subscriber or delivery mechanism in R03. If R03-004..006 emit events at all, they only return/expose `Event` values; any bus, queue or callback is out of scope.

### 5.4 Statistics update policy
- Facts: `Counter`/`Gauge`/`Statistics` are plain, single-writer, not thread-safe, wrap modulo 2^64, no allocation.
- PROPOSAL: there is no global statistics object. A runtime that wants counters owns a `Statistics` and updates it only from the thread of control that calls its (synchronous) operations, for example counting each failed component operation in `error_count`; counters are never part of the `Component` contract and never gate behavior.

### 5.5 Logging backend boundary
- Facts: none exists in Core.
- PROPOSAL: confirm "no logging backend in Core". Optional hardening (not done): extend the `CORE-GEN-003` audit to reject `<iostream>`, `<cstdio>`, `<syslog.h>` in `include/` and `src/`.

## 6. Open items and risks for the reviewer

1. **`CORE-RT-002` / existing `Runtime` interface.** `runtime/runtime.hpp` already defines an abstract `Runtime` (initialize/start/stop/shutdown/state) under authoritative `CORE-RT-002`. R03-004 must decide whether the concrete Runtime Manager implements that interface (extending `CORE-RT-002`) or is a separate type under `CORE-RT-006`. This evidence does not decide it.
2. **Append-only registry.** With no unregister, a runtime cannot remove a component; R03-004 should treat the set of components as fixed once it is created.
3. **Undetected lifetime violation.** Using a registry after a registered component was destroyed is undefined behavior and cannot be detected by design.
4. **Allocating enumeration.** `components()` and `order()` allocate on each call; the runtime should call them during setup, not in a periodic path.
5. **Duplicate-id error code.** `INVALID_ARGUMENT` was used because `ErrorCode` has no `ALREADY_EXISTS`; revisiting this is an R02 error-contract extension, not an R03 change.
6. **Shallow-const registry.** A `const ComponentRegistry` still yields mutable `Component*`; deliberate, documented.
7. **Not re-run at this HEAD:** TSan and GCC `-fanalyzer` (run at each task commit; HEAD differs only by planning documents).

## 7. Reviewer actions

1. Decide each policy proposal in section 5 (accept / change / reject) and record it.
2. Tick the "frozen" items in `R03_FOUNDATION_API_REVIEW.md` and record PASS / CHANGES REQUIRED / BLOCKED.
3. Decide open item 1 (relationship of the Runtime Manager to `CORE-RT-002`) before R03-004 starts, or delegate it explicitly to R03-004.
