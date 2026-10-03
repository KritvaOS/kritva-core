# KF-CORE-R06-003 — Acceptance Criteria

## 1. Task information

- Task: `KF-CORE-R06-003`
- Title: Context Injection Without Runtime Lifecycle Change
- Milestone: `KF-CORE-R06`
- Estimated effort: 3–4 ED
- Dependency: R06-002
- Requirement proposed: `CORE-CTX-003`
- Exact primary commit message: `feat(core): define component context injection boundary`

## 2. Objective


Define how a Component obtains its execution context without giving RuntimeManager new automatic platform-service behavior and without changing the frozen Component lifecycle semantics unless explicitly approved.


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

- [ ] `CORE-CTX-003` is defined in authoritative `REQUIREMENTS.md` only after task acceptance.
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
feat(core): define component context injection boundary
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

Primary commit: `adb0e08` `feat(core): define component context injection boundary` (R06-002 accepted at `0f8b146`). **Purely additive production change:** one explicit constructor (and a deleted rvalue overload) on `ComponentContext`, one include and the INJECTION contract text; `src/`, `Component`, `RuntimeManager` and every R0.3, R0.4 and R0.5 header are unchanged. Test support only: `tests/contract/reference_component.hpp` is no longer `final` and `tests/integration/runtime_scenarios.hpp` gains an optional component factory.

- Changed files: `include/kritva/core/runtime/component_context.hpp` (constructor, contract text, tag `CORE-CTX-003`), `tests/unit/component_context_injection_test.cpp` (new, 8 functions), `tests/contract/reference_component.hpp` and `tests/integration/runtime_scenarios.hpp` (test support), `CMakeLists.txt`, `REQUIREMENTS.md` (`CORE-CTX-003` and traceability row), `API.md` (section 37), `ARCHITECTURE.md`.
- API (approved D13): `explicit ComponentContext(const Component& component, platform::PlatformContext = {}) noexcept`, binding to `component.info()` (valid once the `Component` base is constructed and until destruction), and `ComponentContext(const Component&&, ...) = delete` so a temporary component is refused at compile time. **Injection is by construction only**: the integrator builds the context from the component's own identity and a platform view (from an adapter or `runtime.platform()`) and the component stores it, e.g. `context_(*this, platform)` in its constructor. The Runtime never creates, stores, passes, finds or probes a context; there is no automatic injection, registration hook, factory or lookup; the `Component` base has no context member and its lifecycle signatures are untouched; a context has no lifecycle of its own.
- Unit/contract tests (`kritva_core_component_context_injection`): a component builds its context from its own identity (pointer identity with `worker.info()`, name, version, platform), and the same context can be built from the component or its info, with or without a platform; a temporary component is refused, the constructor is explicit and noexcept, and the context is still two pointers; a context built from `runtime.platform()`, and an unattached one when none is attached; **the Runtime and the Component base are unchanged**: static_asserts on the exact signatures of the five `Component` lifecycle operations and `info()`, on `RuntimeManager::attach_platform`, `platform`, `register_component`, `initialize`, `reset`, and member-detection concepts proving that neither `Component` nor `RuntimeManager` has a context member, factory, injection or lookup; **differential**: 40 seeded scripts x 40 operations replayed with plain reference components and with components that hold a context and **use it in every lifecycle operation** (`require_timer`, `supports`, `require_scheduler`, `has_capability`, `id`), for each of the 16 service combinations with every platform method failing: identical results, states, faults, statistics, retries and invocation traces, and the platform call log stays empty (contexts started and called nothing); the Runtime made **no call to the adapter**: with a spy adapter every one of the 60 adapter calls (3 components x 5 operations incl. `configure` x 4 queries) is accounted for by the components' own context use; a context has no lifecycle of its own (unchanged across Runtime operations, ends with its component, platform call log empty); a component returning its context's availability error is an ordinary component failure (FAULT, one error, no retry, explicit `reset()`).
- Mutation evidence (17 mutants, each reverted, all detected): the injection constructor dropping the identity or the platform, binding a copied identity, probing the adapter, allowing a temporary component, being implicit; `Component` gaining a context member; `RuntimeManager` gaining a context factory, an injection or a lookup; and eight Runtime-coupling mutants (initialize starting the timer, start creating a task, stop stopping the scheduler, FAULT stopping the watchdog, configure reading the clock, attach probing the adapter, start failing without a scheduler, initialize counting an error when an adapter is attached). The `configure` clock-read mutant survived the first run because the script had no `configure` step; it was closed by adding `configure` to the accounting.
- Regression: `ctest` 42/42 in Debug, Release, ASan+UBSan, strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`, TSan (ASLR off); the whole existing suite is unchanged and green; build 0 warnings; GCC `-fanalyzer` clean; coverage 99% (587/593; `component_context.hpp` fully covered); `make check` passes with traceability 76 requirements, 75 traced, 0 errors; `git diff --check` clean; dependency scan clean.
- Out of scope confirmed: no concrete platform, no registry or locator, no automatic dependency injection, no Runtime recovery or retry, no background execution, no Runtime asynchronous redesign.

## 12. Reviewer decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS** |
| Accepted commit | `adb0e08` (evidence `71f1a22`) |
| Evidence reference | section 11a above |
| Date | 05-10-2026 |

Reviewer notes: construction-time injection is additive and explicit (`ComponentContext(const Component&, ...)`, temporaries refused); `src/`, the `Component` lifecycle signatures and `RuntimeManager` are unchanged; the Runtime never creates, stores, passes, finds or probes a context and made no adapter call (all 60 observed calls were the components' own queries); the differential over every service combination with every platform method failing is identical to the plain-component baseline; 17/17 mutants detected. `CORE-CTX-003` is authoritative. Reviewer relied on the supplied evidence; the commits were local-only.

**Reviewer Decision: PASS — KF-CORE-R06-003 is ACCEPTED.**
