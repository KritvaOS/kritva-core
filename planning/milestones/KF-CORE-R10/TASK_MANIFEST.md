# KF-CORE-R10 — Task Manifest

| Task | Title | Est. | Dependency | Gate |
|---|---|---:|---|---|
| KF-CORE-R10-001 | Public API Inventory & Compatibility Classification | 2–3 ED | Scope Confirmation | Compatibility baseline |
| KF-CORE-R10-002 | Source & Semantic Compatibility Contract | 3–4 ED | R10-001 | Compatibility contract |
| KF-CORE-R10-003 | ABI / Binary Compatibility Policy | 2–3 ED | R10-002 | ABI policy |
| KF-CORE-R10-004 | Versioning & API Evolution Policy | 3–4 ED | R10-003 | Review input |
| KF-CORE-R10-005 | Deprecation & Migration Policy | 2–3 ED | R10-004 | Review input |
| R10 API / Compatibility Review | Freeze R1.0 compatibility semantics | 1 ED | R10-001..005 | PASS / FROZEN |
| KF-CORE-R10-006 | Compatibility & Boundary Validation Harness | 3–4 ED | API Review | Contract tests |
| KF-CORE-R10-007 | Package / Install Compatibility | 2–3 ED | R10-006 | Integration Freeze input |
| R10 Integration Freeze | Freeze R1.0 compatibility/package behavior | 0.5 ED | R10-007 | PASS / HONORED |
| KF-CORE-R10-008 | Documentation / Security / Traceability Validation | 2–3 ED | Integration Freeze | Validation |
| KF-CORE-R10-009 | Full Validation & Release Candidate | 2–3 ED | R10-008 | Release Gate input |
| R10 Release Gate | Release 1.0.0 | 1 ED | R10-009 | PASS |

**Working estimate: 24.5–34.5 ED**, including architecture and release gates. The milestone range is rounded to **25–35 ED** for program planning; actual effort remains unrecorded until supported by evidence.
