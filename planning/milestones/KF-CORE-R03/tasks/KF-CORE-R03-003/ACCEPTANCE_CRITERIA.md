# KF-CORE-R03-003 — Dependency Management
## Acceptance Criteria

### 1. Objective

Define and validate the dependency graph between registered components and produce a deterministic dependency-respecting order for runtime orchestration.

### 2. Requirement Traceability

| ID | Requirement |
|---|---|
| CORE-RT-004 | Represent dependencies using stable component identity |
| CORE-RT-005 | Detect dependency cycles and provide deterministic dependency ordering |

### 3. Scope

In scope:
- dependency representation
- dependency validation
- missing/self/duplicate dependency handling
- cycle detection
- deterministic dependency ordering

Out of scope:
- component lifecycle execution
- Runtime Manager
- scheduler/executor
- threads
- recovery
- hardware/platform implementation

### 4. Acceptance Criteria

#### AC-003-01 — Dependency Representation
- [ ] Dependencies are represented using `ComponentId`.
- [ ] Raw pointers are not used as dependency identity.
- [ ] Platform-specific handles are not used.
- [ ] Representation is deterministic.

#### AC-003-02 — Missing Dependency
For `A -> B` when B is not registered:
- [ ] validation fails deterministically;
- [ ] error identifies the missing dependency;
- [ ] graph/registry is not partially modified;
- [ ] no invalid order is returned.

#### AC-003-03 — Self Dependency
For `A -> A`:
- [ ] dependency is rejected;
- [ ] deterministic error is returned;
- [ ] graph remains valid/unchanged.

#### AC-003-04 — Duplicate Dependency
For `A -> B` repeated:
- [ ] behavior is explicitly defined;
- [ ] recommended behavior is rejection;
- [ ] no duplicate accepted edge exists.

Any alternative such as silent deduplication requires explicit reviewer approval.

#### AC-003-05 — Cycle Detection
For `A -> B -> C -> A`:
- [ ] cycle is detected;
- [ ] operation fails deterministically;
- [ ] cycle members are identifiable where supported by the contract;
- [ ] no invalid execution order is returned.

#### AC-003-06 — Multi-Level Dependencies
For `A -> B`, `B -> C`, `C -> D`:
- [ ] D appears before C;
- [ ] C appears before B;
- [ ] B appears before A;
- [ ] dependency-order semantics are documented.

#### AC-003-07 — Independent Components
For `A -> C` and `B -> C`:
- [ ] C precedes A and B;
- [ ] A/B ordering is deterministic;
- [ ] recommended tie-break is deterministic registration order;
- [ ] chosen tie-break rule is documented and tested.

#### AC-003-08 — Empty / Simple Graphs
- [ ] empty graph returns empty order;
- [ ] one independent component returns one-element order;
- [ ] multiple independent components return deterministic order.

#### AC-003-09 — No Runtime Orchestration
- [ ] no component initialization;
- [ ] no component start/stop;
- [ ] no scheduler invocation;
- [ ] no thread creation;
- [ ] no recovery behavior.

### 5. Required Unit Tests

- [ ] no dependencies
- [ ] single dependency
- [ ] multi-level dependency
- [ ] multiple dependencies
- [ ] missing dependency
- [ ] self dependency
- [ ] duplicate dependency
- [ ] simple cycle
- [ ] multi-node cycle
- [ ] independent components
- [ ] deterministic ordering
- [ ] invalid graph does not produce an execution order

### 6. Required Integration / Regression Tests

- [ ] R03-001 tests pass.
- [ ] R03-002 registry tests pass.
- [ ] Dependency graph consumes registered components through public APIs.
- [ ] Valid registry topology produces deterministic dependency order.
- [ ] Invalid topology produces deterministic errors.
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

`feat(core): add runtime dependency management`

Focused post-review corrections must use a separate `fix(core): ...` commit.

### 10. Reviewer Sign-off

| Item | Result |
|---|---|
| Implementation complete | ☐ |
| Unit tests complete | ☐ |
| Integration/regression tests complete | ☐ |
| Validation gates complete | ☐ |
| Dependency API reviewed | ☐ |
| Ordering semantics reviewed | ☐ |
| Architecture constraints satisfied | ☐ |
| Reviewer decision | **PASS / CHANGES REQUIRED / BLOCKED** |
| Reviewer | __________________ |
| Date | __________________ |

Final acceptance is followed by the R03 Foundation API Review.
