# Kritva Core Testing

## 1. Purpose

Testing validates that Kritva Core implements documented requirements and public contracts.

## 2. Test Levels

```text
Unit Test → Contract Test → Integration Test → System Validation
```

R0.2 focused on unit and contract testing. R0.3 adds runtime integration tests (`tests/integration/`) that exercise Component, ComponentRegistry, DependencyGraph and RuntimeManager together through public APIs, and an install-and-consume test (`tests/install/`). R0.5 adds platform integration tests (`tests/unit/platform_context_test.cpp`, `platform_requirements_test.cpp`, `platform_service_test.cpp`, `runtime_platform_lifecycle_test.cpp`, `reference_platform_test.cpp`), the Runtime/platform integration test (`tests/integration/runtime_platform_integration_test.cpp`), a test-only reference platform and a production-isolation CTest. R0.4 adds platform contract tests (`tests/unit/*_contract_test.cpp`, `tests/unit/platform_adapter_test.cpp`) built on reference test doubles (`tests/contract/reference_*.hpp`), the reusable platform conformance suite (`tests/platform/`, see below) and the Runtime-platform differential test (`tests/integration/runtime_platform_test.cpp`).

## 3. Unit Tests

Validate individual Core types and behaviors: IDs, versions, time, lifecycle, status, health, statistics, errors, Result, events, capabilities, and configuration.

## 4. Contract Tests

Contract tests consume public headers from `include/kritva/core/` and verify documented behavior rather than implementation details.

### Platform conformance suite (R0.4)

`tests/platform/` is a reusable, header-only conformance suite for platform adapters (CORE-PLAT-009). A future external adapter includes `tests/platform/adapter_conformance.hpp` (or one service header), builds an `Environment` whose `let_time_pass` sleeps or advances its fake clock, constructs a fresh service or adapter, and runs `check_scheduler`, `check_clock`, `check_timer`, `check_watchdog` or `check_platform_adapter`; failures are collected in a `Report`. The suite uses only public Core headers and the standard library, needs no hardware, network or vendor SDK, and checks only mandatory Core semantics: choices the contracts leave to the adapter are accepted in every permitted form and a feature an adapter cannot provide is a recorded skip, not a failure. It cannot observe a hardware watchdog's expiry action or timing accuracy. `tests/unit/platform_conformance_test.cpp` validates the suite itself: it must pass for the reference adapters under different adapter policies and reject deliberately faulty scheduler, timer, watchdog, clock and adapter doubles. The reference doubles in `tests/contract/` are intentionally not `final`: conformance and mutation tests derive faulty variants from them to inject contract-violating or adapter-defined behavior. Each check must be given a fresh service or adapter, so state left by one service's checks cannot contaminate the next. The suite's own mutation analysis has two levels: level 1 (deliberately faulty doubles are rejected) is the contract evidence; level 2 (neutralizing each individual check) is a known strictness gap, because many checks overlap, and the suite is not claimed to be 100% mutation complete.

### Reference platform and isolation (R0.5)

`tests/platform/reference_platform.hpp` is a test-only, hardware-free reference platform (CORE-PLAT-016). `ReferencePlatform` implements only the public `IPlatformAdapter`, `IScheduler`, `time::IClock`, `time::ITimer` and `IWatchdog` contracts on top of the doubles in `tests/contract/`, and adds controls a real platform never offers: selectable services and capabilities (fixed at construction), deterministic fault injection per service method (the Nth call or every call, atomic, counted and logged), an ordered log of service calls, timer callbacks and task entries, a count of queries made to the adapter, a controllable clock, explicit time advancement (nothing advances by itself: no thread, no loop, no real time), a `LifetimeProbe`, and a conformance `Environment`. It is test support only and is never compiled into, or installed with, the production library: the repository audit and the CTest `kritva_core_test_isolation` (production code includes nothing from `tests/`, every production translation unit compiles with only `include/` on the include path, the production target lists no test source, no `install()` rule installs test support) enforce this.

### Runtime and platform integration tests (R0.5)

`tests/integration/runtime_platform_integration_test.cpp` (CORE-PLAT-017) drives the Runtime with integrator-written components that use scheduler, clock, timer and watchdog services through a `PlatformContext` on the test-only reference platform, through public APIs only. Its central proof is equivalence: a platform failure injected at any service method, on any attempt and with any code must look to the Runtime exactly like a plain component failure in the same place. It also covers service availability for every combination, requirement gating, a seeded differential model with and without a platform (every platform method failing), fault/reset/recovery, statistics, watchdog expiry and callback independence, attachment rules and lifetime. `tests/integration/runtime_scenarios.hpp` provides the seeded scenarios.

### Component context harness (R0.6)

`tests/runtime/reference_context_component.hpp` and `tests/platform/context_conformance.hpp` (CORE-CTX-005) are test-only support for `ComponentContext`. `ReferenceContextComponent` is a conforming Component that holds a context built from its own identity and runs scripted per-operation plans of steps through it (typed service queries, service use, requirement checks, capability queries), records every executed step, and observes its own destruction; `check_component_context()` verifies the context contract (identity, platform view, typed queries, identity-only capability matching, attribution, requirement binding equal to R0.5, determinism) for one context over its adapter and is reusable for any context. Fault injection, call logs, adapter-query counts and lifetime probes come from the R0.5 reference platform. The harness never enters production code: the audit and `kritva_core_test_isolation` fail on any production include of it. Structural contract facts (shape, immutability, the closed access surface, no Runtime or Component members) are enforced by the unit tests of each task, and mutation testing of the production header is run against both.

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
