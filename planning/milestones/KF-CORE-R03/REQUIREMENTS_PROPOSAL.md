# KF-CORE-R03 — Requirements Proposal

These IDs are planning proposals. They become authoritative only after the corresponding implementation/contract is reviewed and the IDs are added to the project's authoritative `REQUIREMENTS.md`.

| ID | Proposed Requirement | Task |
|---|---|---|
| CORE-RT-001 | Platform-independent component contract with stable identity and explicit lifecycle semantics | R03-001 |
| CORE-RT-003 | Component registry with deterministic registration and lookup semantics | R03-002 |
| CORE-RT-004 | Represent dependencies using stable component identity | R03-003 |
| CORE-RT-005 | Detect dependency cycles and provide deterministic dependency ordering | R03-003 |
| CORE-RT-006 | Runtime manager orchestrates registered components without an OS-specific execution mechanism | R03-004 |
| CORE-RT-007 | Runtime lifecycle is deterministic and dependency aware | R03-005 |
| CORE-RT-008 | Runtime failure propagation and explicit recovery/reset are deterministic without implicit automatic retry | R03-006 |
| CORE-RT-009 | Platform-independent runtime integration tests cover registration, dependency, lifecycle and failure behavior | R03-007 |
| CORE-RT-010 | R03 runtime candidate passes final build, test, sanitizer, traceability, regression and release-candidate validation gates | R03-008 |

## Numbering reconciliation (against authoritative `REQUIREMENTS.md`, commit `655c1dd`)

The authoritative file defines exactly two runtime IDs:

| ID | Authoritative definition | Status |
|---|---|---|
| CORE-RT-001 | Lifecycle-managed component contract (extended by R03-001) | in use |
| CORE-RT-002 | Runtime contract (`runtime/runtime.hpp`, R0.1/R0.2) | in use, not renumbered |

The first draft of this proposal used `CORE-RT-002` for the component registry, which collided with the authoritative `CORE-RT-002`. IDs `CORE-RT-003` and above are unused in `REQUIREMENTS.md`, so the proposed IDs after `CORE-RT-001` were shifted by one. The existing `CORE-RT-002` is untouched and remains authoritative. R03-004 implements the existing `runtime::Runtime` interface; `CORE-RT-006` defines the concrete Runtime Manager behavior within that contract. A genuine incompatibility must return to architecture review.

| Draft ID | Subject | Final proposed ID |
|---|---|---|
| CORE-RT-002 | Component registry | CORE-RT-003 |
| CORE-RT-003 | Dependency representation | CORE-RT-004 |
| CORE-RT-004 | Cycle detection and ordering | CORE-RT-005 |
| CORE-RT-005 | Runtime manager | CORE-RT-006 |
| CORE-RT-006 | Runtime lifecycle | CORE-RT-007 |
| CORE-RT-007 | Runtime failure and recovery | CORE-RT-008 |
| CORE-RT-008 | Runtime integration tests | CORE-RT-009 |
| CORE-RT-009 | R03 validation gates | CORE-RT-010 |

Each ID is added to the authoritative `REQUIREMENTS.md` only by the task that implements it, after that task's review. Before each addition, check `REQUIREMENTS.md` for collisions (`make traceability-check` fails on duplicate definitions).

## Cross-Cutting Policy

R03 also reviews, but does not prematurely implement in R03-001..003:

- Error semantics
- Warning semantics
- Info/diagnostic semantics
- Event versus message distinction
- Statistics update policy
- Logging backend boundary

R02 primitives remain authoritative:

- `Result<T>`
- `Status`
- `Error`
- `Counter`
- `Gauge`
- `Statistics`

R03 defines how runtime behavior uses those primitives.
