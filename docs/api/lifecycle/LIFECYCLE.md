# Lifecycle

Contract of `Lifecycle`/`LifecycleState` and the readiness boundary (R0.1/R0.3 lifecycle; R0.9 `CORE-CAP-009`). Lifecycle is the authoritative execution-state model for Core Components and the Runtime. Capability, readiness, health and configuration do **not** create parallel lifecycle state machines. The normative text for the component operations is `runtime/component.hpp` and for the Runtime `runtime/runtime_manager.hpp` (both unchanged by R0.9).

## 1. Purpose

To give every Component and the Runtime one shared, explicit, synchronous state model, and to state which other concepts (capabilities, requirements, readiness, health, configuration) are **not** part of it.

## 2. API surface

| Item | Header |
|---|---|
| `LifecycleState`: `UNKNOWN, INITIALIZING, READY, RUNNING, STOPPING, STOPPED, FAULT, RECOVERING` | `lifecycle/lifecycle_state.hpp` |
| `Lifecycle::state()`, `is_running()`, `transition_to(LifecycleState)` | `lifecycle/lifecycle.hpp` |
| Component operations `configure/initialize/start/stop/shutdown`, `lifecycle_state()` | `runtime/component.hpp` |
| Runtime operations and `reset()` | `runtime/runtime_manager.hpp` |

No state was added by R0.9. In particular there is no `NOT_READY`, `WAITING_FOR_DEPENDENCY`, `CONFIGURED` or `RECONFIGURING` state.

## 3. Semantics and invariants

Valid transitions (anything else fails with `INVALID_STATE` and changes nothing): `UNKNOWN→INITIALIZING`; `INITIALIZING→READY|FAULT`; `READY→RUNNING|STOPPED|FAULT`; `RUNNING→STOPPING|FAULT`; `STOPPING→STOPPED|FAULT`; `STOPPED→INITIALIZING`; `FAULT→RECOVERING|STOPPED`; `RECOVERING→READY|FAULT`. Operations are synchronous, so the transient `INITIALIZING` and `STOPPING` are not observable after an operation returns, and no operation produces `RECOVERING`.

Component operation validity and effects are stated in `runtime/component.hpp`: `configure()` from `UNKNOWN`/`STOPPED` (state unchanged); `initialize()` from `UNKNOWN`/`STOPPED` to `READY`; `start()` from `READY` to `RUNNING`; `stop()` from `READY`/`RUNNING` to `STOPPED`; `shutdown()` from `UNKNOWN`/`STOPPED` (unchanged) and from `FAULT` to `STOPPED`. A valid `initialize()`, `start()` or `stop()` that fails moves the component to `FAULT` and returns its `Error`. The Runtime orders these operations by the dependency order, stops at the first failure and leaves `FAULT` only through an explicit `reset()`.

### Readiness and capability boundary (R0.9, `CORE-CAP-009`)

- **Capability provision, capability requirement, Component dependency ordering, lifecycle, readiness and health are distinct concepts** unless an explicit Core contract says otherwise; none of them implies another.
- **Core calculates no readiness.** Readiness is contextual and Component- or integrator-defined. Core never infers it from capabilities, health, platform state or a satisfied requirement, and exposes no readiness state or query.
- **A Component decides whether a prerequisite is sufficient for its own lifecycle operation.** It may check a requirement inside `initialize()` (or any operation), typically through its `ComponentContext`, and when the prerequisite is missing it fails with an **existing** error (for example the context's `UNSUPPORTED`, attributed to the component) through the ordinary `Result`/`Error` model; the usual failed-operation rule then applies (the component is in `FAULT`).
- **The Runtime only propagates.** It treats that failure like any other component failure (fail-fast, `FAULT`, the component's own `Error` unchanged, explicit `reset()` to leave). It never evaluates a requirement, takes a capability snapshot, resolves or binds anything, polls for a capability to appear, retries, recovers or reorders because of a capability.
- A satisfied check changes nothing outside the documented lifecycle operation, and `Health` never changes the lifecycle or readiness automatically; a Runtime `FAULT` is a lifecycle state and does not mean unhealthy (see `docs/api/runtime/RUNTIME.md`).

## 4. Ownership and lifetime

`Lifecycle` is a plain value owned by whoever holds it; the Runtime holds no component state beyond non-owning references.

## 5. Lifecycle interaction

This page is the lifecycle: see above. Configuration is valid only from `UNKNOWN` and `STOPPED` and never changes the state (see `docs/api/configuration/CONFIGURATION.md`).

## 6. Error behavior

An invalid transition or operation fails with `INVALID_STATE` and has no other effect; an operation failure returns the component's `Error` (source the component). No `ErrorCode` was added.

## 7. Thread safety

Core imposes no universal thread safety: serialize lifecycle operations on one component or Runtime; see `runtime/component.hpp` for what an implementation must document.

## 8. Allocation, blocking and real time

Operations are control-plane and may allocate and block; `Lifecycle::transition_to` is `noexcept` and allocation-free. No hard-real-time claim.

## 9. Compatibility

Unchanged by R0.9. The state set, the transition table and the operation semantics are frozen; adding a state (for example a readiness state) requires architecture review.

## 10. Security considerations

Lifecycle state is operational information, not an authorization or trust signal. See `docs/security/TRUST_BOUNDARIES.md`.

## 11. Examples

```cpp
// A component that needs capability 100 decides for itself, in its own initialize():
Result<void> initialize() override {
    if (auto ok = context_.check_required(needs_); !ok) { /* move to FAULT as a failed initialize does */ return ok; }
    /* acquire resources, then READY */
}
```

## 12. Requirements traceability

`CORE-LIF-002`, `CORE-LIF-003` (transitions), `CORE-RT-001`, `CORE-RT-006`..`CORE-RT-008` (component and Runtime lifecycle), `CORE-CAP-009` (readiness boundary).

## 13. Related headers

`lifecycle/lifecycle_state.hpp`, `lifecycle/lifecycle.hpp`, `runtime/component.hpp`, `runtime/runtime_manager.hpp`, `runtime/component_context.hpp`.

## 14. Related tests

`tests/unit/lifecycle_test.cpp`, `tests/integration/runtime_integration_test.cpp`, `tests/integration/capability_readiness_integration_test.cpp`.

## 15. Explicit exclusions

No readiness, waiting or not-ready state; no automatic readiness calculation; no health- or capability-driven lifecycle transition; no automatic retry, restart or recovery; no dependency resolution.
