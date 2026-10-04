# Documentation Governance

## Canonical format

Maintained documentation is stored as Markdown (`.md`).

## API documentation

Public API contracts are maintained under `docs/api/` and must track accepted headers, semantics, ownership, lifecycle interaction, error behavior, threading/real-time expectations, and explicit exclusions.

## Security documentation

Security architecture is maintained under `docs/security/`. Security documents describe trust boundaries, authority assumptions, threats, and decisions; they do not imply a security subsystem unless explicitly approved.

## Historical records

Milestone-era audits and baseline reviews remain available under `architecture/archive/` when useful for traceability, but they are not current normative architecture.

## Synchronization rule

A public API or semantic change is not accepted until the corresponding API documentation and applicable security-impact assessment are updated.
