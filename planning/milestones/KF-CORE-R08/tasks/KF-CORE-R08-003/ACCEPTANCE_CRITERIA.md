# KF-CORE-R08-003 — Acceptance Criteria

## Task Info

| Field | Value |
|---|---|
| Task | KF-CORE-R08-003 |
| Status | ACCEPTED |
| Primary commit | `feat(core): define configuration version and validation contract` |
| Reviewer | ChatGPT — independent acceptance gate |
| Estimated effort | 2–3 ED |
| Dependency | R08-002 |

## Objective

Freeze `ConfigurationVersion` semantics and the Core-vs-Component validation boundary.

## Proposed Requirements Traceability

CORE-CFG-007, CORE-CFG-008, CORE-CFG-011

The implementing task must update authoritative root `REQUIREMENTS.md` only as part of acceptance reconciliation.

## Acceptance Criteria

- `ConfigurationVersion` is documented and tested as a schema/contract compatibility version.
- It is explicitly not a runtime revision counter, transaction id, update count or history mechanism.
- R0.8 does not create a generic configuration revision API.
- Existing `Configuration::validate()` remains the Core structural validation entry point.
- Component-specific semantic validation remains Component-owned.
- Invalid schema/contract compatibility is reportable using existing configuration/error semantics without adding a per-parameter error-code taxonomy.
- The API documentation does not imply that Core knows domain-specific parameter ranges/enumerations/defaults.
- No unnecessary new version field or API is added merely to expose the alias.
- Mutation testing covers schema/validation boundary conditions.

## New Tests

- ConfigurationVersion semantic tests.
- Structural-valid/semantic-invalid examples.
- Version compatibility rejection tests in the reference Component.
- Mutation tests for boundary confusion between Core structural and Component semantic validation.

## Regression Tests

- Existing configuration unit/contract tests remain green.
- Existing Version tests remain green.

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

## Coverage / Quality

- Newly added executable lines must be covered.
- No unexplained coverage exclusion.
- Overall R08 coverage remains at least 98% and has no unexplained regression greater than 1 percentage point from the R0.7 98.9% baseline.
- Required sanitizer and strict-analysis evidence is supplied at milestone validation.

## Expected Files Changed

Only files directly required for this task, its tests, and traceability/documentation reconciliation. No unrelated files.

## Git Commit

```text
feat(core): define configuration version and validation contract
```

## Evidence Required from Implementor

- `git status` before and after implementation
- `git diff --check`
- build and test output
- focused test evidence
- mutation evidence for contract-sensitive behavior
- diff/stat summary
- commit SHA
- explicit mapping from every acceptance criterion to objective evidence

## Implementor Evidence

Primary commit: `bdb4b93` `feat(core): define configuration version and validation contract`. **No production type, signature or behavior change:** contract text (and tags) in `configuration/configuration.hpp` and `configuration/configuration_version.hpp` only; `src/`, `parameter.hpp` and every `runtime/` header are byte-identical to `kritva-core-r0.7`. `CORE-CFG-011` is not redefined (it was defined once at R08-001 and is referenced here, as you approved).

- Contract (CORE-CFG-007, CORE-CFG-008; API.md section 46): validation has two owners; Core owns only structural validation, whose single entry point is `Configuration::validate()` (const, side-effect free, deterministic) with one invariant (non-empty Parameter name) enforced where a Parameter enters the container (`set()` rejects an empty name with INVALID_ARGUMENT atomically), so validate() succeeds for every Configuration constructible through the public API — documented as by design, with no new check added (your Q2); the Component owns all semantic validation and Core knows no names, types, ranges, enumerations, defaults, required sets or units; no ErrorCode is added and the documented mapping is INVALID_STATE (lifecycle), INVALID_ARGUMENT (structurally malformed), CONFIGURATION_ERROR (component semantic or schema incompatibility) with source the component, returned unchanged by Core and the Runtime (your Q3). `ConfigurationVersion` stays an alias of `Version` meaning schema/contract compatibility version, explicitly not a revision, update/generation counter, transaction id, timestamp or history; Core never creates, increments, stores, compares or interprets it; no version field or revision API is added to Configuration; compatibility is the component's policy (Core reserves no parameter name).
- Tests (`kritva_core_configuration_validation`, 11 functions): validate() signature (const) and repeatable with no effect; an empty name rejected atomically at set(); validate() succeeds for every constructible configuration (ten odd names incl. whitespace, 4096 characters, UTF-8, copies and moves); the container accepts any value as given (empty string, int64 min/max, NaN and infinity stored unnormalized, a type change on replacement) and exposes no range/default/schema member; a structurally valid configuration is semantically rejected by a schema-owning component with CONFIGURATION_ERROR, nothing applied and the previous value kept; the error-code mapping and the unchanged enumeration (CONFIGURATION_ERROR 9, UNSUPPORTED 10, INTERNAL_ERROR 11); the Runtime returns the component's CONFIGURATION_ERROR unchanged (code, source, message), stops at the first failure and applies nothing; ConfigurationVersion is exactly Version (trivially copyable, equality, to_string); Configuration, RuntimeManager and ComponentContext have no version/revision/generation/transaction/history/set_version member and repeated application produces no Core-side counter; schema compatibility is the component's policy (same major accepted, five other majors and a missing schema rejected, previous state kept, no reserved parameter name).
- Mutation evidence (12 mutants, each reverted, **12/12 detected**; the NaN-normalizing mutant was re-checked to compile and be killed by the test itself): set() accepting empty names, rejecting a single space, rejecting names over 64 characters, rejecting duplicates, normalizing NaN, clearing the container above 6 parameters; validate() failing above 6 parameters or when empty; set() returning CONFIGURATION_ERROR instead of INVALID_ARGUMENT; the Runtime rewriting a component's configuration error code, message or source.
- Regression: ctest 54/54 in Release, ASan+UBSan, strict `-Werror` (0 warnings), TSan, Debug; GCC `-fanalyzer` clean; coverage 618/625 unchanged; `make check`, traceability 96 requirements, 95 traced, 0 errors (`CORE-CFG-007`, `CORE-CFG-008` defined with rows); `git diff --check` clean.
- Out of scope confirmed: no new ErrorCode or taxonomy, no revision/history API, no new structural check, no version field in Configuration.

## Reviewer Decision

| Item | Result |
|---|---|
| Reviewer | ChatGPT (via the external review session) |
| Decision | **PASS** |
| Accepted commit | `bdb4b93` (evidence `0ea3ea3`) |
| Evidence reference | Implementor Evidence above |
| Date | 05-10-2026 |

Reviewer notes: no production type, signature or behavior change (contract text in `configuration.hpp` and `configuration_version.hpp`); the two-owner validation boundary with `validate()` as the single structural entry point and no new check, the unchanged `ErrorCode` set and mapping, `ConfigurationVersion` as schema/contract compatibility version only, no revision or history API and component-owned compatibility policy accepted; 12/12 mutants detected. `CORE-CFG-007` and `CORE-CFG-008` are authoritative.

**Reviewer Decision: PASS — KF-CORE-R08-003 is ACCEPTED.**
