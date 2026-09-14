# Kritva Core

## Open Robotic Computing Platform — Core Foundation

`kritva-core` is the platform-independent foundation of the Kritva Open Robotic Computing Platform.

Kritva connects Physical AI to physical action. Core provides the small, stable contracts and primitives required by higher-level Kritva components and multiple compute implementations.

## Scope

Core R0.1 defines foundational concepts:

- Identity
- Lifecycle
- Status
- Health
- Statistics
- Error / Fault
- Result
- Events
- Capability
- Configuration
- Version
- Timestamp
- Duration
- Metadata

Core is intentionally platform-independent.

## Non-Goals

Core R0.1 does not implement ROS2/DDS, EtherCAT, Linux/RTOS integration, hardware drivers, motor or sensor drivers, AI inference, motion planning, cloud services, databases, or robot-specific application logic.

## Repository Structure

```text
kritva-core/
├── include/kritva/core/   # Public C++ API
├── src/                   # Implementation
├── tests/{unit,contract}/
├── examples/
├── docs/
├── scripts/
├── CMakeLists.txt
├── CMakePresets.json
├── README.md
├── ARCHITECTURE.md
├── REQUIREMENTS.md
├── API.md
├── TESTING.md
├── AGENTS.md
├── VERSION
├── CHANGELOG.md
├── LICENSE
├── .gitignore
├── .clang-format
└── .editorconfig
```

## Public Include Convention

```cpp
#include <kritva/core/core.hpp>
#include <kritva/core/types/id.hpp>
```

Namespace:

```cpp
kritva::core
```

## Engineering Model

```text
Requirement → Architecture → API → Implementation → Unit Test
→ Contract Test → Verification
```

## Build

Kritva Core targets C++20 and uses CMake.

```bash
cmake --preset debug
cmake --build --preset debug
ctest --test-dir build/debug --output-on-failure
```

See `AGENTS.md` for repository engineering rules.

## Status

Pre-alpha; R0.1 APIs and implementation are not yet stable.

## License

Apache License 2.0. See `LICENSE`.
