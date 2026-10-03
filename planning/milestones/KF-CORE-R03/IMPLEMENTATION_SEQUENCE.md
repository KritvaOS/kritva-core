# KF-CORE-R03 — Implementation Sequence

## Principle

R03 is implemented bottom-up, with explicit API freeze points to prevent cascading breaking changes.

```text
R03-001 Component Contract & Identity
        ↓
R03-002 Component Registry
        ↓
R03-003 Dependency Management
        ↓
R03 Foundation API Review
        ↓
R03-004 Runtime Manager
        ↓
R03-005 Runtime Lifecycle
        ↓
R03-006 Runtime Failure & Recovery
        ↓
R03 Runtime Contract Review
        ↓
R03-007 Runtime Integration Tests
        ↓
R03 Integration Freeze
        ↓
R03-008 Final Validation
        ↓
R03 Release Gate
```

## Phase A — Foundation

### R03-001

Define the stable component abstraction and identity.

### R03-002

Build the registry on the accepted component contract.

### R03-003

Build dependency representation, validation, cycle detection and deterministic ordering on the accepted component/registry contracts.

### Foundation API Review

Review the combined public API before any Runtime Manager implementation.

No R03-004 implementation starts until this gate is PASS.

## Phase B — Runtime

### R03-004

Implement synchronous, platform-independent Runtime Manager behavior within the existing `runtime::Runtime` / `CORE-RT-002` contract. Compose the frozen Registry and DependencyGraph without changing their public APIs.

### R03-005

Implement deterministic dependency-aware lifecycle orchestration. Forward operations use dependency order; teardown uses reverse dependency order.

### R03-006

Define deterministic runtime failure propagation and explicit caller-driven recovery/reset behavior. No automatic retry or autonomous recovery.

### Runtime Contract Review

Freeze runtime, lifecycle, error, diagnostic and statistics semantics before integration-test completion.

## Phase C — Validation

### R03-007

Add end-to-end runtime integration tests using reference components and no hardware/network dependencies. The task consumes the frozen Runtime Contract and introduces no production runtime behavior.

### Integration Freeze

After R03-007 acceptance, freeze the production API and accepted runtime semantics. Any production API/behavior change returns to architecture review.

### R03-008

Execute final release-candidate validation only. This task must not introduce new runtime functionality. Validate the frozen candidate from a clean build through install-consumer, traceability, sanitizers, static analysis and coverage.

### R03 Release Gate

After R03-008 acceptance, independently verify all R03 exit criteria and authorize the annotated `kritva-core-r0.3` release tag. Tag creation is not part of R03-008.

## Implementation Rules

1. One logical task = one primary implementation commit.
2. Focused review fixes use a separate `fix(core): ...` commit.
3. Do not amend an accepted implementation commit unless explicitly required.
4. Later tasks must not silently change an earlier accepted public API.
5. A required breaking change stops implementation and returns to architecture review.
6. Existing R0.2 contracts remain authoritative unless a reviewed contract gap is demonstrated.
7. Core remains platform independent.
