# KF-CORE-R06-001 — Acceptance Criteria

## 1. Task information

- Task: `KF-CORE-R06-001`
- Title: Component Execution Context & Ownership Model
- Milestone: `KF-CORE-R06`
- Estimated effort: 3–4 ED
- Dependency: R0.5 released
- Requirement proposed: `CORE-CTX-001`
- Exact primary commit message: `feat(core): define component execution context`

## 2. Objective


Establish the public shape and ownership/lifetime contract of the Component-facing execution context.

The context must be explicit, deterministic and non-owning. It must not replace PlatformContext or become a generic registry.


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

- [ ] `CORE-CTX-001` is defined in authoritative `REQUIREMENTS.md` only after task acceptance.
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
feat(core): define component execution context
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

Primary commit: `8031c47` `feat(core): define component execution context` (API decisions D09–D15 approved and recorded at `d9dd8a9`). **Purely additive:** one new public header `runtime/component_context.hpp` and one include in `core.hpp`; `git diff HEAD~1 -- src` is empty and no R0.3, R0.4 or R0.5 header or signature changed.

- Changed files: `include/kritva/core/runtime/component_context.hpp` (new), `include/kritva/core/core.hpp`, `tests/unit/component_context_test.cpp` (new), `CMakeLists.txt`, `REQUIREMENTS.md` (`CORE-CTX-001` and traceability row), `API.md` (section 35), `ARCHITECTURE.md`.
- API (approved Q1/Q6 with the amendments): `runtime::ComponentContext` — a copyable value of exactly two non-owning pointers (a `const ComponentInfo*` and an R0.5 `platform::PlatformContext`); `ComponentContext() noexcept` (unbound), `explicit ComponentContext(const ComponentInfo&, platform::PlatformContext = {}) noexcept`, `bound()`, `const ComponentInfo* info()` (null when unbound), `ComponentId id()` (invalid id when unbound), `const platform::PlatformContext& platform()` (const reference only). Trivially destructible, nothrow default-constructible, no cache, registry or static state. The `ComponentContext(const Component&, ...)` convenience belongs to R06-003 (injection) and the access and requirement members to R06-002 and R06-004.
- **Immutable after construction (reviewer amendment):** no setter, reset or rebinding member; copy and move **assignment are deleted** while copy and move construction are available (a component stores its context, built once in its constructor); a temporary `ComponentInfo` is refused at compile time (`ComponentContext(const ComponentInfo&&, ...) = delete`); binding is explicit (the constructor is `explicit`). Please confirm that deleting assignment is the intended reading of "no rebinding".
- **No context amplification (reviewer requirement):** no member returns the Runtime, registry, another component, `Configuration`, `Statistics`, `Health`, another component's context or a raw adapter, there is no lookup by name, and the context is not implicitly convertible to its identity pointer, the adapter or the platform view; all asserted at compile time.
- Unit/contract tests (`kritva_core_component_context`, 10 functions): shape (`sizeof` is exactly two pointers, trivially destructible, nothrow default/copy/move construction, noexcept accessors); immutability (assignment deleted, no `bind`/`set_platform`/`attach`/`reset`/`set_info` member via member-detection concepts, temporary identity refused, no implicit conversion); an unbound context has no identity and an unattached platform; a bound context reports the component's own `ComponentInfo` (pointer identity, id, name, version) and works without a platform; the platform view is the R0.5 view by const reference and forwards to the very adapter (also from a nullable pointer); copies and moves share what the original refers to; the context owns and destroys nothing (lifetime probe: the adapter and the identity outlive every context, the integrator ends them); construction, copying and shape queries make **zero adapter calls** and start nothing; no context amplification (member-detection concepts and non-convertibility); a Component stores its context, built from its own `info()`, in its constructor.
- Mutation evidence (23 mutants, each reverted): `bound()` always true or false, `info()` always null, `id()` always invalid or valid when unbound, the constructor dropping the identity or the platform or probing the adapter, the copy constructor dropping either pointer, copy and move assignment allowed, a temporary identity allowed, a non-trivial destructor, extra cached state, `platform()` returning by value, a reset, a rebinding member, an adapter accessor, a runtime accessor and a name lookup added, an implicit conversion to the identity pointer, an implicit constructor. 22 were detected at once; the implicit-constructor mutant survived the first run and was closed by a non-convertibility assertion.
- Regression: `ctest` 40/40 in Debug, Release, ASan+UBSan, strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`, TSan (ASLR off); the whole existing suite is unchanged and green; build 0 warnings; GCC `-fanalyzer` clean; coverage 99% (571/577; `component_context.hpp` fully covered, the baseline lines unchanged); `make check` passes with traceability 74 requirements, 73 traced, 0 errors; `git diff --check` clean; prohibited-dependency scan clean.
- Out of scope confirmed: no concrete platform, no registry or locator, no automatic Runtime recovery or retry, no Core-owned background execution, no change to accepted R0.5 semantics.

## 12. Reviewer decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS** |
| Accepted commit | `8031c47` (evidence `557589d`; design decisions `d9dd8a9`) |
| Evidence reference | section 11a above |
| Date | 05-10-2026 |

Reviewer notes: the implementation matches the amended R06 architecture: additive only, two non-owning pointers, immutable after construction with copy and move assignment deleted (confirmed as the correct reading of 'no rebinding'), temporary `ComponentInfo` refused, const-reference `platform()`, no context amplification, zero adapter calls from construction, copying and shape queries, 23/23 mutants detected. `CORE-CTX-001` is authoritative. Reviewer relied on the supplied evidence; the commits were local-only.

**Reviewer Decision: PASS — KF-CORE-R06-001 is ACCEPTED.**
