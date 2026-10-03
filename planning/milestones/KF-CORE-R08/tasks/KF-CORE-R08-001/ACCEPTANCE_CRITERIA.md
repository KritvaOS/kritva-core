# KF-CORE-R08-001 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R08-001 |
| Status | PLANNED |
| Primary commit | `feat(core): define component configuration contract` |
| Reviewer | ChatGPT — independent acceptance gate |
| Estimated effort | 2–3 ED |
| Dependency | R08 Scope Confirmation |

## Objective

Define the normative semantics of `Component::configure()` without introducing a new lifecycle state or dynamic configuration path.

## Proposed Requirements Traceability

CORE-CFG-004, CORE-CFG-011

The implementing task must update authoritative root `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- `configure()` is explicitly valid only from `UNKNOWN` and `STOPPED` for R0.8.
- A configuration call from `READY`, `RUNNING`, `STOPPING` or `FAULT` returns `INVALID_STATE` and leaves the Component unchanged.
- Successful `configure()` leaves lifecycle state unchanged.
- Configuration is synchronous and control-plane; no Core real-time guarantee is implied.
- No `CONFIGURED`, `RECONFIGURING` or equivalent lifecycle state is introduced.
- No `reconfigure()`/`set_parameter()` production API is introduced.
- Existing Component lifecycle semantics remain compatible with R0.3/R0.7.
- Contract wording is explicit in the authoritative public/API documentation.
- Mutation testing detects removal or weakening of invalid-state/state-preservation checks.

## New Tests

- Unit/contract coverage for all lifecycle states against `configure()`.
- Negative tests verify no lifecycle transition on success or invalid call.
- Mutation tests for legal-state boundaries and state preservation.

## Regression Tests

- Full pre-R08 CTest suite remains green.
- Existing Runtime lifecycle/failure/reset tests remain green.

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
feat(core): define component configuration contract
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

Primary commit: `605516b` `feat(core): define component configuration contract` (API consult approved with no amendments: contract text only in the configuration headers; `component.hpp`, `runtime_manager.hpp`, `component_context.hpp` byte-identical). **No production type, signature or behavior change:** the production diff is one contract-text block and tags in `configuration/configuration.hpp`; `src/` is byte-identical to `kritva-core-r0.7`.

- Contract (CORE-CFG-004, CORE-CFG-011; API.md section 44; ARCHITECTURE.md): configuration is a caller-owned, detached control-plane value; `configure()` is valid only from UNKNOWN and STOPPED; from READY, RUNNING and FAULT (and the unobservable transient/never-produced states) it fails with `INVALID_STATE` (source the component) with no other effect; success and a rejected configure in a valid state both leave the lifecycle state unchanged; the same rule for `RuntimeManager::configure()`; no CONFIGURED/RECONFIGURING state and no reconfigure/set_parameter/get_parameter/apply/update/`configuration()` accessor; reconfiguration only by stopping and configuring again from STOPPED; synchronous on the caller's thread, may allocate/block, no Core thread/callback/background activity, no real-time claim; independent of Status/Health/Runtime FAULT.
- Tests (`kritva_core_component_configuration`, 8 functions): signature and absence of the dynamic API by member-detection on Component, RuntimeManager **and** ComponentContext (six forbidden members), and the lifecycle keeps exactly its eight states; valid from UNKNOWN and STOPPED with the state unchanged (and initialize works afterwards); invalid in READY, RUNNING and FAULT with INVALID_STATE, source = component, state unchanged, nothing applied and the one-shot injected failure **not consumed** (proving nothing past the state check ran), shutdown() then configure() as the way back; the state is unchanged after configure in every reachable state (UNKNOWN, READY, RUNNING, STOPPED, FAULT); a rejected valid-state configuration leaves the state unchanged and no retry happens; configure runs on the caller's thread for a component and through the Runtime; Runtime eligibility in every Runtime state (UNKNOWN/STOPPED valid, READY/RUNNING/FAULT invalid with no component invoked and no state or statistics change, reset() then configure() works); a failed Runtime configure leaves the Runtime state unchanged with no FAULT.
- Mutation evidence (11 mutants, each reverted, **11/11 detected**): Runtime configure also allowed in READY, only in UNKNOWN, only refusing RUNNING, never refusing; a configure failure entering FAULT (survived the first run, closed by the new failed-Runtime-configure test); a successful configure moving the Runtime to INITIALIZING; and in the test-double component: READY allowed, only RUNNING refused, success advancing STOPPED to READY, failure entering FAULT.
- Regression: ctest 52/52 in Release, ASan+UBSan, strict `-Werror` (0 warnings), TSan, Debug; GCC `-fanalyzer` clean; coverage 618/625 unchanged; `make check`, traceability 92 requirements, 91 traced, 0 errors (`CORE-CFG-004`, `CORE-CFG-011` defined with rows); `git diff --check` clean.
- Out of scope confirmed: no ownership/atomicity (R08-002), no validation or version semantics (R08-003), no new API.

## Reviewer Decision

`PASS / CHANGES REQUIRED / BLOCKED` — to be completed by ChatGPT only.
