# Kritva Core Deprecation and Migration Policy (R1.0)

**Status:** defined by KF-CORE-R10-005 (CORE-COMPAT-008). Normative for the Core 1.x line once KF-CORE-R10-001..005 are accepted and the R10 API / Compatibility Review has frozen it.
**Authority:** this page is the authoritative lifecycle for withdrawing a stable item. The change classes are `COMPATIBILITY_POLICY.md`; the release impact and the evolution review are `VERSIONING_POLICY.md`; the register of current deprecations is `DEPRECATIONS.md`.

## 1. Purpose

To let a stable item be withdrawn without surprising clients: a deprecation is announced, documented, carries a way forward, lasts long enough to act on, and ends only in a MAJOR release.

## 2. Scope

The policy applies to stable items (`API_INVENTORY.md`), meaning a header, type, function, member, enumerator or constant. It does not apply to `experimental` items (which carry no promise), `internal` or `test-only` items. R1.0 establishes the policy only: **no stable item is deprecated at 1.0.0**, none is deprecated to demonstrate the policy, and a deprecation is introduced only when a real candidate exists and passes evolution review.

## 3. Lifecycle

```text
STABLE  ->  DEPRECATED  ->  REMOVABLE  ->  REMOVED (in a MAJOR release)
```

| State | Meaning |
|---|---|
| STABLE | Fully supported under the compatibility policy. |
| DEPRECATED | Still present with its full contract unchanged, marked and documented as scheduled for removal, with migration guidance. |
| REMOVABLE | The compatibility window has elapsed; the item may be removed in the next MAJOR release. |
| REMOVED | Gone in a MAJOR release. This is the only release kind that removes a stable item. |

## 4. Deprecating an item

Marking an item deprecated is a `Review-required` change (a deprecation marker changes the diagnostics clients see, and warnings become errors under `-Werror`). It is released in a MINOR release after a recorded evolution review (`VERSIONING_POLICY.md`) and never in PATCH. The change must, in the same logical change:

1. mark the item with the standard `[[deprecated("...")]]` attribute whose message names the replacement or states that there is none, and states the earliest removal;
2. state in the header contract text and the owning documentation that the item is deprecated, since which version, why, and what replaces it;
3. add a row to `DEPRECATIONS.md` (the register) and, where a whole header is deprecated, classify that header `deprecated` in `API_INVENTORY.md`;
4. provide migration guidance (section 6);
5. add a `CHANGELOG.md` entry and update requirements traceability and the compatibility tests.

A deprecated item keeps its full contract: its behavior, error codes, ownership, lifetime and thread-safety guarantees are unchanged until removal, and it still compiles. A deprecation never weakens a guarantee.

## 5. Compatibility window and removal

- A stable item is removed only after it has been published as deprecated, except under the emergency exception of `VERSIONING_POLICY.md`.
- Every deprecation names a **replacement**, or, where none exists, a documented **rationale**.
- A deprecated item is published as deprecated in at least one released version before the MAJOR release that removes it: it cannot be deprecated and removed in the same release.
- The earliest removal is the **next MAJOR release after the release that deprecated it** (a deprecation in 1.4 earliest removes in 2.0). A MAJOR release may keep a deprecated item longer, and a removal is never implied by the passage of time alone.
- Removal is itself an `Incompatible` change released as MAJOR with the migration guidance finished and the register row closed (the item leaves the register when it leaves the headers).
- The emergency exception of `VERSIONING_POLICY.md` is the only way to remove a stable item outside a MAJOR release.

## 6. Migration guidance

Every deprecation and every `Incompatible` change ships migration guidance that states: what changed, which release changed it, who is affected, the exact replacement (before and after usage), any behavioral difference the client must handle, and how to detect remaining uses (the deprecation diagnostic). Guidance lives in the owning API documentation and is referenced from the register row and the changelog; a MAJOR release additionally gathers its guidance under `docs/compatibility/migration/` in a page named for that MAJOR version. Guidance never promises ABI compatibility (`ABI_POLICY.md`).

## 7. Register

`DEPRECATIONS.md` lists every deprecated item exactly once and is authoritative for item-level deprecation. The inventory class `deprecated` in `API_INVENTORY.md` applies only when a whole header is deprecated (and that header then also has register rows); an individual function, type, member or enumerator deprecated in an otherwise stable header leaves the header `stable`.

| Header | Item | Deprecated since | Replacement or rationale | Earliest removal | Migration |
|---|---|---|---|---|---|

`scripts/audit/check_compat_policy.py` audits it mechanically: every `[[deprecated` in `include/kritva/core/` has a register row for its header, every register row names a header that contains `[[deprecated`, every row is complete, the deprecated-since version precedes the earliest-removal MAJOR release and a removal is a MAJOR release strictly after the deprecating release. The audit checks the register and the code against each other; it cannot judge whether a replacement is adequate.

## 8. Security considerations

A deprecation or a migration path is not a security statement. A deprecated item keeps its security properties until removal, and migrating to a replacement does not relax any validation or authority boundary. Removing an item because it is unsafe is an emergency exception (`VERSIONING_POLICY.md`) and is recorded with its justification; deprecation is never used to hide a security change. This policy introduces no security mechanism.

## 9. Relationship to other R1.0 policy

| Concern | Owner |
|---|---|
| Source and semantic classes | `COMPATIBILITY_POLICY.md` (R10-002) |
| ABI posture | `ABI_POLICY.md` (R10-003) |
| Release impact and evolution review | `VERSIONING_POLICY.md` (R10-004) |
| Validation harness | R10-006 |
| Package behavior | R10-007 |

## 10. Requirements traceability

- `CORE-COMPAT-008` — deprecation, migration and removal policy (this page and the register).
- `CORE-COMPAT-005`, `CORE-COMPAT-006`, `CORE-COMPAT-007` — the release impact and review this policy uses.

## 11. Explicit exclusions

This page deprecates no Core item, adds no attribute to any header, and changes no header, source or behavior. It introduces no new public API, ABI mechanism, package rule or version change.
