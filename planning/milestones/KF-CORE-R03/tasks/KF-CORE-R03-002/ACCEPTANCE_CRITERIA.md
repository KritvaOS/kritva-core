# KF-CORE-R03-002 — Component Registry
## Acceptance Criteria

### 1. Objective

Provide a deterministic, platform-independent registry for registering and locating runtime components using `ComponentId`.

### 2. Requirement Traceability

| ID | Requirement |
|---|---|
| CORE-RT-003 | Component registry with deterministic registration and lookup semantics |

### 3. Scope

In scope:
- registration
- duplicate identity handling
- lookup
- contains
- deterministic enumeration
- ownership/lifetime contract

Out of scope:
- lifecycle orchestration
- dependency resolution
- scheduler/executor
- threads
- runtime start/stop
- automatic recovery
- hardware/platform implementation

### 4. Acceptance Criteria

#### AC-002-01 — Registration
- [ ] Valid component registers successfully.
- [ ] Invalid/empty identity is rejected.
- [ ] Null component/reference is rejected where applicable.
- [ ] Registration result is deterministic.
- [ ] Successfully registered component is discoverable.

#### AC-002-02 — Duplicate Identity
- [ ] Duplicate identity registration fails deterministically.
- [ ] Existing component is not implicitly replaced.
- [ ] Failed registration leaves registry unchanged.
- [ ] Existing Core error/result semantics are used.

#### AC-002-03 — Lookup
- [ ] Existing component can be found by `ComponentId`.
- [ ] Missing component has deterministic not-found behavior.
- [ ] Lookup has no side effects.

#### AC-002-04 — Contains
- [ ] Registered component returns true.
- [ ] Missing component returns false.
- [ ] Operation has no side effects.

#### AC-002-05 — Enumeration
- [ ] Enumeration is deterministic.
- [ ] Each registered component appears exactly once.
- [ ] Container implementation details are not exposed.
- [ ] Ordering rule is documented and tested.
- [ ] Ordering does not depend on accidental `unordered_map` iteration.

#### AC-002-06 — Ownership/Lifetime
- [ ] Ownership model is explicit.
- [ ] Component lifetime is explicit.
- [ ] The registry is non-owning: it holds non-owning references to components owned by the application/runtime owner (R03-001 ownership model preserved).
- [ ] The registry never owns, copies, moves or deletes a component; components are non-copyable and non-movable (R03-001), and the registry must not require otherwise.
- [ ] The registry does not silently become an owning container.
- [ ] Behavior when a component is destroyed is defined.
- [ ] Validity of returned references/pointers is defined.
- [ ] No dangling reference is permitted.

#### AC-002-07 — Unregister
- [ ] `unregister()` is not required unless a concrete R03 need is demonstrated.
- [ ] If deferred, it is explicitly documented as out of scope.
- [ ] No incomplete public placeholder API is added.

#### AC-002-08 — Runtime Separation
- [ ] Registry does not initialize components.
- [ ] Registry does not start/stop components.
- [ ] Registry does not perform dependency ordering.
- [ ] Registry does not create threads or invoke a scheduler.

### 5. Required Unit Tests

- [ ] register valid component
- [ ] reject invalid component
- [ ] duplicate registration
- [ ] find existing component
- [ ] find missing component
- [ ] contains existing component
- [ ] contains missing component
- [ ] deterministic enumeration
- [ ] registry state after failed registration
- [ ] ownership/lifetime behavior where testable

### 6. Required Integration / Regression Tests

- [ ] R03-001 component contract tests pass.
- [ ] Minimal reference components can be registered and found.
- [ ] Multiple components can coexist.
- [ ] Registry can be consumed through the public API.
- [ ] Existing R0.2 regression suite passes.
- [ ] No platform-specific dependency is introduced.

### 7. Validation / Sign-off Tests

Required evidence:
- [ ] clean Debug build
- [ ] Release build
- [ ] `-Werror`
- [ ] CTest
- [ ] ASan
- [ ] UBSan
- [ ] coverage
- [ ] `make check`
- [ ] traceability audit
- [ ] public-header/API checks
- [ ] forbidden-dependency checks
- [ ] diff/status check

### 8. Expected Evidence

Codex/Claude must report:
- implementation commit SHA
- changed files
- unit-test command/output
- integration/regression-test command/output
- Debug/Release results
- sanitizer results
- coverage
- traceability/dependency checks
- final Git status

### 9. Git Commit

Implementation commit:

`feat(core): add component registry`

Focused post-review corrections must use a separate `fix(core): ...` commit.

### 10. Reviewer Sign-off

| Item | Result |
|---|---|
| Implementation complete | ☐ |
| Unit tests complete | ☐ |
| Integration/regression tests complete | ☐ |
| Validation gates complete | ☐ |
| Registry API reviewed | ☐ |
| Ownership/lifetime reviewed | ☐ |
| Architecture constraints satisfied | ☐ |
| Reviewer decision | **PASS / CHANGES REQUIRED / BLOCKED** |
| Reviewer | __________________ |
| Date | __________________ |

Final acceptance remains subject to the combined R03 Foundation API Review after R03-003.
