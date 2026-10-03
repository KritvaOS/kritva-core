# KF-CORE-R08 — Design Decisions

Status: **APPROVED / ARCHITECTURE CONFIRMED**

This record consolidates the R08 Design Consult and Scope Confirmation. It is architectural guidance and does not itself constitute task acceptance.

| ID | Decision | Resolution |
|---|---|---|
| D01 | Configuration role | Configuration is a detached control-plane value consumed synchronously by `Component::configure()`. |
| D02 | Lifecycle boundary | Configuration is valid only from `UNKNOWN` and `STOPPED`; no dynamic reconfiguration in R0.8. |
| D03 | Input ownership | Caller owns the supplied `Configuration`; conforming Components do not retain its address beyond the call. |
| D04 | Applied-state ownership | Component owns the semantic state it accepts/applies; Core maintains no second authoritative configuration store. |
| D05 | Failure atomicity | Failed `configure()` cannot leave partial applied configuration and leaves lifecycle state unchanged. |
| D06 | Validation boundary | Core performs generic structural validation; Component performs domain-specific semantic validation. |
| D07 | Error model | Use existing `ErrorCode` semantics; no per-parameter ErrorCode taxonomy is added in R0.8. |
| D08 | ConfigurationVersion | `ConfigurationVersion` means schema/contract compatibility version, not runtime revision/history. |
| D09 | Version representation | R0.8 does not require adding a generic version field to `Configuration` unless API review identifies a demonstrated contract gap. |
| D10 | Runtime role | Runtime forwards configuration in established dependency order; it does not interpret, persist, retry or rollback. |
| D11 | Runtime failure boundary | Configuration failure leaves Runtime lifecycle state unchanged and is returned without automatic recovery. |
| D12 | Status/Health | Configuration is independent of Status, Health and Runtime FAULT. |
| D13 | ComponentContext | R0.6 ComponentContext remains unchanged and configuration-neutral. |
| D14 | Dynamic control | Parameter services, live tuning, persistence, remote control and configuration events are deferred. |
| D15 | Platform boundary | No platform/vendor/OS/ROS2/DDS/EtherCAT implementation enters Core. |
| D16 | Freeze discipline | Any breaking or semantic public API change after the R08 Configuration API Review returns to architecture review. |

## R08 Release Principle

R0.8 should make the existing configuration path precise without turning `kritva-core` into a generic configuration-management framework.
