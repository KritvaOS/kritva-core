# Kritva Engineering Contract

Status: DRAFT for owner approval (program decisions D-023 and PD-015). The contract is pinned only after the owner approves this text and the pin file records the approving commit and the file's SHA-256 (see section 9).

Scope: every repository in the Kritva V2 plan. Kritva Core is the pilot. This contract is the controlling convention for lifecycle, branch, commit and tag rules (D-023). It restates the approved acceptance standard (kritva-program `v2/planning/ACCEPTANCE_STANDARD.md`: PD-005 and PD-006 by D-080, PD-007 by D-047, PD-010 by D-053). Where the two differ, this contract wins and the standard is corrected.

This contract creates no program credit. Credit comes only from the baseline, approvals and evidence recorded in `KritvaOS/kritva-program`.

## 1. Authority and roles

1. Dinesh (owner) decides. AI output is advisory.
2. Decisions are recorded in `KritvaOS/kritva-program` (`v2/DECISIONS.md`, append-only).
3. Required approvals per repository come from that repository's `.github/kritva-gate.json` and the program manifest. Interim reviewer roles are not independent review and are never reported as such (D-047).
4. Approvals made before a pull request head changed are stale.

## 2. Branches and merging

1. Work happens on a branch and lands through a pull request. No direct push to `main`, no force-push of shared branches, no history rewrite without the owner's explicit approval.
2. The merge method is a merge commit (D-053).
3. The pull request title uses the commit subject style. The body follows `.github/PULL_REQUEST_TEMPLATE.md`.
4. A pull request merges only after the pull requests it depends on are merged.

## 3. Commit format

```text
type(scope): summary of at most 90 characters

Optional body.

Task: KOS-EXAMPLE-001
```

1. Types: feat, fix, docs, refactor, test, build, ci, chore, perf, revert, security. Scope is lower-case and free-form.
2. Exactly one footer line, `Task:` or `Tasks:`, with comma-separated task IDs.
3. A commit that implements no task is a `docs` or `chore` commit with `Tasks: None (reason)`. The reason is mandatory. No other type may use it.
4. Every other commit needs a task from the repository's task record. Do not reuse an unrelated ID and do not invent one.
5. No AI-session trailer or session URL (`Claude-Session:`, claude.ai session links). `Co-Authored-By` and `Signed-off-by` are allowed.
6. Checker: `python3 scripts/security/check_commit_format.py --range origin/main..HEAD`.

## 4. Task definition of done

A task is done only when all of these hold:

1. It has a stable ID and testable acceptance criteria in the repository's `ACCEPTANCE_CRITERIA.md`.
2. The change merged through the approved pull request process.
3. Every required check passed on the exact closing commit.
4. Required reviews, including any security or safety approval, are recorded against that commit.
5. Documentation and the changelog are updated.
6. Evidence points to immutable references: commit SHAs and CI run IDs, never branch names.
7. No blocking issue was converted into a pass. A waiver is a structured record (approver, scope, rationale, expiry or review condition) and is never reported as PASS.
8. Status is reported from the repository's canonical status record, not from commit counts.

Software acceptance (unit tests, simulation, mocks, versioned interfaces) is never reported as hardware or safety validation.

## 5. Milestone closure

A milestone closes only through an approved `COMPLETION_RECORD.md` in the repository. It names the milestone ID, scope, exact merge and release commits, test runs, the tag if one exists, the disposition of each criterion, deferred work, approvals and evidence. A merge, a passing CI run, a tag, or a task marked complete does not close a milestone.

A milestone may not start while a milestone it depends on is not closed, unless the program records a waiver (program decision D-112).

## 6. Test gates

Every repository exposes the same make targets: `check`, `lint`, `test-unit`, `test-integration`, `test-regression`, `test`, `docs-check`, `acceptance`. A target that is not implemented says so and exits with code 3 (`NOT_IMPLEMENTED`). An unimplemented gate is never a pass. A gate is `NOT_APPLICABLE` only through a structured, approved disposition.

A fixed defect adds a regression test. A breaking change to a public interface needs a MAJOR version, a changelog entry and an approved decision.

## 7. Tags

1. Release tags are annotated SemVer: `vMAJOR.MINOR.PATCH`.
2. Create a tag only after the approved merge and required checks. The tag message names the completion record and the exact commit.
3. A tag is evidence, not closure authority.

## 8. Required files and status

Each repository carries `ACCEPTANCE_CRITERIA.md`, `planning/{TASKS,MILESTONE,MILESTONE_STATUS,REVIEW_CHECKLIST,GIT_COMMIT_STEP,CHANGELOG}.md`, `AGENTS.md`, `SECURITY.md`, `CONTRIBUTING.md`, `LICENSE`, `Makefile`, `.github/CODEOWNERS`, and the `secret-scan` and `commit-message-check` workflows. No empty folders and no placeholder tasks.

`status/status.json` (schema 2.0) is generated, never edited by hand. A repository with no status source stays UNVERIFIED in the program view.

## 9. Pinning this contract

1. `docs/development/ENGINEERING_CONTRACT.pin.json` records `contract_path`, `sha256` (of the exact file bytes) and `commit_sha` (the commit on `main` that contains the approved text).
2. `python3 scripts/contract/check_contract_pin.py` fails when the file's SHA-256 differs from the pin, and, with `--verify-git`, when the file at `commit_sha` differs from the pin.
3. Changing this file needs a new owner approval, a new pin and a program decision. A change without a matching pin update fails the check.
4. The pin is recorded after the approving merge, in a separate pull request, because a commit cannot contain its own SHA.
