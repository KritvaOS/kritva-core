# R06 Component Integration Freeze

## Purpose

Freeze production API and Runtime/Component semantics before final R0.6 validation.

## Entry criteria

- R06-005 accepted
- R06-006 accepted
- component/context integration tests pass
- all pre-R0.6 regression tests remain green
- public APIs only in integration tests
- no unresolved production API change

## Freeze requirements

- no Runtime lifecycle redesign;
- no Component lifecycle signature drift;
- no hidden context ownership;
- no automatic platform-service lifecycle;
- no background Core execution;
- no test-only production hooks;
- production diff from API freeze point is clean unless explicitly approved.

## Decision

PASS / HONORED / CHANGES REQUIRED / BLOCKED
