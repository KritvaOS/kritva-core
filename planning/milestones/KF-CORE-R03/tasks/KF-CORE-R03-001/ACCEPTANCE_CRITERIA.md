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
- [x] Every component has a stable `ComponentId`.
- [x] Identity is non-empty.
- [x] Identity is immutable for the component lifetime.
- [x] Identity comparison is deterministic.
- [x] No platform-specific identifier is used as Core identity.

#### AC-001-02 — Metadata
- [x] Required metadata is exposed by the contract.
- [x] Metadata is platform independent.
- [x] Name/version semantics are documented.
- [x] Metadata lifetime is defined.

#### AC-001-03 — Lifecycle Contract
- [x] Existing Core lifecycle states are reused.
- [x] Valid transitions are documented.
- [x] Invalid transitions have deterministic behavior.
- [x] Lifecycle state is observable.
- [x] No `CONFIGURED` state is introduced solely for R03.
- [x] `initialize()`, `start()`, `stop()`, and `shutdown()` semantics are explicit.

#### AC-001-04 — Result/Error Integration
- [x] Existing `Result<T>`, `Status`, and `Error` contracts are reused.
- [x] No parallel error abstraction is introduced.
- [x] R02 `Result<T>` semantics remain unchanged.
- [x] Failure information is deterministic and inspectable.

#### AC-001-05 — Ownership/Lifetime
- [x] Ownership model is explicit.
- [x] Component lifetime relative to runtime is explicit.
- [x] Registered-component lifetime is defined.
- [x] Returned references/pointers have defined validity.
- [x] No dangling-reference behavior is permitted by the contract.

#### AC-001-06 — Thread Safety
- [x] Thread-safety expectations are documented.
- [x] No universal thread-safety claim is made unless already guaranteed by Core.
- [x] No threading/executor implementation is introduced.
- [x] No hard-real-time guarantee is introduced.

#### AC-001-07 — Platform Independence
- [x] No Linux-specific API.
- [x] No ROS2/DDS dependency.
- [x] No EtherCAT dependency.
- [x] No vendor SDK/HAL.
- [x] No hardware-specific implementation.

### 5. Required Unit Tests

Add focused unit/contract tests covering:
- [x] valid component identity
- [x] empty/invalid identity
- [x] identity comparison
- [x] identity immutability
- [x] metadata
- [x] valid lifecycle transitions
- [x] invalid lifecycle operations
- [x] Result/Error integration
- [x] ownership/lifetime contract where testable

### 6. Required Integration / Regression Tests

- [x] Existing R0.2 Core test suite passes unchanged in expected behavior.
- [x] Component contract can be consumed by a minimal reference component.
- [x] Component header can be included from the public include path.
- [x] No prohibited platform dependency appears in the component implementation.

### 7. Validation / Sign-off Tests

Required evidence:
- [x] clean Debug build
- [x] Release build
- [x] `-Werror`
- [x] CTest
- [x] ASan
- [x] UBSan
- [x] coverage
- [x] `make check`
- [x] traceability audit
- [x] public-header/API checks
- [x] forbidden-dependency checks
- [x] diff/status check

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

### 9a. Implementation Evidence (Claude)

- Commit: `655c1dd` `feat(core): define component runtime contract` (R03 planning at `9eda545`; R0.2 tag `kritva-core-r0.2` = `46af52c` untouched).
- Files changed: `include/kritva/core/runtime/component.hpp` (modified), `component_id.hpp` and `component_info.hpp` (new), `include/kritva/core/core.hpp` (umbrella includes), `tests/unit/component_test.cpp` (new), `tests/contract/component_contract.hpp` and `tests/contract/reference_component.hpp` (new, reusable), `tests/unit/runtime_test.cpp` (one adaptation, see Breaking change), `CMakeLists.txt` (test registration), `REQUIREMENTS.md` (CORE-RT-001 text and traceability row), `ARCHITECTURE.md` (component operation table), `API.md` (section 20).
- **Breaking change (public API, needs human review per AGENTS.md section 12):** `runtime::Component` is no longer a stateless interface. It now has a protected constructor `Component(ComponentInfo)`, a non-virtual `info()`, and deleted copy. Every implementer must be constructed with a `ComponentInfo`. There is exactly one implementer in the repository (`FakeComponent` in `tests/unit/runtime_test.cpp`); its only change is a default constructor that supplies an identity. All its assertions are otherwise unchanged and pass. No other code derives from `Component`. The virtual operation signatures are unchanged.
- Design decisions (all need reviewer confirmation; none are forced by existing code):
  1. **`ComponentId = Id`** (an alias, like `CapabilityId`), in namespace `kritva::core::runtime`. Non-empty means `valid()` (non-zero). Trade-off: it does not prevent mixing with other `Id`-based identities. A distinct strong type is possible later.
  2. **Identity immutability by construction**, not by convention: `Component` holds `const ComponentInfo info_`; `info()` is non-virtual, `noexcept`, and its reference is stable for the component's lifetime. `ComponentInfo` can only be made with `ComponentInfo::create()` (id valid, name non-empty, else `INVALID_ARGUMENT`), so an invalid one cannot exist.
  3. **Metadata** = id + name + `Version`. Name is a non-unique label (never identity or ordering key); version is the component's own, default 0.0.0.
  4. **Lifecycle semantics** (table in `ARCHITECTURE.md`): `configure` valid from UNKNOWN/STOPPED with no state change (no `CONFIGURED` state added); `initialize` UNKNOWN/STOPPED to READY; `start` READY to RUNNING; `stop` READY/RUNNING to STOPPED (a strict operation: STOPPED and UNKNOWN reject it); `shutdown` idempotent no-op in UNKNOWN/STOPPED, FAULT to STOPPED, and invalid in READY/RUNNING. All steps are edges of the existing Core transition table; RECOVERING is not produced by any component operation (recovery is KF-CORE-R03-006).
  5. **Failure semantics:** an invalid operation fails with `INVALID_STATE` and has no effect; a valid `initialize`/`start`/`stop` that fails moves the component to FAULT; a failed `shutdown` leaves the state unchanged; no automatic retry or recovery.
  6. **Error attribution:** every `Error` a component returns has `source == info().id()` (uses the existing `Error::source` field; no new error type).
  7. **Ownership:** non-owning. Core never owns, copies, moves or deletes components; whoever constructs a component owns it and must keep it alive and at a stable address while registered/used. Observer results (`status`, `health`, `capabilities`) are value snapshots and cannot dangle.
  8. **Thread safety:** no universal guarantee; callers serialize operations; `info()` is safe to read concurrently (immutable); everything is control-plane with no real-time claim.
- **Requirement IDs (flagged):** the acceptance table cites proposed `CORE-RT-007`. It is a proposal for KF-CORE-R03-006 (runtime failure propagation) and is NOT in `REQUIREMENTS.md`; I did not add it, per the proposal file's rule that IDs become authoritative after review. Only the existing authoritative `CORE-RT-001` text was extended. **Separate, important:** the proposal's `CORE-RT-002` ("component registry") collides with the existing authoritative `CORE-RT-002` ("runtime contract", `runtime/runtime.hpp`). That must be resolved before R03-002 (for example by giving the registry a new ID); I changed nothing.
- Tests: new CTest `kritva_core_component` (11 test functions) covering identity validity/comparison/hash, `ComponentInfo` valid and invalid creation, ownership shape (non-copyable, non-movable, virtual destructor, `info()` reference stable), the full operation matrix over UNKNOWN/READY/RUNNING/STOPPED through the reusable `check_component_contract()`, FAULT behavior after injected `initialize`/`start`/`stop` failure with attributable errors, failed `shutdown`, rejected configuration not applied, invalid operations without side effects, and a minimal consumer that uses only `Component&`.
- **Mutation evidence** (each temporary edit reverted; files verified identical): `start` allowed from RUNNING; error `source` dropped; `shutdown` allowed from READY; `ComponentInfo::create` accepting an empty name; failure not moving to FAULT; configuration partially applied on failure (all aborted the test); `Component` made copyable (compile-time `static_assert` failure).
- Build: `rm -rf build && cmake -S . -B build && cmake --build build -j$(nproc)` — 0 warnings.
- Tests: `ctest --test-dir build --output-on-failure` — **19/19** (17 R0.2 + install-consumer + new `kritva_core_component`). Release 19/19; ASan+UBSan 19/19; TSan (ASLR disabled, as in R0.2) 19/19; strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror` build 19/19; GCC `-fanalyzer` over `src/*.cpp`: no diagnostics.
- Install-consumer regression: green. It consumes the installed `core.hpp`, which now includes the new component headers.
- Standalone compile of `component_id.hpp`, `component_info.hpp`, `component.hpp` with `-Wall -Wextra -Wpedantic`: OK.
- Forbidden dependencies: new headers include only other Core headers and `<string>`/`<utility>`; no OS, threading, ROS2/DDS, EtherCAT or vendor headers.
- `make check` passed (header-check; traceability-check 50 requirements, 49 traced, 0 errors; format-check and lint are stubs). `git diff --check` clean.
- Coverage: `make coverage` — 98% (186/189 lines); the three uncovered lines are the same pre-existing defensive lines in `lifecycle.cpp`/`configuration.cpp`. The new headers are fully covered.
- Final `git status --short`: clean after the commit.
- Known limitations / not done by design: no registry, dependency, runtime manager, recovery or scheduler; the generic checker cannot reach FAULT (reached only via injected failure in the reference component); `ComponentId` is an alias, so type confusion with other `Id` aliases is not a compile error.

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
