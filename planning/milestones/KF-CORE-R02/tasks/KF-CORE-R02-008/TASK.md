# KF-CORE-R02-008 — Full R0.2 Validation

## 1. Task Information

- Task ID: `KF-CORE-R02-008`
- Milestone: `KF-CORE-R02 — Core Contract Hardening`
- Area: Repository-wide validation
- Dependency: `KF-CORE-R02-007`
- Implementer: Codex / Claude
- Reviewer: ChatGPT

## 2. Objective

Execute the final objective acceptance gate for KF-CORE-R02. This task is validation, not a feature-development task.

## 3. Background

Kritva Core is intended to remain a platform-independent foundation. R0.2 is contract hardening, not a Runtime Manager implementation milestone. Existing architecture should be preserved unless a concrete contract defect requires a minimal correction.

## 4. Technical Requirements

- Start from a clean working tree.
- Perform a clean configure/build.
- Run all registered CTest tests.
- Run the established Core regression suite.
- Review test count and failures.
- Review coverage and explain material gaps.
- Run configured sanitizers/static analysis where available.
- Review public API for accidental platform dependencies.
- Review REQUIREMENTS.md traceability.
- Review documentation and changelog.
- Review Git history and task commits.
- Record exact evidence in the task result.
- Do not introduce unrelated feature work during validation.

## 5. Scope

Repository-wide; do not modify source unless validation exposes a documented defect that must be fixed through a separate focused task/commit.

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
Repository-wide; do not modify source unless validation exposes a documented defect that must be fixed through a separate focused task/commit.

Record the exact files changed in the implementation evidence.

## 9. Tests To Add or Update

- All CTest tests
- All 11 known regression tests
- Coverage
- Sanitizer/static analysis where configured
- Clean rebuild from a fresh build directory

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
