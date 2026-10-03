# KF-CORE-R03-005 — Runtime Lifecycle

## Task Information
| Field | Value |
|---|---|
| Task ID | KF-CORE-R03-005 |
| Milestone | KF-CORE-R03 |
| Title | Runtime Lifecycle |
| Requirement | CORE-RT-007 |
| Status | BLOCKED BY R03-004 |
| Depends on | R03-004 accepted |
| Primary commit | `feat(core): implement runtime lifecycle orchestration` |

## Objective

Implement deterministic Runtime lifecycle orchestration over the accepted R03-004 Runtime Manager and the frozen Component/Registry/DependencyGraph contracts.

Forward lifecycle operations use dependency order. Teardown operations use reverse dependency order.

R03-005 must preserve the accepted Component lifecycle contract and must not introduce automatic retry or autonomous recovery.

## Scope

### In scope
- Runtime lifecycle state machine required by the Runtime contract.
- Configure/initialize/start/stop/shutdown orchestration.
- Dependency-aware forward ordering.
- Reverse dependency-aware teardown.
- Deterministic invocation.
- Runtime state transitions.
- Invalid lifecycle operation handling.
- Explicit repeated-call behavior where required by the Runtime contract.

### Out of scope
- Component API changes.
- Registry/DependencyGraph API changes.
- Automatic retry.
- Background recovery/monitoring.
- Threads/executors.
- Dynamic topology changes.
- Platform-specific lifecycle behavior.
- Runtime failure/recovery policy beyond the failure boundary needed to implement lifecycle behavior; R03-006 owns detailed recovery.

## Frozen Component Lifecycle Contract

| Operation | Accepted Component semantics |
|---|---|
| configure | valid in UNKNOWN/STOPPED; no Component state change |
| initialize | UNKNOWN/STOPPED → READY |
| start | READY → RUNNING |
| stop | READY/RUNNING → STOPPED |
| shutdown | UNKNOWN/STOPPED no-op; FAULT → STOPPED |

Invalid Component operations return `INVALID_STATE` without changing Component state.

## Functional Requirements

### LC-01 Forward ordering
Configure/initialize/start use dependency order where Component invocation is required.

### LC-02 Reverse ordering
Stop/shutdown use reverse dependency order so dependents are torn down before dependencies.

### LC-03 Runtime state
Runtime state transitions are explicit and deterministic. Runtime does not report RUNNING until all required starts succeed.

### LC-04 Configure
Configure does not implicitly initialize or start components.

### LC-05 Initialize
Initialize occurs only from its valid Runtime state and invokes components in dependency order.

### LC-06 Start
Start occurs only after successful initialization and invokes components in dependency order.

### LC-07 Stop
Stop is explicit and invokes components in reverse dependency order.

### LC-08 Shutdown
Shutdown is explicit and invokes components in reverse dependency order.

### LC-09 Repeated calls
Repeated lifecycle calls follow the existing Runtime contract. If `CORE-RT-002` is ambiguous for a particular repeated operation, implementation must not invent silent semantics; the ambiguity must be returned to architecture review.

### LC-10 Call discipline
Runtime invokes only the lifecycle operation being orchestrated. It does not manipulate Component state directly.

### LC-11 No automatic recovery
A lifecycle failure does not trigger an automatic retry or background recovery action.

## Required Tests

### Unit
- Runtime state transition matrix.
- Configure ordering.
- Initialize dependency ordering.
- Start dependency ordering.
- Stop reverse dependency ordering.
- Shutdown reverse dependency ordering.
- Invalid Runtime lifecycle operations.
- Repeated lifecycle operations.
- Exact Component invocation counts.
- Independent dependency branches.
- Ordering independent of registration/edge insertion order.

### Failure boundary tests
Inject failures at each lifecycle operation and verify:
- returned error;
- invocation order;
- completed-operation accounting;
- Runtime state at the failure boundary.

Detailed recovery behavior is tested in R03-006.

### Regression
All R03-001..004 tests remain green.

## Mutation Testing

At minimum:
- reverse ordering changed to forward ordering;
- dependency order replaced by registration order;
- skipped lifecycle call;
- duplicate lifecycle call;
- premature RUNNING state;
- invalid state transition accepted;
- automatic retry inserted.

## Validation

Required:
- Debug
- Release
- ASan + UBSan
- TSan where configured
- `-Werror`
- `-fanalyzer`
- `make check`
- coverage reviewed against R03 baseline
- install-consumer
- `git diff --check`
- clean tree

## Documentation

Update:
- `REQUIREMENTS.md` — add `CORE-RT-007` only after implementation/review
- `ARCHITECTURE.md`
- `API.md`
- runtime lifecycle/state documentation
- task evidence

## Evidence Required

Provide:
1. primary commit;
2. runtime state/transition table;
3. actual component invocation traces;
4. forward and reverse ordering evidence;
5. failure-boundary evidence;
6. tests and regression results;
7. validation results;
8. mutation results;
9. coverage and traceability;
10. install-consumer evidence;
11. confirmation that frozen foundation APIs remain unchanged.

## Acceptance

Reviewer decision only:
- PASS
- CHANGES REQUIRED
- BLOCKED

## Git

Exact implementation commit:

`feat(core): implement runtime lifecycle orchestration`
