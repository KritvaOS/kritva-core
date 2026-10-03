# KF-CORE-R06-003 — Acceptance Criteria

## 1. Task information

- Task: `KF-CORE-R06-003`
- Title: Context Injection Without Runtime Lifecycle Change
- Milestone: `KF-CORE-R06`
- Estimated effort: 3–4 ED
- Dependency: R06-002
- Requirement proposed: `CORE-CTX-003`
- Exact primary commit message: `feat(core): define component context injection boundary`

## 2. Objective


Define how a Component obtains its execution context without giving RuntimeManager new automatic platform-service behavior and without changing the frozen Component lifecycle semantics unless explicitly approved.


## 3. Required acceptance criteria

### Contract correctness

- [ ] The implementation matches the approved R06 architecture.
- [ ] Ownership and lifetime semantics are explicit.
- [ ] No hidden lifecycle side effects are introduced.
- [ ] Existing R0.5 behavior remains intact.
- [ ] Error behavior is deterministic and documented.
- [ ] Invalid/unavailable paths are explicitly tested where applicable.

### API discipline

- [ ] No unrelated public API changes.
- [ ] No breaking change hidden inside the task.
- [ ] No generic registry/locator.
- [ ] No platform name/version inference.
- [ ] No concrete OS/vendor/platform dependency.

### Tests

- [ ] Focused unit/contract tests added or updated.
- [ ] Negative/error/invalid-state tests added where applicable.
- [ ] Mutation testing performed for contract-sensitive behavior.
- [ ] Existing regression suite remains green.
- [ ] Integration tests added when task changes an integration boundary.


## Mandatory quality checks

- `git diff --check`
- Debug build
- Release build
- full CTest regression
- strict `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`
- ASan + UBSan
- TSan with documented ASLR-disabled environment where required
- GCC `-fanalyzer`
- public-header self-containment
- traceability audit
- prohibited dependency/header audit
- install-consumer test
- coverage review
- clean working tree

Passing new tests alone is not acceptance evidence.

## Regression rule

The complete existing regression suite must remain green. R06 tests supplement; they do not replace R0.2–R0.5 coverage.

## Acceptance decision

Only the independent reviewer records:

- PASS
- CHANGES REQUIRED
- BLOCKED

Claude/Codex may provide objective evidence and mark evidence checkboxes but does not record the reviewer verdict.


## 4. Requirement traceability

- [ ] `CORE-CTX-003` is defined in authoritative `REQUIREMENTS.md` only after task acceptance.
- [ ] Artifact and verification references are recorded.
- [ ] `make traceability-check` passes or equivalent repository audit passes.
- [ ] Previous requirement IDs are not renumbered.

## 5. Build commands

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
```

## 6. Test commands

```bash
ctest --test-dir build --output-on-failure
make check
make traceability-check
```

Apply configured sanitizer, strict-warning and analyzer commands required by the repository.

## 7. Coverage

- [ ] Coverage baseline reviewed.
- [ ] No unexplained regression in coverage.
- [ ] Any uncovered new behavior is justified in evidence.

## 8. Static / sanitizer analysis

- [ ] ASan/UBSan pass.
- [ ] TSan pass where configured.
- [ ] Strict `-Werror` pass.
- [ ] GCC `-fanalyzer` pass.
- [ ] Public-header self-containment pass.
- [ ] Dependency/prohibited-header audit pass.

## 9. Expected files

Task-specific paths may include:

- `include/kritva/core/...`
- `src/...`
- `tests/unit/...`
- `tests/integration/...`
- `tests/contract/...`
- `tests/audit/...`
- `REQUIREMENTS.md`
- `API.md`
- `ARCHITECTURE.md`
- corresponding planning/evidence files

Only files relevant to the approved task may change.

## 10. Git commit

Primary implementation commit must be exactly:

```text
feat(core): define component context injection boundary
```

Do not amend an accepted commit. Use a focused follow-up commit if independent review identifies a correction before acceptance.

## 11. Evidence supplied by implementer

- [ ] Commit SHA
- [ ] Test output
- [ ] Regression output
- [ ] Quality checks
- [ ] Traceability result
- [ ] Coverage result
- [ ] Mutation results
- [ ] Diff summary
- [ ] Out-of-scope confirmation

## 12. Reviewer decision

**Reviewer only:**

- PASS
- CHANGES REQUIRED
- BLOCKED

Reviewer: ____________________  
Date: ____________________
