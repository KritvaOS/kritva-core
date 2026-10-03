# KF-CORE-R06-004 — Acceptance Criteria

## 1. Task information

- Task: `KF-CORE-R06-004`
- Title: Context Requirements & Capability Binding
- Milestone: `KF-CORE-R06`
- Estimated effort: 2–3 ED
- Dependency: R06-002, R06-003
- Requirement proposed: `CORE-CTX-004`
- Exact primary commit message: `feat(core): bind context requirements and capabilities`

## 2. Objective


Bind Component context requirements to the R0.5 requirement/capability identity model without introducing platform-name/version inference or a generic registry.


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

- [ ] `CORE-CTX-004` is defined in authoritative `REQUIREMENTS.md` only after task acceptance.
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
feat(core): bind context requirements and capabilities
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

Primary commit: `5b755af` `feat(core): bind context requirements and capabilities` (R06-003 accepted at `30bb727`). **Purely additive:** two members on `ComponentContext` and an include; `src/` and every R0.3, R0.4 and R0.5 header are unchanged; `ComponentContext` is still exactly two non-owning pointers, trivially destructible (asserted).

- Changed files: `include/kritva/core/runtime/component_context.hpp` (members, REQUIREMENT BINDING contract text, tag `CORE-CTX-004`), `tests/unit/component_context_requirements_test.cpp` (new, 8 functions), `CMakeLists.txt`, `REQUIREMENTS.md` (`CORE-CTX-004` and traceability row), `API.md` (section 38), `ARCHITECTURE.md`.
- API (approved D14): `platform::PlatformRequirementReport evaluate(const platform::PlatformRequirements&) const` — the R0.5 report **unchanged** (`platform::evaluate` against the context's platform view); `Result<void> check_required(const platform::PlatformRequirements&) const` — the R0.5 `check_required`, with the D12 rule: when the context is bound the `UNSUPPORTED` error carries `source` = the component's id, with code, severity, timestamp and message exactly R0.5's; an unbound context returns the R0.5 error unchanged and a success is never altered. **Binding, not storing:** the context stores no requirements (still two pointers) and offers no way to declare or list them (asserted by member-detection concepts and `sizeof`); the component keeps its own `PlatformRequirements` and the R0.5 declaration rules and atomicity are untouched. Matching is by service and capability identity only.
- Unit/contract tests (`kritva_core_component_context_requirements`, 8 functions): the surface and statelessness; **`evaluate()` equals the R0.5 report field by field for 16 service combinations x capability present/absent x 5 requirement sets (160 comparisons)**, for bound, unattached and unbound contexts; every missing item is reported in declaration order, split by level; **`check_required()` follows an independent oracle** for every availability, capability and requirement combination (a failing and a passing population of more than 20 each), failing as `UNSUPPORTED`/`ERROR` with the component as source; the error is the R0.5 error plus the source (code, severity, message, timestamp equal; services named before capabilities; the capability named when only a capability is missing), an unbound context returns the R0.5 error unchanged, an empty set and an all-optional set are satisfied; matching is by identity only (a lookalike platform name and version, a capability's own name and version, name lengths and versions used as ids); **side-effect-free and stateless**: a spy adapter shows exactly one `supports()` per declared service, one capability snapshot, zero identity calls, no activation, requirements untouched, deterministic reports, and capability-only or service-only requirements query only what they need, with the R0.5 declaration errors (duplicate and invalid identity) unchanged; **integration**: a component keeps its own requirements, checks them in `initialize()` and returns the context error directly: satisfied (the optional watchdog missing does not matter) it runs, unsatisfied it fails with `UNSUPPORTED` and the component as source, the Runtime enters FAULT, counts one error and no retry, and `reset()` recovers explicitly.
- Mutation evidence (22 mutants, each reverted, all detected): `evaluate()` returning an empty report, using an unattached platform, hiding optional or required services or capabilities, reversing the order, matching by platform name, probing the identity or starting the scheduler; `check_required()` not attributed, attributed to a wrong id, ignoring the platform, always succeeding, failing on optional items, changing the code, message or severity; and a context that declares, lists or stores requirements or holds them as state.
- Regression: `ctest` 43/43 in Debug, Release, ASan+UBSan, strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`, TSan (ASLR off); the whole existing suite is unchanged and green; build 0 warnings; GCC `-fanalyzer` clean; coverage 99% (595/601; `component_context.hpp` fully covered); `make check` passes with traceability 77 requirements, 76 traced, 0 errors; `git diff --check` clean; dependency scan clean.
- Out of scope confirmed: no concrete platform, no registry or locator, no platform name or version inference, no stored requirements, no Runtime-side requirement checking, no recovery or retry, no background execution.

## 12. Reviewer decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS** |
| Accepted commit | `5b755af` (evidence `e027ae4`) |
| Evidence reference | section 11a above |
| Date | 05-10-2026 |

Reviewer notes: the additive requirement binding meets the amended D14 contract: `evaluate()` is the unchanged R0.5 report, `check_required()` is the R0.5 result with the component as the source of a bound context's `UNSUPPORTED`, the context stores and declares nothing, matching is by identity only, evaluation is a side-effect-free query, and 22/22 mutants were detected. `CORE-CTX-004` is authoritative. Reviewer relied on the supplied evidence; the commits were local-only.

**Reviewer Decision: PASS — KF-CORE-R06-004 is ACCEPTED.**
