# KF-CORE-R06-006 — Acceptance Criteria

## 1. Task information

- Task: `KF-CORE-R06-006`
- Title: Runtime/Component Context Integration Tests
- Milestone: `KF-CORE-R06`
- Estimated effort: 3–4 ED
- Dependency: R06-005
- Requirement proposed: `CORE-CTX-006`
- Exact primary commit message: `test(core): add component context integration tests`

## 2. Objective


Prove Context + Component + Runtime integration through public APIs while preserving Runtime lifecycle ordering, failure propagation, reset semantics, statistics and R0.5 platform isolation.


## 3. Required acceptance criteria

### Contract correctness

- [ ] The implementation matches the approved R06 architecture.
- [ ] Ownership and lifetime semantics are explicit.
- [ ] No hidden lifecycle side effects are introduced.
- [ ] Existing R0.5 behavior remains intact.
- [ ] Error behavior is deterministic and documented.
- [ ] Invalid/unavailable paths are explicitly tested where applicable.

### API discipline

- [ ] No unrelated public API changes.
- [ ] No breaking change hidden inside the task.
- [ ] No generic registry/locator.
- [ ] No platform name/version inference.
- [ ] No concrete OS/vendor/platform dependency.

### Tests

- [ ] Focused unit/contract tests added or updated.
- [ ] Negative/error/invalid-state tests added where applicable.
- [ ] Mutation testing performed for contract-sensitive behavior.
- [ ] Existing regression suite remains green.
- [ ] Integration tests added when task changes an integration boundary.


## Mandatory quality checks

- `git diff --check`
- Debug build
- Release build
- full CTest regression
- strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`
- ASan + UBSan
- TSan with documented ASLR-disabled environment where required
- GCC `-fanalyzer`
- public-header self-containment
- traceability audit
- prohibited dependency/header audit
- install-consumer test
- coverage review
- clean working tree

Passing new tests alone is not acceptance evidence.

## Regression rule

The complete existing regression suite must remain green. R06 tests supplement; they do not replace R0.2–R0.5 coverage.

## Acceptance decision

Only the independent reviewer records:

- PASS
- CHANGES REQUIRED
- BLOCKED

Claude/Codex may provide objective evidence and mark evidence checkboxes but does not record the reviewer verdict.


## 4. Requirement traceability

- [ ] `CORE-CTX-006` is defined in authoritative `REQUIREMENTS.md` only after task acceptance.
- [ ] Artifact and verification references are recorded.
- [ ] `make traceability-check` passes or equivalent repository audit passes.
- [ ] Previous requirement IDs are not renumbered.

## 5. Build commands

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
```

## 6. Test commands

```bash
ctest --test-dir build --output-on-failure
make check
make traceability-check
```

Apply configured sanitizer, strict-warning and analyzer commands required by the repository.

## 7. Coverage

- [ ] Coverage baseline reviewed.
- [ ] No unexplained regression in coverage.
- [ ] Any uncovered new behavior is justified in evidence.

## 8. Static / sanitizer analysis

- [ ] ASan/UBSan pass.
- [ ] TSan pass where configured.
- [ ] Strict `-Werror` pass.
- [ ] GCC `-fanalyzer` pass.
- [ ] Public-header self-containment pass.
- [ ] Dependency/prohibited-header audit pass.

## 9. Expected files

Task-specific paths may include:

- `include/kritva/core/...`
- `src/...`
- `tests/unit/...`
- `tests/integration/...`
- `tests/contract/...`
- `tests/audit/...`
- `REQUIREMENTS.md`
- `API.md`
- `ARCHITECTURE.md`
- corresponding planning/evidence files

Only files relevant to the approved task may change.

## 10. Git commit

Primary implementation commit must be exactly:

```text
test(core): add component context integration tests
```

Do not amend an accepted commit. Use a focused follow-up commit if independent review identifies a correction before acceptance.

## 11. Evidence supplied by implementer

- [ ] Commit SHA
- [ ] Test output
- [ ] Regression output
- [ ] Quality checks
- [ ] Traceability result
- [ ] Coverage result
- [ ] Mutation results
- [ ] Diff summary
- [ ] Out-of-scope confirmation

## 11a. Implementation evidence (Claude)

Primary commit: `2fd5424` `test(core): add component context integration tests` (R06-005 accepted at `0c419ab`). **No production code changed** (`git diff 06207c7 HEAD -- include src` is empty since the Component API Review freeze); no production API added.

- Changed/new files: `tests/integration/component_context_integration_test.cpp` (new, 10 functions), `CMakeLists.txt` (registration), `REQUIREMENTS.md` (`CORE-CTX-006` and traceability row), `TESTING.md`. It reuses `tests/integration/runtime_scenarios.hpp`, the test-only reference platform (R05-005) and the test-only reference context component (R06-005). Public APIs only; no private detail; no hardware.
- **Central proof: equivalence through a context.** A three-component chain whose middle component is a `ReferenceContextComponent` (plans: create a task; start scheduler, timer, watchdog, kick, read the clock; stop in reverse) runs next to the same chain with a plain `ReferenceComponent` failing in the same place. For **every one of the 8 service methods x 3 attempts x 3 error codes (72 cases)**, over three lifecycle cycles with `reset()` and with the integrator healing its own services between cycles, the two Runtimes show identical states, fault, error code and source, statistics, topology flag, reset cleanup and the **complete invocation trace of all three components** (the context component is traced too), and every injected platform failure really reached the Runtime as a fault.
- Tests (`kritva_core_component_context_integration`): (1) the 72-case equivalence; (2) **availability**: all 16 combinations give the outcome derived independently from what the component needs (no scheduler fails `initialize()`; no timer, then watchdog, then clock fails `start()`), always `UNSUPPORTED` with the component as source, exactly the R0.5 message, severity and timestamp, FAULT, explicit `reset()`, no retry; (3) **requirement gating through a context**: 16 combinations x capability present/absent x 4 requirement sets against an independent oracle: checking touches no service, a satisfied set runs a full lifecycle, an unsatisfied one fails `initialize()` with the component as source and exactly R0.5's wording; (4) **seeded differential model**: 40 scripts x 40 operations replayed with plain components and with components that use their context in every operation (support, capability and requirement-evaluation queries), for each of the 16 service combinations with **every platform method failing**: identical results, states, faults, statistics, retries and invocation traces, and the platform call log stays empty; (5) **fault, reset and recovery**: a platform timer failure becomes a component failure with the platform's own code and message and every other field of the service's error unchanged, FAULT accepts only `reset()`, exactly one error and one sample are counted, the component's partial platform state is still the integrator's, and after the platform recovers a new explicit attempt works with no Runtime retry; (6) a **failed stop is recorded as faulted**: after the failure `reset()` does not stop that component again but shuts it down, and fail-fast left the earlier component for the cleanup pass; (7) **statistics** count component lifecycle calls (configure, initialize, start, stop, shutdown) and never context queries (15 queries, none counted); (8) **source attribution**: in a four-component chain each component in turn needs a missing service and the Runtime error and fault always name exactly that component, with fail-fast (later components never started, earlier ones started once), and two components sharing a platform attribute to their own ids with their contexts not leaking into each other; (9) **watchdog expiry, callbacks and task entries never enter the Runtime** (still RUNNING, no fault, `reset()` still FAULT-only); (10) **ownership**: the integrator owns the platform; the Runtime, the component and its context never destroy it or a service (lifetime probe), and the context's identity is valid for as long as the component lives.
- Mutation evidence: the 79 production mutants of the context access, injection, requirement-binding and Runtime failure-propagation work were replayed against this test alone. 44 are detected (including every Runtime-coupling and failure-propagation mutant but one, which exposed a real gap); the survivors that concern **integration** were closed by added assertions (exact R0.5 message, severity and timestamp for availability and requirement errors, every field of a service error, and the stop-failure reset accounting), after which the `require`/`check_required` message and severity mutants and the failed-stop mutant are detected. The remaining survivors are **unit-level** mutants that an integration test cannot and should not see, each already killed by its unit test: structural mutants (closed surface, added members, extra state, implicit constructors, temporary component, requirement-declaring members, `RuntimeManager` context members), field-precision mutants of `attribute()` that need an error with a non-default severity, timestamp or pre-existing source (the integration platform errors carry the default ones), `evaluate()` content mutants (the integration decisions use `check_required`), a platform-name capability match and a second accessor call (unit-tested with name lengths and a spy), plus one equivalent mutant (unreachable code).
- Regression: `ctest` 45/45 in Debug, Release, ASan+UBSan, strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`, TSan (ASLR off); the whole existing suite, including the R0.3 runtime group, the R0.4/R0.5 platform and runtime-platform tests and every R0.6 test, is unchanged and green; build 0 warnings; GCC `-fanalyzer` clean; coverage 99% (595/601, unchanged); `make check` passes with traceability 79 requirements, 78 traced, 0 errors; `git diff --check` clean; dependency scan clean.
- Out of scope confirmed: no private access, no new production API, no hardware, no concrete platform.

## 12. Reviewer decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS** |
| Accepted commit | `2fd5424` (evidence `82038a9`) |
| Evidence reference | section 11a above |
| Date | 05-10-2026 |

Reviewer notes: no production code or API change after the Component API freeze; public APIs only; the 72-case equivalence of a platform failure through a context with an ordinary component failure (full invocation traces included), the availability and requirement-gating matrices, the 40x40 differential with every platform method failing, fault/reset/recovery, the failed-stop accounting, statistics, source attribution in every chain position, watchdog/callback independence and ownership are accepted; the integration-relevant mutation survivors were closed and the rest are unit-level mutants already killed by their own unit tests. `CORE-CTX-006` is authoritative. Reviewer relied on the supplied evidence; the commits were local-only.

**Reviewer Decision: PASS — KF-CORE-R06-006 is ACCEPTED.**
