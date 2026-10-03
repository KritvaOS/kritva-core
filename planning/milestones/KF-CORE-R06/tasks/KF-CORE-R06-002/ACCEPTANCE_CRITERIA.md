# KF-CORE-R06-002 — Acceptance Criteria

## 1. Task information

- Task: `KF-CORE-R06-002`
- Title: Operational Context Services & Access Policy
- Milestone: `KF-CORE-R06`
- Estimated effort: 3–4 ED
- Dependency: R06-001
- Requirement proposed: `CORE-CTX-002`
- Exact primary commit message: `feat(core): define operational context access policy`

## 2. Objective


Define which operational information/services may be accessed through the context and the side-effect/error rules for each access path.

Preserve R0.5 explicit-consumption semantics.


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

- [ ] `CORE-CTX-002` is defined in authoritative `REQUIREMENTS.md` only after task acceptance.
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
feat(core): define operational context access policy
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

Primary commit: `072b713` `feat(core): define operational context access policy` (R06-001 accepted at `e6b58ab`). **Purely additive:** members added to the new `ComponentContext` (`runtime/component_context.hpp`, R06-001) and one private helper; `src/` and every R0.3, R0.4 and R0.5 header are unchanged; `PlatformContext` is unchanged; `ComponentContext` is still exactly two non-owning pointers and trivially destructible (asserted).

- Changed files: `include/kritva/core/runtime/component_context.hpp` (members, ACCESS POLICY contract text with a side-effect and error table, tag `CORE-CTX-002`), `tests/unit/component_context_access_test.cpp` (new), `CMakeLists.txt`, `REQUIREMENTS.md` (`CORE-CTX-002` and traceability row), `API.md` (section 36), `ARCHITECTURE.md`.
- API (approved D10, D11, D12): `Result<platform::IScheduler*> require_scheduler() const`, `Result<time::IClock*> require_clock() const`, `Result<time::ITimer*> require_timer() const`, `Result<platform::IWatchdog*> require_watchdog() const`, `bool supports(platform::PlatformService) const noexcept`, `bool has_capability(CapabilityId) const`, `Error attribute(Error) const noexcept`. **Closed surface:** no generic `require(service)`, no lookup by name or runtime key, no capability listing, no translate/wrap helper, no service control; asserted by member-detection concepts. Every access path is a query: at most one adapter accessor call (one capability snapshot for `has_capability`), nothing started, stopped, configured, created or owned.
- **`require_*()` source attribution (the one deliberate extension of R0.5, at the Component-facing layer):** they forward to `PlatformContext::require_*()` and return its result; when the context is bound the Core availability error (`UNSUPPORTED`) carries `source` = the component's id so the component can return it directly as its own failed `Result`; code, severity, timestamp and message are exactly `PlatformContext`'s (tested field by field); an unbound context returns the R0.5 error unchanged; a success is never altered; `PlatformContext`'s own error is still sourceless (tested). Real platform service errors are never touched.
- **`attribute(Error) noexcept` is attribution only (your amendment):** the test mutates **every field of the input Error in turn** (four codes, three severities, empty/short/long messages, four different pre-existing sources, three timestamps including a REALTIME one and a negative value) and verifies that **only `source` changes** (replaced by the component id even when another source was present), that the original is returned unchanged for an unbound context (its source too), and that it is idempotent.
- Unit/contract tests (`kritva_core_component_context_access`, 13 functions): the exact approved surface and return types, noexcept, still two pointers, no generic/by-name/translate/list/control members; `require_*()` forward the adapter-owned service (the same objects as the R0.5 view, usable through their own contracts, nothing started); an unavailable service is an `UNSUPPORTED` error attributed to the component, for an unattached platform and for an attached one that lacks the service, each message naming only its service; the context error equals the R0.5 error plus the source; an unbound context returns the R0.5 error unchanged and a bound success is not altered; all 16 service combinations agree with the adapter, `supports()` and `has_capability()` (identity only: a platform named or versioned like a capability, name lengths and versions used as ids have none; an unknown service enumerator is unsupported; no platform means nothing supported); results are deterministic; every access path is a side-effect-free query (a spy adapter: exactly one accessor call per `require_*`, one for `supports`, one capability snapshot, none for `attribute`, no activation); a real service error (fault injected on the timer) keeps all its fields, including no source, until the component calls `attribute()`; **integration:** an integrator Component returns a context error directly from `start()`, the Runtime propagates it unchanged with the component as source (the Component contract is met), enters FAULT and recovers only by `reset()`.
- Mutation evidence: 29 mutants (each reverted). 28 detected at once or after one added assertion: bound error not attributed, attributed to a wrong id, success path touched, code/severity/message changed by `require_*`, a null success, `require_timer` gated on the watchdog, a second accessor call, `require_scheduler`/`require_watchdog` starting the service, `supports` ignoring the platform or always false, `has_capability` always true or matching the platform name (survived the first run: closed by an assertion over name lengths and versions used as ids), `attribute` doing nothing / changing code, severity, message or timestamp / attributing an unbound context / keeping an existing source, and added generic `require(service)`, name lookup, capability listing, translate helper, service control and extra state. The one remaining survivor (a dereference placed after a `return`) is an equivalent mutant: unreachable code.
- Regression: `ctest` 41/41 in Debug, Release, ASan+UBSan, strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`, TSan (ASLR off); the whole existing suite is unchanged and green; build 0 warnings; GCC `-fanalyzer` clean; coverage 99% (585/591, `component_context.hpp` fully covered); `make check` passes with traceability 75 requirements, 74 traced, 0 errors; `git diff --check` clean; dependency scan clean.
- Out of scope confirmed: no concrete platform, no registry or locator, no automatic recovery or retry, no background execution, no change to R0.5 semantics (the attribution applies only to the new component-facing layer).

## 12. Reviewer decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS** |
| Accepted commit | `072b713` (evidence `cfa67f9`) |
| Evidence reference | section 11a above |
| Date | 05-10-2026 |

Reviewer notes: the closed access surface, the side-effect-free queries, the bound-context attribution of the Core availability error (`PlatformContext` unchanged) and the attribution-only `attribute()` (every field mutated in turn, only `source` changes) are accepted; `src/` and the R0.5 public contracts are unchanged; 28 of 29 mutants detected and the 29th is an equivalent mutant. `CORE-CTX-002` is authoritative. Reviewer relied on the supplied evidence; the commits were local-only.

**Reviewer Decision: PASS — KF-CORE-R06-002 is ACCEPTED.**
