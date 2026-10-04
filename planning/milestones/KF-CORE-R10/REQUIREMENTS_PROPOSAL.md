# KF-CORE-R10 — Requirements Proposal

The following IDs are proposed. They become authoritative only when the corresponding task evidence is accepted and root `REQUIREMENTS.md` traceability is reconciled.

| Requirement | Proposed meaning | Primary task |
|---|---|---|
| CORE-COMPAT-001 | Inventory and classify the installed/public Core API boundary. | R10-001 |
| CORE-COMPAT-002 | Define source compatibility rules for stable Core 1.x APIs. | R10-002 |
| CORE-COMPAT-003 | Define semantic compatibility rules, including ownership, lifetime, threading and observable behavior. | R10-002 |
| CORE-COMPAT-004 | Define the supported ABI/binary compatibility posture and any explicit support matrix. | R10-003 |
| CORE-COMPAT-005 | Define SemVer release-impact classification for public API and semantic changes. | R10-004 |
| CORE-COMPAT-006 | Define compatibility-sensitive rules for enums, ErrorCode values, virtual interfaces and public constants. | R10-004 |
| CORE-COMPAT-007 | Define public API evolution/change-review rules. | R10-004 |
| CORE-COMPAT-008 | Define deprecation, migration and removal policy. | R10-005 |
| CORE-COMPAT-009 | Validate compatibility boundaries with automated reference/boundary tests. | R10-006 |
| CORE-COMPAT-010 | Define and validate installed CMake package/version-selection compatibility. | R10-007 |
| CORE-REL-001 | Require synchronized release documentation, traceability and compatibility evidence for 1.0. | R10-008/R10-009 |
| CORE-SEC-001 | Assess security impact of compatibility, package and release boundaries without introducing a security subsystem. | R10-008 |

## Traceability Rule

Reuse an existing requirement where it already governs the behavior. Proposed IDs must not be silently renumbered merely to remove gaps. Root `REQUIREMENTS.md` remains the authoritative traceability source after acceptance reconciliation.
