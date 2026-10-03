# KF-CORE-R03-002 — Component Registry
## Task

### Objective

Provide a deterministic registry for registered runtime components using `ComponentId`.

### Dependency

R03-001 must be accepted.

### Scope

- registration
- duplicate handling
- lookup
- contains
- deterministic enumeration
- ownership/lifetime

### Out of Scope

Lifecycle orchestration, dependency ordering, scheduler/executor, threads, recovery and hardware/platform implementation.

### Implementation Requirements

1. Build only on the accepted R03-001 contract.
2. Duplicate identities must fail deterministically.
3. Lookup must have deterministic missing-component behavior.
4. Enumeration order must be explicitly defined.
5. Do not rely on accidental unordered-container iteration.
6. Do not implement lifecycle orchestration.
7. Do not add unregister unless a concrete R03 requirement is demonstrated.

### Required Evidence

- implementation commit SHA
- changed files
- unit tests
- integration/regression tests
- Debug/Release results
- sanitizers
- coverage
- traceability/dependency checks
- final Git status

### Commit

`feat(core): add component registry`

### Reviewer Decision

PASS / CHANGES REQUIRED / BLOCKED
