# KF-CORE-R07 — Design Decisions

Status: APPROVED / ARCHITECTURE CONFIRMED

This record consolidates the R0.7 Design Consult and Scope Confirmation decisions. It is architectural guidance for implementation and does not itself grant task acceptance.

| ID | Decision | Resolution |
|---|---|---|
| D01 | Operational state model | No new Component Operational State machine. Lifecycle, Status and Health remain distinct. |
| D02 | Operational ownership | Component is authoritative for its operational information. Core does not mirror operational truth. |
| D03 | Health semantics | Health is Component-reported information, independent of Runtime FAULT and never an automatic recovery trigger. |
| D04 | Diagnostic abstraction | No generic Diagnostic container in R0.7. Existing typed diagnostic/observation values remain authoritative. |
| D05 | Event mechanism | Explicit Component event reporting to an integrator-owned sink. No Core EventBus, queue, broker or dispatcher. |
| D06 | Event meaning | Event describes an occurrence; it is not an implicit lifecycle/recovery command. |
| D07 | Statistics ownership | Component statistics are Component-owned and distinct from Runtime statistics. |
| D08 | Statistics optionality | Statistics are optional. Do not force meaningless statistics onto every Component. |
| D09 | Observation semantics | Observation is read-only, side-effect free and returns value/snapshot data where applicable. |
| D10 | Cross-property coherency | Core makes no atomic cross-property snapshot guarantee. |
| D11 | Threading | No Core-owned observation thread, worker, polling loop or background execution. |
| D12 | Runtime boundary | Runtime remains lifecycle authority and does not automatically poll, interpret or react to operational information. |
| D13 | Context boundary | Do not broaden ComponentContext into a general operational management interface without separate architecture review. |
| D14 | Platform boundary | No platform/vendor/OS/ROS2/DDS/EtherCAT implementation enters `kritva-core`. |
| D15 | Scope control | Breaking or semantic API changes after R07 API Freeze return to architecture review. |

## R07-001 Specific Decision

`statistics()` is **not** added to the mandatory `runtime::Component` base interface by default. R07-004 defines the optional Component statistics contract.

## R07-003 Specific Decision

Operational event reporting is explicit and integrator-owned at the sink boundary. Core does not buffer, retry, persist or asynchronously dispatch Events.

## Implementation API Consult (05-10-2026)

Independent reviewer decision: AMENDMENTS — APPROVED. Surface: `runtime::ComponentObservation` + free `observe(const Component&, const IComponentStatistics* = nullptr)` (R07-001); contract and conformance of the existing `Status`/`Health` with no new production types (R07-002); `IEventSink` + non-owning `ComponentEventReporter` (R07-003); optional `IComponentStatistics`, not a base of `Component` (R07-004). `component.hpp` stays byte-identical; the Runtime gets no observation API.

| ID | Amendment |
|---|---|
| A1 | Accessor order is documented and tested: `info().id()`, `lifecycle_state()`, `status()`, `health()`, then `statistics()` when a provider is supplied; the observation never refers into the component. |
| A2 | Event source integrity: source_id zero is stamped with the component id; equal is forwarded unchanged; non-zero and different is `INVALID_ARGUMENT` with no sink call (never silently overwritten); unbound is `INVALID_STATE`; the sink's `Result` is returned unchanged. |
| A3 | `statistics` semantics: null provider is `std::nullopt`; non-null is an engaged copy of what the provider returned (no further meaning). |
| A4 | Purity wording: the observation contract applies to conforming implementations; `observe()` itself adds no side effect but invokes virtual const accessors Core cannot police. |
| A5 | Event sink lifetime: non-owning and integrator-managed; the sink must outlive every reporter that can invoke it. |
| A6 | `IComponentStatistics` carries no component identity; the caller pairs it with its component. |
