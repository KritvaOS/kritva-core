# KF-CORE-R08-002 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R08-002 |
| Status | ACCEPTED |
| Primary commit | `feat(core): define configuration ownership and atomic application` |
| Reviewer | ChatGPT — independent acceptance gate |
| Estimated effort | 2–3 ED |
| Dependency | R08-001 |

## Objective

Define detached input ownership and atomic/non-partial configuration application semantics.

## Proposed Requirements Traceability

CORE-CFG-005, CORE-CFG-006

The implementing task must update authoritative root `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- The caller-owned `Configuration` remains valid independently of the Component after `configure()` returns.
- A conforming Component does not retain the caller object's address/reference for later use.
- A failed configuration does not partially replace the previously accepted configuration state.
- Lifecycle state remains unchanged on configuration failure.
- The contract does not require a generic `Component::configuration()` accessor.
- A reference/broken-component test can detect retained-reference and partial-application mutations.
- No Runtime/global configuration store is added.
- Mutation testing detects partial-commit and reference-retention defects.

## New Tests

- Reference component that records/commits configuration only after successful validation.
- Broken variant retaining a pointer/reference to input must be detected.
- Broken variant partially mutating state before returning failure must be detected.
- Reuse/mutation of caller configuration after `configure()` verifies detachment.

## Regression Tests

- Existing Configuration value/copy behavior tests remain green.
- Existing Component and Runtime regression tests remain green.

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

## Coverage / Quality

- Newly added executable lines must be covered.
- No unexplained coverage exclusion.
- Overall R08 coverage remains at least 98% and has no unexplained regression greater than 1 percentage point from the R0.7 98.9% baseline.
- Required sanitizer and strict-analysis evidence is supplied at milestone validation.

## Expected Files Changed

Only files directly required for this task, its tests, and traceability/documentation reconciliation. No unrelated files.

## Git Commit

```text
feat(core): define configuration ownership and atomic application
```

## Evidence Required from Implementor

- `git status` before and after implementation
- `git diff --check`
- build and test output
- focused test evidence
- mutation evidence for contract-sensitive behavior
- diff/stat summary
- commit SHA
- explicit mapping from every acceptance criterion to objective evidence

## Implementor Evidence

Primary commit: `6efeaac` `feat(core): define configuration ownership and atomic application`. **No production type, signature or behavior change:** the production diff is a second contract-text block (and tags) in `configuration/configuration.hpp`; `src/` and the Component/Runtime/ComponentContext headers are byte-identical to `kritva-core-r0.7`.

- Contract (CORE-CFG-005, CORE-CFG-006; API.md section 45): the caller owns the Configuration (const reference, valid only for the call); a conforming Component copies what it keeps and retains no address, reference, pointer into (including `get()`'s) or view of it; a Configuration is a copyable value whose copies are independent and `get()`'s pointer is valid only until the next mutation or destruction; Core never copies, stores or caches a component's configuration (the Runtime forwards the caller's own object, whether or not the topology is fixed, and keeps nothing) and has no store, registry, second copy or generic `configuration()` accessor. Application is all-or-nothing: on any failure nothing is applied, the previous accepted configuration (or the initial state) stays exactly as it was, no parameter of the rejected input is visible and the lifecycle state is unchanged; Core states the rule but cannot enforce it on a component; no cross-component transactions or Runtime rollback.
- Tests (`kritva_core_configuration_ownership`, 5 functions): a reusable `check_ownership_and_atomicity` over a test component with an applied state (rate, name) and semantic rules, in four scenarios — the caller **mutates then destroys** its heap-allocated Configuration after configure(); a semantically invalid and an out-of-range configuration after a good one leave the previous rate **and** name untouched with CONFIGURATION_ERROR and an unchanged lifecycle state; a **first-ever** failure leaves the initial state, not a half-populated one; a valid configuration after a failure applies completely. The conforming component passes and **six deliberately broken components are each detected**: retains the caller's Configuration pointer, partial commit (rate before validation), commit-before-validate, keeps the pointer returned by get(), half state after a first failure, failure moving the lifecycle state. Configuration is a detached copyable value (copy, assignment and move independent in both directions); configure() takes const reference and accepts a temporary; the Runtime forwards the caller's own object (same address to every component, before and after the topology is fixed via reconfiguration from STOPPED), keeps nothing and works after the caller destroyed it.
- Mutation evidence (3 production mutants, each reverted): the Runtime copying the Configuration before forwarding when the topology is not fixed (killed), per component (killed), and when it is fixed (**survived the first run**, closed by reconfiguring from STOPPED in the Runtime test; now killed); the six broken test components above are the contract-level mutants of the conforming component (6/6 detected).
- Regression: ctest 53/53 in Release, ASan+UBSan, strict `-Werror` (0 warnings), TSan, Debug; GCC `-fanalyzer` clean; coverage 618/625 unchanged; `make check`, traceability 94 requirements, 93 traced, 0 errors; `git diff --check` clean.
- Out of scope confirmed: no validation or schema-version semantics (R08-003), no reusable harness (R08-004: this task's check is test-local, as the criteria require only that broken components be detectable), no new API.

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS** |
| Accepted commit | `6efeaac` (evidence `6aab91b`) |
| Evidence reference | Implementor Evidence above |
| Date | 05-10-2026 |

Reviewer notes: no production type, signature or behavior change (second contract-text block in `configuration.hpp`); caller ownership, non-retention, copy independence, `get()` pointer lifetime, no Core store or `configuration()` accessor, all-or-nothing application including a first-ever failure, no cross-component rollback, and fixed-topology forwarding accepted; 6/6 broken components and 3/3 production mutants detected after the targeted closure. The reviewer noted its connector could not resolve the abbreviated local SHA, so the verdict relies on the supplied evidence. `CORE-CFG-005` and `CORE-CFG-006` are authoritative.

**Reviewer Decision: PASS — KF-CORE-R08-002 is ACCEPTED.**
