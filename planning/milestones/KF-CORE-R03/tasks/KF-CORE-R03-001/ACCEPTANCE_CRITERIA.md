# KF-CORE-R03-001 — Component Contract & Identity
## Acceptance Criteria

### 1. Objective

Define the platform-independent public contract representing a Kritva Core runtime component, including stable identity, metadata, lifecycle interaction, ownership/lifetime expectations, and existing Core error/result semantics.

### 2. Requirement Traceability

| ID | Requirement |
|---|---|
| CORE-RT-001 | Platform-independent component contract with stable identity and explicit lifecycle semantics |
| CORE-RT-007 | Runtime failures use deterministic Core error/result semantics where applicable |

> R03 requirement IDs are planning proposals until incorporated into the authoritative `REQUIREMENTS.md`.

### 3. Scope

In scope:
- `ComponentId`
- component metadata
- component lifecycle contract
- lifecycle state interaction
- ownership/lifetime contract
- thread-safety documentation
- existing `Result<T>` / `Status` / `Error` integration

Out of scope:
- scheduler/executor
- threads/thread pools
- ROS2/DDS
- EtherCAT
- Linux/vendor APIs
- hardware drivers
- automatic recovery
- runtime orchestration

### 4. Acceptance Criteria

#### AC-001-01 — Component Identity
- [ ] Every component has a stable `ComponentId`.
- [ ] Identity is non-empty.
- [ ] Identity is immutable for the component lifetime.
- [ ] Identity comparison is deterministic.
- [ ] No platform-specific identifier is used as Core identity.

#### AC-001-02 — Metadata
- [ ] Required metadata is exposed by the contract.
- [ ] Metadata is platform independent.
- [ ] Name/version semantics are documented.
- [ ] Metadata lifetime is defined.

#### AC-001-03 — Lifecycle Contract
- [ ] Existing Core lifecycle states are reused.
- [ ] Valid transitions are documented.
- [ ] Invalid transitions have deterministic behavior.
- [ ] Lifecycle state is observable.
- [ ] No `CONFIGURED` state is introduced solely for R03.
- [ ] `initialize()`, `start()`, `stop()`, and `shutdown()` semantics are explicit.

#### AC-001-04 — Result/Error Integration
- [ ] Existing `Result<T>`, `Status`, and `Error` contracts are reused.
- [ ] No parallel error abstraction is introduced.
- [ ] R02 `Result<T>` semantics remain unchanged.
- [ ] Failure information is deterministic and inspectable.

#### AC-001-05 — Ownership/Lifetime
- [ ] Ownership model is explicit.
- [ ] Component lifetime relative to runtime is explicit.
- [ ] Registered-component lifetime is defined.
- [ ] Returned references/pointers have defined validity.
- [ ] No dangling-reference behavior is permitted by the contract.

#### AC-001-06 — Thread Safety
- [ ] Thread-safety expectations are documented.
- [ ] No universal thread-safety claim is made unless already guaranteed by Core.
- [ ] No threading/executor implementation is introduced.
- [ ] No hard-real-time guarantee is introduced.

#### AC-001-07 — Platform Independence
- [ ] No Linux-specific API.
- [ ] No ROS2/DDS dependency.
- [ ] No EtherCAT dependency.
- [ ] No vendor SDK/HAL.
- [ ] No hardware-specific implementation.

### 5. Required Unit Tests

Add focused unit/contract tests covering:
- [ ] valid component identity
- [ ] empty/invalid identity
- [ ] identity comparison
- [ ] identity immutability
- [ ] metadata
- [ ] valid lifecycle transitions
- [ ] invalid lifecycle operations
- [ ] Result/Error integration
- [ ] ownership/lifetime contract where testable

### 6. Required Integration / Regression Tests

- [ ] Existing R0.2 Core test suite passes unchanged in expected behavior.
- [ ] Component contract can be consumed by a minimal reference component.
- [ ] Component header can be included from the public include path.
- [ ] No prohibited platform dependency appears in the component implementation.

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

`feat(core): define component runtime contract`

Follow-up fixes, if required after review, must use a focused `fix(core): ...` commit and must not silently amend an accepted implementation commit.

### 10. Reviewer Sign-off

| Item | Result |
|---|---|
| Implementation complete | ☐ |
| Unit tests complete | ☐ |
| Integration/regression tests complete | ☐ |
| Validation gates complete | ☐ |
| Public API reviewed | ☐ |
| Architecture constraints satisfied | ☐ |
| Reviewer decision | **PASS / CHANGES REQUIRED / BLOCKED** |
| Reviewer | __________________ |
| Date | __________________ |

**Important:** Codex/Claude may provide objective evidence and check implementation/test evidence. Final reviewer decision is made by the architecture reviewer.
