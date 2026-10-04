# KF-CORE-R09 — Capability API Review

## Status

PLANNED — review record to be completed after R09-003.

## Purpose

Freeze the approved R0.9 public Capability semantics before reference harness and integration work begins.

## Entry Criteria

- R09-001 accepted.
- R09-002 accepted.
- R09-003 accepted.
- R0.8 public API remains compatible unless explicitly approved.
- API documentation updates are present for every proposed public API/semantic change.
- Security impact assessment is recorded for affected changes.
- Mutation/negative tests are supplied for contract-sensitive behavior.

## Review Checklist

- [ ] Capability identity semantics are explicit.
- [ ] Capability version semantics are explicit and not confused with runtime/configuration/security state.
- [ ] CapabilitySet invariants are explicit.
- [ ] Ownership/snapshot semantics are explicit.
- [ ] Requirement versus capability-provider roles are distinct.
- [ ] Capability matching does not infer vendor/platform/name semantics.
- [ ] Component dependency ordering remains separate from capability matching.
- [ ] No new readiness lifecycle state exists unless explicitly approved.
- [ ] Runtime does not automatically calculate readiness or recover based on health/capability state.
- [ ] No ServiceRegistry/locator/resolver/DI framework has been introduced.
- [ ] Core remains platform independent.
- [ ] `docs/api/` accurately reflects the accepted production contract.
- [ ] Security impact is acceptable and documented.
- [ ] Public-header self-containment remains intact.
- [ ] Mutation evidence is sufficient.

## Evidence (submitted 05-10-2026)

Baseline: R09-001 `4081dc0`, R09-002 `4bd241e`, R09-003 `e91a51f` (all ACCEPTED; head `4c86b53`); documentation baseline `c250c54` with the disclosed correction `7b6b0f4`.

### Production diff against `kritva-core-r0.8` (`include/` and `src/`)

| File | Change |
|---|---|
| `capability/capability.hpp` | contract text only (+62): identity and provider semantics (CORE-CAP-004) |
| `capability/capability_id.hpp` | contract text only (+11): CapabilityId authority |
| `capability/capability_set.hpp` | contract text only (+118): CapabilitySet invariants, snapshots and version semantics (CORE-CAP-005/006); provision/requirement/matching boundary (CORE-CAP-007/008) |

`git diff kritva-core-r0.8 HEAD -- src` is **empty**. `platform/requirements.hpp`, `platform/context.hpp`, `platform/adapter.hpp`, `runtime/component.hpp`, `runtime/runtime_manager.hpp`, `runtime/dependency_graph.hpp`, `runtime/component_context.hpp` and every other header are byte-identical to the released tag. No type, signature, enumerator or behavior was added or changed: the frozen "API" is the **normative contract text** plus the unchanged public surface it governs.

### Review checklist evidence

| Check | Evidence |
|---|---|
| Capability identity semantics are explicit | `CapabilityId` is the only identity (valid iff non-zero); name is metadata; `capability_contract_test` (name is metadata, invalid identity storable-but-not-authoritative-and-satisfies-nothing); `docs/api/capability/CAPABILITY.md` |
| Version semantics explicit, not confused with runtime/configuration/security state | provided-contract version; no ordering, matching ignores it, replacement has no max/min/merge; shape tests find no revision/availability/health/credential member; type shared with ConfigurationVersion but meanings documented as not interchangeable |
| CapabilitySet invariants are explicit | at most one entry per id, replace in place, first-insertion order, determinism, identity-only lookup, never fails/validates, only grows; absence of remove/merge/priority/subscribe by a self-checked detector |
| Ownership/snapshot semantics are explicit | by-value provider snapshots, independent copies, reference validity until the next `add()`, nothing refers into a source (ASan) |
| Requirement versus provider roles are distinct | provision is a declared `CapabilitySet`; a requirement is an explicit `PlatformRequirements` item; related only by an explicit check by the consumer/integrator |
| Matching does not infer vendor/platform/name semantics | identity-only; names, versions, `PlatformInfo`, platform kind and lookalikes never consulted (tests and 6 mutants) |
| Dependency ordering remains separate | `DependencyGraph` takes ComponentIds only; a 100-seed property test shows the order is independent of capability assignment |
| No new readiness lifecycle state | no lifecycle type or enumerator changed (boundary snapshot of the lifecycle values is added at R09-006) |
| Runtime does not calculate readiness or recover on health/capability state | the Runtime takes zero adapter capability queries through configure/initialize/start/stop/shutdown; no Runtime member evaluates or resolves; R09-005 adds the integration proof |
| No ServiceRegistry/locator/resolver/DI | by-name absence checks on seven surfaces; dependency scan clean |
| Core remains platform independent | no platform, vendor, OS or hardware identifier in the contract text or production code; scan clean |
| `docs/api/` accurately reflects the contract | `CAPABILITY.md`, `CAPABILITY_SET.md`, `CAPABILITY_REQUIREMENTS.md`, `PLATFORM_REQUIREMENTS.md` authored in the same commits as their contract text (15 required sections each); every `API_INDEX.md` path exists; the mechanical audit script is R09-006 |
| Security impact acceptable and documented | no new API, authority boundary, persistence, discovery, background execution or credential; capability claims documented as non-evidence; per task **DOCUMENTATION ONLY**; formal record at R09-006 |
| Public-header self-containment intact | one TU per public header (`kritva_core_header_checks`) passes |
| Mutation evidence sufficient | 11 + 12 + 11 production mutants; survivors closed or classified equivalent (2) |

### Validation (head `4c86b53`)

`ctest` 60/60 in Debug, Release, ASan+UBSan, strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror` and TSan (ASLR off); 0 warnings; GCC `-fanalyzer` clean; `make check` and traceability: 105 requirements, 104 traced, 0 errors, `CORE-CAP-004` to `008` defined once; coverage 618/625 unchanged; `git diff --check` clean.

### Open issues (none blocking)

Carried forward: 32-bit scheduler affinity mask; conformance level-2 mutation gap; `make lint`/`make format-check` stubs; documented non-owning lifetime UB; the invalid-identity entry remains storable by design (documented, `add()` unchanged). R09-004 and R09-005 add tests only; R09-006 adds documentation/security/boundary validation including the mechanical docs audit.

## Decision

To be completed by independent reviewer:

**PASS / FROZEN** or **CHANGES REQUIRED** or **BLOCKED**

## Freeze Rule

After PASS/FROZEN, no production API or semantic change may occur without explicit return to architecture review.
