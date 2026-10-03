# KF-CORE-R06 — Design Decisions

## D01 — R0.5 remains authoritative

`IPlatformAdapter` and `PlatformContext` remain the platform boundaries. R06 must not replace them.

## D02 — Context is not a registry

No generic string/name lookup, dynamic service map, singleton, global context or service locator.

## D03 — Non-owning by default

Core does not own platform services or integrator resources through the context.

## D04 — Explicit access only

A context query must not implicitly start, stop, create, destroy, configure or recover any service.

## D05 — Runtime lifecycle remains unchanged

The Component context must not cause RuntimeManager to start using platform services implicitly. R0.3 lifecycle ordering, failure propagation, reset and statistics remain authoritative.

## D06 — Public API only at integration tests

Integration tests must prove the contract without accessing private members or adding test-only production hooks.

## D07 — API freeze before implementation integration

R06-001 through R06-004 must converge before the Component API Review. No implementation integration work proceeds on an unfrozen public contract.

## D08 — Requirement identity remains authoritative

R0.5 `PlatformRequirements`/capability identity semantics are reused; no platform name/version inference is permitted.

## API Decisions (Q1–Q10), approved 05-10-2026 before implementation

Approved by the independent reviewer (ChatGPT) after confirming that Component Execution Context is the desired R0.6 scope. All R06 production API is **additive**: no R0.3, R0.4 or R0.5 signature or semantic changes (`IPlatformAdapter`, `PlatformContext`, `PlatformRequirements`, `Component`, `ComponentInfo`, `RuntimeManager`). The scope confirmed: a minimal, explicit, non-owning execution context for integrator-written Components, providing stable component identity and controlled access to the frozen R0.5 platform integration model; it does not redesign the Runtime lifecycle, change Component lifecycle signatures, modify `IPlatformAdapter` or `PlatformContext` semantics, introduce a service registry or locator, add automatic dependency injection, background execution, Component-to-Component access, health-driven recovery or a concrete platform.

### D09 — `ComponentContext` shape (Q1, Q6)

`runtime/component_context.hpp`, namespace `kritva::core::runtime`: a small copyable value view of two non-owning pointers: a `const ComponentInfo*` (the component's immutable identity) and a `platform::PlatformContext` (the R0.5 view, one pointer). Nothrow default-constructible (unbound), copyable, trivially destructible, `sizeof` exactly two pointers, no cache, no registry, no static/global state, no name/version inference. Constructors: `ComponentContext() noexcept` (unbound), `explicit ComponentContext(const ComponentInfo&, platform::PlatformContext = {}) noexcept`, and `ComponentContext(const Component&, platform::PlatformContext = {}) noexcept` using `component.info()`. Accessors: `bound()`, `const ComponentInfo* info() const noexcept` (null when unbound), `ComponentId id() const noexcept` (the invalid id when unbound), `const platform::PlatformContext& platform() const noexcept`.

**The context is immutable after construction: no rebinding, reset or setter API exists**, so it cannot become mutable shared state. The `ComponentInfo` must outlive the context (it lives as long as its Component) and the adapter must outlive every context; copying a context neither extends a lifetime nor transfers ownership.

### D10 — Access policy (Q2)

The only approved operational access paths: identity (`id()`, `info()`), the four explicit service queries `require_scheduler()`, `require_clock()`, `require_timer()`, `require_watchdog()` returning `Result<T*>`, `supports(PlatformService)`, `has_capability(CapabilityId)`, `attribute(Error)`, and `platform()` returning the R0.5 `PlatformContext` **by const reference only**. Nothing else: no Runtime, Registry, Configuration, Statistics, Health, component lookup or string lookup.

**No context amplification:** `ComponentContext` must not expose any method that returns, directly or indirectly, a broader authority: no `RuntimeManager*`, `ComponentRegistry*`, other `Component*`, `Configuration*`, `Statistics*`, `Health*`, another component's `ComponentContext`, or a raw `IPlatformAdapter*`. `platform()` returning the R0.5 view is acceptable; returning the underlying adapter is not.

### D11 — Error attribution (Q3)

`Error attribute(Error) const noexcept` is **attribution, not translation or wrapping**: it returns the input with **only `source` replaced by the component's `ComponentId`**; `code`, `severity`, `timestamp`, `message` and every other field are unchanged. For an unbound context it returns the original error unchanged. Tests must mutate every field of the input `Error` and verify that only `source` changes.

### D12 — `require_*()` source attribution (Q2)

This is the one place where R06 deliberately extends R0.5 behavior, at the component-facing layer only. `PlatformContext::require_*()` is unchanged (`UNSUPPORTED`, no source). `ComponentContext::require_*()` forwards to it and, when the context is bound, attributes the **Core availability error** (`UNSUPPORTED`) to the component (`source = ComponentId`) so the component can return it directly as its own failed `Result`; code, severity and message are exactly `PlatformContext`'s. **Real platform service errors are never touched**: a service's own `Error` stays unchanged and the component may call `attribute()` explicitly when it returns one. An unbound context returns the R0.5 error without a source.

### D13 — Injection (Q4)

Injection is by **construction only**: the integrator builds the context from the component's own info and a `PlatformContext` (from an adapter or `runtime.platform()`) and the component stores it. No Runtime change, no Component lifecycle-signature change, no new `RuntimeManager` member; the Runtime never creates, stores, passes or probes a `ComponentContext`; R0.3 ordering, failure, reset and statistics are unchanged.

### D14 — Requirement binding (Q5)

`evaluate(const PlatformRequirements&)` delegates unchanged to R0.5 and returns the R0.5 `PlatformRequirementReport`; `check_required(const PlatformRequirements&)` is the R0.5 `check_required` with the D12 attribution rule. The context stores no requirements (stateless); matching stays by service/capability identity only; the R0.5 requirement and report semantics remain authoritative.

### D15 — Test support and gates (Q7–Q10)

The reference context harness lives entirely under `tests/`; the existing R0.5 isolation audit and CTest are extended to cover it; no production test hook. Integration tests use public APIs only and must prove platform-failure-through-a-context equals plain component failure and must test source attribution explicitly. Requirement domain `CORE-CTX-001..007` (existing IDs untouched); commit messages exactly as in `R06_TESTING_AND_COMMIT_POLICY.md`. Gates: Component API Review after R06-004, Integration Freeze after R06-006, Release Gate after R06-007; release metadata 0.6.0 is prepared in R06-007.

