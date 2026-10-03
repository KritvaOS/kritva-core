# KF-CORE-R05 Planning Package

This package defines the approved R0.5 architecture, task sequence, acceptance criteria, testing rules and release gates.

## Start Here

1. `MILESTONE.md`
2. `R05_DESIGN_DECISIONS.md`
3. `IMPLEMENTATION_SEQUENCE.md`
4. `REQUIREMENTS_PROPOSAL.md`
5. `R05_TESTING_AND_COMMIT_POLICY.md`
6. Task `TASK.md` and `ACCEPTANCE_CRITERIA.md` files
7. `R05_PLATFORM_API_REVIEW.md`
8. `R05_PLATFORM_INTEGRATION_FREEZE.md`
9. `R05_RELEASE_GATE.md`

## Important

R0.5 is planned only. No implementation commit is authorized by this package alone.

The independent reviewer controls task acceptance and release-gate decisions.


### R05-001 Review Update

R05-001 acceptance criteria were tightened following architecture review to make non-ownership, adapter lifetime, service pointer/capability consistency, deterministic null-context behavior, and mandatory Runtime/platform integration evidence explicit.
