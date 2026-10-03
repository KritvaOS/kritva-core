# Kritva Core Testing

## 1. Purpose

Testing validates that Kritva Core implements documented requirements and public contracts.

## 2. Test Levels

```text
Unit Test → Contract Test → Integration Test → System Validation
```

R0.2 focused on unit and contract testing. R0.3 adds runtime integration tests (`tests/integration/`) that exercise Component, ComponentRegistry, DependencyGraph and RuntimeManager together through public APIs, and an install-and-consume test (`tests/install/`).

## 3. Unit Tests

Validate individual Core types and behaviors: IDs, versions, time, lifecycle, status, health, statistics, errors, Result, events, capabilities, and configuration.

## 4. Contract Tests

Contract tests consume public headers from `include/kritva/core/` and verify documented behavior rather than implementation details.

### Platform conformance suite (R0.4)

`tests/platform/` is a reusable, header-only conformance suite for platform adapters (CORE-PLAT-009). A future external adapter includes `tests/platform/adapter_conformance.hpp` (or one service header), builds an `Environment` whose `let_time_pass` sleeps or advances its fake clock, constructs a fresh service or adapter, and runs `check_scheduler`, `check_clock`, `check_timer`, `check_watchdog` or `check_platform_adapter`; failures are collected in a `Report`. The suite uses only public Core headers and the standard library, needs no hardware, network or vendor SDK, and checks only mandatory Core semantics: choices the contracts leave to the adapter are accepted in every permitted form and a feature an adapter cannot provide is a recorded skip, not a failure. It cannot observe a hardware watchdog's expiry action or timing accuracy. `tests/unit/platform_conformance_test.cpp` validates the suite itself: it must pass for the reference adapters under different adapter policies and reject deliberately faulty scheduler, timer, watchdog, clock and adapter doubles.

## 5. Required Categories

Each applicable API should test normal behavior, boundary conditions, invalid input, failure behavior, recovery behavior, compatibility assumptions, and thread-safety assumptions where applicable.

## 6. Real-Time Testing

Where an API may be used in real-time paths, explicitly consider allocation, blocking, synchronization, latency, bounded execution, and lock contention. Functional tests alone do not establish hard real-time suitability.

## 7. Build Validation

```bash
cmake --preset debug
cmake --build --preset debug
ctest --test-dir build/debug --output-on-failure
```

## 8. Formatting

Use the repository `.clang-format`. Limit formatting changes to affected code unless a dedicated formatting change is intended.

## 9. Traceability

Map test cases to requirement IDs where practical. `make traceability-check` audits the chain (REQUIREMENTS.md → header → implementation → test) and fails on undefined, duplicate, untraced, or dangling requirement IDs.

Example:

```text
CORE-LIF-001
    ↓
lifecycle_state.hpp
    ↓
lifecycle_test.cpp
```

## 10. Completion Criteria

A feature is complete only after requirement coverage, API documentation, implementation, relevant tests, successful build/test, and final diff review.
