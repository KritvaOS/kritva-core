# Kritva Core — R0.5 Git Commit and Release Procedure

## Per Task

```bash
git status
git diff --check
cmake -S . -B build
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
make check
make traceability-check
git diff --stat
git diff
git status
git add <new-or-changed-files>
git commit -m "<exact task message>"
git rev-parse HEAD
```

Run additional sanitizer/static-analysis/coverage/install checks required by the task acceptance criteria.

## R0.5 Exact Task Commit Messages

| Task | Exact implementation commit |
|---|---|
| KF-CORE-R05-001 | `feat(core): add platform context` |
| KF-CORE-R05-002 | `feat(core): define platform service requirements` |
| KF-CORE-R05-003 | `feat(core): define explicit platform service consumption` |
| KF-CORE-R05-004 | `feat(core): preserve runtime platform lifecycle boundary` |
| KF-CORE-R05-005 | `test(core): add platform integration reference harness` |
| KF-CORE-R05-006 | `test(core): add platform runtime integration tests` |
| KF-CORE-R05-007 | `test(core): complete R0.5 validation` |

## Commit Rules

- One logical task = one primary implementation commit.
- The task acceptance document defines the exact commit message.
- Do not mix unrelated changes.
- Do not amend accepted task history.
- Review fixes use focused follow-up commits.
- A later task must not silently alter an earlier accepted public API.
- Breaking changes stop implementation and return to architecture review.

## Freeze Gates

### After R05-004

R05 Platform API Review must PASS/FROZEN.

### After R05-006

R05 Platform Integration Freeze must PASS/HONORED.

### After R05-007

R05 Release Gate reviews the release candidate.

## Release

After all tasks are accepted:

1. Update release documentation.
2. Run clean Debug and Release validation.
3. Run full regression and quality checks.
4. Verify working tree is clean.
5. Obtain independent Release Gate PASS.
6. Create annotated tag:

```bash
git tag -a kritva-core-r0.5 -m "Kritva Core R0.5"
git show kritva-core-r0.5
```

7. Push branch and tag.
8. Verify remote refs:

```bash
git ls-remote origin refs/heads/main refs/tags/kritva-core-r0.5 refs/tags/kritva-core-r0.5^{}
```

9. Reconcile release documentation with actual remote state.
