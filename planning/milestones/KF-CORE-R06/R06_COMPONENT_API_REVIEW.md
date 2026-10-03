# R06 Component API Review

## Purpose

Freeze the R06 Component Context public contract before integration implementation.

## Entry criteria

- R06-001 accepted
- R06-002 accepted
- R06-003 accepted
- R06-004 accepted
- focused unit/contract tests green
- full regression green
- traceability clean
- independent architecture review complete

## Review checklist

- [ ] Context ownership/lifetime is explicit.
- [ ] Context is not a service registry/locator.
- [ ] No Core-owned platform-service lifecycle exists.
- [ ] Existing `PlatformContext` remains authoritative for platform access.
- [ ] Context injection does not alter Runtime lifecycle semantics.
- [ ] Error propagation remains deterministic.
- [ ] Capability identity semantics are reused.
- [ ] No platform name/version inference.
- [ ] No breaking change is hidden.
- [ ] API is additive or explicitly architecture-approved.
- [ ] Public API self-containment passes.
- [ ] Contract/mutation evidence is sufficient.
- [ ] PASS / CHANGES REQUIRED / BLOCKED is recorded.

## Freeze rule

After PASS/FROZEN, R06-005 onward must not change the frozen production API except through an explicit architecture-review exception.
