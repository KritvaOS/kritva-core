# KF-CORE-R09 — API Documentation Plan

## Decision

**APPROVED — Markdown is the canonical maintained API documentation format.**

HTML is a generated publication format and is not an authoritative source document.

## Repository Structure

```text
docs/
├── api/
│   ├── README.md
│   ├── API_INDEX.md
│   ├── API_GUIDELINES.md
│   ├── capability/
│   │   ├── CAPABILITY.md
│   │   ├── CAPABILITY_SET.md
│   │   └── CAPABILITY_REQUIREMENTS.md
│   ├── configuration/
│   │   ├── CONFIGURATION.md
│   │   └── CONFIGURATION_VERSION.md
│   ├── context/
│   │   ├── COMPONENT_CONTEXT.md
│   │   └── PLATFORM_CONTEXT.md
│   ├── lifecycle/
│   │   └── LIFECYCLE.md
│   ├── platform/
│   │   ├── PLATFORM_ADAPTER.md
│   │   └── PLATFORM_REQUIREMENTS.md
│   ├── runtime/
│   │   ├── COMPONENT.md
│   │   ├── RUNTIME.md
│   │   └── DEPENDENCY_GRAPH.md
│   └── error/
│       ├── ERROR.md
│       └── ERROR_CODES.md
```

## Documentation Authority

```text
Accepted architecture decision
        ↓
Accepted public API contract
        ↓
Root requirements / traceability
        ↓
docs/api/*.md
        ↓
Contract tests / examples
```

The documentation shall not introduce behavior or semantics absent from the accepted API contract.

## Required Content

Each API document should cover, where applicable:

1. Purpose.
2. Scope.
3. Public API surface.
4. Semantics and invariants.
5. Ownership/lifetime.
6. Lifecycle interaction.
7. Error behavior.
8. Thread-safety.
9. Allocation/blocking/real-time expectations.
10. Compatibility semantics.
11. Security considerations.
12. Examples where useful.
13. Requirements traceability.
14. Related headers.
15. Related tests.
16. Explicit exclusions.

## R0.9 Minimum Documentation Deliverables

- `docs/api/README.md`
- `docs/api/API_INDEX.md`
- `docs/api/API_GUIDELINES.md`
- Capability domain documentation.
- Capability requirement/matching documentation.
- Cross-reference to lifecycle/dependency/readiness boundaries.
- Security notes for affected APIs.

## Acceptance Rule

Every accepted public API or semantic contract change must update the corresponding `docs/api/` Markdown in the same task. Header/documentation inconsistency is an acceptance blocker.

## Publication

A future documentation toolchain may generate HTML from Markdown. Generated HTML shall not become the authoritative repository specification.
