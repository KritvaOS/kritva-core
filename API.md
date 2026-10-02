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

Lifecycle transitions are validated by the Core lifecycle implementation. The allowed transition table is documented in `ARCHITECTURE.md` and must be covered by unit/contract tests.

## 12. Configuration Contract

`Configuration::validate()` provides the Core validation entry point. R0.2 validates structural correctness; richer constraints such as ranges, enumerations, and required/default semantics remain a later extension.

## 13. Result Contract

`Result<T>` and `Result<void>` hold exactly one outcome: success (`has_value()`) or failure (an `Error`). `value()` requires success, `error()` requires failure, and `failure()` requires `error.code != ErrorCode::NONE`. Violations are programming errors: `assert` in debug builds, undefined behavior in release builds. Accessors never throw. Results are copyable/movable when `T` is; a moved-from Result keeps its outcome but its payload is unspecified. There is no default constructor. Results are not thread-safe. See `include/kritva/core/error/result.hpp` (CORE-ERR-004).

## 14. Status Contract

`StatusCode` (`status/status_code.hpp`) is the authoritative status enumeration; `UNKNOWN` (zero, default) means undetermined and `OK` is the only success code. `Status` (`status/status.hpp`) pairs a `StatusCode` with an optional message. The code and message are independent and mutated in place; an empty message is valid for any code. `Status()` is `UNKNOWN`/empty; `Status(code)` is explicit. Only `set_message()` and copies allocate. Not thread-safe for concurrent mutation. Both headers are self-contained. See CORE-STA-001.

## 15. Statistics Contract

`Counter` is a monotonic `uint64_t` that wraps modulo 2^64 and is cleared only by `reset()`. `Gauge` is an `int64_t` that stores the last value set, unclamped. `Statistics` aggregates four counters (`sample_count`, `error_count`, `retry_count`, `drop_count`) and two gauges (`queue_depth`, `utilization` as whole percent by convention, not enforced). None of these types is thread-safe, atomic, or a synchronization primitive, and none allocates or blocks; concurrent access needs external synchronization, and a copied `Statistics` is not an atomic snapshot. They are not hard-real-time guarantees. No telemetry transport or serialization exists in Core. See CORE-STS-001..003.
