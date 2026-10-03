# KF-CORE-R04 — Platform Integration Freeze

## Purpose

Freeze the accepted platform contracts and conformance suite after R04-006.

## Entry Criteria

- R04-001 through R04-006 accepted.
- Platform API Review is PASS / FROZEN.
- Platform conformance suite passes.
- Requirements traceability is clean.

## Freeze Rule

After this gate, no production platform API or platform semantic change is permitted during R04-007/R04-008 without explicit architecture review.

## Required Verification

- Compare production headers/sources against the Platform API Review baseline.
- Verify no platform-specific implementation has entered Core.
- Verify conformance tests use only public APIs.
- Verify R0.3 Runtime behavior remains unchanged.
- Verify no thread/background execution was introduced.
- Record the exact freeze commit.

## Decision

Status: PASS / HONORED — platform API frozen at production commit `f7231c1` (evidence: `R04_PLATFORM_INTEGRATION_FREEZE_EVIDENCE.md`, commit `84046b0`)

Possible outcomes:
- PASS / HONORED
- CHANGES REQUIRED
- BLOCKED

Reviewer: ChatGPT architecture/review gate.

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Date | 04-10-2026 |
| Decision | **PASS / HONORED** |

**Production freeze point: `f7231c1`.** R04-001 through R04-006 are accepted (`d1c5f13`, `eb06fa0`, `67114bb`, `4af4756`, `f7231c1`, `460de87`).

Frozen contracts: `Callback` and the platform boundary rules (R04-001); `platform::IScheduler` (R04-002, including the documented 32-bit affinity mask limitation); `time::IClock`, its `platform::IClock` alias, and `time::ITimer` with `TimerMode` (R04-003); `platform::IWatchdog` (R04-004); `platform::IPlatformAdapter`, `PlatformInfo`, `PlatformService` (R04-005); the platform conformance suite in `tests/platform/` (R04-006). The R0.3 Runtime is byte-identical to `kritva-core-r0.3`.

**The only production change permitted after this freeze** is the additive `RuntimeManager::attach_platform()` / `platform()` extension defined by R04-007, an explicit exception reviewed on its own. R04-007 must not alter the Scheduler, Clock, Timer or Watchdog signatures or semantics, make the Runtime start or stop platform services automatically, introduce Core-owned threads, transfer adapter ownership, introduce singleton or service-locator behavior, or modify R0.3 lifecycle or recovery semantics.

Non-blocking open issues retained: the 32-bit scheduler affinity mask (documented frozen limitation); the level-2 mutation strictness gap of the conformance suite; the deferred `make lint` / `make format-check` stubs.

**Reviewer Decision: PASS / HONORED — KF-CORE-R04 Platform Integration Freeze is ACCEPTED.**
