# KF-CORE-R09-004 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R09-004 |
| Status | ACCEPTED |
| Primary commit | `test(core): add capability contract reference harness` |
| Reviewer | ChatGPT — independent acceptance gate |
| Estimated effort | 3–4 ED |
| Dependency | R09 Capability API Review PASS/FROZEN |

## Objective

Provide reusable reference conformance checks that detect deliberately broken capability and requirement behavior without expanding production APIs.

## Proposed Requirements Traceability

CORE-CAP-010

The implementing task must update authoritative root `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- Harness uses public APIs only.
- Each intentional contract defect is detected.
- Reference component/provider behavior is deterministic.
- Harness verifies ownership/lifetime semantics.
- Harness does not require Nexus/Edge/platform-specific code.
- No production source/header changes unless explicitly approved to fix a demonstrated contract gap.

## New Tests

- Reusable conformance suite.
- Mutation matrix with all consequential defects detected.
- Repeated-order deterministic checks.

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

- `tests/runtime/reference_capability.hpp or equivalent.`
- `tests/unit/reference_capability_test.cpp.`
- `tests/unit/capability_conformance.hpp or equivalent.`
- `CMake/test registration.`

## Git Commit

```text
test(core): add capability contract reference harness
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

Primary commit: `2608795` `test(core): add capability contract reference harness`. **Test-only:** `git diff 4c86b53 HEAD -- include src` is empty (production freeze commit `4c86b53`; Capability API Review PASS / FROZEN, evidence `c84bb9c`, recorded at `7e44049`). Changes: `tests/runtime/reference_capability.hpp` and `tests/runtime/capability_conformance.hpp` (new), `tests/unit/reference_capability_test.cpp` (new), `CMakeLists.txt`, `REQUIREMENTS.md` (`CORE-CAP-010` + row), `TESTING.md`. No platform-specific code is used or needed.

- Harness (public APIs only): abstract fixtures for one capability-set implementation (`SetFixture`), one provider (a Component; `ReferenceCapabilityProvider` is a conforming reference with a controlled, observable by-value snapshot and a query counter on top of the plain reference component) and one requirement evaluator (`RequirementFixture`); the **production** `CapabilitySet` and the production `PlatformRequirements`/`evaluate()`/`check_required()` over the test-only `ReferencePlatform` are wrapped as the real implementations; and **independent deliberately defective implementations**: **21 set defects** (duplicate entries; a replacement that moves, keeps the old name, the old/higher/lower version, or is ignored after the first; sorted and newest-first order; lookup by name; an unfindable empty name; an invalid id dropped, ignored on replacement or not an ordinary key; a copy that aliases or writes through to its source; size counted by name; a hidden growth limit; a non-empty new set; a lost append after a replacement; absent ids found; an order differing between instances), **7 provider defects** (a declaration that grows or changes content on every call; a query that changes the lifecycle; a declaration that follows the lifecycle; an invalid id published; an entry dropped; a wrong version) and **17 requirement defects** (matching by capability name, capability version, provider name or provider version; a snapshot per item or when none is needed; a side effect; an optional item that decides; a required item ignored; duplicates decided by (id, level) or accepted exactly; an invalid id accepted; a rejection that changes the declaration, atomically or not; a reversed, non-deterministic or optional-omitting report).
- Reusable checks: `check_set_contract(factory)` (empty set; at most one entry per identity; replacement in place of name and version; first-insertion order; a version never ordered; identity as the only key incl. lookalike names, empty names and absent ids; the invalid identity as an ordinary key; independent snapshots in both directions; unlimited growth; determinism), `check_provider_contract(component, expected, drive_to_running)` (queries change nothing and are repeatable in size and content; the declaration is exactly the expected one; no invalid identity; a returned snapshot is independent; the declaration does not follow the lifecycle) and `check_requirement_contract(factory)` against an **independent identity oracle** (identity-only matching over names, versions, provider name/version and lookalikes; declaration order; optional never decides; determinism; no side effect; exactly one snapshot per evaluation pass and none when no capability is required; invalid identity rejected; duplicates by identity; atomic rejection).
- Tests (`kritva_core_reference_capability`, 5 functions): the real set conforms and **each of the 21 set defects is detected by the clause written for it** (expected message asserted per defect); the reference provider conforms and each of the 7 provider defects is detected by its clause; the provider exposes a controlled observable snapshot; the real evaluator conforms and **each of the 17 requirement defects is detected by its clause**; the independent defect-free models conform too; repeated runs give identical verdicts. ASan+UBSan clean (a copy that aliases or writes through is detected by behavior, not by a use-after-free).
- Mutation evidence: (i) **38 clause-removal mutants** of the conformance checks (each clause's line removed in turn). The first sweep left 14 survivors; each was closed by a new defect written for that clause, by splitting two clauses that shared one message (size versus content; invalid-id rejection versus duplicate rejection), or by a new scenario (only optional items missing). In the final sweep **37 of 38 are killed**; the one survivor, "a change to a returned snapshot showed up in the provider", is **explained and not hidden**: `CapabilitySet` is returned by value and vector-backed, so no in-tree provider can violate it without undefined behavior; the clause stays for out-of-tree providers and the by-value return type is asserted. (ii) **11 production mutants** run against the real implementations through the harness (replace updating only the name; replace as erase+append; new ids at the front; lookup also by name; add dropping invalid ids; add with a size cap; requirement matching also by provider name; also by version; optional items deciding; duplicates by (id, level); the invalid-id check removed): all killed at run time (no compile-failure kills).
- Isolation: `kritva_core_test_isolation` and `make check` pass; no production include of the harness; no hook added to production.
- Regression: ctest 61/61 in Release, ASan+UBSan, strict `-Werror` (0 warnings), TSan, Debug; GCC `-fanalyzer` clean; coverage 618/625 unchanged; traceability 106 requirements, 105 traced, 0 errors (`CORE-CAP-010` defined with a row); `git diff --check` clean.
- Security-impact: test-only; no production change; **SECURITY IMPACT: NONE** for this task.

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS** |
| Accepted commit | `2608795` (evidence `9d0a81d`) |
| Evidence reference | Implementor Evidence above |
| Date | 05-10-2026 |

Reviewer notes: test-only (no production diff since the freeze commit `4c86b53`); the reusable set, provider and requirement conformance checks over the real implementations and independent defect models, the 45 defects each detected by the clause written for it, the 38 clause-removal mutants (37 killed; the one survivor is explained by the by-value return type) and the 11 production mutants are accepted; the one amendment of the unreviewed evidence commit corrected a count. `CORE-CAP-010` is authoritative.

**Reviewer Decision: PASS — KF-CORE-R09-004 is ACCEPTED.**
