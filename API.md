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
