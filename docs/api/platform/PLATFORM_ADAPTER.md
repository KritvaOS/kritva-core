# Platform Adapter

Contract of the client-implemented platform boundary `IPlatformAdapter` and the four platform services it exposes by reference (`CORE-PLAT-008`; shared boundary rules `CORE-PLAT-004`; Runtime attachment `CORE-PLAT-010`). The normative text is the contract block in `platform/adapter.hpp`; this page restates it and the cited requirement rows and adds no design. Where a contract source is silent this page says so: per `docs/compatibility/COMPATIBILITY_POLICY.md` section 4, behavior that no contract source states is not promised.

## 1. Purpose

To give an integrator one minimal, platform-independent interface through which a platform says who it is, which of four platform services it provides and which capabilities it reports. Core defines the contract; platform code outside `kritva-core` implements it. Core contains no concrete platform.

## 2. API surface

| Item | Meaning |
|---|---|
| `PlatformInfo { std::string name; Version version; }` | adapter identity: a non-empty label and the adapter's version, nothing else |
| `PlatformService { SCHEDULER, CLOCK, TIMER, WATCHDOG }` | the four services (underlying type `std::uint8_t`) |
| `info() const noexcept` | the identity, by const reference (pure virtual) |
| `scheduler() const noexcept` | `IScheduler*` or `nullptr` (pure virtual) |
| `clock() const noexcept` | `time::IClock*` or `nullptr` (pure virtual) |
| `timer() const noexcept` | `time::ITimer*` or `nullptr` (pure virtual) |
| `watchdog() const noexcept` | `IWatchdog*` or `nullptr` (pure virtual) |
| `capabilities() const` | `CapabilitySet` snapshot by value (pure virtual, may allocate) |
| `supports(PlatformService) const noexcept` | non-virtual; derived from the accessors |

The services follow their own contracts: `platform/scheduler.hpp` (`IScheduler`), `time/clock.hpp` (`IClock`), `time/timer.hpp` (`ITimer`), `platform/watchdog.hpp` (`IWatchdog`). Their pure-virtual sets are pinned by `tests/unit/api_compat_boundary_test.cpp`.

## 3. Semantics and invariants

- **Identity is immutable.** `info()` never changes during the adapter's life, two calls return equal values and the reference stays valid until the adapter is destroyed. `PlatformInfo::name` is a label, not an identity key. There are no CPU, operating-system, board or vendor fields or types; such detail is reported, if at all, through adapter-defined capabilities.
- **Services are optional and non-owning.** A non-null pointer refers to an adapter-owned object that stays valid and is the same object on every call until the adapter is destroyed. `nullptr` means "not supported"; it is never an error and the adapter never returns a placeholder that pretends to support a service.
- **Asking activates nothing.** Querying a service never starts a task, timer or watchdog, never opens or instantiates hardware and has no side effect. The accessors are `const` yet return mutable service pointers because the services are the adapter's non-owning handles.
- **`supports()` cannot disagree.** It is true exactly when the matching accessor is non-null and false exactly when it returns `nullptr`; an unknown enumerator value gives false.
- **Capabilities are a snapshot.** `capabilities()` returns an owned copy that stays valid and unchanged whatever the adapter does later. Identities are chosen by the adapter; Core reserves no identity range for platforms and an adapter shall not give an identity in its own set a different meaning or version. The same adapter reports equal sets (same entries, same order) unless it documents otherwise. Capabilities describe, they do not enable: reporting one instantiates no hardware, activates no feature and changes no Runtime behavior. See `docs/api/capability/CAPABILITY.md`.
- **The Runtime never calls the interface.** `RuntimeManager::attach_platform()` only stores a non-owning reference (setup-time, before the first successful `initialize()`, once, no detach); no Runtime operation calls `info()`, `supports()`, any accessor or `capabilities()`.
- **Contract silent on:** the content of `PlatformInfo::name` beyond "non-empty"; which capabilities an adapter reports; how a caller should react to a missing service (the declarative model is `docs/api/platform/PLATFORM_REQUIREMENTS.md`).

## 4. Ownership and lifetime

The integrator owns the adapter and every service it exposes. Core holds only non-owning references, never creates or destroys a platform object, and has no singleton, global adapter, adapter registry or service locator. The adapter must outlive every Core object that references it and keep a stable address while referenced; use after destruction is undefined behavior Core cannot detect (`platform/boundary.hpp`). No platform contract transfers ownership.

## 5. Lifecycle interaction

None by Core. The Runtime lifecycle and the lifecycle of the platform's services are independent: the integrator or adapter creates, configures, starts, stops and destroys services, and Core never does (`CORE-PLAT-015`). Attaching an adapter changes no Runtime state, dependency order, fault handling or statistics. A platform failure becomes a Runtime failure only when an integrator-written component returns it as its own failed `Result`. See `docs/api/lifecycle/LIFECYCLE.md`.

## 6. Error behavior

The adapter interface itself returns no `Result` and contract operations do not throw. A missing service is `nullptr`, not an error. Errors belong to the services (`Result<T>` with the shared meanings in `platform/boundary.hpp`) and to `attach_platform()`, which fails with `INVALID_STATE` and changes nothing when called after the first successful `initialize()` or while an adapter is attached. Consumption-time failures (`UNSUPPORTED`) come from `platform/context.hpp` and `platform/requirements.hpp`, not from the adapter.

## 7. Thread safety

Adapter-defined; Core imposes no universal guarantee and callers serialize calls unless the adapter documents otherwise. The contract is silent on concurrent use of the services beyond what each service contract states.

## 8. Allocation, blocking and real time

The members are control-plane queries. `capabilities()` may allocate. The contract states no blocking behavior for the accessors and Core makes no hard-real-time, latency or jitter claim for any member.

## 9. Compatibility

`IPlatformAdapter` is a client-implemented interface. Under `docs/compatibility/VERSIONING_POLICY.md` section 4: no virtual member is removed, retyped or reordered and no pure virtual member is added in a MINOR or PATCH release; the documented call contract may not change; a non-pure virtual member with a default needs evolution review and must not alter behavior for implementations that do not override it. `PlatformService` is a stable enumeration: values are never renumbered or reused, an enumerator is added only after review, and clients must tolerate unknown values (`supports()` already returns false for one). `supports()` is non-virtual and is not an extension point. See `docs/compatibility/COMPATIBILITY_POLICY.md`.

## 10. Security considerations

The adapter is trusted integrator code; Core does not authenticate, sandbox or verify it. Reported capabilities and non-null services are claims, not proof that a feature exists or works, and are not authorization. `PlatformInfo` is not an attestation. See `docs/security/TRUST_BOUNDARIES.md`.

## 11. Examples

Illustrative only (it mirrors the fixture in `tests/unit/api_compat_boundary_test.cpp`); it is not a platform implementation.

```cpp
struct MyAdapter final : kritva::core::platform::IPlatformAdapter {
    kritva::core::platform::PlatformInfo info_{"my-platform", {}};  // set once, never changed
    const kritva::core::platform::PlatformInfo& info() const noexcept override { return info_; }
    kritva::core::platform::IScheduler* scheduler() const noexcept override { return nullptr; }  // not supported
    kritva::core::time::IClock* clock() const noexcept override { return &clock_; }               // same object each call
    kritva::core::time::ITimer* timer() const noexcept override { return nullptr; }
    kritva::core::platform::IWatchdog* watchdog() const noexcept override { return nullptr; }
    kritva::core::CapabilitySet capabilities() const override { return {}; }                      // snapshot by value
    mutable MyClock clock_;  // owned by the adapter; MyClock implements time::IClock
};
// Integrator: MyAdapter adapter; runtime.attach_platform(adapter);  // adapter must outlive the runtime
```

## 12. Requirements traceability

`CORE-PLAT-008` (adapter contract), `CORE-PLAT-004` (boundary), `CORE-PLAT-010` (Runtime attachment), `CORE-PLAT-015` (separate lifecycles), `CORE-PLAT-012` (context view), `CORE-PLAT-013` (requirements), `CORE-CAP-006`.

## 13. Related headers

`platform/adapter.hpp`, `platform/boundary.hpp`, `platform/scheduler.hpp`, `platform/watchdog.hpp`, `time/clock.hpp`, `time/timer.hpp`, `platform/context.hpp`, `platform/requirements.hpp`, `capability/capability_set.hpp`, `types/version.hpp`, `runtime/runtime_manager.hpp`.

## 14. Related tests

`tests/unit/platform_adapter_test.cpp`, `tests/unit/platform_boundary_test.cpp`, `tests/unit/api_compat_boundary_test.cpp`, `tests/contract/reference_adapter.hpp`, `tests/integration/runtime_platform_test.cpp`.

## 15. Explicit exclusions

No concrete platform implementation, no vendor types, and no operating-system, RTOS, ROS 2, DDS, EtherCAT or hardware-driver code in Core. No adapter registry, singleton or service locator, no discovery, no service lifecycle management, no authentication of adapters, and no real-time or thread-safety guarantee from Core.
