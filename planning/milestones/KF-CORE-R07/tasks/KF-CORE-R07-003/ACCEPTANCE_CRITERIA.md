# KF-CORE-R07-003 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R07-003 |
| Status | PLANNED |
| Primary commit | `feat(core): define component operational event contract` |
| Reviewer | ChatGPT — independent acceptance gate |

## Objective

Define explicit operational Event reporting to an integrator-owned sink without introducing a Core EventBus or asynchronous infrastructure.

## Scope

Reuse the existing Event envelope. Define source identity, explicit synchronous reporting boundary, sink ownership, delivery/no-buffering/no-retry semantics and event-versus-command separation.

## Proposed Requirements Traceability

R0.7 requirement domain: `CORE-OPS-*`. The implementing task must update authoritative `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- Component explicitly reports an Event; Core does not auto-generate events from observations.
- Event source identity is the reporting Component.
- Sink is integrator-owned and non-owning.
- No Core queue, broker, dispatcher, worker or persistence is introduced.
- No implicit retry, buffering or background delivery is introduced.
- Event reporting never triggers lifecycle/recovery automatically.
- Existing Event envelope remains authoritative or changes are explicitly reviewed.

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
feat(core): define component operational event contract
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

Primary commit: `56ff226` `feat(core): define component operational event contract` (consult amendments A2 and A5 implemented). **Purely additive:** one new public header `runtime/component_events.hpp` and one include in `core.hpp`; `git diff HEAD~1 -- src` is empty; the `Event` envelope, `Component`, `ComponentContext`, `RuntimeManager` and every earlier header are unchanged.

- API: `IEventSink` (integrator-owned, `virtual Result<void> report(const Event&)`) and `ComponentEventReporter` (copyable value of two non-owning pointers: `const ComponentInfo*`, `IEventSink*`; `bound()`, `id()`, `report(Event) const`; constructed from `(const ComponentInfo&, IEventSink&)` or `(const Component&, IEventSink&)`; unbound default; immutable; assignment deleted; trivially destructible; a temporary identity is refused by deleted overloads and a temporary sink because the parameter is a non-const lvalue reference).
- Behavior per amendment A2: unbound gives `INVALID_STATE` with no sink call; non-zero mismatching `source_id` gives `INVALID_ARGUMENT` (source = the component) with no sink call; zero is stamped; a match is forwarded unchanged; `source_id` is the only field touched; exactly one synchronous call on the caller's thread; the sink's `Result` is returned unchanged (not re-attributed); no buffering, retry, queue, persistence, filtering, ordering or async dispatch; an exception propagates; re-entrant, no lock/state. Amendment A5 (sink non-owning and integrator-managed, must outlive every reporter) is in the contract text. Event versus command separation and integrator ownership of sinks/telemetry/policy are stated (CORE-OPS-005, CORE-OPS-008; API.md section 41; ARCHITECTURE.md).
- Tests (`kritva_core_component_events`, 13 functions): shape (two pointers, trivially destructible, nothrow copy/move, no assignment, no setter/bind/reset, no path to the sink or identity, temporary identity or sink refused, a sink is required); unbound; zero stamped (caller's copy untouched); match unchanged; four mismatches rejected, attributed to the reporting component, **sink never called**, reporter still usable; all other fields forwarded exactly over 7 EventTypes × 4 severities; exactly one synchronous call on the caller's thread; the sink's failure returned unchanged (code, severity, foreign source, message), **no retry**, a failed event kept nowhere after the sink recovers; reporting events of every type through a Runtime-registered component in RUNNING changes no state, fault, lifecycle call or Runtime statistic, the Runtime never calls the sink during configure/initialize/start/stop/shutdown; re-entrant reporting from inside the sink; sink exception propagates and the reporter stays usable; copies, moves and the `ComponentInfo` constructor share identity and sink; two components through one sink cannot speak for each other.
- Mutation evidence (22 mutants, each reverted): mismatch check removed/inverted/loosened, stamping removed, each of severity/timestamp/correlation/event_id/type overwritten, retry on failure, failure swallowed, failure re-attributed, double call, mismatch error wrong source/code, unbound code changed, assignment allowed, a deleted overload removed, extra state. 19 detected at once. Three survivors classified, not hidden: two are **equivalent** (`bound()`/`!bound()` weakened to test only the identity pointer: the public constructors set both pointers together so a half-bound reporter is unreachable) and one is **redundant** (the deleted `IEventSink&&` overloads: a temporary cannot bind to a non-const lvalue reference anyway); I removed those two overloads and kept the compile-time assertion that a temporary sink is refused.
- Regression: ctest 48/48 in Release, ASan+UBSan, strict `-Werror` (0 warnings), TSan, Debug; GCC `-fanalyzer` clean; coverage 617/624, `component_events.hpp` 13/13; `make check`, traceability 86 requirements, 85 traced, 0 errors (`CORE-OPS-005`, `CORE-OPS-008` defined with rows); `git diff --check` clean.
- Out of scope confirmed: no EventBus/queue/broker/dispatcher, no logger or telemetry backend, no Runtime event path, no background execution.

## Reviewer Decision

- [ ] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Reviewer decision is independent of implementor checkboxes.
