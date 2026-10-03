# R07 Scope Confirmation

## Decision

APPROVED — IMPLEMENTATION SCOPE CONFIRMED.

## In Scope

- Component operational observation semantics
- Existing Status and Health reporting ownership/snapshot semantics
- Explicit operational Event reporting to integrator-owned sinks
- Optional Component-owned Statistics contract
- Reference operational test harness
- Runtime/Component operational integration preserving R0.3 lifecycle semantics
- Full R0.7 validation and release gate

## Out of Scope

- New OperationalState state machine
- Runtime lifecycle redesign
- Generic Diagnostic container
- Core EventBus/queue/broker/dispatcher
- Telemetry transport/export/persistence
- Logging backend
- Core worker/thread/polling loop
- Automatic recovery/restart/retry
- Health-driven Runtime control
- Platform-specific operational implementations
- ROS2/DDS/EtherCAT dependencies
- Broad ComponentContext expansion

## Operational Vocabulary

- Lifecycle: Runtime-controlled execution state.
- Status: current coarse operational condition reported by the Component.
- Health: Component's health assessment.
- Statistics: quantitative operational measurements owned by the Component when used.
- Event: discrete occurrence explicitly reported by the Component.
- Error: failed Result / operation failure.
- Runtime FAULT: Runtime lifecycle state caused by failed initialize/start/stop.

## Boundary Rule

Operational information flows outward to observers/integrators. R0.7 does not establish an internal Core control loop around that information.
