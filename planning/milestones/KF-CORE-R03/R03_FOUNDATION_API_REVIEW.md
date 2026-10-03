# KF-CORE-R03 — Foundation API Review
## Gate After R03-003

### Purpose

Freeze the combined Component, Registry, and Dependency contracts before R03-004 Runtime Manager implementation.

### Entry Criteria

Objective evidence and the API snapshot: `R03_FOUNDATION_API_REVIEW_EVIDENCE.md`. The boxes below are ticked on that evidence; every box from "Component Contract" onward and the gate decision remain the reviewer's.

- [x] R03-001 accepted
- [x] R03-002 accepted
- [x] R03-003 accepted
- [x] all required unit tests pass
- [x] all required integration/regression tests pass
- [x] validation evidence complete

### Component Contract

- [x] `ComponentId` semantics frozen
- [x] lifecycle contract frozen
- [x] metadata frozen
- [x] ownership/lifetime frozen
- [x] Result/Status/Error interaction frozen

### Registry Contract

- [x] registration semantics frozen
- [x] duplicate handling frozen
- [x] lookup semantics frozen
- [x] enumeration semantics frozen
- [x] deterministic ordering frozen
- [x] ownership/lifetime frozen
- [x] unregister scope explicitly decided

### Dependency Contract

- [x] dependency representation frozen
- [x] missing dependency semantics frozen
- [x] self-dependency semantics frozen
- [x] duplicate dependency semantics frozen
- [x] cycle detection semantics frozen
- [x] topological ordering frozen
- [x] deterministic tie-break frozen

### Cross-Cutting Policy Review

These are reviewed here but are not fully implemented by R03-001..003:

- [x] Error policy
- [x] Warning policy
- [x] Info/diagnostic policy
- [x] Event vs message distinction
- [x] Statistics update policy
- [x] No logging backend in Core

### Architecture Constraints

- [x] no ROS2/DDS
- [x] no EtherCAT
- [x] no Linux/vendor APIs
- [x] no hardware dependencies
- [x] no scheduler/executor
- [x] no threads/thread pool
- [x] no runtime lifecycle orchestration
- [x] no automatic recovery

### Gate Decision

**PASS / FROZEN**

| Field | Sign-off |
|---|---|
| Architecture reviewer | ChatGPT |
| Date | 03-10-2026 |
| Decision | PASS / FROZEN |
| Notes | See "Recorded decisions" below. Evidence: `R03_FOUNDATION_API_REVIEW_EVIDENCE.md` (HEAD `6f9db3d`; production code last changed in `795fb94`). TSan and `-fanalyzer` were run at the task commits and not re-run on the documentation-only gate commit; this is recorded as evidence lineage, not a gap. |

A PASS authorizes implementation of `KF-CORE-R03-004`.

## Recorded decisions

Frozen (any change returns to architecture review): `ComponentId`; `ComponentInfo`; the `Component` lifecycle contract; registry ownership (strictly non-owning); registry enumeration (ascending `ComponentId`); dependency edge semantics (dependent -> dependency); dependency ordering (dependencies first); tie-break (lowest `ComponentId`); cycle rejection at insertion; failed dependency mutation leaves the graph unchanged; missing registered component -> `CONFIGURATION_ERROR`; `order()` returns a `ComponentId` sequence; duplicate registration -> `INVALID_ARGUMENT` with no replacement; `ALREADY_EXISTS` is NOT introduced; registered objects must outlive registry use; the registry is append-only for R03.

Open items resolved:
1. **Runtime interface.** `CORE-RT-002` remains the authoritative Runtime contract. R03-004 implements the existing `runtime::Runtime` interface and does not create a competing public Runtime abstraction. `CORE-RT-006` defines the concrete Runtime Manager behavior within that contract. The existing interface is not modified; a genuine incompatibility is an architecture-review issue, not something to solve silently.
2. **Append-only registry.** There is no `unregister_component()`. The Runtime Manager establishes a component set during initialization and treats it as fixed for the runtime instance. Dynamic topology is a future milestone.
3. **Component lifetime violation.** Accepted as an API precondition; no reference counting, ownership, destruction callbacks or unregister.
4. **Allocating `components()` / `order()`.** Frozen usage policy: construction, configuration, initialization and topology validation (control plane); not periodic or real-time paths. R03 makes no real-time claim.
5. **`ALREADY_EXISTS`.** Not added.
6. **Shallow-const registry.** Accepted and must stay clearly documented.

Policy decisions (with the reviewer refinement to the evidence-pack proposals):
- **Warning:** no dedicated Core Warning API in R03. `HealthState` (`HEALTHY`/`DEGRADED`/`UNHEALTHY`) represents health independently; a degraded state is **not** itself a warning. The evidence-pack wording that equates a warning with `DEGRADED` is withdrawn.
- **Distinctions frozen:** Health is not Warning; Event is not Message; Diagnostic is not Log. Diagnostics are structured values or state associated with an operation or condition; events represent Core-internal state/condition transitions; messages carry application/component data.
- **Error propagation:** the runtime preserves the originating component's `Error` (code and source) rather than replacing it with a generic runtime error; it may add context only where the existing error contract permits, never destroying the original source/error semantics (an R03-006 requirement).
- **Statistics:** a runtime owns its own `Statistics` instance and updates it according to defined runtime operations. No global Core statistics singleton, no cross-runtime shared object, no atomic-snapshot claim, no generic aggregation service. "Updated only from its calling thread" is an implementation policy, not a claim that the whole library is single-threaded.
- **Logging:** Core provides no logging backend. R03 introduces no `Logger`, `LogSink`, `LogLevel`, console/syslog logger, spdlog or ROS logging. Presentation of errors, status, diagnostics and events belongs to platform/application layers.
