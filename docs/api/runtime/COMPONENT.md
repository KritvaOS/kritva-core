# Component

Contract of `runtime::Component`, the client-implemented interface of a lifecycle-managed unit of Kritva Core. The normative text is the contract comment in `runtime/component.hpp`; this page restates it and adds no design. Where the header is silent this page says so, and by `docs/compatibility/COMPATIBILITY_POLICY.md` (section 4) behavior that no contract source states is not promised.

## 1. Purpose

To give every lifecycle-managed component one platform-independent contract: an immutable identity, five synchronous lifecycle operations, and by-value snapshots of lifecycle state, status, health and capabilities. It reuses the Core lifecycle states and transition table and the Core `Result`/`Error`/`Status`/`Health` types; it introduces no second lifecycle or error abstraction and no `CONFIGURED` state. A concrete component adds platform-specific work in its own implementation, not in this interface.

## 2. API surface

| Member | Kind | Notes |
|---|---|---|
| `const ComponentInfo& info() const noexcept` | non-virtual | Immutable id, name, version (`runtime/component_info.hpp`, `runtime/component_id.hpp`). |
| `Result<void> configure(const Configuration&)` | pure virtual | Supplies configuration; does not change the lifecycle state. |
| `Result<void> initialize()`, `start()`, `stop()`, `shutdown()` | pure virtual | Synchronous lifecycle operations. |
| `LifecycleState lifecycle_state() const noexcept` | pure virtual | State after the last completed operation; a new component is `UNKNOWN`. |
| `Status status() const`, `Health health() const` | pure virtual | Snapshots by value. |
| `CapabilitySet capabilities() const` | pure virtual | Snapshot by value. |
| `explicit Component(ComponentInfo) noexcept` | protected constructor | The only way to construct the base. |
| copy constructor, copy assignment | deleted | The header states the component is neither copyable nor movable; see section 4. |
| `virtual ~Component() = default` | public | |

## 3. Semantics and invariants

All operations are synchronous: each returns only when it has completed, so the transient `INITIALIZING` and `STOPPING` are not observable after an operation returns.

| Operation | Valid from | On success | Core transitions used |
|---|---|---|---|
| `configure()` | `UNKNOWN`, `STOPPED` | state unchanged | none |
| `initialize()` | `UNKNOWN`, `STOPPED` | `READY` | `INITIALIZING` then `READY` |
| `start()` | `READY` | `RUNNING` | `READY` to `RUNNING` |
| `stop()` | `READY`, `RUNNING` | `STOPPED` | optional `STOPPING`, then `STOPPED` |
| `shutdown()` | `UNKNOWN`, `STOPPED` | state unchanged | none (nothing to release) |
| `shutdown()` | `FAULT` | `STOPPED` | `FAULT` to `STOPPED` |

- A rejected configuration is reported with its `Error`, is not partially applied and leaves the state unchanged.
- `shutdown()` releases what `initialize()`/`start()` acquired and leaves the component re-initializable; it is idempotent in `UNKNOWN` and `STOPPED` and not valid in `READY` or `RUNNING` (call `stop()` first).
- `FAULT` can only be left by `shutdown()`. `RECOVERING` is in the Core state set but no Component operation produces it.
- `info()` is non-virtual and `noexcept`; the reference stays valid and unchanged until destruction and is unaffected by lifecycle operations. The id is held in a `const` member.
- Not stated by the header: which `Status`/`Health` values a component reports in a given state, the content of `capabilities()`, and the contents accepted by `configure()` (see `docs/api/configuration/CONFIGURATION.md`).

## 4. Ownership and lifetime

Core never owns, copies or deletes components; the owner is whoever constructed it. Core types that refer to a component (the registry and the Runtime) hold a non-owning reference, so the owner must keep it alive at a stable address while registered or used. Components are neither copyable nor movable, because the address is part of the identity for non-owning holders. The owner should `shutdown()` before destroying; Core makes no call on the component after the owner unregisters it or destroys the Runtime. Values from `status()`, `health()` and `capabilities()` are snapshots by value and cannot dangle.

## 5. Lifecycle interaction

The component uses the Core lifecycle table (see `docs/api/lifecycle/LIFECYCLE.md`); the Runtime that calls these operations in dependency order is described in `docs/api/runtime/RUNTIME.md`. Platform access, if any, is through `ComponentContext` (`runtime/component_context.hpp`), not through this interface. Readiness is not a Core concept: a component decides inside its own operations whether a prerequisite suffices and fails with an existing `Error` if not. Capability snapshots do not drive lifecycle (`docs/api/capability/CAPABILITY.md`).

## 6. Error behavior

- An invalid operation (any state/operation pair not marked valid above) returns a failed `Result` with `ErrorCode::INVALID_STATE`, leaves the state unchanged and has no other effect.
- Failure of a valid `initialize()`, `start()` or `stop()` moves the component to `FAULT` and returns the cause as the `Error`. Failure of `shutdown()` leaves the state unchanged (a `FAULT` component stays `FAULT`) and returns the `Error`.
- There is no automatic retry or recovery.
- Failures are reported only through `Result<void>`. The operations are not `noexcept` in the interface, but a conforming implementation must not let exceptions escape.
- Every `Error` returned by an operation of an instantiated component carries `source == info().id()`. Errors from `ComponentInfo::create()` occur before a component exists and have no source.
- Suggested codes: `INVALID_STATE` for invalid operations; `INVALID_ARGUMENT` or `CONFIGURATION_ERROR` for a rejected configuration; otherwise the most specific Core `ErrorCode`.

## 7. Thread safety

Core imposes no universal thread-safety guarantee on components. Callers must serialize all lifecycle operations on one component. `info()` is safe to read concurrently because it is immutable. `lifecycle_state()`, `status()`, `health()` and `capabilities()` are safe concurrently with other calls only if the implementation documents it. Core provides no scheduler, executor or threads for components.

## 8. Allocation, blocking and real time

All operations are control-plane: they may allocate and block and are not for use in real-time paths. Core makes no hard-real-time claim. `info()` is `noexcept`; the header states no further allocation or complexity bounds for the accessors, so none are promised.

## 9. Compatibility

`Component` is a client-implemented interface, so by `docs/compatibility/COMPATIBILITY_POLICY.md` and `docs/compatibility/VERSIONING_POLICY.md` (section 4 and the change table): no virtual member may be removed, retyped or reordered, and no pure virtual may be added (MAJOR only). A non-pure virtual with a default may be added only after recorded evolution review. When or in what order Core calls a virtual, or what it expects back, is fixed. The pure-virtual set is pinned by a compile-time check in `tests/unit/api_compat_boundary_test.cpp`.

## 10. Security considerations

`capabilities()` is a claim made by the component, not proof that it can do what it declares. `health()` and `status()` are operational information, not a security or trust signal. `Component` performs no authentication or authorization; identity (`info().id()`) is a label, not a credential. See `docs/security/TRUST_BOUNDARIES.md`.

## 11. Examples

Illustrative only; not a complete implementation. It mirrors the compile-time fixture used by the compatibility test.

```cpp
struct MyComponent final : kritva::core::runtime::Component {
    explicit MyComponent(ComponentInfo info) : Component(std::move(info)) {}
    Result<void> configure(const Configuration&) override { return Result<void>::success(); }
    Result<void> initialize() override { /* UNKNOWN/STOPPED -> READY */ return Result<void>::success(); }
    Result<void> start() override { /* READY -> RUNNING */ return Result<void>::success(); }
    Result<void> stop() override { return Result<void>::success(); }
    Result<void> shutdown() override { return Result<void>::success(); }
    LifecycleState lifecycle_state() const noexcept override { return state_; }
    Status status() const override { return {}; }
    Health health() const override { return {}; }
    CapabilitySet capabilities() const override { return {}; }
private:
    LifecycleState state_ = LifecycleState::UNKNOWN;
};
```

A conforming implementation must also enforce the validity table, set `state_` accordingly and attribute every `Error` to `info().id()`; the skeleton omits that.

## 12. Requirements traceability

`CORE-RT-001` (component contract), `CORE-RT-006`..`CORE-RT-008` (how the Runtime orchestrates components), `CORE-CFG-004`..`CORE-CFG-006` (configuration eligibility and atomicity), `CORE-OPS-002`, `CORE-OPS-003` (Status and Health ownership), `CORE-CAP-009` (readiness boundary).

## 13. Related headers

`runtime/component.hpp`, `runtime/component_info.hpp`, `runtime/component_id.hpp`, `runtime/component_context.hpp`, `runtime/runtime_manager.hpp`, `lifecycle/lifecycle.hpp`, `lifecycle/lifecycle_state.hpp`, `status/status.hpp`, `health/health.hpp`, `capability/capability_set.hpp`, `configuration/configuration.hpp`.

## 14. Related tests

`tests/unit/component_test.cpp`, `tests/unit/api_compat_boundary_test.cpp`, `tests/unit/component_configuration_test.cpp`, `tests/unit/component_status_health_test.cpp`, `tests/integration/runtime_integration_test.cpp`, `tests/contract/component_contract.hpp`.

## 15. Explicit exclusions

No platform-specific dependency management in the interface; no readiness state or query; no `CONFIGURED` state; no scheduler, executor or threads; no automatic retry, restart or recovery; no ownership of components by Core; no universal thread-safety or real-time guarantee.
