# KF-CORE-R05 — Implementation Sequence

## Principle

R0.5 proceeds from a minimal context/view, to declarative service requirements, to explicit service consumption, then freezes the public model before building reference-platform integration tests.

```text
R05-001 Platform Context & Service Access Model
        ↓
R05-002 Platform Service Requirement Model
        ↓
R05-003 Explicit Platform Service Consumption
        ↓
R05-004 Runtime–Platform Lifecycle Boundary
        ↓
R05 Platform API Review
        ↓
R05-005 Reference Platform Integration
        ↓
R05-006 Platform Integration & Runtime Tests
        ↓
R05 Platform Integration Freeze
        ↓
R05-007 Full R0.5 Validation
        ↓
R05 Release Gate
```

## Phase A — Integration Model

### R05-001
Define `PlatformContext` as a small non-owning view over the already accepted `IPlatformAdapter`. It must not own services or become a registry/locator.

### R05-002
Define how Core functionality expresses required and optional platform services using existing capability identity and a minimal requirement model. Avoid platform-name or version inference.

### R05-003
Define explicit service consumption semantics for scheduler, clock, timer and watchdog, including unavailable-service behavior and error propagation.

### R05-004
Define Runtime/platform lifecycle separation. Preserve R0.3 Runtime state and failure semantics and prohibit implicit platform-service startup/shutdown/recovery.

## R05 Platform API Review

Freeze the combined public model before reference-platform implementation begins.

## Phase B — Reference Integration

### R05-005
Build a test-only reference platform and reusable service fakes that can exercise availability, capabilities, callbacks, service failures and lifetime behavior without physical hardware.

### R05-006
Add deterministic Runtime/platform integration tests, including differential tests with and without an attached platform and negative tests for unavailable/invalid services.

## R05 Platform Integration Freeze

After R05-006, production platform-integration API and semantics are frozen. Required breaking changes return to architecture review.

## Phase C — Validation

### R05-007
Run the complete release-candidate validation suite, including unit, integration and regression testing, sanitizers, static analysis, coverage, traceability, install-consumer and dependency audits.

## Testing Rule

Every R0.5 task that changes production behavior must add or update focused **unit tests** for the changed contract.

Integration behavior must be validated separately by **integration tests** using only public APIs and test-only reference/fake platform services.

Every task must run the **full existing regression suite** in addition to its focused tests. A task is not accepted merely because its new tests pass.

Minimum regression command:

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
```

Where repository targets exist, also run:

```bash
make check
make traceability-check
```

The task acceptance criteria may require additional configurations such as ASan/UBSan, TSan, `-Werror`, GCC analyzer and install-consumer validation.

## Commit Rule

One logical task = one primary implementation commit.

The exact commit message is defined by the task's `ACCEPTANCE_CRITERIA.md`.

Do not amend an accepted task commit. Review fixes use a new focused commit when necessary.

No unrelated changes are permitted.

## API Freeze Rule

Any breaking public API or semantic change discovered after the applicable freeze gate stops implementation and returns to architecture review.
