# Kritva Core API

## 1. Purpose

Public API organization for Kritva Core R0.2. The API is designed before implementation.

## 2. Include Root

`include/kritva/core/`

## 3. Namespace

```cpp
namespace kritva::core {
}
```

## 4. Public API Organization

```text
kritva/core/
├── types/{id.hpp,version.hpp,timestamp.hpp,duration.hpp,metadata.hpp}
├── lifecycle/{lifecycle_state.hpp,lifecycle.hpp}
├── status/{status_code.hpp,status.hpp}
├── health/{health_state.hpp,health.hpp}
├── statistics/{counter.hpp,gauge.hpp,statistics.hpp}
├── error/{error_code.hpp,error.hpp,result.hpp}
├── event/{event_type.hpp,event.hpp}
├── capability/{capability_id.hpp,capability.hpp,capability_set.hpp}
├── configuration/{parameter.hpp,configuration.hpp,configuration_version.hpp}
├── runtime/{component.hpp,runtime.hpp}
├── messaging/{message.hpp,topic.hpp}
├── time/{clock.hpp,timer.hpp}
├── platform/{scheduler.hpp,clock.hpp,watchdog.hpp}
└── core.hpp
```

## 5. API Design Rules

Public types should have explicit ownership semantics, strong types where practical, documented thread-safety, documented allocation behavior where relevant, documented error behavior, minimal dependencies, and no hidden global state.

## 6. Umbrella Header

`core.hpp` is the convenience public header and must not expose private implementation details.

## 7. Stability

Public does not automatically mean stable. Stability requires documented behavior, requirement traceability, tests, review, and compatibility assessment.

## 8. API Changes

Public API changes require justification, impact analysis, updated tests, documentation, and human review when compatibility is affected.


## 9. Architectural Dependency Direction

The Core `time` domain owns the platform-neutral `IClock` contract. Platform adapters may implement that contract; Core time APIs must not depend on Linux, RTOS, PTP, or vendor clock implementations.

## 10. Real-Time Contract

Each API intended for a real-time path must document allocation, blocking, synchronization, execution-boundedness, and thread-safety expectations. Topic construction, configuration mutation, and other potentially allocating operations are control-plane APIs unless explicitly documented otherwise.

## 11. Lifecycle Contract

Lifecycle transitions are validated by the Core lifecycle implementation. The allowed transition table is documented in `ARCHITECTURE.md` ("Lifecycle transitions") and is verified exhaustively (all 64 state pairs) by `tests/contract/foundation_contract_test.cpp`.

## 12. Configuration Contract

`Configuration::validate()` provides the Core validation entry point. R0.2 validates structural correctness; richer constraints such as ranges, enumerations, and required/default semantics remain a later extension.

## 13. Result Contract

`Result<T>` and `Result<void>` hold exactly one outcome: success (`has_value()`) or failure (an `Error`). `value()` requires success, `error()` requires failure, and `failure()` requires `error.code != ErrorCode::NONE`. Violations are programming errors: `assert` in debug builds, undefined behavior in release builds. Accessors never throw. Results are copyable/movable when `T` is; a moved-from Result keeps its outcome but its payload is unspecified. There is no default constructor. Results are not thread-safe. See `include/kritva/core/error/result.hpp` (CORE-ERR-004).

## 14. Status Contract

`StatusCode` (`status/status_code.hpp`) is the authoritative status enumeration; `UNKNOWN` (zero, default) means undetermined and `OK` is the only success code. `Status` (`status/status.hpp`) pairs a `StatusCode` with an optional message. The code and message are independent and mutated in place; an empty message is valid for any code. `Status()` is `UNKNOWN`/empty; `Status(code)` is explicit. Only `set_message()` and copies allocate. Not thread-safe for concurrent mutation. Both headers are self-contained. See CORE-STA-001.

## 15. Statistics Contract

`Counter` is a monotonic `uint64_t` that wraps modulo 2^64 and is cleared only by `reset()`. `Gauge` is an `int64_t` that stores the last value set, unclamped. `Statistics` aggregates four counters (`sample_count`, `error_count`, `retry_count`, `drop_count`) and two gauges (`queue_depth`, `utilization` as whole percent by convention, not enforced). None of these types is thread-safe, atomic, or a synchronization primitive, and none allocates or blocks; concurrent access needs external synchronization, and a copied `Statistics` is not an atomic snapshot. They are not hard-real-time guarantees. No telemetry transport or serialization exists in Core. See CORE-STS-001..003.

## 16. Scheduler Contract

`platform::IScheduler` (`platform/scheduler.hpp`) is a contract only; Core contains no scheduler implementation, and policies that belong to platform adapters are left to them. A scheduler is STOPPED or RUNNING. `create_task()` while STOPPED registers an inactive task (it never starts it or invokes `entry`). While RUNNING it is an adapter policy: either dynamic creation is supported, or the call fails atomically with `INVALID_STATE`; Core does not mandate a static task set. `start()` starts all created tasks and is idempotent and all-or-nothing; `stop()` is idempotent and returns only when no `entry` is running. `stop()` must not be called from a task entry because the synchronous, scheduler-wide stop would wait on the calling task itself. Implementations must reach an orderly stop before releasing scheduler resources; Core does not specify destructor behavior. `entry` must be non-null and valid for the scheduler's lifetime; `context` is caller-owned and must outlive the running tasks. `TaskConfig`: `priority` is an implementation-independent relative value (higher = more urgent within one scheduler) that the adapter maps to its platform, with no Core-defined range or maximum; `cpu_affinity` is a bit mask of logical CPUs where `0` means *no constraint* (pin to CPU 0 with `0x1`), an invalid mask fails with `INVALID_ARGUMENT` and a well-formed request the platform cannot provide fails with `UNSUPPORTED` (the 32-bit mask is an R0.2 limitation, not a long-term architectural limit); `period` zero means aperiodic and negative is invalid. `TaskId` is opaque; 0 is invalid; valid ids are unique among existing tasks of one scheduler; reuse after a future destroy operation is implementation-defined. All `create_task()` errors are atomic. Failures use `INVALID_ARGUMENT`, `INVALID_STATE`, `UNSUPPORTED` and `RESOURCE_UNAVAILABLE` via `Result`. Control-plane only, not thread-safe by default, no real-time guarantee. Conformance checks: `tests/contract/scheduler_contract.hpp`. See CORE-PLAT-001.

## 17. Clock and Timestamp Contract

`time::IClock` (`time/clock.hpp`) is the canonical Core clock abstraction; `platform/clock.hpp` provides only the compatibility alias `platform::IClock` (a migration path, with no removal date set), which is the identical type and adds no contract. Platform adapters should implement `time::IClock` directly. Each clock instance has one fixed `ClockDomain`: `MONOTONIC` never decreases and has an unspecified epoch (only differences are meaningful), `REALTIME` is wall-clock time that may step backward or forward. `Timestamp` equality includes the domain, and `Timestamp` deliberately has no ordering or subtraction, so timestamps of different domains cannot be compared or combined silently; callers computing on `nanoseconds()` must check `domain()` first. Same domain is necessary but not sufficient for comparison: use timestamps from the same clock source. `now()` is `const noexcept`; allocation, blocking and latency are adapter properties, and Core makes no hard-real-time claim. Thread-safety is adapter-defined: `IClock` imposes no universal guarantee, and an adapter documents whether concurrent `now()` calls are supported or restricted. Core contains no OS clock implementation, timers or callbacks (`ITimer` is a separate contract). See CORE-TIME-001, CORE-PLAT-002.

## 18. Requirements Traceability

Every public header carries a `Requirements:` tag and appears in the traceability table in `REQUIREMENTS.md` (Requirement → Public header → Implementation → Test). Process and build-level requirements are traced separately to their artifact and verification (a file, or `inspection`). The table records only what exists; it does not imply behavior. `make traceability-check` audits it.

## 19. Installation and Consumption

Core installs as a CMake package (CORE-BUILD-002): `cmake --install` (or `make install PREFIX=<dir>`) installs the `kritva_core` library, the public headers under `include/kritva/core/`, and `kritva_coreConfig.cmake` / `kritva_coreConfigVersion.cmake` / `kritva_coreTargets.cmake` under `lib/cmake/kritva_core`. Consumers use `find_package(kritva_core CONFIG)` and link `kritva_core::kritva_core`, which also carries the C++20 requirement. The package has no third-party dependencies. Version compatibility is `SameMinorVersion` while Core is pre-1.0. The installed targets file does not reference the source or build tree. Tests, examples and scripts are not installed. `tests/install/` verifies the installed layout and builds a consumer against the install prefix only.

## 20. Runtime Component Contract

`runtime::Component` (`runtime/component.hpp`) is the platform-independent contract of a lifecycle-managed component (CORE-RT-001). It is constructed with a `ComponentInfo` (`runtime/component_info.hpp`): a valid `ComponentId` (`runtime/component_id.hpp`, an alias of `Id`, non-zero), a non-empty name that is a label and never an identity, and a `Version`. `ComponentInfo::create()` returns a failed `Result` with `INVALID_ARGUMENT` for an invalid id or empty name. `Component::info()` is non-virtual and `noexcept`; the identity is immutable for the component's lifetime and the returned reference stays valid until destruction. Components are neither copyable nor movable and are owned by the integrator: Core types hold non-owning references and never delete a component. The lifecycle operations reuse the Core lifecycle states and transition table (see ARCHITECTURE.md, "Runtime component contract" for the full operation table); an invalid operation fails with `INVALID_STATE` and has no effect, and a valid operation that fails moves the component to `FAULT`. Every `Error` returned by an operation of an instantiated component carries `source == info().id()` (errors from `ComponentInfo::create()` precede any component and are exempt). `status()`, `health()` and `capabilities()` return snapshots by value. Core makes no universal thread-safety guarantee (callers serialize operations; `info()` is safe to read concurrently), operations are control-plane, and there is no real-time claim. Conformance checks: `tests/contract/component_contract.hpp`; reference component: `tests/contract/reference_component.hpp`.

Note: the breaking change to the R0.1/R0.2 `Component` interface is that a component must now be constructed with a `ComponentInfo`.
