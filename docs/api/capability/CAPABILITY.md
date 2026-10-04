# Capability

Contract of `Capability` and `CapabilityId` (R0.9, `CORE-CAP-001`, `CORE-CAP-002`, `CORE-CAP-004`). The normative text is the capability contract block in `capability/capability.hpp`; this page documents it and may not add semantics the header does not state.

## 1. Purpose

A `Capability` **describes functionality an entity reports as providing**. It lets a provider (a Component, a platform adapter, integrator code) declare *what contract it offers* in a form a consumer can check by identity. It is descriptive metadata about a contract and nothing more.

## 2. API surface

| Item | Header | Shape |
|---|---|---|
| `CapabilityId` | `capability/capability_id.hpp` | alias of `Id`; valid iff non-zero |
| `Capability` | `capability/capability.hpp` | aggregate of exactly three members: `CapabilityId id`, `std::string name`, `Version version` |
| provider entry points | `runtime/component.hpp`, `platform/adapter.hpp` | `Component::capabilities()` and `IPlatformAdapter::capabilities()` return a `CapabilitySet` **by value** |

No new type or member was added by R0.9.

## 3. Semantics and invariants

- **Identity is authoritative.** Two capabilities are the same capability exactly when their `CapabilityId`s are equal. `CapabilityId{}` (zero) is the invalid identity.
- **`name` is metadata for humans.** It is not unique, may be empty, may repeat across different ids, is never parsed or matched, and Core gives no meaning to its spelling (a dotted or vendor-looking name is just text).
- **`version` describes the provided capability contract.** It is not runtime state, availability, health, a configuration revision, an ordering value or evidence (see `CAPABILITY_SET.md`).
- **Provision is a claim.** A declaration is the provider's claim. Core does not verify, probe, grant, revoke, negotiate, cache or notify about it; a snapshot says nothing about whether the functionality currently works.
- **Provision is distinct from requirement.** What a provider declares and what a consumer needs are related only by an explicit, identity-based check made by the consumer or integrator (see `CAPABILITY_REQUIREMENTS.md`). Core never resolves, binds, injects or orders anything because a capability exists.
- **Invalid identity.** `CapabilitySet::add()` accepts a value unchanged, so a set can physically hold an entry whose `CapabilityId` is invalid. Such an entry is **storable data, not an authoritative capability declaration, and not a valid capability contract**; a conforming provider must not publish one, and it can never satisfy a requirement because a requirement cannot name an invalid identity. Three things are distinct: a *storable entry*, an *authoritative capability* and a *satisfied requirement*.

## 4. Ownership and lifetime

A `Capability` owns its `name` and refers to nothing: copies are independent and Core retains no pointer or reference into one. Providers return snapshots by value, so a returned set never refers into the provider and cannot dangle; changing a snapshot never changes the provider.

## 5. Lifecycle interaction

None. A declaration does not follow, drive or imply a lifecycle state; querying `capabilities()` has no effect on a component or the Runtime. Capability presence never changes dependency order and creates no readiness state (see `docs/api/lifecycle/LIFECYCLE.md`).

## 6. Error behavior

`Capability` and `CapabilityId` have no failing operation. Checks that involve capabilities report through the existing `Result`/`Error` model (see `docs/api/error/ERROR.md`); no `ErrorCode` was added.

## 7. Thread safety

A plain value type: concurrent const use is safe; concurrent mutation needs external synchronization. Core provides no locking.

## 8. Allocation, blocking and real time

Constructing, copying and moving never block and use no thread; copying a `name` may allocate. No real-time claim is made for operations that copy names.

## 9. Compatibility

R0.9 is API-neutral: the public surface of `capability.hpp` and `capability_id.hpp` is unchanged and the semantics above restate and harden existing behavior. A change to identity, the members of `Capability` or the by-value provider snapshots needs architecture review.

## 10. Security considerations

A `Capability` is **not** an authentication credential, authorization token, proof of trust or cryptographic evidence, and holding or declaring one grants no authority. `CapabilityId` is contract identity, not an authenticated identity (and the type system does not distinguish it from `ComponentId` or other `Id` aliases: the contract does). Capability claims are descriptive and must not be used as security evidence. See `docs/security/TRUST_BOUNDARIES.md`.

## 11. Examples

```cpp
CapabilitySet provided;                                   // a provider's snapshot
provided.add(Capability{CapabilityId{100}, "motion.control", Version{1, 2, 0}});

const bool declared = provided.contains(CapabilityId{100});   // identity decides
// "motion.control" is a label for people: another capability with the same name and id 101 is a different capability.
```

## 12. Requirements traceability

`CORE-CAP-001` (identity), `CORE-CAP-002` (record), `CORE-CAP-004` (provider contract and authoritative identity).

## 13. Related headers

`capability/capability_id.hpp`, `capability/capability.hpp`, `capability/capability_set.hpp`, `runtime/component.hpp`, `platform/adapter.hpp`.

## 14. Related tests

`tests/unit/capability_contract_test.cpp`, `tests/unit/capability_test.cpp`.

## 15. Explicit exclusions

No capability credentials or tokens, authentication or authorization, dynamic discovery or change notification, service registry or locator, automatic dependency resolution or readiness calculation, and no platform-, vendor- or hardware-specific capability definitions.
