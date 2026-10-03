# KF-CORE-R04 — Platform Integration Freeze Evidence

Status: SUBMITTED

Freeze candidate: production sources are final as of `f7231c1` (R04-005, the last production change). `460de87` (R04-006) changed tests only. Documentation and planning commits follow (`5f755fe` is the current head). Platform API Review baseline: PASS / FROZEN, production code state `4af4756` (evidence `380ade3`).

## Entry criteria

R04-001 (`d1c5f13`), R04-002 (`eb06fa0`), R04-003 (`67114bb`), R04-004 (`4af4756`), R04-005 (`f7231c1`), R04-006 (`460de87`) are ACCEPTED. Platform API Review is PASS / FROZEN. The conformance suite passes (`kritva_core_platform_conformance`). Traceability is clean.

## Production API diff against the Platform API Review baseline (`4af4756` to head)

| File | Change |
|---|---|
| `platform/adapter.hpp` | new (R04-005): `PlatformService`, `PlatformInfo`, `IPlatformAdapter` |
| `core.hpp` | one added include |

Nothing else in `include/` or `src/` changed since the Platform API Review: the frozen callback, scheduler, clock, timer and watchdog contracts are byte-identical to the reviewed baseline, and R04-005 only adds the adapter header that the review had listed as the next task. `git diff kritva-core-r0.3 head -- src include/kritva/core/runtime` is empty: the R0.3 Runtime is unchanged.

## Verification items

- **No platform-specific implementation in Core:** `include/` and `src/` contain only contracts, vocabulary types and the R0.3 runtime; platform implementations exist only as test doubles in `tests/`. The prohibited-header audit (CORE-PLAT-004, CORE-RT-010, CORE-GEN-003) reports no violation.
- **Conformance tests use only public APIs:** the includes of `tests/platform/*.hpp` are the public Core headers `kritva/core/{error/error_code, types/duration, time/clock, time/timer, platform/scheduler, platform/watchdog, platform/adapter}.hpp` and the standard library (`<atomic>`, `<functional>`, `<set>`, `<string>`, `<vector>`, `<cstddef>`); no private header, no `src/` file, no reference double. The reference doubles appear only in `tests/unit/platform_conformance_test.cpp`, which validates the suite.
- **R0.3 Runtime behavior unchanged:** the runtime sources are identical to the `kritva-core-r0.3` tag, and the whole R0.3 runtime test group (`kritva_core_runtime`, `component`, `component_registry`, `dependency_graph`, `runtime_manager`, `runtime_lifecycle`, `runtime_failure`, `runtime_integration`) passes in all configurations.
- **No thread or background execution introduced:** no `<thread>`, `<mutex>`, `<atomic>`, `<condition_variable>`, `<future>` or `pthread` use in `include/` or `src/` (the audit enforces this); `<atomic>` and `<functional>` appear only in test code.
- **Freeze commit:** production freeze point `f7231c1`; recorded after review in a docs-only commit.

## Validation (head `5f755fe`; production state `f7231c1`)

`ctest` 31/31 in Debug, Release, ASan+UBSan, strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`, and TSan (ASLR off via `setarch -R`); 0 warnings; GCC `-fanalyzer` clean; public-header self-containment check passes; `make traceability-check`: 64 requirements, 63 traced (CORE-ERR-003 reserved), 0 errors, 0 warnings; coverage 98% (446/451), same five justified uncovered lines as the R0.3 baseline; no `find_package` of third-party packages and no `FetchContent`; `git diff --check` clean.

## Open issues

None blocking. Carried forward: 32-bit scheduler affinity mask (documented limitation, frozen); level-2 mutation strictness gap of the conformance suite (documented in R04-006); `make lint` / `make format-check` deferred stubs. The only production change still permitted by the milestone is the additive `RuntimeManager::attach_platform` / `platform()` of R04-007, which needs its own review.

Reviewer decision: PENDING
