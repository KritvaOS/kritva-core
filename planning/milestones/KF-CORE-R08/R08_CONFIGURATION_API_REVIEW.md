# R08 Configuration API Review

## Gate

PASS required before R08-004 begins.

## Entry Criteria

- R08-001 accepted.
- R08-002 accepted.
- R08-003 accepted.
- Focused configuration tests pass.
- Existing regression suite remains green.
- Proposed requirement traceability is complete for the reviewed surface.

## Review Checklist

- [ ] `configure()` valid-state semantics are frozen.
- [ ] Invalid-state behavior is frozen and state-preserving.
- [ ] Configuration input ownership/lifetime is explicit.
- [ ] Partial application is prohibited on failure.
- [ ] Core structural validation versus Component semantic validation is explicit.
- [ ] `ConfigurationVersion` means schema/contract compatibility version only.
- [ ] No generic runtime revision/history API is introduced.
- [ ] Runtime forwarding semantics are frozen.
- [ ] Configuration failure does not enter Runtime FAULT.
- [ ] Configuration is independent of Status and Health.
- [ ] `ComponentContext` remains unchanged.
- [ ] No dynamic reconfiguration API is introduced.
- [ ] No configuration service/event/persistence infrastructure is introduced.
- [ ] Public headers are self-contained.
- [ ] Mutation evidence is sufficient for contract-sensitive behavior.

## Production Freeze Boundary

The reviewer records the exact production commit that establishes the frozen R08 configuration semantics. After this point, any production API or semantic change requires explicit architecture review.

## Evidence (submitted 05-10-2026)

Baseline: R08-001 `605516b`, R08-002 `6efeaac`, R08-003 `bdb4b93` (all ACCEPTED; head `efd6675`). Consult decisions (no amendments) are recorded in the reviewer exchange: contract text only in the configuration headers, `validate()` documented with no new check, error mapping INVALID_STATE / INVALID_ARGUMENT / CONFIGURATION_ERROR.

### Production diff against `kritva-core-r0.7` (`include/` and `src/`)

| File | Change |
|---|---|
| `configuration/configuration.hpp` | contract text only (+127 lines): lifecycle eligibility (CORE-CFG-004, 011), ownership and detachment (CORE-CFG-005), atomic application (CORE-CFG-006), validation boundary and error mapping (CORE-CFG-007) |
| `configuration/configuration_version.hpp` | contract text only (+34 lines): schema/contract compatibility version (CORE-CFG-008) |

`git diff kritva-core-r0.7 HEAD -- src` is **empty**. `runtime/component.hpp`, `runtime/runtime_manager.hpp`, `runtime/component_context.hpp`, `configuration/parameter.hpp` and every other header are byte-identical to the released tag. No type, signature, enumerator or behavior was added or changed anywhere; the "API" frozen by this review is therefore the **normative contract text** and the unchanged public surface it governs.

### Review checklist evidence

| Check | Evidence |
|---|---|
| `configure()` valid-state semantics are frozen | valid only from UNKNOWN and STOPPED; exhaustive state tests on the component and the Runtime (R08-001) |
| Invalid-state behavior is frozen and state-preserving | INVALID_STATE, source the component, no other effect; the injected failure is not even consumed (nothing past the state check runs) |
| Input ownership/lifetime is explicit | caller owns the object; const reference; no retention of address, reference, pointer or view; caller mutates then destroys its heap object (R08-002); the Runtime forwards the caller's own object, fixed topology or not |
| Partial application is prohibited on failure | all-or-nothing; previous state kept, first-ever failure leaves the initial state; six broken components detected |
| Structural versus semantic validation is explicit | `validate()` is the single structural entry point; it cannot fail for a constructible Configuration by design; the container stores any value; semantics are the component's (R08-003) |
| `ConfigurationVersion` means schema/contract compatibility only | alias of `Version`; not a revision, counter, transaction id, timestamp or history; no version member on Configuration, Runtime or context (concepts) |
| No generic runtime revision/history API | member-detection for version/revision/generation/transaction_id/history/set_version on Configuration, RuntimeManager and ComponentContext |
| Runtime forwarding semantics are frozen | the same caller object to every component; dependency order, first failure stops, error returned unchanged — pre-existing R0.3 behavior, restated and tested here, with the full order/permutation proof assigned to R08-005 |
| Configuration failure does not enter Runtime FAULT | tested (a failed Runtime configure leaves UNKNOWN and no fault); mutant entering FAULT detected |
| Configuration is independent of Status and Health | contract text; the Runtime never reads them (R0.7 spies); R08-005 integration proves it for configuration |
| `ComponentContext` remains unchanged | byte-identical; no configuration member (concepts) |
| No dynamic reconfiguration API | no reconfigure/set_parameter/get_parameter/apply/update/`configuration()` on Component, RuntimeManager or ComponentContext (concepts); lifecycle keeps its eight states |
| No configuration service/event/persistence infrastructure | no store, registry, server, broker, persistence, event or background worker; dependency scan clean |
| Public headers are self-contained | one TU per public header (`kritva_core_header_checks`) passes |
| Mutation evidence is sufficient | 11 (lifecycle eligibility and Runtime), 3 + 6 broken components (ownership and atomicity), 12 (validation, version, error boundary), survivors closed |

### Validation (head `efd6675`; production unchanged since `bdb4b93`)

`ctest` 54/54 in Debug, Release, ASan+UBSan, strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror` and TSan (ASLR off via `setarch -R`); 0 warnings; GCC `-fanalyzer` clean; `make check` and traceability: 96 requirements, 95 traced, 0 errors, `CORE-CFG-004` to `008` and `011` defined once; coverage 618/625 unchanged; `git diff --check` clean.

### Open issues (none blocking)

Carried forward: 32-bit scheduler affinity mask; conformance level-2 mutation gap; `make lint`/`make format-check` stubs; documented non-owning lifetime UB. Honest note: because the contract rules for Components (non-retention, atomicity) cannot be enforced by Core, they are verified by reusable conformance checks that R08-004 builds; R08-004 and R08-005 add tests only.

## Decision

`PASS / FROZEN / CHANGES REQUIRED / BLOCKED`

## Evidence

Record the review evidence commit and accepted production baseline here after the gate is executed.
