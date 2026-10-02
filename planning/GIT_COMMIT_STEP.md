# Kritva Core — Git Commit and Release Procedure

## Per Task

```bash
git status
git diff --check
cmake -S . -B build
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
git diff --stat
git diff
git status
git commit -am "<exact task message>"
git rev-parse HEAD
```

Use `git add` explicitly when new files are created.

## Commit Rules

- One logical task has one primary implementation commit.
- The task acceptance document defines the exact commit message.
- Do not mix unrelated changes.
- Do not rewrite accepted history unless explicitly instructed.
- Fix follow-up defects with a new focused commit if the task is already reviewed.

## Milestone Release

After all eight tasks are accepted:

1. Update `MILESTONE_STATUS.md`.
2. Update `CHANGELOG.md`.
3. Run a clean build.
4. Run full regression.
5. Run final quality checks.
6. Perform independent milestone review.
7. Commit documentation changes.
8. Create the release tag.

Example:

```bash
git tag -a kritva-core-r0.2 -m "Kritva Core R0.2"
git show kritva-core-r0.2
```
