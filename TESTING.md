# Kritva Core Testing

## 1. Purpose

Testing validates that Kritva Core implements documented requirements and public contracts.

## 2. Test Levels

```text
Unit Test → Contract Test → Integration Test → System Validation
```

R0.1 primarily focuses on unit and contract testing.

## 3. Unit Tests

Validate individual Core types and behaviors: IDs, versions, time, lifecycle, status, health, statistics, errors, Result, events, capabilities, and configuration.

## 4. Contract Tests

Contract tests consume public headers from `include/kritva/core/` and verify documented behavior rather than implementation details.

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

Map test cases to requirement IDs where practical.

Example:

```text
CORE-LIF-001
    ↓
lifecycle_state.hpp
    ↓
test_lifecycle_state.cpp
```

## 10. Completion Criteria

A feature is complete only after requirement coverage, API documentation, implementation, relevant tests, successful build/test, and final diff review.
