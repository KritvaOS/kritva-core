# KF-CORE-R05-001 — Acceptance Criteria

    ## Task Information

    - **Task ID:** `KF-CORE-R05-001`
    - **Title:** Platform Context & Service Access Model
    - **Requirement:** `CORE-PLAT-012`
    - **Status:** PLANNED
    - **Primary commit message:** `feat(core): add platform context`

    ## Acceptance Decision

    Reviewer decision:
    - [ ] PASS
    - [ ] CHANGES REQUIRED
    - [ ] BLOCKED

    Reviewer: ChatGPT architecture/review gate  
    Implementation agent: Claude/Codex

    ## Detailed Acceptance Criteria

    - [ ] PlatformContext is a small non-owning view over IPlatformAdapter.
- [ ] No service ownership, allocation, singleton, service locator or registry is introduced.
- [ ] Adapter/service lifetime rules are explicit and safe.
- [ ] Existing IPlatformAdapter semantics remain unchanged.
- [ ] Public headers are self-contained and documented.

    ## New Unit / Contract Tests Required

    - [ ] PlatformContext construction/access
- [ ] non-owning lifetime
- [ ] null/unattached behavior
- [ ] service pointer consistency
- [ ] public-header self-containment

    ## Integration Testing Rule

    A lightweight public-API integration test may verify a Runtime-attached context does not alter Runtime behavior.

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
    feat(core): add platform context
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

Primary commit: `c5910e7` `feat(core): add platform context` (API decisions Q1–Q9 approved and recorded at `ef1ad34`). **Purely additive:** one new public header `platform/context.hpp`, one include in `core.hpp`; no R0.4 signature changed and **no `RuntimeManager` change** (Q2).

- Changed files: `include/kritva/core/platform/context.hpp` (new), `include/kritva/core/core.hpp` (include), `tests/unit/platform_context_test.cpp` (new), `CMakeLists.txt` (test registration), `REQUIREMENTS.md` (`CORE-PLAT-012` and its traceability row), `API.md` (section 31), `ARCHITECTURE.md`.
- API (approved Q1/Q9): `platform::PlatformContext` — a small **copyable** value holding one nullable non-owning `IPlatformAdapter*`; default-constructed = unattached; `explicit PlatformContext(IPlatformAdapter&) noexcept`, `explicit PlatformContext(IPlatformAdapter*) noexcept` (so `runtime.platform()` can be passed directly); `attached()`, `const PlatformInfo* info()` (null when unattached), `scheduler()/clock()/timer()/watchdog()` (adapter pointer or null when unattached/unsupported), `supports(PlatformService)` (false when unattached or for an unknown enumerator), `capabilities()` (owned snapshot, empty when unattached), `has_capability(CapabilityId)` (= `CapabilitySet::contains`, identity only). It owns/creates/destroys nothing, caches nothing, `sizeof == sizeof(void*)`, trivially destructible, no static/global state, never infers from name or version. The contract states that copying a `PlatformContext` does not extend the adapter's lifetime and does not transfer or duplicate ownership, and that the adapter must outlive every context and every pointer obtained through one.
- Unit/contract tests (`kritva_core_platform_context`, 9 functions): shape via `static_assert` (copyable, nothrow, trivially destructible, one pointer, explicit constructors only, noexcept accessors); unattached/null/`nullptr` context reports nothing; attached context forwards the adapter's own objects and identity; service-pointer consistency for all 16 service combinations and identity with the adapter's `supports()`; copies share the adapter and rebinding a copy touches neither the adapter nor other copies; lifetime (contexts never own or destroy the adapter); no caching and no side effects (a counting adapter shows exactly one forwarded call per query, none at construction, copying or `attached()`, and no service activation); capability matching by identity only (a platform named like a capability, numeric name lengths and versions used as ids, a capability's version) and the owned snapshot never leaks back; **integration check** through public APIs: a Runtime-attached context (built from `runtime.platform()`) leaves Runtime behavior unchanged (the Runtime makes zero adapter calls, states and statistics unchanged, using the context changes no Runtime state, an unattached Runtime gives an unattached context). Public-header self-containment is covered by the build's per-header check.
- Mutation evidence (17 mutants, each reverted, all detected): `attached()` always true, `info()` null, scheduler forwarding broken, timer gated on another service, `supports` true when unattached / ignoring the adapter / true for an unknown enumerator, `capabilities()` empty, `has_capability` inverted / always true / matching the platform name, a cached scheduler (hidden state), the context owning (deleting) the adapter, extra state in the context, reference constructor detaching, an implicit conversion from a reference. Two mutants (platform-name-based match, unknown enumerator on an attached context) survived the first run and were closed by added tests.
- Regression: `ctest` 33/33 in Debug, Release, ASan+UBSan, strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`, TSan (ASLR off): the whole existing suite (including the R0.4 platform, conformance and runtime-platform tests) is unchanged and green; build 0 warnings; GCC `-fanalyzer` clean; coverage 98% (464/469, the same five uncovered lines); `make check` passes with traceability 67 requirements, 66 traced, 0 errors; `git diff --check` clean. Prohibited-dependency scan clean (no threading, OS or vendor header; `platform/context.hpp` includes only Core headers).
- Out of scope confirmed: no service registry or locator, no ownership, no Component lifecycle change, no Runtime change, no concrete platform, no threads or background execution.

## Reviewer Sign-Off

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS** |
| Accepted commit | `c5910e7` (evidence `bea6e41`) |
| Evidence reference | evidence section above |
| Date | 05-10-2026 |

Reviewer notes: `PlatformContext` is a correct small copyable non-owning value view (one pointer, trivially destructible, no cache, no ownership, no registry or second platform authority); the copy-and-lifetime sentence is present; capability matching is identity only; no `RuntimeManager` change; 17/17 mutants detected after closing two survivors. `CORE-PLAT-012` is authoritative. Reviewer relied on the supplied evidence; the commits were local-only.

**Reviewer Decision: PASS — KF-CORE-R05-001 is ACCEPTED.**



### API Design Guardrail

The implementation must first inspect the frozen R0.4 public APIs and introduce only the minimum additive API required for the approved PlatformContext model. A conceptual shape may expose adapter identity, scheduler/clock/timer/watchdog access, and capability access, but the exact API must follow the existing R0.4 types and ownership semantics. Do not mechanically introduce a `const CapabilitySet&` accessor if the authoritative adapter API returns `CapabilitySet` by value.

