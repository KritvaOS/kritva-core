# Changelog

## 0.2.0 — Core Contract Hardening (KF-CORE-R02)

Contract-hardening release of the Kritva Core foundation. No Runtime Manager,
platform implementation, or new top-level domain was added.

### Contracts
- `Result<T>` / `Result<void>`: explicit success/failure semantics, documented
  copy/move behavior, and a `failure()` precondition (`ErrorCode::NONE` is
  rejected; asserted in debug builds).
- `Status` / `StatusCode`: self-contained headers with documented construction,
  mutability, allocation, and thread-safety.
- Statistics: `Counter` (wraps modulo 2^64), `Gauge` (stored as-is), and the
  `Statistics` aggregate documented, including field meanings and units.
- Scheduler (`platform::IScheduler`): `TaskConfig` field semantics, entry and
  context lifetime, create/start/stop behavior, `TaskId` rules, and error
  mapping. Contract only; adapters own platform policy.
- Clock: `time::IClock` is the canonical clock contract with fixed clock
  domains; `Timestamp` has no ordering or subtraction across domains.
  `platform::IClock` remains a compatibility alias of the same type.

### Quality
- Requirement/API/test traceability closed: all referenced IDs defined, process
  requirements traced, and `make traceability-check` added to `make check`.
- New `kritva_core_foundation_contract` test, including an exhaustive 8x8
  lifecycle transition matrix; reusable `IScheduler` conformance checks.
- Validated from a fresh clone: ASan/UBSan, TSan, strict warnings, GCC
  `-fanalyzer`, 98% line coverage, 17/17 tests.

### Documentation and build
- Lifecycle transition table documented in `ARCHITECTURE.md`.
- `make coverage` now works from a fresh clone.

### Known follow-ups
- clang-tidy / cppcheck / clang-format are not configured (`make lint` and
  `make format-check` are placeholders).

## 0.2.0-proposed
- Freeze Core/platform boundary.
- Add runtime, messaging, time and platform contracts.
- Replace header-only CMake target with compiled library.
- Add lifecycle validation and initial contract tests.
- Add machine-readable architecture manifest.
