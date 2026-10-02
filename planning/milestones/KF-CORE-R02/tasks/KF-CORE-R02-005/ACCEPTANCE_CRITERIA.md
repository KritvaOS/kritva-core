# KF-CORE-R02-005 — Acceptance Criteria

## 1. Task Information

- Task: Clock Abstraction Cleanup
- Milestone: `KF-CORE-R02`
- Dependency: `KF-CORE-R02-004`
- Expected commit:
  `refactor(core): canonicalize clock abstraction`

## 2. Objective

Establish one canonical platform-neutral clock abstraction and eliminate conceptual duplication without breaking compatibility unnecessarily.

## 3. Requirement Traceability

The implementer must identify the authoritative requirement IDs affected by this task.

| Requirement ID | Header/API | Implementation | Test | Evidence |
|---|---|---|---|---|
| CORE-TIME-001, CORE-PLAT-002 | `include/kritva/core/time/clock.hpp`, `types/timestamp.hpp`, `platform/clock.hpp` (alias) | header-only | `tests/unit/time_test.cpp` (`kritva_core_time`), `tests/unit/platform_test.cpp` (`kritva_core_platform`) | commits `6ed8762`, `e1e6cfb`; 16/16 ctest |

**Acceptance:** No requirement referenced by the implementation may remain undefined.

## 4. Functional Acceptance

- [x] AC-001: `time/clock.hpp` is the canonical IClock abstraction.
- [x] AC-002: Clock domain semantics must distinguish monotonic and realtime clocks.
- [x] AC-003: Callers must not silently compare timestamps from incompatible clock domains.
- [x] AC-004: `platform/clock.hpp` may remain only as a compatibility alias if needed.
- [x] AC-005: If a compatibility alias remains, it must not become a second independent contract.
- [x] AC-006: Do not implement OS clocks in Core.
- [x] AC-007: Do not add timers, scheduling, or callbacks beyond the existing contract.

## 5. Test Acceptance

- [x] TEST-001: Clock interface compile test
- [x] TEST-002: Timestamp/clock domain contract test if supported
- [x] TEST-003: Compatibility include test if alias retained
- [x] TEST-004: Timer interaction compile/API regression

## 6. Regression Acceptance

- [x] All known regression tests pass.
- [x] `ctest --test-dir build --output-on-failure` passes.
- [x] No previously passing test is removed or disabled without explicit review.

## 7. Build Acceptance

- [x] Clean configure succeeds.
- [x] Clean build succeeds.
- [x] No new compiler errors.
- [x] No new unexplained compiler warnings.

## 8. Coverage Acceptance

- [x] Coverage is generated/reviewed if configured.
- [x] New logic has appropriate test coverage.
- [x] Any material uncovered branch is documented.

## 9. Sanitizer / Static Analysis Acceptance

- [x] Required configured sanitizer runs pass.
- [x] Required configured static analysis passes.
- [x] Any existing unrelated finding is explicitly identified rather than hidden.

## 10. Scope Acceptance

- [x] No Runtime Manager implementation added.
- [x] No platform-specific implementation added to Core.
- [x] No unrelated refactoring.
- [x] Public API changes are limited to this task's contract needs.

## 11. Documentation Acceptance

- [x] Relevant API/requirements documentation updated.
- [x] Requirement IDs are traceable.
- [x] No documentation contradicts the implementation.

## 12. Git Acceptance

- [x] Working tree was clean before implementation.
- [x] Diff reviewed.
- [x] Commit contains only this task's logical changes.
- [x] Exact commit message used:

```text
refactor(core): canonicalize clock abstraction
```

- [x] Commit hash recorded: `6ed8762` (initial), `e1e6cfb` (review round 1 follow-up).

## 12a. Implementation Evidence (Claude)

- Commit: `6ed8762` `refactor(core): canonicalize clock abstraction` (R02-004 accepted at `dcfa7a5`).
- Starting point: `platform::IClock` was already a `using` alias of `time::IClock` (commit `4b7e1e0`); this task completes the contract around it rather than restructuring it.
- Files changed: `include/kritva/core/time/clock.hpp`, `include/kritva/core/types/timestamp.hpp`, `include/kritva/core/platform/clock.hpp` (documentation only; no declaration changed), `tests/unit/time_test.cpp`, `tests/unit/platform_test.cpp`, `REQUIREMENTS.md` (CORE-TIME-001), `API.md` (section 17).
- Contract decisions (for reviewer confirmation):
  1. `time/clock.hpp` is the canonical `IClock`. `platform/clock.hpp` is a documented compatibility alias: same type, no members, cannot diverge; adapters should implement `time::IClock` directly.
  2. The alias is NOT marked `[[deprecated]]`: it is used by `platform_test.cpp` and `core.hpp`, so the attribute would add warnings (acceptance requires no new warnings). It is documented as deprecated instead.
  3. The earlier comment "Planned removal: R0.3 or later" was replaced with "no removal date is set; removal is a public API compatibility change requiring human review", so R02-005 does not commit R03 to a removal.
  4. Each `IClock` instance has one fixed `ClockDomain` for its lifetime.* MONOTONIC: non-decreasing, unspecified epoch, only differences meaningful. REALTIME: wall clock, may step; adapter documents the epoch.
  5. Timestamps of different domains are incomparable. `Timestamp::operator==` already includes the domain; `Timestamp` has no `<`, `<=>` or `-`, which is now documented as intentional and locked by tests. No new helper API was added.
  6. Same domain is necessary but not sufficient for comparison (two MONOTONIC clocks on different devices need not share an epoch).*
  7. `now()` is `const noexcept`; allocation/blocking/latency/thread-safety are adapter properties (round 2 below: thread-safety is adapter-defined, not a Core guarantee); no hard-real-time claim.
  8. Timers/scheduling/callbacks are not part of `IClock`; `ITimer` stays an independent contract (test locks that neither derives from the other and that one class can implement both).
- Tests added: no `<`/`-` on `Timestamp` (SFINAE static_asserts), trivially-copyable, cross-domain inequality with identical nanoseconds, defaults and negative ns, per-instance domain stability and `now()` noexcept, alias identity via references, clock+timer co-implementation.
- Build: `rm -rf build && cmake -S . -B build && cmake --build build -j$(nproc)` — 0 warnings.
- Tests: `ctest --test-dir build` — 16/16 passed (includes `kritva_core_time`, `kritva_core_platform`).
- Sanitizers: separate Debug build `-fsanitize=address,undefined` — 16/16 passed.
- Coverage: `make coverage` — 98% overall (unchanged).
- `time/clock.hpp`, `platform/clock.hpp`, `types/timestamp.hpp` each compile standalone with `-Wall -Wextra`; header check passed; `git diff --check` clean.
- `make format-check`/`make lint`: TODO stubs, not executed.
- Known limitations: no runtime check can detect domain mixing by callers who use `nanoseconds()` directly; the contract can only document it. Epoch/source identity within a domain is not representable in `Timestamp`. Items marked * are choices needing reviewer confirmation (public API semantics, AGENTS.md section 12).

### Review round 1 — CHANGES REQUIRED (ChatGPT, on `6ed8762`)

All design decisions approved (fixed domain per instance, compatibility alias without `[[deprecated]]`, removal of the R0.3 removal promise, Timestamp domain behavior, known `nanoseconds()` limitation). One wording correction required: "implementations should be safe for concurrent `now()` calls" conflicts with "thread-safety belongs to the adapter" and must not become an implicit Core-wide guarantee.

### Review round 2 — follow-up commit `e1e6cfb` `fix(core): clarify IClock thread-safety is adapter-defined`

`6ed8762` is unchanged (not amended). `time/clock.hpp` now states: thread-safety is adapter-defined; `IClock` imposes no universal guarantee; an adapter that supports concurrent `now()` documents that guarantee, one that does not documents the restriction; callers must not assume concurrent `now()` is safe. `API.md` and `REQUIREMENTS.md` (CORE-TIME-001) match. Per review, `platform::IClock` is now described as a compatibility alias / migration path with no removal date (comment wording only; no `[[deprecated]]`). No implementation or test change.

Files changed in round 2: `include/kritva/core/time/clock.hpp`, `include/kritva/core/platform/clock.hpp` (comments only), `API.md`, `REQUIREMENTS.md`.

Round 2 validation: clean build 0 warnings; `ctest --output-on-failure` 16/16; ASan+UBSan build 16/16; `make header-check` passed; `git diff --check` clean.

## 13. Evidence Required From Codex/Claude

Provide the following in the implementation response:

1. Summary of changes
2. Exact files changed
3. Requirement IDs addressed
4. New tests added/modified
5. Build command and output summary
6. Task-specific test command/output summary
7. Full regression command/output summary
8. Coverage result
9. Sanitizer/static-analysis result
10. Git commit hash
11. Known limitations
12. Any follow-up recommendation

## 14. Independent Reviewer Decision

Reviewer: ChatGPT

- [ ] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Review notes:

TBD

## 15. Acceptance Rule

The task cannot be marked ACCEPTED solely because the code compiles. All applicable functional, test, regression, quality, scope, documentation, and Git criteria must have evidence.
