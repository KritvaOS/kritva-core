# KF-CORE-R04 — Platform Abstraction

## Status

PLANNED

## Objective

Establish a stable, platform-independent abstraction boundary for Kritva Core platform services and future platform adapters.

R0.4 defines contracts for scheduler, clock, timer, watchdog and platform capability discovery, plus the boundary by which future platform services may integrate with RuntimeManager.

## Architectural Principle

`kritva-core` defines contracts; platform-specific repositories implement them.

```text
kritva-core
    │
    └── platform contracts
             │
             ├── Linux / PREEMPT_RT adapter
             ├── MCU / RTOS adapter
             ├── Kritva Nexus adapter
             └── Kritva Edge adapter
```

No Linux, RTOS, vendor BSP/HAL, EtherCAT, ROS2/DDS or hardware-driver implementation belongs in R0.4 Core production sources.

## Task Sequence

001 → 002 / 003 / 004 → Platform API Review → 005 → 006 → Platform Integration Freeze → 007 → 008 → Release Gate

R04-002, R04-003 and R04-004 depend on R04-001 but are contract work that can be developed independently after the boundary is accepted. The Platform API Review requires all four to be accepted.

## Gates

| Gate | Entry | Purpose |
|---|---|---|
| R04 Platform API Review | R04-004 accepted | Freeze adapter/scheduler/clock/timer/watchdog contracts |
| R04 Platform Integration Freeze | R04-006 accepted | Freeze platform contracts before Runtime integration |
| R04 Release Gate | R04-008 accepted | Final acceptance and release authorization |

## Exit Criteria

- all eight tasks accepted;
- Platform API Review PASS/FROZEN;
- Platform Integration Freeze PASS/HONORED;
- Debug and Release builds pass;
- full regression passes;
- ASan/UBSan pass;
- TSan passes where configured;
- `-Werror` passes;
- GCC `-fanalyzer` passes;
- coverage reviewed;
- traceability reports zero errors;
- install-consumer remains green;
- prohibited dependency scan passes;
- no platform implementation is present in Core;
- Runtime/platform boundary is documented;
- final reviewer sign-off recorded;
- release tag created only after the Release Gate.

## Out of Scope

- Linux implementation
- PREEMPT_RT implementation
- RTOS implementation
- STM32/TI/NXP vendor BSP/HAL
- EtherCAT
- ROS2/DDS
- sensor drivers
- motor drivers
- AI/CV/SLAM
- motion planning
- robot skills
- background runtime execution
- automatic watchdog-driven recovery
