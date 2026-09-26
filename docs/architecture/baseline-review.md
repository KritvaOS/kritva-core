# Kritva Core R0.2 Baseline Review

## Decision

The current folder/file structure is aligned with the intended Kritva Core boundary.
The R0.2 skeleton API is suitable as a baseline after the contract corrections in this package.

## Keep

- types
- lifecycle
- status
- health
- statistics
- error/result
- event
- capability
- configuration
- runtime
- messaging
- time
- platform contracts

## Corrected in this baseline

1. Core time owns `time::IClock`; platform code adapts to it.
2. `Timestamp` explicitly records its clock domain.
3. Configuration exposes `validate()`.
4. Public API documentation includes the R0.2 domains.
5. Testing documentation is aligned with R0.2.
6. Public-header include hygiene is improved.
7. The file manifest contains initial file-level traceability entries.

## Intentionally not included

- ROS2/DDS
- EtherCAT
- Linux/PREEMPT_RT implementation
- RTOS implementation
- vendor BSP/SDK
- motor/sensor drivers
- AI/motion algorithms
- transport implementation
- PTP implementation

These belong in platform, hardware, or higher Kritva layers.

## Freeze recommendation

Freeze the folder structure and skeleton API at R0.2 after human review.
Allow interns to implement within the declared work packages without changing
public architecture unless the change is explicitly reviewed.
