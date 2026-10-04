# KF-CORE-R10-003 — ABI / Binary Compatibility Policy

## Objective

Decide whether ABI compatibility is supported and, if so, define the bounded compiler/platform/build matrix.

## Requirement

CORE-COMPAT-004, defined by this task in root `REQUIREMENTS.md`.

## Concrete Scope (policy-only, as approved at plan alignment)

- `docs/compatibility/ABI_POLICY.md`: the decision (Core 1.x promises no ABI/binary compatibility; no universal promise implied by 1.0), the rationale from the actual build and interface (C++20 header-based inline/templated interface, exposed layouts, virtual interfaces, standard-library types, compiler-dependent mangling, default static library with no visibility/export control, SOVERSION or ABI tooling), a bounded "not promised" matrix, what clients may rely on, conditions for any future ABI promise, relationship to R10-002/004/005/007.
- `scripts/audit/check_compat_policy.py` extended: audits `ABI_POLICY.md` for structure and references and fails if `CMakeLists.txt` introduces ABI machinery (SOVERSION, visibility settings, export-header generator, shared library target); self-test extended.
- Requirement CORE-COMPAT-004 with a process-traceability row; links from `docs/api/README.md` and `docs/api/API_GUIDELINES.md`; `TESTING.md`; `CHANGELOG.md`.

## Exclusions

- No ABI mechanism, export/visibility control, shared-library support or ABI-checking tool; no header, `src/` or behavior change; no new public API.
- No version-number mapping or evolution procedure (R10-004), deprecation (R10-005) or package rule (R10-007).

## Dependencies

R10-002 (ACCEPTED).
