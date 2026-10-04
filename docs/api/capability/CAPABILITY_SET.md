# CapabilitySet and Capability Version

Contract of `CapabilitySet` and the meaning of `Capability::version` (R0.9, `CORE-CAP-003`, `CORE-CAP-005`, `CORE-CAP-006`). The normative text is the CapabilitySet contract block in `capability/capability_set.hpp`; this page documents it and may not add semantics the header does not state.

## 1. Purpose

A `CapabilitySet` is the **value** a provider returns to declare what it provides, and the structure consumers look capabilities up in by identity. It is a snapshot, not a registry.

## 2. API surface

| Member | Meaning |
|---|---|
| `void add(Capability)` | append a new identity, or replace the entry with the same identity in place; never fails, validates nothing |
| `bool contains(CapabilityId) const noexcept` | identity lookup |
| `const Capability* find(CapabilityId) const noexcept` | identity lookup; `nullptr` when absent |
| `const std::vector<Capability>& all() const noexcept` | the entries in first-insertion order |
| `bool empty() const noexcept`, `std::size_t size() const noexcept` | emptiness and the number of distinct identities |

Copyable and movable value type. There is **no** remove, erase, clear, merge, union, priority, sort, subscription or notification operation and no global or static set. No new member was added by R0.9.

## 3. Semantics and invariants

1. **At most one entry per `CapabilityId`.** `add(c)` with a present id replaces that entry's name **and** version in place; otherwise it appends. The replaced entry keeps its position and `size()` does not change. Latest wins; no history is kept. (The invalid id, zero, is just another key; an entry with it is storable data, not an authoritative capability: see `CAPABILITY.md`.)
2. **Order is first-insertion order** of the distinct ids: not sorted, not affected by replacement. The same sequence of `add()` calls always gives the same contents and order; the same contents added in a different order have a different observable order.
3. Names and versions may repeat across ids; only the id is a key.
4. `add()` never fails and validates nothing. A set only grows; build a different set to drop an entry.
5. `size()` counts distinct ids; a default set is empty.
6. `contains()` and `find()` use identity only (never name or version), are linear, `noexcept` and allocation-free.

### Capability version

`Capability::version` is the `Version` of the **capability contract** being provided: which revision of the contract (the meaning of the identity) the provider claims to implement. It is **not** runtime state, availability, health, a configuration revision or `ConfigurationVersion` (the type is shared, the meanings are not interchangeable), a build, package or firmware version of the provider or of Core, an update counter, an ordering or dependency value, or authorization or security evidence.

Core never compares, orders, ranges, defaults or interprets a capability version: `Version` has no ordering, matching ignores it, re-adding an id with another version simply replaces the old one (no maximum, minimum or merge), and no version-range matching exists. Whether a provided version is acceptable to a consumer is the consumer's or integrator's policy.

## 4. Ownership and lifetime

The set owns its entries and their names. The pointer from `find()` and the reference from `all()` are valid only until the next `add()` on that set, or until it is moved from or destroyed: do not retain them. A copy is an independent snapshot (changing either never changes the other) and nothing in a set refers into another object. Core keeps no copy of a provider's set.

## 5. Lifecycle interaction

None. A set does not follow, drive or imply a lifecycle state; a provider's snapshot is taken when it is returned (see `docs/api/runtime/COMPONENT.md`).

## 6. Error behavior

No operation fails and no `ErrorCode` is involved. `add()` and copying may throw `std::bad_alloc`.

## 7. Thread safety

Concurrent const use is safe; any concurrent mutation needs external synchronization. Core provides no lock.

## 8. Allocation, blocking and real time

`add()` and copying allocate (vector growth, name copies); lookups are O(n), allocation-free and `noexcept`. Control-plane only; no real-time claim.

## 9. Compatibility

R0.9 is API-neutral: the public surface of `capability_set.hpp` is unchanged and the rules above restate and harden existing behavior (replace-in-place and first-insertion order are the existing, now documented and tested, behavior). A change to the invariants, the order or the by-value snapshot semantics needs architecture review.

## 10. Security considerations

A set is descriptive data: it is not a credential store, proof of trust or an access-control list, and membership grants no authority. Treat declared names and versions as untrusted metadata (they are never used for decisions by Core). See `docs/security/TRUST_BOUNDARIES.md`.

## 11. Examples

```cpp
CapabilitySet s;
s.add(Capability{CapabilityId{5}, "five", Version{1, 0, 0}});
s.add(Capability{CapabilityId{3}, "three", Version{1, 0, 0}});
s.add(Capability{CapabilityId{5}, "five", Version{2, 0, 0}});   // replaces in place: order stays {5, 3}, size() == 2
```

## 12. Requirements traceability

`CORE-CAP-003` (set, replace), `CORE-CAP-005` (version meaning), `CORE-CAP-006` (invariants, ownership/snapshot, determinism).

## 13. Related headers

`capability/capability_set.hpp`, `capability/capability.hpp`, `types/version.hpp`, `configuration/configuration_version.hpp`.

## 14. Related tests

`tests/unit/capability_set_contract_test.cpp`, `tests/unit/capability_test.cpp`.

## 15. Explicit exclusions

No registry, locator, broker or global store; no dynamic discovery or change events; no version-range matching; no removal, merge or priority; no capability credentials.
