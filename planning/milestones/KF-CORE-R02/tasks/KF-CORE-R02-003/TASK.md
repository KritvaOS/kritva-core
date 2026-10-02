# KF-CORE-R02-003 — Statistics Contract Clarification

## 1. Task Information

- Task ID: `KF-CORE-R02-003`
- Milestone: `KF-CORE-R02 — Core Contract Hardening`
- Area: `include/kritva/core/statistics/`
- Dependency: `KF-CORE-R02-002`
- Implementer: Codex / Claude
- Reviewer: ChatGPT

## 2. Objective

Define the contract of Counter, Gauge, and Statistics clearly without turning Core into a telemetry framework.

## 3. Background

Kritva Core is intended to remain a platform-independent foundation. R0.2 is contract hardening, not a Runtime Manager implementation milestone. Existing architecture should be preserved unless a concrete contract defect requires a minimal correction.

## 4. Technical Requirements

- Preserve the current simple counter/gauge model.
- Do not automatically convert fields to atomics.
- Document that current statistics objects are not thread-safe unless externally synchronized.
- Document that they are not hard-real-time synchronization primitives.
- Counter increment/read behavior must be deterministic.
- Gauge set/read behavior must be deterministic.
- Statistics aggregate fields must have clear meaning and units where applicable.
- Do not add telemetry transport, Prometheus, OpenTelemetry, or serialization.

## 5. Scope

Inspect `counter.hpp`, `gauge.hpp`, `statistics.hpp`, existing statistics tests and REQUIREMENTS.md.

The implementation must remain limited to the contract and validation work described above.

## 6. Out of Scope

- Runtime Manager implementation
- Component registry/dependency graph implementation
- ROS2/DDS implementation
- EtherCAT implementation
- Linux/STM32/TI/Nexus/Edge platform implementation
- Vendor HAL/SDK integration
- AI, perception, motion planning, or robot skills
- Unrelated refactoring

## 7. Dependencies

Complete and accept the dependency task before treating this task as ready for final review.

## 8. Expected Files

Inspect:
Inspect `counter.hpp`, `gauge.hpp`, `statistics.hpp`, existing statistics tests and REQUIREMENTS.md.

Record the exact files changed in the implementation evidence.

## 9. Tests To Add or Update

- Counter initial value
- Counter increment and read
- Gauge set and read
- Statistics default state
- Statistics field updates
- Boundary values appropriate to existing types

## 10. Regression Tests

Run the established regression suite:

- `kritva_core_lifecycle`
- `kritva_core_contract`
- `kritva_core_status`
- `kritva_core_health`
- `kritva_core_error`
- `kritva_core_result`
- `kritva_core_capability`
- `kritva_core_configuration`
- `kritva_core_event`
- `kritva_core_statistics`
- `kritva_core_types_test`

## 11. Build

Preferred baseline commands:

```bash
rm -rf build
cmake -S . -B build
cmake --build build -j$(nproc)
```

If the repository's documented build command differs, use it and record the exact command.

## 12. Test

```bash
ctest --test-dir build --output-on-failure
```

Also run task-specific tests and record exact commands/results.

## 13. Quality Checks

Where configured:

```bash
git diff --check
```

Run repository-supported coverage, sanitizer, and static-analysis commands. Do not invent results.

## 14. Implementation Constraints

1. Preserve established public API unless this task explicitly requires a contract correction.
2. Prefer minimal, reviewable changes.
3. Do not introduce hidden platform dependencies.
4. Do not modify unrelated code.
5. Add deterministic tests for changed behavior.
6. Keep documentation synchronized with implementation.
7. Record any ambiguity or discovered defect rather than silently making architectural decisions.

## 15. Deliverables

The implementer must provide:

- implementation summary
- exact files changed
- new/modified tests
- build command and result
- task-specific test result
- full regression result
- quality-check results
- Git commit hash
- known limitations or follow-up items

## 16. Definition of Done

The task is ready for independent review only when all acceptance criteria in `ACCEPTANCE_CRITERIA.md` are demonstrably satisfied.
