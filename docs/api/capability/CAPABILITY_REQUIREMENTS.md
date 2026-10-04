# Capability Requirements, Provision and Matching

Contract of the boundary between a capability **provided** by an entity and a capability **required** by a consumer (R0.9, `CORE-CAP-007`, `CORE-CAP-008`). The normative text is the provision/requirement/matching block in `capability/capability_set.hpp`; the requirement mechanism itself is `platform/requirements.hpp` (R0.5, unchanged). This page documents them and may not add semantics the headers do not state.

## 1. Purpose

To state, in one place, how a provider's `CapabilitySet` relates to a consumer's prerequisites, using only the existing generic mechanisms, and to rule out implicit matching, implicit resolution and any coupling to Component dependency ordering.

## 2. API surface

No new type or member. The relevant existing surface:

| Item | Header |
|---|---|
| `CapabilitySet::contains()`, `find()` | `capability/capability_set.hpp` |
| `platform::PlatformRequirements::add_capability(CapabilityId, Requirement)`, `capabilities()` | `platform/requirements.hpp` |
| `platform::CapabilityRequirement { CapabilityId id; Requirement level; }`, `platform::Requirement { REQUIRED, OPTIONAL }` | `platform/requirements.hpp` |
| `platform::evaluate()`, `platform::check_required()`, `platform::PlatformRequirementReport` | `platform/requirements.hpp` |
| `ComponentContext::has_capability()`, `evaluate()`, `check_required()` | `runtime/component_context.hpp` |

There is no `require_capability`, `resolve`, `bind`, `inject`, `satisfy`, `discover`, `provider_of` or capability-dependency operation anywhere.

## 3. Semantics and invariants

- **Provision is not requirement.** Provision is what a provider *declares*; a requirement is what a consumer *needs*. They are related only by an explicit check made by the consumer or integrator. Core never derives one from the other, or either from a Component's type, name or behavior.
- **Matching is identity-only.** A requirement for `CapabilityId` X is satisfied exactly when the provider's snapshot contains an entry with that id. Nothing else is consulted: not the capability's name or version (no equality, range or compatibility test), not the provider's name, version, vendor or kind (`PlatformInfo`), not the platform type, operating system or hardware, and no lookalike or other inference. Whether a provided *version* is acceptable is the consumer's or integrator's policy, applied outside the identity match.
- **Invalid identity.** A requirement cannot name an invalid id (it is rejected with `INVALID_ARGUMENT` when declared), so a storable entry with an invalid id can never satisfy a requirement.
- **Declaration is atomic and identity-deduplicated.** A rejected declaration changes nothing; a duplicate is decided by identity, not by (identity, level), with no silent upgrade or merge.
- **`REQUIRED` decides, `OPTIONAL` informs.** `check_required()` succeeds when no `REQUIRED` item is missing (otherwise `UNSUPPORTED`); `evaluate()` is the authoritative structured report and lists every missing item in declaration order.
- **Evaluation is explicit, side-effect free and deterministic.** It takes at most **one** capability snapshot from the provider per evaluation (none when no capability is required), starts, stops, configures, creates or changes nothing, and gives the same report for the same inputs.
- **The Runtime never evaluates.** The Runtime does not evaluate, resolve, bind, inject or satisfy any capability requirement, and takes no capability snapshot, in any lifecycle operation.
- **Not a Component dependency.** `DependencyGraph` orders `ComponentId`s only. Capability provision or requirement never adds, removes or reorders an edge, never changes the dependency order or a registration, and a Component providing what another requires is not thereby ordered before it. Ordering is declared with `add_dependency()`; requirements are checked by the integrator.

## 4. Ownership and lifetime

`PlatformRequirements` is a plain value the consumer owns. Evaluation copies one provider snapshot and refers to nothing afterwards; the report is an independent value. A `ComponentContext`/`PlatformContext` is a non-owning view (see `docs/api/context/`).

## 5. Lifecycle interaction

None by Core. Whether a missing prerequisite makes a Component's own lifecycle operation fail is decided by that Component or the integrator, using the existing `Result`/`Error` model; Core adds no lifecycle state and no readiness calculation, and health does not change any of it (see `docs/api/lifecycle/LIFECYCLE.md`).

## 6. Error behavior

Declaring an invalid or duplicate requirement fails with `INVALID_ARGUMENT`; `check_required()` fails with `UNSUPPORTED` (message for humans only; decide on the report); `ComponentContext::check_required()` attributes that error to the component. No `ErrorCode` was added.

## 7. Thread safety

That of the provider's adapter for evaluation; none for a shared mutable `PlatformRequirements`.

## 8. Allocation, blocking and real time

Declaring and evaluating allocate (a snapshot copy, report vectors); control-plane only, no real-time claim.

## 9. Compatibility

R0.9 is API-neutral and `platform/requirements.hpp` is unchanged: the rules above restate the R0.5 behavior and its R0.6 context binding. A version-range or name-based matcher, a Component requirement API, or any automatic resolution would be a new architecture decision.

## 10. Security considerations

Capability claims are descriptive, not proof: matching by identity establishes only that a provider *declared* an id, never that it is trustworthy, authorized or working. Do not use a satisfied requirement as an authorization decision. See `docs/security/TRUST_BOUNDARIES.md`.

## 11. Examples

```cpp
platform::PlatformRequirements needs;
needs.add_capability(CapabilityId{100}, platform::Requirement::REQUIRED);
needs.add_capability(CapabilityId{101}, platform::Requirement::OPTIONAL);

const auto report = platform::evaluate(needs, platform::PlatformContext(adapter));   // explicit, side-effect free
if (!report.satisfied()) { /* the integrator or component decides what to do */ }
```

## 12. Requirements traceability

`CORE-CAP-007` (provision vs requirement boundary), `CORE-CAP-008` (identity-only matching), with `CORE-PLAT-013` (requirement model) and `CORE-CTX-004` (context binding) unchanged.

## 13. Related headers

`capability/capability_set.hpp`, `platform/requirements.hpp`, `platform/context.hpp`, `runtime/component_context.hpp`, `runtime/dependency_graph.hpp`.

## 14. Related tests

`tests/unit/capability_requirement_boundary_test.cpp`, `tests/unit/platform_requirements_test.cpp`, `tests/unit/component_context_requirements_test.cpp`.

## 15. Explicit exclusions

No generic dependency injection, service registry or locator, dynamic discovery, automatic dependency resolution, version-range matching, Component requirement API, capability credentials, or platform-, vendor- or hardware-specific matching.
