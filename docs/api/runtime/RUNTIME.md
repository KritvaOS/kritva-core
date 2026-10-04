# Runtime

Contract of the `runtime::Runtime` interface and its concrete implementation `runtime::RuntimeManager` (R0.3, `CORE-RT-002`, `CORE-RT-006`..`CORE-RT-008`; platform attachment R0.4/R0.5, `CORE-PLAT-010`, `CORE-PLAT-015`; readiness boundary R0.9, `CORE-CAP-009`). The normative text is `runtime/runtime.hpp` and the contract block in `runtime/runtime_manager.hpp`; this page restates it and adds no design. Where those sources are silent, this page says so: behavior no contract source states is not promised (`docs/compatibility/COMPATIBILITY_POLICY.md`, section 4).

## 1. Purpose

`Runtime` is the single top-level runtime abstraction: four lifecycle operations and a state query. `RuntimeManager` is its concrete, synchronous, platform-independent implementation. It orchestrates registered Components over the existing `ComponentRegistry` and `DependencyGraph` (see `docs/api/runtime/DEPENDENCY_GRAPH.md`) and the Core lifecycle (see `docs/api/lifecycle/LIFECYCLE.md`). It owns neither the components nor any thread, scheduler, timer or executor.

## 2. API surface

`Runtime` (pure virtual; virtual destructor): `initialize()`, `start()`, `stop()`, `shutdown()` (each returns `Result<void>`) and `state() const noexcept` (returns `LifecycleState`).

`RuntimeManager final : Runtime` (default constructible; copy deleted; the header states it is neither copyable nor movable):

| Member | Role |
|---|---|
| `register_component(Component&)`, `add_dependency(dependent, dependency)` | Setup; forward the registry/graph `Result` unchanged |
| `topology_fixed()` | `true` once an `initialize()` has succeeded; `noexcept` |
| `attach_platform(IPlatformAdapter&)`, `platform()` | Setup-time, non-owning platform reference; `platform()` is the adapter or `nullptr` |
| `component_order()` | `Result<std::vector<ComponentId>>`, exactly `DependencyGraph::order(registry)` |
| `registry()`, `dependencies()` | Read-only inspection views |
| `configure(const Configuration&)` | Configure every component (not part of `Runtime`) |
| `reset()` | Explicit recovery from FAULT |
| `fault_error()` | `const Error*`, non-null exactly while in FAULT |
| `statistics()` | The runtime-owned `Statistics` |
| `initialize/start/stop/shutdown/state` | Overrides of `Runtime` |

## 3. Semantics and invariants

**Setup and fixed topology.** `register_component()` and `add_dependency()` return the registry/graph `Result` unchanged (duplicate id, self dependency, duplicate edge, cycle and invalid id are rejected exactly as those classes reject them, leaving the manager unchanged). Setup is allowed only until the first successful `initialize()` validates and fixes the topology; afterwards both fail with `INVALID_STATE` and change nothing, including after `stop()` and re-`initialize()`. There is no unregister and no topology mutation. `component_order()` lists every registered component once, dependencies first, ties by lowest `ComponentId`, independent of insertion order; the manager has no ordering algorithm of its own.

**Orchestration.** Each operation first checks the Runtime's own state; an invalid call fails with `INVALID_STATE` (no component source), changes nothing and invokes no component. Otherwise it invokes the one matching Component operation once per component:

| Operation | Valid from | Component call | Order | On success |
|---|---|---|---|---|
| `configure` | `UNKNOWN`, `STOPPED` | `configure(cfg)` | forward | state unchanged |
| `initialize` | `UNKNOWN`, `STOPPED` | `initialize()` | forward | `READY` |
| `start` | `READY` | `start()` | forward | `RUNNING` |
| `stop` | `READY`, `RUNNING` | `stop()` | reverse | `STOPPED` |
| `shutdown` | `UNKNOWN`, `STOPPED` | `shutdown()` | reverse | state unchanged |

"Forward" is dependency order; "reverse" is exactly the reverse sequence. The order validated by the first `initialize()` is never recomputed. The Runtime state changes only after the whole sequence succeeded. Every component receives the same `Configuration`; `configure()` uses the current order (components registered later are not configured), does not fix the topology, and fails without invoking any component if the topology is invalid (the graph's `CONFIGURATION_ERROR`, unchanged). A failed topology validation in `initialize()` returns the graph's error unchanged, leaves the Runtime `UNKNOWN` and invokes nothing.

**Progress is recorded by the Runtime, never read.** The Runtime never reads `Component::lifecycle_state()` and never changes a component's state itself. `shutdown()` invokes only live components that have not yet completed a shutdown in the current live period (so none is shut down twice), and is a no-op in `UNKNOWN` or after all are shut down.

**Failure propagation.** The first failing component ends the sequence: remaining components are not invoked, nothing is retried, no completed step is rolled back by the operation itself. The component's own `Error` is returned unchanged (code, severity, source = that component's id, message). A failed `configure()` or `shutdown()` leaves the state unchanged (a `shutdown()` retry resumes with the failing component). A failed `initialize()`, `start()` or `stop()` moves the Runtime to `FAULT` and records that `Error`, observable through `fault_error()` (storage owned by the Runtime, valid only while in `FAULT`, otherwise `nullptr`). In `FAULT` every operation except `reset()`, including `shutdown()`, fails with `INVALID_STATE` and invokes nothing.

**Recovery.** `reset()` is the only way out of `FAULT`, only the caller can invoke it, and it is valid only in `FAULT`. Nothing retries, supervises or recovers automatically, and component health is never consulted. It performs cleanup, each step once, in reverse dependency order: pass 1 `stop()` every component recorded initialized or started; pass 2 `shutdown()` every component recorded stopped or faulted. On success `FAULT` becomes `STOPPED` and `fault_error()` becomes null; the failed operation is not retried. A cleanup failure returns that component's `Error` unchanged, the Runtime stays `FAULT`, `fault_error()` still reports the original failure, and progress is kept. A new `initialize()` after `reset()` is a new explicit attempt. `FAULT -> RECOVERING -> READY` exists in the lifecycle table but the Runtime does not use it.

**Statistics.** `statistics()` is updated only inside the call that causes the event: `sample_count` counts component invocations that returned success, `error_count` those that returned failure (cleanup included), `retry_count` is never incremented, and `drop_count`, `queue_depth`, `utilization` are not used. It is plain data, not an atomic snapshot, not for synchronization or recovery decisions.

**Platform attachment.** `attach_platform()` is a setup operation: after the topology is fixed, or while an adapter is already attached, it fails with `INVALID_STATE` and changes nothing; there is no detach. It, and every other Runtime operation, calls no adapter member; nothing about the adapter (presence, services, capabilities, failures, watchdog expiry) changes a state, order, fault, `reset()` or statistics. A platform failure becomes a Runtime failure only when an integrator-written Component returns it as its own failed `Result`. Platform service lifecycle is independent of the Runtime's (`docs/api/platform/PLATFORM_ADAPTER.md`).

**Readiness boundary.** The Runtime never evaluates capabilities or requirements and calculates no readiness; it only propagates component failures. See the readiness boundary in `docs/api/lifecycle/LIFECYCLE.md`. Health is independent of Runtime `FAULT`, which does not mean unhealthy.

`state()` reports the state after the last completed operation; transient `INITIALIZING` and `STOPPING` are never observable after a call returns.

## 4. Ownership and lifetime

Components are held as non-owning references: the manager never owns, copies, moves or deletes one, destroying it never touches one, and a registered component must outlive any use of the manager. The platform adapter is likewise a single non-owning reference owned by the integrator and must outlive any use of `platform()`. `RuntimeManager` is neither copyable nor movable. `registry()` and `dependencies()` return views of manager-owned state; a const registry still yields mutable `Component*`. No singleton, global adapter or service locator exists.

## 5. Lifecycle interaction

The Runtime has the Core lifecycle states and the same operation table as Component (`docs/api/lifecycle/LIFECYCLE.md`, `docs/api/runtime/COMPONENT.md`). Configuration is valid only from `UNKNOWN` and `STOPPED` and does not change the state (`docs/api/configuration/CONFIGURATION.md`).

## 6. Error behavior

`INVALID_STATE` for an invalid Runtime operation (no source, no effect, no component invoked) and for setup after the topology is fixed. Registry, graph and topology-validation errors are returned unchanged. Component errors are returned unchanged and never replaced by a generic Runtime error. There is no warning API and no logging: diagnostics are structured values only (the returned `Error`, `fault_error()`, `statistics()`, `state()`). Beyond what is stated here the contract is silent on messages. See `docs/api/error/ERROR_CODES.md`.

## 7. Thread safety

Synchronous and single-threaded: no thread-safety guarantee is made and callers serialize all calls, including `statistics()`, `fault_error()` and `platform()`. The contract is silent on any concurrent access.

## 8. Allocation, blocking and real time

Control-plane. Setup, `configure()`, `initialize()` and `component_order()` allocate; `start()`, `stop()` and `shutdown()` do not allocate beyond what components do; `attach_platform()` and `platform()` allocate only to build the `Error` of a failed attach. Blocking is whatever the components do; the Runtime adds no wait. R0.3 makes no real-time claim, and none is made here. Complexity is not stated beyond `DependencyGraph::order()` (see `docs/api/runtime/DEPENDENCY_GRAPH.md`).

## 9. Compatibility

`Runtime` is the only Runtime abstraction; `RuntimeManager` adds no second one and is `final`. The client-visible call contract (the five `Runtime` operations, the `RuntimeManager` members above, the orchestration table, fail-fast, `reset()` semantics) is frozen per `docs/compatibility/COMPATIBILITY_POLICY.md` and `docs/compatibility/VERSIONING_POLICY.md`; changing it, or the pure-virtual set of `Runtime`, needs the evolution review of `docs/compatibility/VERSIONING_POLICY.md` (section 5). The R0.4/R0.5 platform additions are additive and optional. Using `FAULT -> RECOVERING` or adding a readiness state is an architecture-review matter.

## 10. Security considerations

The Runtime is not a trust boundary: it does not authenticate, authorize or isolate components. `FAULT` is a lifecycle state, not a security event, and `fault_error()` and `statistics()` are operational information only. See `docs/security/TRUST_BOUNDARIES.md`.

## 11. Examples

Illustrative only; not a tested snippet.

```cpp
RuntimeManager rt;                       // components a, b, c outlive rt
rt.register_component(a);                // returns Result<void>; check it
rt.register_component(b);
rt.add_dependency(b.info().id(), a.info().id());   // b depends on a
rt.configure(cfg);                       // optional; same cfg to every component
if (auto r = rt.initialize(); !r) {      // forward order; first failure ends it
    // r.error() is the component's own Error; rt.state() == FAULT
    // rt.fault_error() reports it until reset() succeeds
    if (rt.reset()) { /* STOPPED; a new initialize() is a new, explicit attempt */ }
    return;
}
rt.start();                              // forward; READY -> RUNNING
rt.stop();                               // reverse; -> STOPPED
rt.shutdown();                           // reverse; state unchanged
```

## 12. Requirements traceability

`CORE-RT-002`, `CORE-RT-006`, `CORE-RT-007`, `CORE-RT-008`, `CORE-RT-009`, `CORE-PLAT-010`, `CORE-CAP-009`.

## 13. Related headers

`runtime/runtime.hpp`, `runtime/runtime_manager.hpp`, `runtime/component.hpp`, `runtime/component_registry.hpp`, `runtime/dependency_graph.hpp`, `runtime/component_id.hpp`, `statistics/statistics.hpp`, `configuration/configuration.hpp`, `lifecycle/lifecycle.hpp`, `platform/adapter.hpp`.

## 14. Related tests

`tests/unit/runtime_test.cpp`, `tests/unit/runtime_manager_test.cpp`, `tests/unit/runtime_lifecycle_test.cpp`, `tests/unit/runtime_failure_test.cpp`, `tests/unit/runtime_platform_lifecycle_test.cpp`, `tests/integration/runtime_integration_test.cpp`, `tests/integration/runtime_platform_test.cpp`, `tests/integration/runtime_platform_integration_test.cpp`, `tests/integration/capability_readiness_integration_test.cpp`, `tests/unit/api_compat_boundary_test.cpp`.

## 15. Explicit exclusions

No scheduler, thread, executor or timer; no supervision, automatic retry, restart or recovery; no health-driven action; no readiness calculation or capability/requirement evaluation; no service registry, locator or singleton; no unregister, topology change after the first successful `initialize()`, or adapter detach; no logging or warning API; no real-time claim.
