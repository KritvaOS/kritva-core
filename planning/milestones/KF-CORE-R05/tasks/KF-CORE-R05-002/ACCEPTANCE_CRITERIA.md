# KF-CORE-R05-002 — Acceptance Criteria

    ## Task Information

    - **Task ID:** `KF-CORE-R05-002`
    - **Title:** Platform Service Requirement Model
    - **Requirement:** `CORE-PLAT-013`
    - **Status:** PLANNED
    - **Primary commit message:** `feat(core): define platform service requirements`

    ## Acceptance Decision

    Reviewer decision:
    - [ ] PASS
    - [ ] CHANGES REQUIRED
    - [ ] BLOCKED

    Reviewer: ChatGPT architecture/review gate  
    Implementation agent: Claude/Codex

    ## Detailed Acceptance Criteria

    - [ ] Required and optional services are represented explicitly.
- [ ] Capability identity is authoritative; platform name/version is never used for inference.
- [ ] Missing required services produce documented deterministic results.
- [ ] Requirement evaluation has no side effects.
- [ ] Existing CapabilitySet semantics are preserved.

    ## New Unit / Contract Tests Required

    - [ ] required service present/absent
- [ ] optional service absent
- [ ] capability matching
- [ ] unsupported requirement
- [ ] no side effects

    ## Integration Testing Rule

    Integration coverage must prove requirement evaluation does not mutate Runtime or platform service state.

    - [ ] Integration tests use only public APIs.
    - [ ] No physical hardware is required.
    - [ ] Test-only reference/fake services are used where needed.
    - [ ] No production test hook or private API is introduced solely to make integration testing possible.

    ## Regression Testing Rule

    Every task must run the complete existing regression suite, not only newly added tests.

    ```bash
    cmake -S . -B build
    cmake --build build -j$(nproc)
    ctest --test-dir build --output-on-failure
    ```

    Required where supported by the repository:

    ```bash
    make check
    make traceability-check
    ```

    - [ ] New/focused tests pass.
    - [ ] Full CTest regression passes.
    - [ ] No previously passing test regresses.
    - [ ] Regression output is supplied as evidence.

    ## Quality Requirements

    - [ ] No compiler warnings in the supported clean build.
    - [ ] `-Werror` passes where configured.
    - [ ] ASan/UBSan passes where configured.
    - [ ] TSan passes where configured for concurrency-sensitive changes.
    - [ ] GCC `-fanalyzer` passes where configured.
    - [ ] Coverage is reviewed.
    - [ ] Traceability audit reports zero errors.
    - [ ] Prohibited dependency audit reports zero production violations.
    - [ ] Public API changes are documented.
    - [ ] No unrelated generated files or changes are present.

    ## Expected Files Changed

    Actual changed files must be listed by the implementation agent. Do not pre-authorize unrelated files.

    Expected categories may include:

    - public headers;
    - production source where required;
    - focused unit/contract tests;
    - integration tests;
    - CMake/test registration;
    - `REQUIREMENTS.md` when the proposed requirement becomes authoritative;
    - directly relevant architecture/API documentation.

    ## Commit

    Use exactly:

    ```text
    feat(core): define platform service requirements
    ```

    One logical task = one primary implementation commit.

    ## Evidence Required From Implementation Agent

    1. [ ] Primary commit SHA.
    2. [ ] Exact changed files.
    3. [ ] `git diff --check` result.
    4. [ ] Build command/result.
    5. [ ] Focused unit-test command/result.
    6. [ ] Full regression command/result.
    7. [ ] Integration-test command/result, where applicable.
    8. [ ] Sanitizer/static-analysis result, where applicable.
    9. [ ] Coverage result, where applicable.
    10. [ ] Traceability result.
    11. [ ] Prohibited-dependency scan result.
    12. [ ] Public API diff/summary.
    13. [ ] Explicit out-of-scope confirmation.
    14. [ ] Final working-tree status.

## Implementation Evidence (Claude)

Primary commit: `f9d6007` `feat(core): define platform service requirements` (R05-001 accepted at `6f9dde8`). **Purely additive:** one new public header `platform/requirements.hpp`, one include in `core.hpp`; no R0.4 signature, no `CapabilitySet`, `IPlatformAdapter`, `PlatformContext` or `RuntimeManager` change.

- Changed files: `include/kritva/core/platform/requirements.hpp` (new), `include/kritva/core/core.hpp`, `tests/unit/platform_requirements_test.cpp` (new), `CMakeLists.txt`, `REQUIREMENTS.md` (`CORE-PLAT-013` and traceability row), `API.md` (section 32), `ARCHITECTURE.md`.
- API (approved Q3/Q6/Q7 with the two clarifications): `enum class Requirement : uint8_t { REQUIRED, OPTIONAL }`; `ServiceRequirement`, `CapabilityRequirement`; copyable declarative `PlatformRequirements` with `add_service()`, `add_capability()` (`Result<void>`), `services()`, `capabilities()`, `empty()`; `PlatformRequirementReport` (four vectors: missing required/optional services/capabilities in declaration order; `satisfied()`, `complete()`); `evaluate(requirements, context)` and `check_required(requirements, context)`.
- Contract: declarations fail atomically with `INVALID_ARGUMENT` for an unknown service or level, an invalid (zero) `CapabilityId`, or a **duplicate decided by service/capability identity, not by (item, level)** (so `SCHEDULER, REQUIRED` then `SCHEDULER, OPTIONAL` is rejected; no silent upgrade, downgrade or merge). A capability requirement is satisfied exactly by `CapabilitySet::contains(id)`; a service requirement exactly by `PlatformContext::supports()`; platform name/version, capability name/version, string matching and vendor/OS inference are never used. `evaluate` never fails, treats an unattached context as providing nothing, is deterministic, queries `supports()` once per declared service and `capabilities()` at most once (one snapshot) and never starts, stops, configures or creates a service. `check_required` succeeds when nothing required is missing (optional items never matter), otherwise `UNSUPPORTED` with a message naming the first missing required declaration (services before capabilities, then declaration order); the **report is authoritative** and consumers never need to parse the message.
- Unit/contract tests (`kritva_core_platform_requirements`, 14 functions): shape; declaration order and levels kept; invalid declarations rejected atomically (unknown service/level, zero id; nothing added; still usable); duplicates decided by identity not level for services and capabilities; empty requirements satisfied by full, bare and unattached platforms; required service present/absent; optional service absent is not an error (`satisfied` but not `complete`); `check_required` message names each kind of missing service and only that one; capability matching by identity only (a lookalike platform name/version, a capability's own name/version); every missing item reported in declaration order and the first missing required one named (services before capabilities); unattached context provides nothing (all-OPTIONAL satisfied but incomplete); **no side effects and determinism** (a spy adapter: exactly one `supports()` call per declared service, one capability snapshot, zero identity calls, no service activation, requirements unchanged; capability-only and service-only evaluations query only what they need); copies are independent values; existing `CapabilitySet` semantics preserved. Public-header self-containment is covered by the build's per-header check.
- Mutation evidence (30 mutants, each reverted, all detected): duplicate decided by (item, level) for services and capabilities, silent merge/upgrade, duplicate checks removed, invalid id / unknown service / unknown level accepted, non-atomic append, wrong rejection code, declaration order not kept, required/optional swapped for services and capabilities, present service reported missing, capability matched by platform name, platform version or a capability's version, one snapshot per capability, a snapshot taken without capabilities, `evaluate` starting the scheduler or consulting the platform identity, `satisfied()` ignoring required services or capabilities, `complete()` ignoring optional items, `check_required` failing on optional items / wrong code / capability before service / naming the last instead of the first missing item.
- Regression: `ctest` 34/34 in Debug, Release, ASan+UBSan, strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`, TSan (ASLR off); whole existing suite unchanged and green; build 0 warnings; GCC `-fanalyzer` clean; coverage 98% (514/520): `context.hpp` 12/12 and `requirements.hpp` 50/51, the single uncovered line being the exception-unwind closing brace of `evaluate()` (the same kind as `component_registry.cpp:55`), plus the same five baseline lines (a first run showed an unexercised `scheduler` message case, which a test now covers); `make check` passes with traceability 68 requirements, 67 traced, 0 errors; `git diff --check` clean. Prohibited-dependency scan clean (the header includes only Core headers).
- Out of scope confirmed: no platform-specific capability inference, no service locator, no automatic service startup/shutdown, no hardware capability inventory.

## Reviewer Sign-Off

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS** |
| Accepted commit | `f9d6007` (evidence `2de3a5e`) |
| Evidence reference | evidence section above |
| Date | 05-10-2026 |

Reviewer notes: the additive requirement model is accepted: identity-based duplicate detection with atomic rejection, `supports()` / `CapabilitySet::contains()` as the only matching rules, no name or version inference (mutation-tested), deterministic declaration-order reporting, an authoritative structured report, and a side-effect-free evaluation (30/30 mutants detected). The single uncovered line is an exception-unwind closing brace, not behavioral coverage, and is not an acceptance blocker. `CORE-PLAT-013` is authoritative. Reviewer relied on the supplied evidence; the commits were local-only.

**Reviewer Decision: PASS — KF-CORE-R05-002 is ACCEPTED.**

