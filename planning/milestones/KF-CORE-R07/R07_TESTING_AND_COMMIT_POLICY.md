# R0.7 Testing and Commit Policy

## Required Testing Layers

### Unit/Contract
- Every new or changed production contract has focused tests.
- Negative/failure behavior is mandatory.
- Mutation testing is required for contract-sensitive semantics.

### Integration
- Public APIs only.
- Validate Component/Runtime operational boundaries.
- Prove operational information does not alter lifecycle automatically.

### Regression
- Complete pre-R07 suite remains green.
- New tests do not replace historical tests.
- Determinism is tested where part of the contract.

## Commit Policy

One logical task = one primary implementation commit. Exact commit message is defined in each task acceptance file. No amend of accepted task commits. Corrective follow-up commits are focused and reviewable.
