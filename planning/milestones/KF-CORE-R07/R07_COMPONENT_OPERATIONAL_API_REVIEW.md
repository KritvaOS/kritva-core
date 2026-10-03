# R07 Component Operational API Review

## Gate

PASS / FROZEN is required before R07-005 implementation begins.

## Entry Criteria

- R07-001 through R07-004 accepted.
- All operational API proposals reconciled with existing R0.2–R0.6 contracts.
- No unapproved Runtime lifecycle change.
- Status/Health/Statistics/Event semantics internally consistent.

## Review Checks

- Observation is read-only and side-effect free.
- No new OperationalState machine.
- Statistics optional, not forced onto base Component without explicit approval.
- Event sink ownership remains integrator-side.
- No EventBus/queue/worker/telemetry backend.
- No hidden lifecycle or recovery path.
- Public headers self-contained.
- Requirements traceability complete.
- Contract/mutation tests sufficient.

## Evidence (submitted 05-10-2026)

Baseline: R07-001 `6849a73`, R07-002 `61e0067`, R07-003 `56ff226`, R07-004 `16654e9` (all ACCEPTED; head `4db2040`). API decisions: `R07_DESIGN_DECISIONS.md` (Implementation API Consult A1–A6).

### Public API diff against `kritva-core-r0.6` (`include/` and `src/`)

| File | Change |
|---|---|
| `runtime/component_observation.hpp` | **new** (R07-001/002): `runtime::ComponentObservation`, `runtime::observe()`; Status/Health reporting contract |
| `runtime/component_statistics.hpp` | **new** (R07-001/004): optional `runtime::IComponentStatistics` |
| `runtime/component_events.hpp` | **new** (R07-003): `runtime::IEventSink`, `runtime::ComponentEventReporter` |
| `core.hpp` | three added includes |

`git diff kritva-core-r0.6 HEAD -- src` is **empty**. `runtime/component.hpp`, `component_context.hpp`, `runtime_manager.hpp`, `component_info.hpp`, `runtime.hpp`, `status/status.hpp`, `health/health.hpp`, `event/event.hpp`, `statistics/statistics.hpp`, `counter.hpp`, `gauge.hpp` and `platform/context.hpp` are byte-identical to the released tag. All R07 production API is additive: three headers, three includes.

### The frozen contract

- `ComponentObservation` (aggregate: `id`, `lifecycle`, `status`, `health`, `std::optional<Statistics> statistics`) and `observe(const Component&, const IComponentStatistics* = nullptr)`: read-only, detached, documented accessor order, no Core mirror, no atomic cross-property snapshot, purity a contract on conforming implementations, no new operational state machine.
- Status/Health: component-owned value snapshots, independent of lifecycle and of each other and of Runtime FAULT; any combination legal; never read, set or reset by the Runtime; never a recovery trigger.
- `IComponentStatistics`: optional, not a base of `Component`, by-value, pass-through, caller-paired, separate from Runtime statistics.
- `IEventSink` (integrator-owned) and `ComponentEventReporter` (two non-owning pointers, immutable, unbound default): unbound `INVALID_STATE`; mismatching non-zero source `INVALID_ARGUMENT` with no sink call; zero stamped; match unchanged; only `source_id` touched; one synchronous call; sink `Result` unchanged; no buffering, retry, queue or dispatch; sink must outlive reporters; an Event never commands.

### Review checklist evidence

| Check | Evidence |
|---|---|
| Observation is read-only and side-effect free | spy component: exact accessor order and count, no `capabilities()`, no lifecycle call; observing a Runtime-registered component leaves state and statistics unchanged (R07-001); 20 observations of the worst reports change nothing (R07-002) |
| No new OperationalState machine | none exists; no new status, health or lifecycle type or enumerator |
| Statistics optional, not forced on `Component` | `static_assert`s: no `statistics()` on `Component`, not a base either way; a plain component is fully usable (R07-004) |
| Event sink ownership remains integrator-side | non-owning pointers, temporary sink refused, Core stores/creates/destroys no sink (R07-003) |
| No EventBus, queue, worker or backend | one synchronous call, no buffering/retry (mutants retry/swallow/double call detected); no thread or telemetry code in production |
| No hidden lifecycle or recovery path | the Runtime never calls `status()`, `health()`, a provider or a sink (spies = 0 through lifecycle, FAULT and reset); reported Health never changes failure handling (4 health values, identical outcome) |
| Public headers self-contained | one TU per public header (`kritva_core_header_checks`) |
| Requirements traceability complete | `CORE-OPS-001` to `006` and `008` defined with rows (007 and the release validation requirement come with R07-006/R07-007); 87 requirements, 86 traced, 0 errors |
| Contract and mutation tests sufficient | 10 + 9 + 22 + 7 mutants; survivors closed (Runtime masking by Health) or classified equivalent/redundant (event reporter half-bound states, redundant deleted overload — removed) |

### Validation (head `4db2040`)

`ctest` 49/49 in Debug, Release, ASan+UBSan, strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror` and TSan (ASLR off); 0 warnings; GCC `-fanalyzer` clean; `make check` and traceability clean; coverage 617/624; `git diff --check` clean.

### Open issues (none blocking)

Carried forward: 32-bit scheduler affinity mask; conformance level-2 mutation gap; `make lint`/`make format-check` stubs; a context, reporter or provider used after what it refers to is destroyed is documented undefined behavior (non-owning by design). R07-005 and R07-006 build the reference harness and integration tests on this API and are not part of this freeze.

## Decision

Reviewer decision: **PASS / FROZEN** (05-10-2026), ChatGPT (independent reviewer). Evidence commit `6b1296f`.

Freeze rule (approved): R07-005 onward must not change the frozen production API (`runtime/component_observation.hpp`, `runtime/component_statistics.hpp`, `runtime/component_events.hpp` and their public declarations) or its observable semantics except through an explicit architecture-review exception. Test-only additions and documentation may continue normally.
