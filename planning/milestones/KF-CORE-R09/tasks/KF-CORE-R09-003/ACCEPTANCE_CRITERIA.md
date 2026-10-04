# KF-CORE-R09-003 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R09-003 |
| Status | ACCEPTED |
| Primary commit | `feat(core): define capability requirement matching boundary` |
| Reviewer | ChatGPT — independent acceptance gate |
| Estimated effort | 2–3 ED |
| Dependency | R09-002 |

## Objective

Clarify and test the relationship between capability provision and capability requirements using existing mechanisms, without adding a generic dependency resolver or Component requirement API by default.

## Proposed Requirements Traceability

CORE-CAP-007, CORE-CAP-008

The implementing task must update authoritative root `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- The distinction between provided Capability and required Capability is normative.
- Capability matching rules are explicit.
- No implicit name/platform/vendor matching exists.
- DependencyGraph remains independent and ComponentId-based.
- Existing PlatformRequirements behavior remains compatible unless an explicitly reviewed correction is necessary.
- `docs/api/capability/CAPABILITY_REQUIREMENTS.md` and/or `docs/api/platform/PLATFORM_REQUIREMENTS.md` are synchronized.
- Mutation tests detect weakened matching/side-effect boundaries.

## New Tests

- Required/optional capability tests.
- Identity mismatch tests.
- Side-effect-free evaluation tests.
- DependencyGraph independence test.

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

- `Platform requirements/capability files only if needed.`
- `tests/unit capability requirement tests.`
- `docs/api/capability/CAPABILITY_REQUIREMENTS.md.`

## Git Commit

```text
feat(core): define capability requirement matching boundary
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

Primary commit: `e91a51f` `feat(core): define capability requirement matching boundary`. **No production type, signature or behavior change:** the production diff is one contract block (and tags) in `capability/capability_set.hpp`; `src/`, `platform/requirements.hpp`, `runtime/dependency_graph.hpp` and every Component/Runtime/context/platform header are byte-identical to `kritva-core-r0.8` (`CORE-CAP-007`/`008` therefore list only `capability_set.hpp` as their header artifact; the traceability audit requires the artifact's tag, and I did not touch a frozen header to satisfy it).

**Disclosed correction (separate commit `7b6b0f4`, `docs(core): flatten API documentation domain directories`):** while writing the R09-003 pages I found that my earlier baseline commit `c250c54` had nested the seven domain directories one level too deep (`docs/api/<domain>/<domain>/`) because empty `docs/api/<domain>/` directories already existed in the upload, so the `API_INDEX.md` paths did not resolve and the R09-001/002 pages sat beside stale stubs. I told you the baseline was fixed; that was not fully true. The correction moves every page to `docs/api/<domain>/` and removes only the superseded 3-line stubs; a script check confirms every path in `API_INDEX.md` now exists. No content of any accepted page changed.

- Contract (CORE-CAP-007, CORE-CAP-008; API.md section 50; new `docs/api/capability/CAPABILITY_REQUIREMENTS.md` and `docs/api/platform/PLATFORM_REQUIREMENTS.md`, each with the 15 required sections): **provision is not requirement** (a provider declares a `CapabilitySet`; a consumer needs explicit `PlatformRequirements` items; related only by an explicit check; the Runtime never evaluates, resolves, binds, injects or satisfies anything and takes no capability snapshot); **matching is identity-only** (satisfied exactly when the snapshot contains the id; capability name/version, provider name/version/vendor/kind, platform type, OS, hardware, lookalikes and any inference are never consulted; no version-range matching; version acceptability is consumer policy outside the match; an invalid-id entry can never satisfy a requirement); **evaluation is explicit, side-effect free and deterministic** (at most one snapshot per evaluation, none when no capability is required; declaration atomic; duplicates by identity; REQUIRED decides, OPTIONAL informs); **capability requirements are not Component dependencies** (`DependencyGraph` orders ComponentIds only; capabilities never add, remove or reorder an edge, change the order or a registration).
- Tests (`kritva_core_capability_requirement_boundary`, 7 functions): no resolve/bind/inject/satisfy/discover/provider_of/require_capability/capability-dependency member on PlatformRequirements, PlatformContext, ComponentContext, RuntimeManager, DependencyGraph, Component or CapabilitySet (self-checked by-name detector, final classes probed directly), `CapabilityRequirement` shape and `add_dependency` taking only ComponentIds; **identity-only matching** (names that look like ids, a platform named `vendor-x-linux-13`, versions 0.0.1 to 99.99.99 and five absent ids including the maximum all behave by identity); declarations atomic and identity-deduplicated, REQUIRED versus OPTIONAL; **exactly one snapshot for four capability requirements, none when no capability is required, repeatable, nothing started or changed (call log empty)**; the same model through a `ComponentContext` (report equal to the direct R0.5 report, `UNSUPPORTED` attributed to the component, `has_capability` false for four absent ids); **a 100-seed property test: the dependency order is identical for three different random assignments of capabilities to components over the same acyclic topology**; the Runtime with an attached platform performs configure/initialize/start/stop/shutdown with **zero adapter queries**.
- Mutation evidence (11 production mutants, each reverted; the first run's "killed" results were re-checked because some were compile failures): matching by capability name, by version, by platform name (killed at run time), a snapshot taken per requirement (killed at run time), optional items deciding success, duplicates by (id, level), always taking a snapshot, a capability snapshot taken by the Runtime in `initialize()` and in `configure()`, and `ComponentContext::has_capability` also true for id 99 (survived the first run; closed by the absent-id loop; now killed at run time). All consequential mutants detected. The mutant that deleted the snapshot line was a compile failure and is not counted; it is covered by the snapshot-per-requirement mutant.
- Regression: ctest 60/60 in Release, ASan+UBSan, strict `-Werror` (0 warnings), TSan, Debug; GCC `-fanalyzer` clean (including the new test); coverage 618/625 unchanged; `make check`, traceability 105 requirements, 104 traced, 0 errors; `git diff --check` clean.
- Security-impact: no new API, authority boundary, persistence, discovery, background execution or credential; the pages state that a satisfied requirement is not authorization and capability claims are not proof; **SECURITY IMPACT: DOCUMENTATION ONLY**.
- Out of scope confirmed: no Component requirement API, no version-range matching, no resolver; the readiness/lifecycle boundary is R09-005.

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS** |
| Accepted commit | `e91a51f` (evidence `e53464a`; correction `7b6b0f4`) |
| Evidence reference | Implementor Evidence above |
| Date | 05-10-2026 |

Reviewer notes: no production type, signature or behavior change (one contract block in `capability_set.hpp`; `requirements.hpp`, `dependency_graph.hpp` and the Component/Runtime/context/platform headers byte-identical); provision distinct from requirement, identity-only matching, side-effect-free single-snapshot evaluation, the Runtime never evaluating or resolving, and capability requirements independent of Component dependency ordering are accepted together with the synchronized `CAPABILITY_REQUIREMENTS.md` and `PLATFORM_REQUIREMENTS.md`; the disclosed documentation-path correction `7b6b0f4` is accepted. `CORE-CAP-007` and `CORE-CAP-008` are authoritative.

**Reviewer Decision: PASS — KF-CORE-R09-003 is ACCEPTED.**
