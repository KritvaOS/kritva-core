# KF-CORE-R05 — Platform Integration Freeze

## Purpose

Freeze the R0.5 production platform-integration API and semantics before final validation.

## Entry Criteria

- R05-005 accepted.
- R05-006 accepted.
- Reference platform tests are deterministic.
- Runtime/platform integration tests pass.
- Full regression passes.
- No concrete platform implementation exists in production Core.
- No unresolved integration API change exists.

## Freeze Checklist

- [x] PlatformContext API frozen.
- [x] Service requirement semantics frozen.
- [x] Explicit service consumption semantics frozen.
- [x] Runtime/platform lifecycle boundary honored.
- [x] Reference platform is test-only.
- [x] Public API self-containment passes.
- [x] Traceability passes.
- [x] Prohibited dependency audit passes.
- [x] Full CTest regression passes.

## Evidence (submitted 05-10-2026)

Freeze candidate: production sources are final as of `f23777b` (R05-004, the last production change, comment-only in `runtime_manager.hpp`); the last functional production change is `f8cd523` (R05-003). R05-005 `7f30626` and R05-006 `fe04d35` changed tests and audit tooling only. Head: `d66eda6`. Platform API Review baseline: PASS / FROZEN at `ea6ae16` (evidence `05d981e`).

### Entry criteria

R05-005 and R05-006 are ACCEPTED (`8362103`, `d66eda6`). The reference platform tests and the Runtime/platform integration tests are deterministic (seeded; 10 randomized-order repetitions of the nine platform test groups all pass). The full regression passes (39/39). No concrete platform exists in production Core. No unresolved integration API change exists.

### Production diff against the Platform API Review baseline

`git diff ea6ae16 HEAD -- include src` is **empty**. Against `kritva-core-r0.4` the production change is exactly the reviewed R05 set: `platform/context.hpp` (new), `platform/requirements.hpp` (new), `runtime/runtime_manager.hpp` (comment only), `core.hpp` (two includes); `src/` is unchanged. Every R0.4 and R0.3 frozen header is byte-identical to the released `kritva-core-r0.4` tag.

### Checklist evidence

- **API frozen:** `PlatformContext` (with `require_*()`), `PlatformRequirements`, `PlatformRequirementReport`, `evaluate()`, `check_required()` and the lifecycle-separation contract are unchanged since the Platform API Review.
- **Lifecycle boundary honored:** the Runtime made zero adapter queries and zero service calls in every operation of a 40 x 40 seeded differential over every service combination with every platform method failing; results, states, faults, statistics and traces are identical to the no-platform baseline.
- **Reference platform is test-only:** `tests/platform/reference_platform.hpp` is compiled only by test targets; the repository audit (`CORE-PLAT-016`) and the CTest `kritva_core_test_isolation` prove that no production file includes test support, that every production translation unit and the umbrella header compile with only `include/` on the include path, that the production library target lists no test source and that no `install()` rule installs test support; each guard was verified by a planted violation.
- **Conformance and integration tests use only public APIs:** the Core headers they include are `core.hpp`, `error/error_code.hpp`, `platform/adapter.hpp`, `platform/scheduler.hpp`, `platform/watchdog.hpp`, `time/clock.hpp`, `time/timer.hpp` and `types/duration.hpp`; nothing under `src/`, no private member, no production helper added for tests.
- **No thread or background execution:** no `<thread>`, `<mutex>`, `<atomic>`, `<condition_variable>`, `<future>`, `<iostream>` or `<cstdio>` in `include/` or `src/` (the audit enforces it); the reference platform advances only when a test says so.
- **Public API self-containment passes** (one translation unit per public header in the build). **Traceability:** 72 requirements, 71 traced (CORE-ERR-003 reserved), 0 errors; `CORE-PLAT-012` to `017` defined once. **Prohibited dependency audit:** clean; no `find_package`, `FetchContent` or `ExternalProject`. **Full CTest regression:** 39/39 in Debug, Release, ASan+UBSan, strict `-Werror` and TSan (ASLR off); GCC `-fanalyzer` clean; coverage 98% (565/571, unchanged).
- **Freeze commit:** production freeze point `f23777b`; recorded after review in a docs-only commit.

### Open issues (none blocking)

Carried forward: the 32-bit scheduler affinity mask; the conformance suite's level-2 mutation strictness gap; the `make lint` / `make format-check` stubs; a `PlatformContext` outliving its adapter is documented undefined behavior (non-owning view by design). The only remaining work is R05-007 (validation, release metadata 0.5.0) and the Release Gate; neither may change production API.

Reviewer decision: **PASS / HONORED** (05-10-2026)

## Decision

Possible outcomes:

- PASS / HONORED
- CHANGES REQUIRED
- BLOCKED

Reviewer: ChatGPT architecture/review gate.

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Date | 05-10-2026 |
| Decision | **PASS / HONORED** |

**Production freeze point: `f23777b`** (last production-tree change, comment-only); last functional production change `f8cd523`. R05-005 `7f30626` and R05-006 `fe04d35` changed tests and audit tooling only; R05-006 accepted at `d66eda6`; evidence `cf6e617`; Platform API Review baseline `ea6ae16`. `git diff ea6ae16 HEAD -- include src` is empty: production API and behavior are unchanged after the freeze.

Frozen: `PlatformContext` (with `require_*()`), `PlatformRequirements`, `PlatformRequirementReport`, `evaluate()`, `check_required()`, the Runtime/platform lifecycle-separation contract, and the test-only reference platform and isolation guards. R05-007 is validation and release metadata only; the production API must not be redesigned or expanded.

Non-blocking open issues retained: the 32-bit scheduler affinity mask; the conformance suite's level-2 mutation strictness gap; the `make lint` / `make format-check` stubs; a `PlatformContext` outliving its adapter is documented undefined behavior.

**Reviewer Decision: PASS / HONORED — KF-CORE-R05 Platform Integration Freeze is ACCEPTED.**

## Post-Freeze Rule

No production API or semantic changes after this gate without explicit architecture review.
