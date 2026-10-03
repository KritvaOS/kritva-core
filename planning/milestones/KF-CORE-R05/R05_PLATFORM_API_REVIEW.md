# KF-CORE-R05 — Platform API Review

## Purpose

Freeze the R0.5 platform runtime integration API before reference-platform implementation.

## Entry Criteria

- R05-001 through R05-004 accepted.
- R0.4 platform API remains unchanged except approved R0.5 additions.
- Unit tests for all four tasks pass.
- Full regression passes.
- Proposed requirements are traceable.
- No unresolved public API ambiguity remains.

## Review Items

- PlatformContext is a view, not an owner.
- No generic ServiceRegistry or service locator exists.
- Service lifetime remains external.
- Required/optional service semantics are explicit.
- Capability identity is authoritative.
- Explicit service consumption is deterministic.
- Runtime lifecycle remains unchanged.
- No implicit service startup/shutdown/recovery exists.
- No Core-owned background execution exists.
- Callback and re-entry rules are documented.
- Error propagation is deterministic.
- Concrete platforms remain outside Core.

## Decision

Possible outcomes:

- PASS / FROZEN
- CHANGES REQUIRED
- BLOCKED

Reviewer: ChatGPT architecture/review gate.

## Freeze Rule

After PASS/FROZEN, breaking public API or semantic changes require architecture re-review before implementation continues.

## Evidence (submitted 05-10-2026)

Baseline: R05-001 `c5910e7`, R05-002 `f9d6007`, R05-003 `f8cd523`, R05-004 `f23777b` (all ACCEPTED; head `798b906`). API decisions Q1–Q9: `R05_API_DECISIONS.md` (`ef1ad34`).

### Public API diff against `kritva-core-r0.4` (`include/` and `src/`)

| File | Change |
|---|---|
| `platform/context.hpp` | **new** (R05-001, R05-003): `PlatformContext` view and `require_*()` queries |
| `platform/requirements.hpp` | **new** (R05-002): `Requirement`, `PlatformRequirements`, `PlatformRequirementReport`, `evaluate()`, `check_required()` |
| `runtime/runtime_manager.hpp` | **comment only** (R05-004): the lifecycle-separation section and the `CORE-PLAT-015` tag; no declaration changed |
| `core.hpp` | two added includes |

`git diff kritva-core-r0.4 HEAD -- src` is empty. Every R0.4 header is byte-identical to the released tag: `platform/adapter.hpp`, `scheduler.hpp`, `watchdog.hpp`, `boundary.hpp`, `clock.hpp`, `time/clock.hpp`, `time/timer.hpp`, `types/callback.hpp`, `capability/capability_set.hpp`, and the frozen R0.3 `runtime/component.hpp`, `runtime.hpp`, `component_registry.hpp`, `dependency_graph.hpp`. All R05 production API is additive.

### Review items

| Item | Evidence |
|---|---|
| `PlatformContext` is a view, not an owner | one pointer (`sizeof == sizeof(void*)`), trivially destructible, copyable, no cache; copying neither extends the adapter's lifetime nor transfers or duplicates ownership; the adapter must outlive every context |
| No generic `ServiceRegistry` or service locator | none exists; no static, global or thread-local state; a context holds exactly the adapter it was built with |
| Service lifetime remains external | the integrator owns the adapter and services; contexts and `require_*()` never start, stop, configure, create or own anything |
| Required/optional semantics explicit | `Requirement::{REQUIRED, OPTIONAL}`; duplicates rejected by identity, atomically; `satisfied()` ignores optional items |
| Capability identity is authoritative | matching is `CapabilityId == CapabilityId` only (mutation-tested against platform name, platform version and capability version) |
| Explicit consumption is deterministic | `require_*()` returns the adapter pointer or `UNSUPPORTED` (ERROR, no source) deterministically and independently per service; the report is authoritative and the message is never parsed |
| Runtime lifecycle unchanged | `src/` unchanged; `runtime_manager.hpp` comment-only; differential tests (every service combination, platform used between steps) give transcripts identical to the no-adapter baseline |
| No implicit startup/shutdown/recovery | service state is identical after every Runtime operation in every state; the Runtime makes zero adapter calls |
| No Core-owned background execution | no thread, loop or timer; services advance only when the integrator ticks them; production scan finds no threading header |
| Callback and re-entry rules documented | R0.4 rules unchanged; a callback re-entering its timer through a pointer obtained from a context gets `INVALID_STATE` as before |
| Error propagation deterministic | service errors are never translated; a Component that propagates one sets its own source; the Runtime propagates it unchanged; a platform failure is an ordinary component failure (proved equal to an injected failure) |
| Concrete platforms remain outside Core | only contracts, a view, a requirement model and test doubles; the prohibited-header audit is clean |

### Validation (head `798b906`; production unchanged since `f23777b`)

`ctest` 36/36 in Debug, Release, ASan+UBSan, strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror` and TSan (ASLR off via `setarch -R`); 0 warnings; GCC `-fanalyzer` clean; public-header self-containment passes; `make traceability-check`: 70 requirements, 69 traced (CORE-ERR-003 reserved), 0 errors, `CORE-PLAT-012` to `015` defined once; coverage 98% (565/571), the same baseline lines plus one exception-unwind brace; no `find_package` or `FetchContent`; `git diff --check` clean. Mutation testing per task: PlatformContext 17, requirements 30, consumption 20 (19 plus one equivalent), Runtime lifecycle 20, all detected.

### Open issues (none blocking)

Carried forward from R0.4: the 32-bit scheduler affinity mask; the conformance suite's level-2 mutation strictness gap; `make lint` and `make format-check` stubs. New: `PlatformContext` is documented as unsafe to use after its adapter is destroyed (no detection is possible by design). R05-005 and R05-006 build the reference platform and integration tests on this API and are not part of this freeze.

Reviewer decision: **PASS / FROZEN** (05-10-2026)

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Date | 05-10-2026 |
| Decision | **PASS / FROZEN** |

Gate record: R05-001 `c5910e7`, R05-002 `f9d6007`, R05-003 `f8cd523`, R05-004 `f23777b` (accepted at `798b906`); API decisions Q1–Q9 `ef1ad34`; evidence `05d981e`.

**Frozen R05 platform API:** `PlatformContext` (including `require_scheduler()`, `require_clock()`, `require_timer()`, `require_watchdog()`), `PlatformRequirements`, `PlatformRequirementReport`, `evaluate()`, `check_required()`, and the R05 Runtime/platform lifecycle-separation contract. No further API redesign during R05-005 through R05-007 unless a genuine blocking defect is found; any correction is a controlled exception to the freeze and is reviewed explicitly.

Non-blocking open issues retained: the 32-bit scheduler affinity mask; the conformance suite's level-2 mutation strictness gap; the `make lint` / `make format-check` stubs; and the documented undefined behavior if a `PlatformContext` outlives its adapter, a consequence of the deliberately non-owning view that is not a reason to introduce ownership or reference counting.

**Reviewer Decision: PASS / FROZEN — KF-CORE-R05 Platform API Review gate is ACCEPTED.**
