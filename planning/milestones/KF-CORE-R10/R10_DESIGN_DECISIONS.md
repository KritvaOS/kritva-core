# KF-CORE-R10 — Design Decisions

Status: **APPROVED / ARCHITECTURE CONFIRMED FOR SCOPE CONFIRMATION**

| ID | Decision | Resolution |
|---|---|---|
| D01 | Milestone role | R1.0 is an API maturity/compatibility milestone, not a feature-accumulation milestone. |
| D02 | Baseline | R0.9 is the normative pre-1.0 functional/API baseline. |
| D03 | Compatibility dimensions | Source, semantic and ABI compatibility are distinct dimensions. |
| D04 | ABI posture | No universal ABI guarantee is implied; any supported ABI requires an explicit support matrix. |
| D05 | SemVer | Release impact is classified by public API and semantic contract change, not file-change size. |
| D06 | Semantic stability | Signature-preserving behavior changes are compatibility events. |
| D07 | Enums/errors | Public numeric values and meanings are compatibility-sensitive. |
| D08 | Ownership/lifetime | Ownership, lifetime and thread-safety guarantee changes are compatibility-sensitive. |
| D09 | Deprecation | Deprecation is a formal migration mechanism; stable API removal normally requires a major release. |
| D10 | Package | CMake package/version-selection behavior is part of the supported installation contract. |
| D11 | API classification | Public API items are explicitly classified; internal/test artifacts are not contractual by default. |
| D12 | Documentation | Markdown remains canonical; compatibility policy is authoritative and linked from API documentation governance. |
| D13 | Stub policy | R0.9 stub API pages remain stubs unless an explicit R1.0 decision or public contract change requires maintenance. |
| D14 | Security | R1.0 performs security-impact assessment of compatibility/package/release boundaries; no security subsystem is introduced. |
| D15 | Platform neutrality | No Nexus/Edge/Linux/MCU/EtherCAT/ROS2/DDS/vendor semantics enter Core. |
| D16 | History | R0.9 release records are historical evidence and are not retroactively rewritten by R1.0. |
