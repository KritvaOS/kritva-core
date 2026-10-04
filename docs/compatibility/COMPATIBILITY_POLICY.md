# Kritva Core Source and Semantic Compatibility Policy (R1.0)

**Status:** defined by KF-CORE-R10-002 (CORE-COMPAT-002, CORE-COMPAT-003). Normative for the Core 1.x line once KF-CORE-R10-001..005 are accepted and the R10 API / Compatibility Review has frozen it.
**Authority:** this page is the authoritative source and semantic compatibility policy. `API_INVENTORY.md` is the inventory it applies to. The ABI posture, the release-impact (SemVer) mapping, the evolution/review process and the deprecation policy are defined by later R1.0 tasks and are not defined here.

## 1. Purpose

To state, for the stable public Core API, what a client may rely on across Core 1.x releases at the source level (it still compiles) and at the semantic level (it still behaves as documented), and which kinds of change threaten either. The policy classifies changes; it does not map them to version numbers (R10-004) and it does not define how a removal is announced (R10-005).

## 2. Scope

The policy applies to everything classified `stable` in `API_INVENTORY.md`, which at the R0.9 baseline is all 49 installed public headers. It does not apply to:

- `experimental`, `internal` or `test-only` items;
- `src/`, `tests/`, `scripts/` and generated documentation, which are not installed and not contractual;
- behavior that no contract states (see section 4);
- the CMake package, whose version selection is classified by R10-004 and tested by R10-007;
- binary compatibility (R10-003).

## 3. Compatibility dimensions

| Dimension | Question | Defined by |
|---|---|---|
| Source | Does a client written against an earlier 1.x release still compile, with the same meaning, against a later one? | this page, section 5 |
| Semantic | Does a client that compiles still observe the documented behavior? | this page, section 6 |
| ABI / binary | Does a previously built binary still link and run? | R10-003 (no promise is made here) |

The dimensions are independent. A change can be source-compatible and semantically breaking (a signature-preserving change of documented behavior), and a change that keeps every guarantee in a document can still break source (an added overload). Either is a compatibility event.

## 4. Contract sources and precedence

The contract of a stable item is stated, in decreasing precedence, by:

1. the contract text in the item's header, together with the requirement(s) it carries;
2. the owning API page under `docs/api/` named in the inventory (a `stub` page adds nothing beyond the header);
3. the requirement row in `REQUIREMENTS.md`.

Where these disagree, the disagreement is a defect to be resolved by review, never a license to pick the weaker reading. **Behavior that none of them states is not promised.** Implementation details, undocumented side effects, incidental performance, the exact text of an `Error` message and the contents of `src/` are not part of the contract. A header that carries no contract text and no requirement beyond its signature is contractually only its signature until its contract is documented; documenting an existing behavior is itself compatible (section 5, class `Compatible`).

## 5. Source compatibility

A change is **source-compatible** when every well-formed client that uses only documented members of stable items in a documented way continues to compile and to mean the same thing. Changes fall into three classes:

| Class | Meaning |
|---|---|
| `Compatible` | Permitted in a stable 1.x line without a recorded compatibility review. |
| `Review-required` | May break some clients; permitted only through the API evolution review (`VERSIONING_POLICY.md`) with a recorded decision. A change that does not pass that review is treated as `Incompatible`. |
| `Incompatible` | Breaks well-formed clients; a stable item may not change this way in the 1.x line (removal and replacement follow the deprecation policy, R10-005). |

| Change to a stable item | Class | Reason |
|---|---|---|
| Add a new header, or a new type, function or constant with a new name in `kritva::core` | Compatible | Existing clients do not name it. |
| Document existing behavior, fix a comment, add an example | Compatible | No behavior or declaration changes. |
| Remove or rename a header, type, function, member, enumerator or constant | Incompatible | Clients that name it fail to compile. |
| Change a signature: parameter or return type, constness, reference qualification, template parameters, or `explicit` | Incompatible | Existing calls no longer bind or change meaning. |
| Change an alias or `using` to a different type | Incompatible | Clients may depend on the type. |
| Add an enumerator to a public enumeration or add an `ErrorCode` value | Review-required | Clients with an exhaustive `switch` under strict diagnostics fail to compile, and clients with `default` change behavior (design decision D07). |
| Add a data member or member function to an existing class | Review-required | It can change aggregate initialization and structured bindings, overload resolution, name hiding in derived classes, and the address-of-member use. |
| Add an overload or a defaulted parameter to an existing function | Review-required | It can make previously unambiguous calls ambiguous and breaks taking the function's address. |
| Add `noexcept`, `constexpr`, `[[nodiscard]]` or `inline` to an existing declaration | Review-required | It changes the function type or the diagnostics seen by clients. |
| Remove `noexcept` where the contract does not document a no-throw guarantee | Review-required | It changes the function type, and clients may still have relied on it. |
| Remove `noexcept` where the contract documents the no-throw guarantee | Incompatible | It weakens a documented guarantee (section 6). |
| Add a pure virtual member to an interface that clients implement | Incompatible | Every existing implementation becomes abstract. |
| Add a non-pure virtual member with a default to an interface that clients implement | Review-required | It is source-compatible for implementers but changes the interface Core may call; its semantic effect is classified in section 6. |
| Change or remove a virtual member's signature | Incompatible | Existing overrides stop overriding. |
| Make a stable type non-copyable, non-movable, non-default-constructible, or change a documented aggregate or trivially-copyable property | Incompatible | Clients use these properties. |
| Make a public header stop being self-contained, or make it depend on a header outside `kritva/core` or the C++ standard library | Incompatible | Clients include headers individually and Core stays platform independent. |

**Most severe class.** Where a change matches more than one row of the source and semantic tables, the most severe class applies (`Incompatible` over `Review-required` over `Compatible`). For example, removing a documented `noexcept` matches both a source row and a semantic row and is `Incompatible`.

## 6. Semantic compatibility

A change is **semantically compatible** when every client that relies only on the documented contract continues to observe it. The following properties of a stable item are protected because clients depend on them even when no signature changes:

| Property | What is protected |
|---|---|
| Ownership and lifetime | Who owns a value, how long a returned reference, pointer or view stays valid, what invalidates it (for example `CapabilitySet::find()` and `all()` in `capability/capability_set.hpp` remain valid only until the next `add()`), and which objects must outlive which. |
| Thread safety and real time | The documented thread-safety, blocking, synchronization and allocation behavior of an operation, and any documented real-time guarantee. |
| Error behavior | Which `Result`/`Error`/`ErrorCode` an operation returns for a documented condition, that a failed operation changes nothing where that is documented, and that an operation documented not to fail does not. |
| Observable identity and ordering | Identity rules (for example `CapabilityId` is the only capability identity), uniqueness, replacement rules and the documented order of enumeration (for example first-insertion order). |
| Enumeration values and meanings | The numeric value and meaning of every enumerator of a public enumeration, including every `ErrorCode`, `LifecycleState` and `StatusCode`. |
| Lifecycle legality | Which lifecycle transitions and Component operations are valid from which state, and what state each documented outcome produces. |
| Virtual-interface call contract | For an interface Core calls (for example `Component` in `runtime/component.hpp`), when Core calls each virtual, in what order, from which thread assumptions and with what expectations of the result. |
| Defaults, preconditions and invariants | The documented default state of a value, the preconditions an operation accepts and the invariants a type maintains. |

| Change that preserves every signature | Class | Reason |
|---|---|---|
| Weaken a precondition so inputs that previously failed now succeed, where the previous failure was not a documented guarantee | Review-required | A client may rely on the rejection. |
| Strengthen a precondition, or reject an input that was documented as accepted | Incompatible | Previously valid calls now fail. |
| Change the `ErrorCode` returned for a documented condition, or change when an operation fails | Incompatible | Clients branch on the code. |
| Change a numeric value or the meaning of an existing enumerator | Incompatible | Clients store, compare and switch on it. |
| Weaken a documented ownership, lifetime, thread-safety, blocking, allocation or real-time guarantee | Incompatible | Clients built their own guarantees on it. |
| Strengthen such a guarantee (for example remove a lock) without changing documented behavior | Compatible | Existing reliance remains valid. |
| Change the documented order, uniqueness or identity rule of an observable collection | Incompatible | Clients rely on iteration order and lookup. |
| Change a lifecycle transition rule or the state a documented operation produces | Incompatible | Runtimes and Components rely on it. |
| Change when or in what order Core calls a virtual of a client-implemented interface, or what it expects back | Incompatible | Implementers rely on the call contract. |
| Change observable behavior that no contract states (section 4) | Compatible | It is not promised; a client that relied on it was outside the contract. |
| Change the text of an `Error` message | Compatible | The message is for humans and is not parsed or compared. |
| Fix behavior that contradicts its own documented contract, restoring the implementation to the already-authoritative contract | Compatible | The documented contract is what is promised; the fix must be recorded in the changelog and covered by a regression test, and it must not be used to reinterpret an ambiguous contract. |

If the existing contract is itself ambiguous, a fix that chooses one interpretation is an evolution decision (R10-004), not a compatible bug fix.

A signature-preserving change of documented behavior is therefore a semantic API change and is classified and reviewed exactly like a signature change (design decision D06).

## 7. Compatibility is not security

Compatibility, versioning or package-selection claims say nothing about the authenticity, provenance or trustworthiness of an artifact, and compatibility does not authorize any caller. This policy introduces no security mechanism and no new trust boundary (`docs/security/`, R10 security planning).

## 8. Relationship to other R1.0 policy

| Concern | Owner |
|---|---|
| ABI / binary compatibility posture | R10-003 |
| Release-impact (major/minor/patch) classification, public enum/ErrorCode/virtual-interface/constant evolution rules, change review process | R10-004 |
| Deprecation, migration and removal | R10-005 |
| Validation harness for these rules | R10-006 |
| Package and version-selection behavior | R10-004 / R10-007 |

## 9. Requirements traceability

- `CORE-COMPAT-002` — source compatibility rules (section 5).
- `CORE-COMPAT-003` — semantic compatibility rules (section 6).
- `CORE-COMPAT-001` — the inventory this policy applies to (`API_INVENTORY.md`).

The structure and references of this page are audited mechanically by `scripts/audit/check_compat_policy.py`; the audit never judges whether a classification is right.

## 10. Explicit exclusions

This page defines no ABI promise, no version-number mapping, no deprecation window, no migration procedure, no package-selection rule and no new public API. It does not change any header, source or behavior.
