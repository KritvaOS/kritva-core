# KF-CORE-R10 — Implementation Sequence

```text
R10 Design Consult
        ↓
R10 Scope Confirmation
        ↓
KF-CORE-R10-001 Public API Inventory & Compatibility Classification
        ↓
KF-CORE-R10-002 Source & Semantic Compatibility Contract
        ↓
KF-CORE-R10-003 ABI / Binary Compatibility Policy
        ↓
KF-CORE-R10-004 Versioning & API Evolution Policy
        ↓
KF-CORE-R10-005 Deprecation & Migration Policy
        ↓
R10 API / Compatibility Review
        ↓
KF-CORE-R10-006 Compatibility & Boundary Validation Harness
        ↓
KF-CORE-R10-007 Package / Install Compatibility
        ↓
R10 Integration Freeze
        ↓
KF-CORE-R10-008 Documentation / Security / Traceability Validation
        ↓
KF-CORE-R10-009 Full Validation & Release Candidate
        ↓
R10 Release Gate
```

No implementation task may silently expand the scope of an earlier accepted task. Any new production public API or semantic contract after the R10 API/Compatibility Review requires explicit return to architecture review.
