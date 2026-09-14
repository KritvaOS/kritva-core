# Kritva Core API

## 1. Purpose

Public API organization for Kritva Core R0.1. The API is designed before implementation.

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
