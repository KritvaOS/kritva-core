# R06 Component API Review

## Purpose

Freeze the R06 Component Context public contract before integration implementation.

## Entry criteria

- R06-001 accepted
- R06-002 accepted
- R06-003 accepted
- R06-004 accepted
- focused unit/contract tests green
- full regression green
- traceability clean
- independent architecture review complete

## Review checklist

- [ ] Context ownership/lifetime is explicit.
- [ ] Context is not a service registry/locator.
- [ ] No Core-owned platform-service lifecycle exists.
- [ ] Existing `PlatformContext` remains authoritative for platform access.
- [ ] Context injection does not alter Runtime lifecycle semantics.
- [ ] Error propagation remains deterministic.
- [ ] Capability identity semantics are reused.
- [ ] No platform name/version inference.
- [ ] No breaking change is hidden.
- [ ] API is additive or explicitly architecture-approved.
- [ ] Public API self-containment passes.
- [ ] Contract/mutation evidence is sufficient.
- [ ] PASS / CHANGES REQUIRED / BLOCKED is recorded.

## Freeze rule

After PASS/FROZEN, R06-005 onward must not change the frozen production API except through an explicit architecture-review exception.

## Evidence (submitted 05-10-2026)

Baseline: R06-001 `8031c47`, R06-002 `072b713`, R06-003 `adb0e08`, R06-004 `5b755af` (all ACCEPTED; head `cdc55b7`). API decisions: `R06_DESIGN_DECISIONS.md` D09–D15 (`d9dd8a9`).

### Public API diff against `kritva-core-r0.5` (`include/` and `src/`)

| File | Change |
|---|---|
| `runtime/component_context.hpp` | **new** (R06-001..004): `runtime::ComponentContext` |
| `core.hpp` | one added include |

`git diff kritva-core-r0.5 HEAD -- src` is **empty**. Every R0.3, R0.4 and R0.5 header is byte-identical to the released `kritva-core-r0.5` tag, including `platform/context.hpp`, `platform/requirements.hpp`, `platform/adapter.hpp`, `runtime/component.hpp`, `runtime/component_info.hpp`, `runtime/runtime.hpp`, `runtime/runtime_manager.hpp`, `runtime/component_registry.hpp` and `runtime/dependency_graph.hpp`. All R06 production API is additive: one header, one include.

### The frozen contract (`ComponentContext`)

A copyable value of exactly two non-owning pointers (a `const ComponentInfo*` and an R0.5 `PlatformContext`), trivially destructible, nothrow default-constructible. Construction: `ComponentContext()` (unbound), `explicit ComponentContext(const ComponentInfo&, PlatformContext = {})`, `explicit ComponentContext(const Component&, PlatformContext = {})`, both with a deleted rvalue overload; **immutable after construction** (no setter, reset or rebinding; copy and move assignment deleted). Members: `bound()`, `info()`, `id()`, `platform()` (const reference), `require_scheduler()`, `require_clock()`, `require_timer()`, `require_watchdog()`, `supports()`, `has_capability()`, `attribute()` (`noexcept`, replaces only `source`), `evaluate()`, `check_required()`.

### Review checklist evidence

| Item | Evidence |
|---|---|
| Ownership and lifetime are explicit | the contract text; a lifetime probe shows contexts never destroy the adapter or the identity; copying neither extends a lifetime nor transfers ownership; a temporary `ComponentInfo` or `Component` is refused at compile time |
| Not a registry or locator | closed, typed surface; no generic `require(service)`, no lookup by name, no listing, no translate or wrap helper, no service control (member-detection concepts); no static or global state; no context amplification (no Runtime, registry, other component, `Configuration`, `Statistics`, `Health` or raw adapter reachable) |
| No Core-owned platform-service lifecycle | every access path is a query: at most one adapter accessor call; nothing started, stopped, configured, created or owned (spy adapters) |
| `PlatformContext` remains authoritative | `platform()` returns the R0.5 view by const reference; `PlatformContext` is byte-identical and its own errors still have no source (tested) |
| Injection does not alter Runtime semantics | construction only; `RuntimeManager` and `Component` have no context member, factory, injection or lookup (concepts and signature `static_assert`s); a 40 x 40 seeded differential over every service combination with every platform method failing, with components that use their context in every operation, is identical to the plain baseline, and the Runtime made zero adapter calls |
| Error propagation deterministic | `require_*()` and `check_required()` attribute only the Core availability error to a bound component (code, severity, timestamp and message are R0.5's); `attribute()` replaces only `source` (every field mutated in turn); service errors are never touched; a context error returned by a component is an ordinary component failure |
| Capability identity reused | `has_capability()`, `evaluate()` and `check_required()` delegate to the R0.5 identity model |
| No platform name or version inference | lookalike names, versions and name lengths used as identities provide nothing (tested for `has_capability`, `evaluate` and `check_required`) |
| No hidden breaking change | `src/` and all earlier headers byte-identical; all existing tests unchanged |
| Public API self-containment passes | the build compiles one translation unit per public header (`kritva_core_header_checks`) |
| Contract and mutation evidence sufficient | 23 (shape), 29 (access policy, 1 equivalent), 17 (injection, incl. 8 Runtime-coupling), 22 (requirement binding) mutants, all detected after closing the survivors found on first runs |

### Validation (head `cdc55b7`; production unchanged since `5b755af`)

`ctest` 43/43 in Debug, Release, ASan+UBSan, strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror` and TSan (ASLR off via `setarch -R`); 0 warnings; GCC `-fanalyzer` clean; `make traceability-check`: 77 requirements, 76 traced (CORE-ERR-003 reserved), 0 errors, `CORE-CTX-001` to `004` defined once; coverage 99% (595/601); no `find_package` or `FetchContent`; no threading header in production; `git diff --check` clean.

### Open issues (none blocking)

Carried forward: the 32-bit scheduler affinity mask; the conformance suite's level-2 mutation strictness gap; the `make lint` / `make format-check` stubs; a context (like `PlatformContext`) used after what it refers to is destroyed is documented undefined behavior (non-owning by design). R06-005 and R06-006 build the reference harness and integration tests on this API and are not part of this freeze.

Reviewer decision: PENDING

