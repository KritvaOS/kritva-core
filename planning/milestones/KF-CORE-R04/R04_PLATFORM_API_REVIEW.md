# KF-CORE-R04 — Platform API Review

## Purpose

Freeze the platform abstraction API after R04-001 through R04-004 and before capability/adapter integration work.

## Entry Criteria

- R04-001 accepted.
- R04-002 accepted.
- R04-003 accepted.
- R04-004 accepted.
- Required contract tests pass.
- Requirements traceability is clean.

## Review Scope

Freeze:
- platform adapter boundary;
- ownership and lifetime;
- context semantics;
- scheduler contract;
- clock contract;
- timer contract;
- watchdog contract;
- thread-safety boundary;
- real-time guarantee boundary;
- unsupported-operation semantics;
- error propagation.

## Mandatory Architectural Decisions

1. Core remains platform independent.
2. Platform implementations remain outside `kritva-core`.
3. No Core API may depend on Linux/POSIX/RTOS/vendor headers.
4. `time::IClock` remains the canonical clock abstraction.
5. Scheduler priority/affinity mapping remains adapter-defined where not explicitly portable.
6. Timer callback execution context must not imply a Core-owned thread.
7. Watchdog expiration must not imply automatic Runtime recovery.
8. No global platform singleton is introduced.
9. Existing R0.3 Runtime contracts remain authoritative.

## Evidence Required

- clean build;
- complete relevant CTest suite;
- public-header compile test;
- sanitizer results;
- dependency/header scan;
- API diff;
- traceability report;
- documented open issues.

## Decision

Status: SUBMITTED (evidence: `R04_PLATFORM_API_REVIEW_EVIDENCE.md`)

Possible outcomes:
- PASS / FROZEN
- CHANGES REQUIRED
- BLOCKED

Reviewer: ChatGPT architecture/review gate.
