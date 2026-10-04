# Platform Requirements

Contract of the declarative platform requirement model (R0.5, `CORE-PLAT-013`; capability matching boundary R0.9, `CORE-CAP-007`, `CORE-CAP-008`). The normative text is the contract block in `platform/requirements.hpp`; R0.9 changed no code there. For the provision/requirement boundary see `docs/api/capability/CAPABILITY_REQUIREMENTS.md`.

## 1. Purpose

`PlatformRequirements` lets integrator-written functionality **declare** which platform services and which capabilities it needs and whether each is `REQUIRED` or `OPTIONAL`, so that the integrator can **check** them against a platform without starting or holding anything.

## 2. API surface

| Item | Meaning |
|---|---|
| `Requirement { REQUIRED, OPTIONAL }` | how much an item matters |
| `ServiceRequirement { PlatformService service; Requirement level; }` | scheduler, clock, timer or watchdog |
| `CapabilityRequirement { CapabilityId id; Requirement level; }` | a capability, by identity |
| `PlatformRequirements::add_service()`, `add_capability()`, `services()`, `capabilities()`, `empty()` | declare and read, in declaration order |
| `PlatformRequirementReport` | every missing item, split into required/optional services and capabilities; `satisfied()`, `complete()` |
| `platform::evaluate(requirements, context)` | the authoritative report; never fails |
| `platform::check_required(requirements, context)` | `Result<void>`: success when no `REQUIRED` item is missing, else `UNSUPPORTED` |

## 3. Semantics and invariants

- **Declarative.** A requirement describes a need; it does not look for, start, hold or bind anything.
- **Identity-based matching.** A capability requirement is satisfied exactly when the platform's snapshot contains that `CapabilityId`; a service requirement exactly when `PlatformContext::supports(service)` is true. No name, version, vendor, platform-kind, operating-system or hardware inference, and no version-range matching.
- **Declaration is atomic.** An unknown enumerator, an invalid (zero) `CapabilityId` or a duplicate fails with `INVALID_ARGUMENT` and changes nothing; a duplicate is decided by the service or capability identity, not by (identity, level), with no silent upgrade, downgrade or merge.
- **Evaluation has no side effect.** It queries the context only (`supports()` once per declared service and at most **one** capability snapshot), starts, stops, configures or creates nothing, and gives the same report for the same inputs. An unattached context provides nothing.
- **`REQUIRED` decides; `OPTIONAL` informs.** `check_required()` names the first missing required declaration (services before capabilities, then declaration order) for humans; decide on the report, never by parsing the message.
- **Independent of the Runtime.** The Runtime never evaluates requirements; evaluation is the integrator's or component's explicit act, and requirements never affect `DependencyGraph` ordering.

## 4. Ownership and lifetime

`PlatformRequirements` is a plain copyable value owned by its creator; the report is an independent value. Evaluation reads through a non-owning `PlatformContext` that must not outlive its adapter.

## 5. Lifecycle interaction

None by Core: no lifecycle state or readiness is derived from a requirement. A component may check its requirements inside one of its own lifecycle operations and fail with an existing error (see `docs/api/lifecycle/LIFECYCLE.md`).

## 6. Error behavior

`INVALID_ARGUMENT` for a rejected declaration; `UNSUPPORTED` from `check_required()` (and, through a bound `ComponentContext`, attributed to the component). No `ErrorCode` was added by R0.9.

## 7. Thread safety

That of the context's adapter for evaluation; none for a shared mutable `PlatformRequirements`.

## 8. Allocation, blocking and real time

Declaring and evaluating allocate; control-plane only; no real-time claim.

## 9. Compatibility

Unchanged by R0.9. The model is frozen at R0.5; a change to matching, atomicity or the report needs architecture review.

## 10. Security considerations

A satisfied requirement shows only that a provider declared an identity or exposes a service accessor; it is not authorization, authentication or proof the provider is trustworthy or working. See `docs/security/TRUST_BOUNDARIES.md`.

## 11. Examples

See `docs/api/capability/CAPABILITY_REQUIREMENTS.md`.

## 12. Requirements traceability

`CORE-PLAT-013` (requirement model), `CORE-CTX-004` (binding through a context), `CORE-CAP-007`, `CORE-CAP-008`.

## 13. Related headers

`platform/requirements.hpp`, `platform/context.hpp`, `platform/adapter.hpp`, `runtime/component_context.hpp`.

## 14. Related tests

`tests/unit/platform_requirements_test.cpp`, `tests/unit/component_context_requirements_test.cpp`, `tests/unit/capability_requirement_boundary_test.cpp`.

## 15. Explicit exclusions

No service lifecycle, dependency injection, registry or locator, discovery, automatic resolution, version-range matching, authorization, or platform-, vendor- or hardware-specific rules.
