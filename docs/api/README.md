# Kritva Core API Documentation

This directory contains the human-readable public API contracts for Kritva Core.

Markdown is authoritative. Generated HTML or PDF is publication output only.

## Domains

- Capability
- Configuration
- Context
- Error
- Lifecycle
- Platform
- Runtime

The public-header inventory and compatibility classification (R1.0) lives in `../compatibility/API_INVENTORY.md`; the source and semantic compatibility policy for stable items is `../compatibility/COMPATIBILITY_POLICY.md` and the ABI posture (no ABI promise) is `../compatibility/ABI_POLICY.md`.

See `API_INDEX.md` for the current map (every document and whether it is `maintained` or a `stub`).

The structure and references of this directory are audited mechanically by `scripts/audit/check_api_docs.py` (run by `make check`): it checks that indexed pages exist, that maintained pages contain every section required by `API_GUIDELINES.md` and name only headers, tests, documents and requirement identifiers that exist, and that no generated HTML or PDF is stored here. It never judges prose quality; that remains architecture and API review.
