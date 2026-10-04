# Kritva Core Versioning and API Evolution Policy (R1.0)

**Status:** defined by KF-CORE-R10-004 (CORE-COMPAT-005, CORE-COMPAT-006, CORE-COMPAT-007). Normative for the Core 1.x line once KF-CORE-R10-001..005 are accepted and the R10 API / Compatibility Review has frozen it.
**Authority:** this page maps the change classes of `COMPATIBILITY_POLICY.md` to release impact, fixes the evolution rules for enumerations, error codes, virtual interfaces and public constants, defines the review process for changing a stable item and states the installed-package version-selection rule. Binary compatibility is out of scope (`ABI_POLICY.md`); deprecation and migration are defined by R10-005.

## 1. Purpose

To make the version number of a Core release a reliable statement about what a client can expect, and to make every change to a stable item pass through one explicit, recorded decision.

## 2. Version identity

A Core release is identified by `MAJOR.MINOR.PATCH` (Semantic Versioning 2.0.0, applied to the contracts below, not to file counts or line counts). The repository `VERSION` file and the CMake project version are the same string, and a release is tagged `kritva-core-rMAJOR.MINOR`. Release impact is decided by the contract change, never by the size of the diff.

The 0.x line (R0.2 to R0.9) predates this policy and carries no compatibility promise; `1.0.0` is the first release the policy governs.

## 3. Release impact

Every change to a stable item has a class under `COMPATIBILITY_POLICY.md` and a release impact. The release impact is the lowest version component that may carry the change.

| Change | Compatibility class | Release impact | Condition |
|---|---|---|---|
| Remove or rename a stable header, type, function, member, enumerator or constant | Incompatible | MAJOR | Preceded by deprecation (R10-005). |
| Change a signature, alias, type property or documented semantic guarantee in an incompatible way | Incompatible | MAJOR | Migration guidance (R10-005). |
| Change an enumerator value or the meaning of an existing enumerator or `ErrorCode` | Incompatible | MAJOR | Never permitted in MINOR or PATCH. |
| Add a pure virtual member to a client-implemented interface | Incompatible | MAJOR | Never permitted in MINOR or PATCH. |
| Add an enumerator or `ErrorCode` value | Review-required | MINOR | Recorded evolution review (section 5) and changelog entry. |
| Add a member, an overload or a defaulted parameter to an existing stable type or function | Review-required | MINOR | Recorded evolution review. |
| Add a non-pure virtual member with a default to a client-implemented interface | Review-required | MINOR | Recorded evolution review of its semantic effect. |
| Add `noexcept`, `constexpr`, `[[nodiscard]]` or `inline` to an existing declaration | Review-required | MINOR | Recorded evolution review. |
| Add a new header, or a new type, function or constant with a new name | Compatible | MINOR | Inventory row, documentation and requirement added in the same change. |
| Promote an `experimental` item to `stable` | Compatible | MINOR | Inventory updated by evolution review. |
| Fix behavior that contradicts its own documented contract | Compatible | PATCH | Changelog entry and a regression test; not a reinterpretation of an ambiguous contract. |
| Documentation, comment, example or message-text change; internal change that alters no contract | Compatible | PATCH | None. |

A release whose changes include several rows carries the highest impact among them. A change classed `Review-required` that does not pass evolution review may not be released in any MINOR or PATCH release and is treated as `Incompatible` (MAJOR). Removing a stable item never occurs in a MINOR or PATCH release; an emergency exception is permitted only through explicit architecture review, must be recorded in the changelog and the release record, and is never silent.

## 4. Enumerations, error codes, virtual interfaces and constants

**Enumerations and `ErrorCode`.** The numeric value, underlying type and meaning of every enumerator of a public enumeration is part of the contract. An enumerator is added only with a new value and is released in a MINOR release after evolution review, with the changelog and owning documentation naming it. An existing value is never renumbered, reused for a different meaning or removed outside a MAJOR release; the value of a removed enumerator is retired and not reassigned. The underlying type of a public enumeration does not change outside a MAJOR release. Clients must therefore treat an unknown enumerator value as a possibility when they switch over a stable enumeration.

**Virtual interfaces.** For an interface that clients implement (for example `Component` in `runtime/component.hpp` and `IPlatformAdapter` in `platform/adapter.hpp`) no virtual member may be removed, retyped or reordered in a MINOR or PATCH release, no pure virtual member may be added, and the documented call contract (when Core calls a member, in what order and with what expectation of the result) may not change. A non-pure virtual member with a default may be added in a MINOR release only after evolution review, and Core's use of it must not change the observable behavior for an implementation that does not override it. For an interface that clients only call (for example `Runtime` in `runtime/runtime.hpp`) adding a member is `Review-required` as for any member.

**Public constants and macros.** The value of a public constant is part of the contract and changes only in a MAJOR release; a new constant is a compatible addition.

**Version type.** The `Version` value type in `types/version.hpp` and the capability and configuration versions are data defined by their own contracts; this policy does not change what they mean, and their compatibility rules are those of their owning contracts.

## 5. API evolution review

Every change to a stable item that is not plainly classed `Compatible` PATCH passes this process, and a change to the public API surface or its contract also requires human review (repository `AGENTS.md`).

1. **Proposal.** A written change record names the affected inventory rows (`API_INVENTORY.md`), the class under `COMPATIBILITY_POLICY.md`, the proposed release impact, the reason, the alternatives considered, the documentation and requirement updates, the test updates (R10-006), the migration or deprecation plan where applicable (R10-005) and the security impact (exactly one of NONE, DOCUMENTATION ONLY, ARCHITECTURE REVIEW REQUIRED).
2. **Review.** An independent reviewer decides PASS, CHANGES REQUIRED or BLOCKED. A `Review-required` change needs an explicit PASS; an `Incompatible` change needs an explicit PASS and a MAJOR release plan.
3. **Same-change obligations.** An accepted change updates, in the same logical change: the header contract text, the owning documentation, `API_INVENTORY.md`, `REQUIREMENTS.md` traceability, the compatibility tests and `CHANGELOG.md`.
4. **Record.** The decision, its reference and its release impact are recorded in the changelog and the release record.
5. **Freeze.** Within a milestone, the existing review, freeze and release gates apply unchanged.

A change record for a stable item is a planning or review artifact, not a public header or an API.

## 6. Installed package version selection

The installed CMake package selects a version by this rule, which applies to Core 1.x installs. Let the installed version be `I` and the version a consumer requests with `find_package(kritva_core <version>)` be `R`:

- the package is acceptable only when `I` and `R` have the **same MAJOR** version and `I` is **greater than or equal to** `R` (compared as MAJOR, MINOR, PATCH);
- a request for a **newer** MINOR or PATCH than is installed is rejected;
- a request for a **different MAJOR** (including 0.x against a 1.x install) is rejected;
- a request with no version is accepted;
- an `EXACT` request is accepted only when the versions are equal.

This is the CMake `SameMajorVersion` behavior, stated here so that it is a decision of this policy and not an inherited default. It replaces the pre-1.0 `SameMinorVersion` behavior for Core 1.x (R10-007 implements it in the package build, validates it with the installed-consumer test and defines the package requirement that owns that validation). Selecting a package version says nothing about the authenticity or provenance of the artifact.

Examples for an installed version of `1.2.3`:

| Installed | Requested | Result |
|---|---|---|
| 1.2.3 | 1.0 | Accept |
| 1.2.3 | 1.2.3 | Accept |
| 1.2.3 | 1 | Accept |
| 1.2.3 | 1.2.4 | Reject |
| 1.2.3 | 1.3 | Reject |
| 1.2.3 | 0.9 | Reject |
| 1.2.3 | 2.0 | Reject |

## 7. Compatibility is not security

Versioning, release impact and package selection are not authenticity, provenance or authorization statements, and introduce no security mechanism.

## 8. Relationship to other R1.0 policy

| Concern | Owner |
|---|---|
| Source and semantic compatibility classes | `COMPATIBILITY_POLICY.md` (R10-002) |
| ABI posture | `ABI_POLICY.md` (R10-003) |
| Deprecation, migration and removal procedure | R10-005 |
| Validation harness for these rules | R10-006 |
| Package implementation and installed-consumer validation | R10-007 |

## 9. Requirements traceability

- `CORE-COMPAT-005` — release-impact classification (section 3).
- `CORE-COMPAT-006` — enumeration, `ErrorCode`, virtual-interface and constant evolution rules (section 4).
- `CORE-COMPAT-007` — API evolution review (section 5).
- `CORE-COMPAT-002`, `CORE-COMPAT-003`, `CORE-COMPAT-004` — the compatibility classes and ABI posture this policy builds on.

## 10. Explicit exclusions

This page introduces no public API, no ABI mechanism and no deprecation procedure, and it changes no header, source or behavior. It does not change the package build (R10-007) or the version (R10-009).
