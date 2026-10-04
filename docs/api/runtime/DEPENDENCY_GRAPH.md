# Dependency Graph

Contract of `runtime::DependencyGraph` (R0.3, `CORE-RT-004`, `CORE-RT-005`; boundary with capabilities R0.9, `CORE-CAP-007`, `CORE-CAP-009`). The normative text is the contract block in `runtime/dependency_graph.hpp` (unchanged); this page documents it.

## 1. Purpose

`DependencyGraph` expresses explicit **ComponentId-to-ComponentId ordering relationships** and computes a deterministic dependency-respecting order. It is distinct from capability requirements and never infers dependencies from capabilities.

## 2. API surface

`add_dependency(dependent, dependency)`, `dependencies_of(dependent)`, `size()`, `empty()`, `order(registry)`; value type (copyable, movable). `RuntimeManager::add_dependency()` and `component_order()` forward to it.

## 3. Semantics and invariants

- An edge "dependent → dependency" means the dependency comes first in the order. Identity is the `ComponentId` only; pointers, names and capabilities are never used.
- `add_dependency()` fails with `INVALID_ARGUMENT` (source the dependent) for an invalid id, a self dependency, a duplicate edge or an edge that would create a cycle, and leaves the graph unchanged; the graph is therefore always acyclic.
- `order(registry)` returns every registered component exactly once, dependencies first, ties by ascending `ComponentId`, as a pure function of the registered set and the edges (independent of registration or insertion order); an edge endpoint that is not registered fails with `CONFIGURATION_ERROR`; no partial order is ever returned.
- **Capabilities are not dependencies.** A capability being provided or required never adds, removes or reorders an edge, never changes the order or a registration, and a Component that provides what another requires is not thereby ordered before it. Ordering is declared by the integrator with `add_dependency()`; capability requirements are checked by the integrator or component (see `docs/api/capability/CAPABILITY_REQUIREMENTS.md`). The Runtime does not derive edges, readiness or resolution from either.

## 4. Ownership and lifetime

Pure data about `ComponentId`s: it holds no component, pointer or registry and extends no lifetime.

## 5. Lifecycle interaction

None by itself: it never calls a component or starts, stops or initializes anything. The Runtime uses its order for lifecycle operations (forward for initialize/start/configure, reverse for stop/shutdown).

## 6. Error behavior

`INVALID_ARGUMENT` and `CONFIGURATION_ERROR` as above; messages are for humans (a cycle is listed in the message only).

## 7. Thread safety

Not thread-safe: serialize calls on one graph; concurrent const calls are safe only if nobody adds edges.

## 8. Allocation, blocking and real time

Control-plane: `add_dependency()` and `order()` allocate; `add_dependency()` is O(V + E) worst case, `order()` O((V + E) log V); no real-time claim.

## 9. Compatibility

Unchanged by R0.9. The tie-break rule and the failure codes are frozen.

## 10. Security considerations

An edge is an ordering fact, not a trust or authorization relationship. See `docs/security/TRUST_BOUNDARIES.md`.

## 11. Examples

```cpp
runtime.add_dependency(ComponentId{2}, ComponentId{1});   // 2 depends on 1: 1 is initialized first
```

## 12. Requirements traceability

`CORE-RT-004`, `CORE-RT-005`, `CORE-CAP-007`, `CORE-CAP-009`.

## 13. Related headers

`runtime/dependency_graph.hpp`, `runtime/component_registry.hpp`, `runtime/runtime_manager.hpp`.

## 14. Related tests

`tests/unit/dependency_graph_test.cpp`, `tests/unit/capability_requirement_boundary_test.cpp`, `tests/integration/capability_readiness_integration_test.cpp`.

## 15. Explicit exclusions

No capability-derived edges, resolution, discovery, registry-as-locator, runtime reordering or dynamic edges.
