# KF-CORE-R09-002 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R09-002 |
| Status | ACCEPTED |
| Primary commit | `feat(core): define capability set and version semantics` |
| Reviewer | ChatGPT — independent acceptance gate |
| Estimated effort | 2–3 ED |
| Dependency | R09-001 |

## Objective

Define Capability version meaning and CapabilitySet invariants, ownership/snapshot behavior and deterministic observable semantics while avoiding a registry abstraction.

## Proposed Requirements Traceability

CORE-CAP-005, CORE-CAP-006

The implementing task must update authoritative root `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- Version semantics are explicit and unambiguous.
- CapabilitySet invariants are documented and testable.
- Ownership/lifetime rules are explicit.
- Duplicate identity behavior is either defined and enforced or demonstrably absent by the existing contract.
- No hidden global state or registry is introduced.
- `docs/api/capability/CAPABILITY_SET.md` documents the accepted contract.
- Focused tests and mutation evidence cover contract-sensitive invariants.

## New Tests

- Copy/snapshot lifetime tests.
- Identity lookup tests.
- Duplicate/invalid capability tests as applicable.
- Mutation tests for identity and snapshot semantics.

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

- `CapabilitySet public/source files only if needed.`
- `tests/unit/capability_set_contract_test.cpp or equivalent.`
- `docs/api/capability/CAPABILITY_SET.md.`

## Git Commit

```text
feat(core): define capability set and version semantics
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

Primary commit: `4bd241e` `feat(core): define capability set and version semantics`. **No production type, signature or behavior change:** the production diff is the CapabilitySet contract block (and tags) in `capability/capability_set.hpp`; `src/` is byte-identical to `kritva-core-r0.8`. `docs/api/capability/CAPABILITY_SET.md` is authored in the same commit with all 15 required sections; API.md section 49; `CORE-CAP-005` and `CORE-CAP-006` defined with rows.

- Contract: a CapabilitySet is an ownership-safe **value and snapshot, not a registry**. Invariants (all observable, deterministic): at most one entry per CapabilityId; `add()` replaces a present identity's name and version **in place** (position and size unchanged, latest wins, no history) or appends; **observable order is first-insertion order**, not sorted, unaffected by replacement; names/versions may repeat across ids and only the id is a key; `add()` never fails and validates nothing (the invalid id is just another key — the approved wording is kept: a storable entry, not an authoritative capability); no remove/erase/clear/merge/priority/sort/subscription (a set only grows); `contains`/`find` identity-only, linear, noexcept, allocation-free; `find()`/`all()` references valid only until the next `add()` or move/destruction; copies are independent snapshots. **Version semantics:** the version of the *provided capability contract*; not runtime state, availability, health, a configuration revision or ConfigurationVersion (type shared, meanings not interchangeable), a build/package/firmware version, a counter, an ordering/dependency value or evidence; Core never compares, orders, ranges, defaults or interprets it; `Version` has no ordering; matching ignores it; no version-range matching.
- Tests (`kritva_core_capability_set_contract`, 10 functions): exact member signatures by `static_assert` and absence of registry-style operations by a self-checked by-name detector (remove/erase/clear/merge/insert/unite/priority/sort/subscribe/notify/instance/resolve/discover/lookup_by_name/find_by_name); default set empty; **at most one entry per identity with replacement in place** (interleaved adds, name and version replaced, position kept, latest wins over five re-adds); **first-insertion order and determinism** (same sequence gives the same state; another order gives another observable order); names/versions repeating across ids and identity-only keys; the invalid identity as an ordinary distinct key; `find()` pointing at the stored entry and `all()` being the same storage; independent snapshots (copy, assignment, move, source destroyed — ASan); version as metadata (no `<`, `>` or `<=>` on `Version`; an older version added later still replaces; a missing version is not defaulted or interpreted; the type is shared with ConfigurationVersion); **matching ignores version and name** (satisfied for provided versions 0.0.0, 1.2.3 and 99.99.99).
- Mutation evidence (12 production mutants, each reverted): replace implemented as erase+append, replace updating only the name, replace keeping the max major version, replace ignored, new identities inserted at the front, set re-sorted after add, find returning nullptr for empty names, find searching from the back, and `clear()`, `merge()`, `remove()` and `add_checked()` added. **10 detected; 2 survivors classified equivalent:** a find that scans in reverse (the unique-identity invariant, tested separately, makes the result identical) and a find with a dead extra loop after the match (unreachable effect). 
- Regression: ctest 59/59 in Release, ASan+UBSan, strict `-Werror` (0 warnings), TSan, Debug; GCC `-fanalyzer` clean; coverage 618/625 unchanged; `make check`, traceability 103 requirements, 102 traced, 0 errors; `git diff --check` clean.
- Security-impact: no new API, authority boundary, persistence, discovery, background execution or credential; the page documents set data as untrusted descriptive metadata; classification **SECURITY IMPACT: DOCUMENTATION ONLY**.
- Out of scope confirmed: no matching/requirement boundary (R09-003), no API change.

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS** |
| Accepted commit | `4bd241e` (evidence `7bcd90d`) |
| Evidence reference | Implementor Evidence above |
| Date | 05-10-2026 |

Reviewer notes: no production type, signature or behavior change (CapabilitySet contract text in `capability_set.hpp`); at-most-one-entry-per-identity, replace-in-place, first-insertion order, determinism, identity-only lookup, reference-validity and snapshot rules, the provided-contract meaning of capability version with no ordering, and the synchronized `CAPABILITY_SET.md` are accepted; the two equivalent mutants are accepted as classified. `CORE-CAP-005` and `CORE-CAP-006` are authoritative.

**Reviewer Decision: PASS — KF-CORE-R09-002 is ACCEPTED.**
