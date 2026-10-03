# KF-CORE-R04-001 — Acceptance Criteria

## Task Information

- **Task ID:** `KF-CORE-R04-001`
- **Title:** Platform Adapter Boundary & Context
- **Requirement:** `CORE-PLAT-004`
- **Status:** PLANNED
- **Primary commit message:** `feat(core): define platform adapter boundary`

## Acceptance Decision

Reviewer decision:
- [ ] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Reviewer: ChatGPT architecture/review gate  
Implementation agent: Codex/Claude

## Objective

Define the stable boundary between platform-independent Core contracts and external platform adapter implementations.

## Scope

['adapter interface/boundary semantics', 'ownership and lifetime rules', 'opaque context semantics', 'error propagation', 'platform-independence constraints', 'public-header contract tests']

## Out of Scope

['Linux/POSIX implementation', 'RTOS implementation', 'vendor BSP/HAL', 'hardware drivers', 'platform singleton', 'background execution']

## Requirement Traceability

The implementation must define/trace `CORE-PLAT-004` without duplicating or renumbering existing requirements. The authoritative `REQUIREMENTS.md` is updated as part of the task when the requirement is actually implemented.

## Detailed Acceptance Criteria

['Core-facing platform contracts are implementable outside kritva-core.', 'No production Core source includes Linux/POSIX/RTOS/vendor SDK/hardware headers.', 'Ownership and lifetime of adapter objects and opaque contexts are explicit.', 'Core does not create, own or destroy platform-global services unless a later contract explicitly requires it.', 'Adapter failures use existing Result/Error semantics.', 'Thread-safety is explicitly documented rather than implied.', 'No hard-real-time guarantee is introduced.', 'Fake adapters can exercise the boundary without hardware.']

## New Tests Required

['public-header compile test', 'adapter-boundary contract test', 'ownership/lifetime test', 'negative/error propagation test', 'prohibited dependency scan']

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
feat(core): define platform adapter boundary
```

One logical task = one primary implementation commit. Review fixes after review use a separate focused commit.

## Implementation Evidence (Claude)

Primary commit: `d1c5f13` `feat(core): define platform adapter boundary`; design approved in `R04_DESIGN_DECISIONS.md` (`500769d`). R0.3 released at `kritva-core-r0.3` (`cc16ec9`); frozen R0.3 runtime sources unchanged.

- Changed files: `include/kritva/core/types/callback.hpp` and `include/kritva/core/platform/boundary.hpp` (new), `include/kritva/core/core.hpp` (umbrella), `tests/unit/platform_boundary_test.cpp` (new), `CMakeLists.txt` (test registration and the generated header self-containment check), `scripts/audit/check_traceability.py` (platform-header scan), `REQUIREMENTS.md` (`CORE-PLAT-004` and traceability row), `ARCHITECTURE.md`, `API.md` (section 25).
- Public API (additive): `kritva::core::Callback { Function function; void* context; bool valid() const noexcept; }` and `platform::make_error(code, message, severity = ERROR) -> Error`. No existing API changed. `boundary.hpp` otherwise holds the contract (platform independence, ownership/lifetime, callbacks/context, error codes, thread safety/real time) and the sentence "R0.4 establishes platform contracts and integration boundaries; it does not implement a concrete ... platform adapter."
- Acceptance mapping: implementable outside Core (abstract contracts plus test-only fakes); no Linux/POSIX/RTOS/vendor header in production (new audit rule `CORE-PLAT-004` rejects `unistd.h`, `pthread.h`, `sys/*`, `linux/*`, `windows.h`, RTOS/vendor headers in `include/` and `src/`; verified by planting `<unistd.h>` and `<sys/socket.h>`); adapter and context ownership/lifetime explicit; Core creates/owns no platform-global service and has no singleton or locator; adapter failures use `Result`/`Error` with fixed code meanings; thread safety adapter-defined; no real-time guarantee; fake adapters exercise the boundary without hardware.
- New tests (`kritva_core_platform_boundary`, 6 functions): Callback is a trivially copyable plain value (null function invalid, null context valid, passes context unchanged, copies interchangeable); `make_error` fields; adapter errors (`INVALID_ARGUMENT`, `UNSUPPORTED`, `RESOURCE_UNAVAILABLE`, `INVALID_STATE`) propagate unchanged and failed requests have no effect; services and contexts are never owned by the contract (adapter destroyed, caller-owned context untouched and still alive; non-owning holder); no global state (two independent adapters). New build-level public-header compile check: one generated translation unit per public header (42 headers) built as `kritva_core_header_checks` (all self-contained; removing an include from `boundary.hpp` fails the build).
- Mutation evidence (each reverted): `Callback::valid()` ignoring the function; `valid()` requiring a context; `make_error` default severity changed; message dropped; header made non-self-contained (build failure). All detected.
- Build 0 warnings; `ctest` 26/26 in Debug, Release, ASan+UBSan, TSan (ASLR off), strict `-Werror`; `-fanalyzer` clean; `make check` passes (59 requirements, 58 traced, 0 errors); install-consumer passes; `git diff --check` clean.
- Out of scope confirmed: no Linux/POSIX/RTOS/vendor implementation, no singleton, no background execution added.

## Reviewer Sign-off

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Date | 04-10-2026 |
| Decision | **PASS** |

Accepted commit: `d1c5f13` `feat(core): define platform adapter boundary`; evidence `5130b89`. No changes required.

**Reviewer Decision: PASS — KF-CORE-R04-001 is ACCEPTED.** R04-002, R04-003 and R04-004 are authorized, each reviewed independently.
