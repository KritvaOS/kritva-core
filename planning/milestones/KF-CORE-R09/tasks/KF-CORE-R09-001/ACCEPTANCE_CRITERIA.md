# KF-CORE-R09-001 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R09-001 |
| Status | PLANNED |
| Primary commit | `feat(core): define capability contract` |
| Reviewer | ChatGPT — independent acceptance gate |
| Estimated effort | 2–3 ED |
| Dependency | R09 Scope Confirmation |

## Objective

Define the normative generic meaning of Capability and CapabilityId, including provider semantics, identity ownership, descriptive metadata and explicit non-security semantics, without introducing a new public type.

## Proposed Requirements Traceability

CORE-CAP-004

The implementing task must update authoritative root `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- The accepted contract states the authoritative identity rule.
- The accepted contract distinguishes provider declaration from consumer requirement.
- Capability metadata is explicitly non-security evidence.
- No new production type/API is introduced unless justified and approved by API Review.
- `docs/api/capability/CAPABILITY.md` is created or updated in the same task.
- Requirements, API documentation and tests are mutually traceable.
- Negative tests cover zero/invalid capability identity where applicable.

## New Tests

- Capability identity equality/mismatch tests.
- Provider metadata independence tests.
- Negative invalid-id tests where the existing API supports them.

## Regression Tests

- Full pre-R09 CTest suite remains green.
- Existing R0.8 Runtime, Component, Configuration, Platform and Context behavior remains green.

## Build

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
```

## Test

```bash
ctest --test-dir build --output-on-failure
```

Where applicable:

```bash
make check
make traceability-check
```

## Coverage / Quality

- Newly added executable production lines must be covered.
- No unexplained coverage exclusion.
- Overall R0.9 coverage remains at or above 98% and has no unexplained regression greater than 1 percentage point from the R0.8 98.9% baseline.
- Required sanitizer, strict-analysis and install-consumer evidence is supplied at milestone validation.

## API Documentation

- Any accepted public API or semantic contract change is documented in the corresponding `docs/api/` Markdown in the same task.
- Documentation must match accepted headers, semantics, ownership/lifetime, lifecycle interaction, errors, threading/real-time expectations and exclusions.
- API/documentation mismatch is an acceptance blocker.

## Security Impact

- The implementor records `SECURITY IMPACT: NONE`, `SECURITY IMPACT: DOCUMENTATION ONLY`, or `SECURITY IMPACT: ARCHITECTURE REVIEW REQUIRED`.
- No new security mechanism is introduced without explicit architecture approval.

## Expected Files Changed

- `Capability-related public headers only if semantics require clarification.`
- `tests/unit/ capability contract tests.`
- `docs/api/capability/CAPABILITY.md.`
- `Traceability/requirements updates as acceptance reconciliation.`

## Git Commit

```text
feat(core): define capability contract
```

## Evidence Required from Implementor

- `git status` before and after implementation.
- `git diff --check`.
- Build/test output.
- Focused contract-test evidence.
- Mutation evidence for contract-sensitive behavior where applicable.
- API documentation diff.
- Security-impact assessment.
- Diff/stat summary.
- Commit SHA.
- Explicit mapping from every acceptance criterion to objective evidence.

## Implementor Evidence

Primary commit: `4081dc0` `feat(core): define capability contract` (consult approved with two amendments; baseline corrections A1–A4 are in the earlier commit `c250c54`, `docs(core): reconcile R09 documentation baseline`, which restores `intern-work-package.md`, places the domain docs under `docs/api/`, keeps one `BOUNDARIES.md` and archives the R0.2 files with provenance — `make check` passed there). **No production type, signature or behavior change:** the production diff is contract text and tags in `capability/capability.hpp` and `capability/capability_id.hpp`; `src/` is byte-identical to `kritva-core-r0.8`.

- Contract (CORE-CAP-004; API.md section 48; `docs/api/capability/CAPABILITY.md`, authored in this same commit as D12 requires, with all 15 sections of `API_GUIDELINES.md`): a Capability is a plain copyable value that **describes** functionality an entity reports as providing (CapabilityId, name, Version) and is not a credential, authorization token, proof of trust, lifecycle state, availability or readiness flag or health signal; `CapabilityId` (valid iff non-zero) is the only identity; `name` is metadata for humans (not unique, may be empty, repeats across ids, never parsed or matched); providers publish by-value `CapabilitySet` snapshots that are claims Core never verifies, probes, grants, revokes, caches or notifies about and that do not follow the lifecycle; **provision is distinct from requirement**; no id ranges, namespaces or vendor encodings.
- Amendment 2 wording (invalid identity), exactly as approved: `CapabilitySet::add()` accepts the value unchanged, so a set can physically hold an entry with an invalid `CapabilityId`; that entry is **storable data, not an authoritative capability declaration and not a valid capability contract**; a conforming provider must not publish one; it can never satisfy a requirement because a requirement cannot name an invalid id. The three notions are kept distinct in the header, the API page and the requirement text: a storable entry, an authoritative capability, a satisfied requirement. `add()` is unchanged.
- Tests (`kritva_core_capability_contract`, 7 functions): exact shape (aggregate of exactly identity, name, version via structured binding; a by-name detector, checked against itself, finds none of token/credential/secret/signature/authorize/authenticate/grant/available/is_available/health/state/ready/vendor/platform); identity valid iff non-zero and compared by value, default Capability has no identity; **name is metadata, not identity** (same name different ids coexist, same id different name replaces, empty name legal, a name that looks like another id proves nothing); a Capability is a detached value (copy, move, 300-character name); providers publish by-value snapshots (return types by `static_assert`), querying has no effect, is repeatable, changing a snapshot never changes the provider and the declaration does not follow the lifecycle; a declaration is neither availability nor proof (an identity-based check is the only relation; any undeclared identity, including 2,000,000 and the maximum, is missing); **an invalid-identity entry is storable but not authoritative and satisfies nothing** (stored and found by its invalid key, a requirement cannot name it, a valid requirement stays missing, only a real identity satisfies it).
- Mutation evidence (11 production mutants, each reverted): find matching by name, requiring a non-empty name, replace-only-if-same-name, add() silently dropping invalid ids, `Id::valid()` also rejecting 1, requirement-side id check removed, a requirement satisfied by id+1, a size-dependent skip (survived the first run; closed by the missing-identity loop), and `Capability` gaining a `credential` or an `available` member (killed by the structured-binding and detector assertions). All detected after closure.
- Regression: ctest 58/58 in Release, ASan+UBSan, strict `-Werror` (0 warnings), TSan, Debug; GCC `-fanalyzer` clean; coverage 618/625 unchanged; `make check`, traceability 101 requirements, 100 traced, 0 errors (`CORE-CAP-004` defined with a row); `git diff --check` clean.
- Out of scope confirmed: no CapabilitySet/version/ordering semantics (R09-002), no matching/requirement boundary (R09-003), no API change.
- Security-impact assessment (per task checklist): no new API, no new authority boundary, no persistence, discovery, background execution or credential; capability metadata is documented as non-security evidence. Classification **SECURITY IMPACT: DOCUMENTATION ONLY** for this task (the formal R0.9 security review record is completed at R09-006).

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT |
| Decision | **PENDING** |
| Accepted commit | Pending |
| Evidence reference | Pending |
| Date | Pending |

**Reviewer Decision: PENDING — KF-CORE-R09-001 is not yet accepted.**
