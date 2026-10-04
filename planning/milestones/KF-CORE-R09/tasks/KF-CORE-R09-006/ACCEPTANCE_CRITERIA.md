# KF-CORE-R09-006 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R09-006 |
| Status | ACCEPTED |
| Primary commit | `test(core): validate R0.9 documentation security and boundaries` |
| Reviewer | ChatGPT — independent acceptance gate |
| Estimated effort | 2–3 ED |
| Dependency | R09 Integration Freeze PASS/HONORED |

## Objective

Validate the final R0.9 API documentation, security boundary, public API shape, platform-independence and regression boundary before release-candidate preparation.

## Proposed Requirements Traceability

CORE-CAP-011

The implementing task must update authoritative root `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- All R0.9 public API/semantic changes are documented in Markdown.
- Documentation matches accepted headers/semantics.
- Security assessment is recorded.
- No unapproved new public API exists.
- No ServiceRegistry/DI/discovery framework exists in production Core.
- No Nexus/Edge/platform-specific production dependency exists.
- Traceability audit is clean except explicitly reserved items.
- Fresh-clone install consumer validates the accepted API.
- Coverage remains within the milestone policy.

## New Tests

- Boundary snapshot tests.
- Header self-containment.
- Documentation/link/traceability checks.
- Install consumer.

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

- `tests/unit/capability_boundary_test.cpp or equivalent.`
- `docs/api and docs/security material.`
- `planning/requirements reconciliation.`

## Git Commit

```text
test(core): validate R0.9 documentation security and boundaries
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

Primary commit: `04f0859` `test(core): validate R0.9 documentation security and boundaries`. **No production change:** `git diff 4c86b53 HEAD -- include src` is empty (production freeze commit `4c86b53`; Integration Freeze PASS / HONORED, evidence `553258b`, recorded `1e68451`). Changes: `scripts/audit/check_api_docs.py` (new tooling), `Makefile` (`api-docs-check` in `make check`), `CMakeLists.txt` (two audit CTests; `find_program`, because `find_package` is forbidden by the repository audit), `tests/unit/capability_boundary_test.cpp` (new), `tests/install/consumer/main.cpp` (capability exercise, codes 84–93), `docs/api/API_INDEX.md` and `README.md`, `docs/security/*` and the security review record, `REQUIREMENTS.md` (`CORE-CAP-011` defined **once**, covering R09-006 and R09-007, as `CORE-CFG-013` did), `TESTING.md`.

- **API documentation audit (`check_api_docs.py`, tooling only, deterministic, standard library):** structure and references only, never prose quality (your amendment). It checks that README/INDEX/GUIDELINES exist and that README states Markdown is authoritative; every document in `API_INDEX.md` exists with status `maintained` or `stub`; no unlisted page and no generated HTML/PDF under `docs/api`; every **maintained** page has all 15 sections required by `API_GUIDELINES.md` (heading keywords) and names only headers (resolved under `include/kritva/core/`), tests, documents and requirement ids that exist. Current result: **15 documents indexed, 6 maintained, 9 stubs, 0 errors**. **Self-test (also a CTest): 10 deliberate defects — a missing indexed page, an orphan page, a generated HTML page, a missing section, a nonexistent header, a nonexistent test, an undefined requirement id, an invalid status, a README without the authority statement, a missing index — are each detected.** Wired into `make check` and two CTests.
- **Documentation scope decision, stated plainly (please confirm):** the six pages R0.9 changed or relied on are maintained (capability ×3, `PLATFORM_REQUIREMENTS`, `LIFECYCLE`, `DEPENDENCY_GRAPH`). Nine pages for unchanged domains (configuration ×2, context ×2, error ×2, `PLATFORM_ADAPTER`, `COMPONENT`, `RUNTIME`) are still the uploaded short placeholders; I did **not** claim them complete. `API_INDEX.md` now labels each `stub`, the audit enforces the label, and a stub must be replaced by a maintained page in the task that next changes that domain's public contract. Authoring those nine pages was not required by R09-006's criteria ("all R0.9 public API/semantic changes are documented") and would be documentation of unchanged contracts; if you want them done before release, say so and I will do it as a separate documentation task.
- **Boundary snapshot (`kritva_core_capability_boundary`, compile-time):** pins signatures and shapes of `Capability`, `CapabilitySet`, by-value provider snapshots, `CapabilityRequirement`/`Requirement`, `PlatformRequirements`, `evaluate`/`check_required`, `PlatformContext`/`ComponentContext::has_capability`, `DependencyGraph`/`RuntimeManager::add_dependency` (ComponentIds only), the lifecycle values (eight, no readiness state) and `ErrorCode` (twelve, none added); a by-name detector (checked against itself, final classes probed directly) finds any resolve/bind/inject/satisfy/discover/lookup/locate/provider_of/require_capability/registry/subscribe/notify, ready/is_ready/readiness/wait_for/prerequisites, or token/credential/authorize/authenticate/grant/signature member on **eleven surfaces**, and any available/health/status/state/lifecycle/priority/revision member on `Capability` and `CapabilitySet`; exhaustive no-default switches make an added enumerator fail under the strict build. **Sensitivity: 19 production-header mutants** (a resolver, subscription, token, readiness query, wait, injection, `require_capability`, state-like fields, changed signatures, an added `NOT_READY` state and an added `ErrorCode`, among others); the first run left one survivor (an added `available` field), closed by the state-like member detector; all 19 killed (the added enumerators under `-Wall -Wextra -Werror`).
- **Security review record (`R09_SECURITY_REVIEW.md`):** all twelve required questions answered with evidence; classification **SECURITY IMPACT: DOCUMENTATION ONLY**; explicit non-authorization list kept; `docs/security/SECURITY_DECISIONS.md` gains SD-R09-01..07, `TRUST_BOUNDARIES.md` the capability/requirement boundary diagram, `THREAT_MODEL.md` the R0.9 notes, `SECURITY_ARCHITECTURE.md` the classification. No security mechanism is introduced. **The reviewer decision field is left for you.**
- **Install consumer (through the installed package):** capability replace-in-place and first-insertion order, identity-only lookup (a name and a provider name/version that look like an id prove nothing), independent snapshots, a storable invalid-identity entry, requirement declaration (invalid id rejected, duplicate decided by identity), identity-only evaluation with exactly one snapshot, `check_required` failing `UNSUPPORTED`. The install test passes.
- **Fresh-clone validation of this commit** (clean tree): Debug and Release 0 warnings, ctest **65/65** in both; `make check`: header-check passed, traceability **108 requirements, 107 traced, 0 errors**, API documentation audit **0 errors**; word-bounded dependency scan clean; `git diff --check` clean; production diff since the freeze empty and, against `kritva-core-r0.8`, exactly the three reviewed contract-text headers (+62, +11, +118). The same commit passes in the working tree under ASan+UBSan, strict `-Werror`, TSan (ASLR off), `-fanalyzer` and coverage 618/625 (unchanged).
- Deferred known issues were not pulled into scope. Version metadata (`VERSION`, CMake, README, CHANGELOG) is R09-007.

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS** |
| Accepted commit | `04f0859` (evidence `87a2960`) |
| Evidence reference | Implementor Evidence above |
| Date | 05-10-2026 |

Reviewer notes: no production diff since the freeze commit `4c86b53`; the mechanical API documentation audit with its 10-defect self-test (15 indexed, 6 maintained, 9 stubs), the compile-time capability boundary snapshot with 19 detected mutants, the installed-package capability exercise and the fresh-clone validation are accepted; the stub-page policy (maintained pages for changed or relied-upon contracts, labelled stubs for unchanged domains, replaced when the contract next changes) is confirmed; the Security Architecture Review is PASS with SECURITY IMPACT: DOCUMENTATION ONLY. `CORE-CAP-011` is authoritative.

**Reviewer Decision: PASS — KF-CORE-R09-006 is ACCEPTED.**
