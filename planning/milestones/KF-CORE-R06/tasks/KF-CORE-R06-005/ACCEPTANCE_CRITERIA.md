# KF-CORE-R06-005 — Acceptance Criteria

## 1. Task information

- Task: `KF-CORE-R06-005`
- Title: Reference Context Harness & Contract Tests
- Milestone: `KF-CORE-R06`
- Estimated effort: 3–4 ED
- Dependency: R06 Component API Review PASS/FROZEN
- Requirement proposed: `CORE-CTX-005`
- Exact primary commit message: `test(core): add component context reference harness`

## 2. Objective


Create test-only reference context/services and contract tests covering ownership, lifetime, side effects, access policy, requirements and deterministic behavior.


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

- [ ] `CORE-CTX-005` is defined in authoritative `REQUIREMENTS.md` only after task acceptance.
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
test(core): add component context reference harness
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

Primary commit: `c7f7b46` `test(core): add component context reference harness` (Component API Review PASS / FROZEN recorded at `6136cab`). **No production code changed** (`git diff 06207c7 HEAD -- include src` is empty); no production API added; the frozen `ComponentContext` API is consumed, not modified.

- Changed/new files: `tests/runtime/reference_context_component.hpp` (new, test-only), `tests/platform/context_conformance.hpp` (new, reusable), `tests/unit/reference_context_test.cpp` (new, 11 functions), `CMakeLists.txt` (registration), `REQUIREMENTS.md` (`CORE-CTX-005` and traceability row), `TESTING.md`, `ARCHITECTURE.md`.
- **Reference context component:** `ReferenceContextComponent` implements only the public Component contract on top of the reference component, holds a `ComponentContext` built from its own identity in its constructor, and runs **scripted per-operation plans** of steps through that context: the typed queries (`REQUIRE_*`), use of a service through its own contract (create task, start scheduler/timer/watchdog, kick, stop), requirement checks and evaluation, capability and support queries. The first failing step ends the plan and its error is returned as an integrator would return it (a Core availability error from the context already attributed to the component; a platform service error attributed with `attribute()`), while the lifecycle state moves exactly as a failed operation of the reference component does. It records every executed step with its outcome and source, observes its own destruction, and keeps the reference component's `fail_next_*` injection.
- **Reusable contract checks:** `conformance::check_component_context(context, adapter_or_null, report)` verifies, for one context and the adapter it was built on: identity (bound/unbound), the platform view, every typed query (the adapter's own object, or `UNSUPPORTED` with the component as source when bound and no source when unbound, naming only its service), `supports` and capability identity (and that no platform name, version number or name length is ever a capability), attribution (only the source changes), requirement binding equal to the R0.5 report and result (required, optional and mixed sets) with the source rule, and determinism. It collects failures in the existing `conformance::Report`.
- Tests (`kritva_core_reference_context`): the contract checks pass for **every one of 16 service combinations x capability present/absent**, for bound contexts built from a platform view or a nullable pointer, for **copies and moves**, for a bound context without a platform and for an unbound one, and checking starts and uses no service; they **fail** for a context checked against the wrong adapter, against none, or against one it does not have; a scripted plan drives the platform in order through the context (create-task, scheduler start, timer start, watchdog start, kick; stop in reverse), with every executed step recorded and nothing running until time advances; **every unavailable service** (16 combinations) is an attributed `UNSUPPORTED` failure naming the right service, leaves the component in FAULT and is recorded with the component as source; an injected platform service error comes back with the service's own code and message and only its source set to the component; requirement checks run through the context (a missing optional item does not fail, a missing required capability does and ends the plan); **side effects are counted on the reference platform**: construction, copying and shape queries make zero adapter queries, each typed query exactly one, attribution none, evaluation one `supports()` per declared service plus one capability snapshot, no service started; **the Runtime makes no adapter query and no service call, in any state including FAULT and reset, for components that hold a context and do not use it**, and never reads or advances time; ownership and lifetime are observable (the context never destroys the platform, a service or its identity; the component ends before the platform; the integrator ends the platform); replay is deterministic; the harness runs under a `RuntimeManager` like any component and a plan failure is an ordinary component failure.
- **Test-target isolation:** the harness lives only under `tests/`; the existing repository audit (`CORE-PLAT-016`) and `kritva_core_test_isolation` already forbid any production include of `tests/` or of a reference or conformance header. I verified the guard with a planted violation (a production header including `tests/runtime/reference_context_component.hpp`): both the audit and the isolation script fail, and the clean tree passes.
- Mutation evidence: the 91 production mutants of R06-001 to R06-004 were replayed against this harness. 62 are detected by the harness alone; two rounds found real gaps that I closed (conformance on copies and moves, optional-item requirement sets, platform-name/version/name-length identity checks, per-operation adapter-query counts, and a Runtime-coupling test with components that hold but do not use a context). The remaining 29 are **structural** mutants that a behavioral harness is not meant to see and that their own unit tests already killed: copy/move assignment allowed, a temporary identity or component allowed, a non-trivial destructor, extra state, `platform()` by value, implicit conversion or constructor, added generic or by-name members, added `RuntimeManager` context members, added requirement-declaring members, plus one equivalent mutant (unreachable code) and five whose patterns no longer exist in the header (killed at R06-001).
- Regression: `ctest` 44/44 in Debug, Release, ASan+UBSan, strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`, TSan (ASLR off); the whole existing suite is unchanged and green; build 0 warnings; GCC `-fanalyzer` clean; coverage 99% (595/601, production lines unchanged); `make check` passes with traceability 78 requirements, 77 traced, 0 errors; `git diff --check` clean; dependency scan clean.
- Out of scope confirmed: no concrete platform, no registry or locator, no production API, no hardware, no change to accepted R0.5 semantics.

## 12. Reviewer decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS** |
| Accepted commit | `c7f7b46` (evidence `ad083ed`) |
| Evidence reference | section 11a above |
| Date | 05-10-2026 |

Reviewer notes: the harness consumes the frozen `ComponentContext` API without changing production code or API; the reference context component, the reusable `check_component_context()` checks (passing for every combination, copies and moves, failing for a wrong platform), the counted side effects, the Runtime-isolation test, lifetime observation and the isolation guard (verified by a planted violation) are accepted; the mutation survivors are structural mutants killed by their own unit tests. `CORE-CTX-005` is authoritative. Reviewer relied on the supplied evidence; the commits were local-only.

**Reviewer Decision: PASS — KF-CORE-R06-005 is ACCEPTED.**
