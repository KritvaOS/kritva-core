# Kritva Core — Claude Handoff

## 1. Project

Repository:

`KritvaOS/kritva-core`

Purpose:

Kritva Core is the hardware-independent foundation/runtime contract for KritvaOS. It must remain independent of ROS2, DDS, EtherCAT, vendor SDKs, Linux-specific implementation, STM32 HAL, TI SDKs, and robot-domain algorithms.

Core should provide reusable contracts and foundational services for:

* Kritva Nexus
* Kritva Edge
* Kritva Sense
* Kritva Mind
* Kritva Motion
* Kritva Skill
* Kritva SDK

---

## 2. Current repository status

The repository has progressed beyond an initial skeleton.

Current major structure:

```text
include/kritva/core/
├── types/
├── lifecycle/
├── status/
├── health/
├── statistics/
├── error/
├── event/
├── capability/
├── configuration/
├── runtime/
├── messaging/
├── time/
├── platform/
└── core.hpp

src/
├── version.cpp
├── metadata.cpp
├── lifecycle.cpp
├── capability_set.cpp
└── configuration.cpp

tests/
├── unit/
└── ...
```

Current public API already covers:

* Types
* ID
* Version
* Timestamp
* Duration
* Metadata
* Lifecycle
* Status
* Health
* Statistics
* Error
* Result
* Event
* Capability
* Configuration
* Runtime contract
* Messaging contract
* Time contract
* Platform contracts

The repository also contains:

* `REQUIREMENTS.md`
* `API.md`
* `ARCHITECTURE.md`
* `TESTING.md`
* `CHANGELOG.md`
* `README.md`

The current architecture should NOT be redesigned unless a concrete contract inconsistency is found.

---

## 3. Current implementation maturity

The current stage is:

**Core Foundation / API Contract implementation**

It is NOT yet the full Kritva Runtime.

Implemented source files currently include:

```text
src/version.cpp
src/metadata.cpp
src/lifecycle.cpp
src/capability_set.cpp
src/configuration.cpp
```

Lifecycle implementation is already functional, including valid/invalid transition handling.

Configuration and capability-set implementations are started.

The repository has a broad unit/contract test structure.

---

## 4. Current architectural principle

Keep this rule:

> Core defines WHAT Kritva provides. Platform implementations define HOW it is implemented.

Core must remain hardware/platform independent.

Do not add:

```text
ROS2
DDS
EtherCAT
PTP implementation
Linux-specific code
STM32 HAL
TI SDK
vendor drivers
camera algorithms
SLAM
motion planning
AI
robot skills
```

to `kritva-core`.

---

# 5. Current assessment

Approximate maturity:

```text
Requirements          Complete
Architecture          Complete
API definition        ~90%
Foundation            ~75%
Lifecycle             ~90%
Configuration         ~70%
Capability            ~80%
Error/Result          ~80%
Health                ~70%
Events                ~70%
Statistics            ~70%
Time                  ~50%
Runtime               ~20%
Messaging             ~20%
Platform              ~20%
```

These are engineering estimates, not repository metrics.

The key point:

> The API structure is now sufficiently mature. Stop expanding the API unnecessarily and concentrate on contract correctness, tests, and implementation.

---

# 6. Important findings from the latest review

## A. `Result<T>` needs contract hardening

Review:

```text
include/kritva/core/error/result.hpp
```

Current `value()` / `error()` behavior needs to be explicitly defined for invalid access.

Determine whether the intended contract is:

```cpp
result.has_value()
result.value()
result.error()
```

with preconditions:

```text
value()  → success result only
error()  → failure result only
```

Do not redesign Result unnecessarily.

Add appropriate documentation and tests.

---

## B. `status/status.hpp`

Review:

```text
include/kritva/core/status/status.hpp
```

Ensure required standard headers are explicitly included rather than relying on transitive includes.

In particular check use of:

```cpp
std::move
```

and include `<utility>` if required.

---

## C. Statistics threading semantics

Review:

```text
include/kritva/core/statistics/counter.hpp
include/kritva/core/statistics/gauge.hpp
```

Current implementation is non-atomic.

Do NOT automatically convert them to atomics.

Instead document the intended R0.2 contract:

* not thread-safe unless externally synchronized
* not intended as hard-real-time synchronization primitives
* allocation behavior where applicable

Atomic/lock-free statistics can be considered later.

---

## D. Scheduler contract

Review:

```text
include/kritva/core/platform/scheduler.hpp
```

Current scheduler contract uses a low-level function pointer:

```cpp
void (*entry)(void*)
```

Do not immediately redesign it.

First clarify:

* task lifetime
* priority semantics
* CPU affinity semantics
* `cpu_affinity = 0` meaning
* start/stop behavior
* failure behavior
* ownership of task resources

Keep implementation platform-independent.

---

## E. Duplicate clock header

Review:

```text
include/kritva/core/time/clock.hpp
include/kritva/core/platform/clock.hpp
```

`time/clock.hpp` should remain the canonical Core clock abstraction.

`platform/clock.hpp` currently aliases it.

Decide whether the platform alias should remain temporarily for compatibility or be deprecated.

Do not create two separate clock abstractions.

---

## F. Requirements traceability gap

Some headers/API documentation reference detailed requirement IDs such as:

```text
CORE-TYP-001
CORE-TYP-002
CORE-STA-001
CORE-HEA-001
CORE-ERR-001
CORE-EVT-001
CORE-CAP-001
CORE-CFG-001
```

but the current `REQUIREMENTS.md` does not fully define all of these IDs.

This should be cleaned up.

Preferred direction:

```text
Requirement
    ↓
Public header
    ↓
Implementation
    ↓
Test
```

Do not leave undocumented requirement IDs in the API.

---

# 7. NEXT IMMEDIATE ACTION

Do NOT implement Runtime Manager yet.

First execute:

## "R0.2 Contract Hardening"

Perform these tasks in this exact order:

### Task 1

Review and fix:

```text
include/kritva/core/error/result.hpp
```

Define and test valid/invalid access semantics.

### Task 2

Review/fix:

```text
include/kritva/core/status/status.hpp
```

Remove dependency on transitive includes.

### Task 3

Review/document:

```text
include/kritva/core/statistics/counter.hpp
include/kritva/core/statistics/gauge.hpp
```

Clearly define thread-safety and allocation behavior.

### Task 4

Review:

```text
include/kritva/core/platform/scheduler.hpp
```

Document scheduler contract without introducing platform-specific implementation.

### Task 5

Review:

```text
include/kritva/core/time/clock.hpp
include/kritva/core/platform/clock.hpp
```

Ensure one canonical clock abstraction.

### Task 6

Review `REQUIREMENTS.md` and `API.md`.

Create complete traceability for every public Core contract.

### Task 7

Run the complete test suite.

At minimum:

```bash
cmake ...
cmake --build ...
ctest --output-on-failure
```

Use the repository's existing build/preset mechanism rather than inventing a new build process.

### Task 8

Report:

```text
Files changed
Tests added/changed
Tests passed
Coverage
API changes
Remaining issues
```

---

# 8. Do NOT do these things in this task

Do not:

```text
❌ Add Runtime Manager
❌ Add Component Registry
❌ Add dependency graph
❌ Add Linux implementation
❌ Add PREEMPT_RT implementation
❌ Add EtherCAT
❌ Add ROS2
❌ Add DDS
❌ Add zero-copy messaging
❌ Add lock-free queues
❌ Add platform-specific code
❌ Expand the API without a requirement
```

Those belong to subsequent milestones.

---

# 9. Expected outcome

The desired result of this task is:

```text
Kritva Core R0.2
        │
        ├── Public API contract reviewed
        ├── Result semantics defined
        ├── Status contract cleaned
        ├── Statistics semantics defined
        ├── Scheduler contract clarified
        ├── Clock abstraction clarified
        ├── Requirements traceability complete
        ├── Unit/contract tests passing
        └── API ready for freeze
```

After this milestone, the next development milestone will be:

# R0.3 — Runtime Foundation

Expected future work:

```text
runtime/component_registry.hpp
runtime/component_registry.cpp

runtime/runtime_manager.hpp
runtime/runtime_manager.cpp

Component registration
Component lifecycle orchestration
Runtime state management
Basic dependency handling
Runtime integration tests
```

But do NOT start R0.3 until R0.2 contract hardening is complete.

---

## 10. Guiding principle

When making any implementation decision, prefer:

> Small, deterministic, platform-independent Core contracts over feature-rich abstractions.

The Core should remain small enough to run on an MCU-class target while also providing the foundation for Linux/PREEMPT_RT-based Kritva Nexus.

