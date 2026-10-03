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
