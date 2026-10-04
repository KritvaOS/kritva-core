# KF-CORE-R09 — Design Decisions

Status: **APPROVED / ARCHITECTURE CONFIRMED**

This record consolidates the R09 Design Consult and Scope Confirmation. It is architectural guidance and does not itself constitute task acceptance.

| ID | Decision | Resolution |
|---|---|---|
| D01 | Core posture | R0.9 is API-neutral by default; existing APIs are preferred. |
| D02 | Capability | Capability is descriptive functionality metadata with authoritative `CapabilityId`. |
| D03 | Version | Capability version describes the provided capability contract; it is not runtime or security state. |
| D04 | CapabilitySet | CapabilitySet is a deterministic, ownership-safe value/snapshot collection; no global registry is introduced. |
| D05 | Requirement | Requirement expresses a consumer prerequisite; it does not imply Component dependency ordering. |
| D06 | Matching | Identity-based capability matching remains the default; no generic version-range framework is introduced. |
| D07 | Dependency | Runtime DependencyGraph remains ComponentId-based and independent of capability matching. |
| D08 | Readiness | Core does not calculate or own a generic readiness state in R0.9. |
| D09 | Lifecycle | Missing prerequisites may cause a Component lifecycle operation to fail through existing Error/Result semantics; Core adds no new lifecycle state. |
| D10 | Health | Health remains independent of capability availability and lifecycle. |
| D11 | Platform | Existing PlatformRequirements/PlatformContext mechanisms remain the generic platform boundary; R0.9 does not duplicate them. |
| D12 | Documentation | Markdown under `docs/api/` is canonical; every accepted public API/semantic change updates docs in the same task. |
| D13 | Security | R0.9 performs trust/authority/security-impact assessment but does not add a security subsystem. |
| D14 | Genericity | No Nexus/Edge/Linux/MCU/EtherCAT/ROS2/DDS/vendor assumptions enter Core. |
| D15 | API Review | Any new public production API requires explicit R09 Capability API Review approval. |

## Architectural Rule

> Capability provision, capability requirement, Component dependency ordering, lifecycle, readiness and health are distinct concepts unless an explicit Core contract states otherwise.
