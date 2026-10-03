# KF-CORE-R04-005 — Acceptance Criteria

## Task Information

- **Task ID:** `KF-CORE-R04-005`
- **Title:** Platform Capability & Adapter Contract
- **Requirement:** `CORE-PLAT-008`
- **Status:** PLANNED
- **Primary commit message:** `feat(core): define platform capability contract`

## Acceptance Decision

Reviewer decision:
- [ ] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Reviewer: ChatGPT architecture/review gate  
Implementation agent: Codex/Claude

## Objective

Define minimal platform identity/capability reporting so higher layers can discover adapter capabilities without embedding platform implementation into Core.

## Scope

['platform identity', 'version', 'capability identities', 'supported/unsupported reporting', 'adapter metadata', 'non-owning reporting']

## Out of Scope

['large hardware inventory API', 'driver enumeration', 'automatic feature activation', 'platform singleton', 'vendor-specific capability types']

## Requirement Traceability

The implementation must define/trace `CORE-PLAT-008` without duplicating or renumbering existing requirements. The authoritative `REQUIREMENTS.md` is updated as part of the task when the requirement is actually implemented.

## Detailed Acceptance Criteria

['Platform identity is stable and represented with existing Core identity/version primitives where appropriate.', 'Capability identities are explicit and deterministic.', 'Unsupported capabilities are represented without pretending support exists.', 'Capability reporting does not instantiate hardware.', 'Capability reporting does not silently change Runtime behavior.', 'Adapter metadata does not expose vendor-specific implementation types through Core.', 'Ownership/lifetime of returned capability data is explicit.', 'No global singleton service registry is introduced.']

## New Tests Required

['identity tests', 'capability presence/absence tests', 'determinism tests', 'ownership/lifetime tests', 'unsupported capability tests']

## Regression Tests

At minimum:

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
```

All existing R0.3 tests must remain green.

## Quality Requirements

- No compiler warnings in the supported clean build.
- `-Werror` must pass where configured.
- ASan/UBSan must pass where configured.
- TSan must pass where configured for concurrency-sensitive changes.
- GCC `-fanalyzer` must pass where configured.
- `make traceability-check` must report zero errors.
- Prohibited platform dependency scan must report zero production violations.
- Public API changes must be documented.
- No unrelated generated files or changes.

## Evidence Required From Implementation Agent

Provide:

1. implementation commit SHA;
2. exact files changed;
3. `git diff --check` result;
4. build commands and results;
5. relevant CTest output;
6. sanitizer/static-analysis results where applicable;
7. coverage result where applicable;
8. traceability result;
9. prohibited-dependency scan result;
10. public API diff/summary;
11. explicit confirmation that out-of-scope platform implementations were not added;
12. working-tree status.

## Expected Files Changed

The implementation agent must list the actual files. Do not pre-authorize unrelated files. Public headers, implementation files, tests, CMake registration, requirements and directly relevant documentation may change.

## Commit

Use exactly:

```text
feat(core): define platform capability contract
```

One logical task = one primary implementation commit. Review fixes after review use a separate focused commit.

## Implementation Evidence (Claude)

Primary commit: `f7231c1` (`feat(core): define platform capability contract`; Platform API Review PASS/FROZEN recorded at `81b2571`). **Purely additive:** new header `platform/adapter.hpp`; no existing signature changed; the frozen scheduler, clock, timer and watchdog contracts and the R0.3 Runtime are untouched.

- Changed files: `include/kritva/core/platform/adapter.hpp` (new), `include/kritva/core/core.hpp` (include), `tests/contract/reference_adapter.hpp` (new), `tests/unit/platform_adapter_test.cpp` (new, 10 functions), `CMakeLists.txt`, `REQUIREMENTS.md` (`CORE-PLAT-008`, traceability row), `API.md` (section 29), `ARCHITECTURE.md`.
- API (design decision Q4): `enum class PlatformService : uint8_t { SCHEDULER, CLOCK, TIMER, WATCHDOG }`; `struct PlatformInfo { std::string name; Version version; }` (no CPU/OS/board/vendor field); `class IPlatformAdapter` with `const PlatformInfo& info() const noexcept`, `IScheduler* scheduler() const noexcept`, `time::IClock* clock() const noexcept`, `time::ITimer* timer() const noexcept`, `IWatchdog* watchdog() const noexcept`, `CapabilitySet capabilities() const`, and a **non-virtual** `bool supports(PlatformService) const noexcept` implemented as "the accessor is non-null". Making `supports` non-virtual enforces the approved "supports == false iff nullptr" invariant structurally in both directions, so an adapter cannot make them disagree. An unknown enumerator is unsupported.
- Contract: `info()` immutable, equal on every call, valid until the adapter is destroyed; each accessor returns a stable adapter-owned object or `nullptr` (never an error, never a placeholder); queries never start a service, instantiate hardware or change Runtime behavior; `capabilities()` is an owned by-value snapshot (same as `Component::capabilities()`), deterministic in content and order, adapter-chosen capability identities, no Core-reserved range, an adapter must not reinterpret an identity; integrator owns the adapter, Core holds non-owning references, no singleton/registry/locator; thread safety adapter-defined, no real-time claim. The header refers to `RuntimeManager::attach_platform` only as "planned for a later R0.4 task" because that requirement is defined in R04-007.
- Design choices to confirm: (1) `capabilities()` returns a snapshot by value (not a reference), making ownership/lifetime unambiguous at the cost of a copy in a control-plane call; (2) accessors are `const` but return mutable service pointers (non-owning handles), so a const adapter can still be queried and its services used; (3) `info()` is virtual and returns a reference valid for the adapter's life rather than a by-value copy.
- Tests (`kritva_core_platform_adapter`): shape and noexcept; default `PlatformInfo` is empty; identity stable across calls (same object, same values); two adapters never alias identity or services (no singleton); all services present and stable and usable through their own contracts; `supports` agrees with the accessors for all 16 provide-combinations in both directions; unsupported services are `nullptr` and an unknown enumerator is unsupported; capabilities are a deterministic owned snapshot (order, content, caller changes do not leak back, absence reported as absence); an adapter with no services reports no capabilities; queries never activate anything (activation counters stay 0); service pointers are adapter-owned.
- Mutation evidence (header and reference adapter, 12 mutants + 2 first-run survivors closed): `supports` mapping swapped / always true / inverted / default true, accessor ignoring the provide flag, wrong flag, `nullptr` always, `info()` returned from shared static, capabilities with an extra entry / reversed order / a side effect on a service, a clock shared across adapters; all detected after adding the two-adapter independence test (the shared-static `info()` and shared-static clock mutants survived the first run).
- Build 0 warnings; `ctest` 30/30 in Debug, Release, ASan+UBSan, strict `-Werror`, TSan (ASLR off); `-fanalyzer` clean; coverage 98% (446/451, same five uncovered lines); `make check` passes, traceability 63 requirements, 62 traced, 0 errors; `git diff --check` clean.
- Out of scope confirmed: no hardware inventory, driver enumeration, automatic feature activation, platform singleton or vendor-specific types.

## Reviewer Sign-off

- [ ] Scope satisfied
- [ ] Requirement traceability satisfied
- [ ] Tests satisfied
- [ ] Quality checks satisfied
- [ ] Evidence reproducible
- [ ] Architecture boundary preserved
- [ ] No unresolved blocker

Final reviewer decision is made independently after evidence review.
