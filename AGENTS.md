# Kritva Core Engineering Agent Instructions

## 1. Purpose

These instructions apply to the `kritva-core` repository.

Kritva Core is the platform-independent foundation of the Kritva Open Robotic Computing Platform.

> Kritva connects intelligence to physical action.

## 2. Authority

Consult, in order:

1. `ARCHITECTURE.md`
2. `REQUIREMENTS.md`
3. `API.md`
4. `TESTING.md`
5. parent KritvaOS architecture for cross-repository boundaries
6. relevant ADRs and repository instructions

Do not silently contradict the parent Kritva architecture.

## 3. Core Boundary

Core Foundation owns Identity, Lifecycle, Status, Health, Statistics, Error/Fault, Result, Events, Capability, Configuration, Version, Timestamp, Duration, and Metadata.

Do NOT introduce ROS2, DDS, EtherCAT, hardware drivers, vendor SDKs, motor control, sensor implementations, AI inference, motion planning, cloud services, databases, or robot-specific application logic into Core Foundation.

If functionality appears to require one of these, propose the architectural boundary first.

## 4. Public API Boundary

Public headers belong under `include/kritva/core/`. Implementation belongs under `src/`. Tests belong under `tests/`.

## 5. API-First Development

Before coding:

1. identify requirement
2. identify owning Core domain
3. define API
4. define behavior
5. define error behavior
6. define tests
7. implement
8. verify

## 6. Intern Development Model

Core development may be performed by interns in small bounded work packages. Do not assign the entire Core architecture as one implementation task.

Preferred sequence:

```text
Identity + Version
    ↓
Lifecycle
    ↓
Status + Health
    ↓
Error + Result
    ↓
Statistics
    ↓
Events
    ↓
Capability
    ↓
Configuration
```

Each work package requires requirement IDs, API definition, implementation scope, tests, review criteria, and completion criteria.

## 7. AI Development Model

AI agents are engineering assistants, not autonomous architects.

Agents must inspect before editing, follow requirements, preserve boundaries, make small changes, add tests, report assumptions/failures, avoid speculative APIs, and review the final diff.

Do not silently redesign Core, add unrelated dependencies, remove tests to pass CI, weaken validation, claim unexecuted tests passed, or invent hardware behavior.

## 8. Real-Time Discipline

Any API intended for real-time use must document allocation, blocking, synchronization, complexity, thread-safety, and failure behavior.

## 9. Dependency Discipline

Evaluate necessity, licensing, portability, security, maintenance, build complexity, runtime overhead, and real-time impact before adding dependencies.

## 10. Testing

Code is not complete because it compiles. Run relevant unit tests, contract tests, build validation, and format checks.

## 11. Git Discipline

Prefer small logical commits. Do not perform destructive Git operations without explicit authorization.

Examples:

```text
feat(core): add identity foundation
test(core): add identity contract tests
feat(core): add lifecycle foundation
docs(core): define lifecycle API
```

## 12. Human Review

Human review is required for public API compatibility changes, architectural boundary changes, safety/security behavior, real-time guarantees, and major dependency changes.

## 13. Parent Repository Integration

`kritva-core` may be integrated into KritvaOS as a Git submodule. The parent repository should reference a deliberate Core commit/tag. Do not modify the parent's submodule pointer from this repository.

## 14. Final Rule

> Understand the architecture before changing the code.

## 15. Program Governance Rules

These rules apply in addition to everything above and follow the program's governance (D-035, D-040).

- Never commit credentials, tokens, private keys or production data. Use obviously fake values in examples. The `secret-scan` workflow runs on every push and pull request; do not add allowlist entries to silence it, propose them to the owner.
- Do not add `Claude-Session:` or session-URL trailers to commit messages or pull request descriptions; `commit-message-check` rejects them. A `Co-Authored-By:` line is allowed.
- Commit and PR standard (kritva-program D-053, v2/planning/ACCEPTANCE_STANDARD.md section 4): `type(scope): summary` (types feat, fix, docs, refactor, test, build, ci, chore, perf, revert, security) with one `Task:` or `Tasks:` footer, `Tasks: None (reason)` only on a docs or chore commit that implements no task; the merge method is a merge commit; PR text follows `.github/PULL_REQUEST_TEMPLATE.md`. Check with `python3 scripts/security/check_commit_format.py --range origin/main..HEAD`. This repository has not adopted the format in CI yet (PD-019), so the check is advisory here.
- Work on a branch and open a pull request; do not push to `main`, force-push shared branches or rewrite history without the owner's explicit approval.
- `status/status.json` is a generated planning report. Regenerate it, do not edit it by hand. Repository edits never create program credit: credit comes only from the baseline, approvals and evidence recorded in `KritvaOS/kritva-program`.
- Program decisions are recorded in `KritvaOS/kritva-program` (`v2/DECISIONS.md`). Dinesh decides; AI advice is advisory.
- `main` requires `secret-scan`, `commit-message-check` and Code Owner review (owner statement, D-040; not independently verified). With one owner the PR author cannot self-approve: use the owner bypass or a second reviewer.

README changes: run `make check-readme` after editing README.md. It is a structural check only (metadata fields, topic headings, relative links, placeholders); it does not show that the content is accurate, so review the content against the repository as well.
