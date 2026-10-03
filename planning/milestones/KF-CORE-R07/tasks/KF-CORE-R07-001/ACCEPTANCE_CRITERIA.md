# KF-CORE-R07-001 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R07-001 |
| Status | PLANNED |
| Primary commit | `feat(core): define component operational observation contract` |
| Reviewer | ChatGPT — independent acceptance gate |

## Objective

Define a deterministic, read-only Component operational observation boundary using existing Core concepts without introducing a new operational state machine.

## Scope

Clarify Component authority, snapshot/value semantics, side-effect rules, coherency limits, thread-safety expectations, allocation/real-time expectations, and Runtime independence. Do not add `statistics()` to the mandatory base Component as part of this task.

## Proposed Requirements Traceability

R0.7 requirement domain: `CORE-OPS-*`. The implementing task must update authoritative `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- Observation queries do not change Component lifecycle, Runtime state or operational data.
- Existing `status()` and `health()` value/snapshot semantics remain intact.
- No new OperationalState API is introduced.
- Cross-property atomic snapshot is not claimed.
- No Runtime polling or background monitoring is introduced.
- Any public API addition is minimal, self-contained and explicitly documented.
- Existing R0.2–R0.6 behavior remains regression-compatible.

## New Tests

- Add focused unit/contract tests for each accepted semantic rule introduced by this task.
- Add negative/failure tests where applicable.
- Add mutation testing for contract-sensitive behavior.

## Regression Tests

- Complete existing CTest suite remains green.
- Existing R0.2–R0.6 contracts remain unchanged unless explicitly approved.

## Build

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
```

## Test

```bash
ctest --test-dir build --output-on-failure
```

Where applicable:

```bash
make check
make traceability-check
```

## Quality

- No unexplained regression from the accepted baseline.
- Public headers self-contained.
- Strict warnings/static analysis clean for affected code.
- Required sanitizer/coverage evidence supplied at milestone validation.

## Expected Files Changed

Implementation and tests only for this task, plus directly required traceability/documentation updates. No unrelated files.

## Git Commit

```text
feat(core): define component operational observation contract
```

## Evidence Required from Implementor

- `git status`
- `git diff --check`
- build output
- test output
- focused test evidence
- mutation evidence where required
- diff/stat summary
- commit SHA
- explicit mapping from each acceptance criterion to objective evidence

## Implementor Evidence

Primary commit: `6849a73` `feat(core): define component operational observation contract` (API consult with amendments A1–A6 recorded in `R07_DESIGN_DECISIONS.md`). **Purely additive:** two new public headers and one include in `core.hpp`; `git diff HEAD~1 -- src` is empty; `component.hpp`, `ComponentContext`, `RuntimeManager` and every R0.2–R0.6 header are byte-identical.

- API: `runtime::ComponentObservation` (aggregate value: `id`, `lifecycle`, `status`, `health`, `std::optional<Statistics> statistics`) and free `observe(const Component&, const IComponentStatistics* = nullptr)`; `IComponentStatistics` (`runtime/component_statistics.hpp`, one pure `statistics() const`) is an optional interface that is **not** a base of `Component` (no `statistics()` added, no RTTI, no identity in the interface; its contract is completed by R07-004).
- Contract text (CORE-OPS-001, CORE-OPS-006): component authority and no Core mirror/cache; detached owning value; null provider gives nullopt, non-null gives an engaged copy; documented accessor order (`info().id()`, `lifecycle_state()`, `status()`, `health()`, `statistics()`), once each, nothing else; purity is a contract on conforming implementations (A4); no cross-property atomicity; determinism; allocation (string copies only), control plane, no real-time claim, no error path, exceptions propagate; no Runtime polling, event, export or logging. Status/Health independence text is stated here and formalized and tested in R07-002.
- Tests (`kritva_core_component_observation`, 11 functions): shape (copyable aggregate, optional interface not a base of Component, observe takes a const Component); default observation; reported values; **exact accessor order and count via a spy** (without and with a provider; `capabilities()` never called); statistics null/engaged/all-zero-engaged and field values; detachment (a later change to component or provider does not alter an earlier observation; a fresh observation sees the new truth); the observation outlives a destroyed component (200-char strings, ASan clean); no effect on the component (no lifecycle operation, state unchanged, provider unchanged); determinism; provider not tied to the component; observing a Runtime-registered component leaves Runtime state and statistics unchanged.
- Mutation evidence (10 mutants, each reverted, **10/10 detected**): accessor order swapped, lifecycle/id/status/health dropped, statistics engaged for a null provider, provider read twice, status read twice, `capabilities()` added, default statistics engaged.
- Regression: ctest 46/46 in Release, ASan+UBSan, strict `-Werror` (0 warnings), TSan (ASLR off); Debug 46/46; GCC `-fanalyzer` clean; coverage 604/611 (the new header's only uncovered line is the exception-unwind closing brace of `observe()`, the same class as `evaluate()`); `make check` passes, traceability 82 requirements, 81 traced, 0 errors (CORE-OPS-001, CORE-OPS-006 defined with rows; the two new public headers are in the table); `git diff --check` clean.
- Out of scope confirmed: no new state machine, no Runtime observation API, no event emission, no background execution, no platform code.

## Reviewer Decision

- [ ] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Reviewer decision is independent of implementor checkboxes.
