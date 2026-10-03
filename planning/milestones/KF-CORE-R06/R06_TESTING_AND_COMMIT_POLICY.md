# KF-CORE-R06 — Testing and Commit Policy

## Required test layers

### Unit / contract

Every new public/production contract receives focused tests for:
- success;
- invalid arguments;
- unavailable resources;
- lifetime/ownership;
- side effects;
- determinism;
- copy/move behavior where applicable;
- failure propagation.

Mutation testing is required for contract-sensitive behavior.

### Integration

Integration tests must use only:
- public Core APIs;
- reference/fake services;
- deterministic fixtures.

No private member access and no production-only testing hooks.

### Regression

Every accepted task runs:

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
```

Final validation additionally runs Debug, Release, ASan+UBSan, TSan, strict warnings, GCC analyzer, traceability, install-consumer, self-containment and dependency/isolation audits.

## Commit policy

| Task | Exact commit message |
|---|---|
| R06-001 | `feat(core): define component execution context` |
| R06-002 | `feat(core): define operational context access policy` |
| R06-003 | `feat(core): define component context injection boundary` |
| R06-004 | `feat(core): bind context requirements and capabilities` |
| R06-005 | `test(core): add component context reference harness` |
| R06-006 | `test(core): add component context integration tests` |
| R06-007 | `test(core): complete R0.6 validation` |

One logical task = one primary implementation commit.
