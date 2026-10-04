# Threat Model

R0.9 keeps the threat model lightweight and architectural.

Initial concerns to track:

- spoofed or incorrect capability claims;
- configuration supplied by an unauthorized caller;
- lifetime misuse across non-owning boundaries;
- malformed inputs to public contracts;
- accidental privilege or trust expansion through future API additions.

No security implementation is implied by this document.

## R0.9 notes

| Concern | Status in R0.9 |
|---|---|
| Spoofed or incorrect capability claims | Documented as untrusted descriptive metadata; matching by identity establishes only that an id was declared (`SD-R09-01`..`03`). No verification is provided. |
| Malformed capability metadata (empty or very long names, invalid ids, repeated names) | Plain values compared by identity only; an invalid-identity entry is storable but never satisfies a requirement; covered by tests. |
| Lifetime misuse of returned references | `CapabilitySet` references (`find()`, `all()`) are valid only until the next `add()`; snapshots are returned by value. Documented, ASan-tested. |
| Trust or privilege expansion through future API additions | The compile-time boundary snapshot fails the build on any resolver, registry, discovery, readiness or credential member. |

## R1.0 notes (compatibility, package and release boundaries)

| Concern | Status in R1.0 |
|---|---|
| Package or version substitution (a different package that satisfies the requested version) | Out of Core's control: package selection is a version rule, not a trust decision; the integrator controls the search path and artifact provenance (`SD-R10-01`, `SD-R10-02`, `SD-R10-05`). No signing or verification is provided. |
| Use of an unexpected compatible-looking version | The selection rule (same MAJOR, installed >= requested, EXACT textual) is documented and tested against `find_package` by a version matrix; a newer or different-MAJOR request is rejected (`docs/compatibility/VERSIONING_POLICY.md`). |
| Stale or misleading compatibility metadata (inventory, policy, snapshot or register drifting from the headers) | Mechanically audited: the inventory, compatibility-policy, API-surface and deprecation-register audits fail on drift, and the compile-time boundary test pins enumerations, type properties and virtual-interface shape. |
| Lifetime or ownership regressions hidden behind source-compatible signatures | Semantic compatibility protects ownership, lifetime and thread-safety guarantees (`docs/compatibility/COMPATIBILITY_POLICY.md`); existing contract tests and sanitizer runs cover behavior (`SD-R10-03`). |
| Deprecated API migration crossing a trust boundary | Deprecation never relaxes validation or authority; a replacement must not weaken a security property (`SD-R10-04`). |
| Downstream assumption that compatibility implies security | Stated as a non-guarantee in every compatibility policy page (`SD-R10-01`, `SD-R10-03`). |
| Future remote or IPC consumers of a stable API | A stable C++ API is not a wire or serialization contract; no such compatibility is implied (`SD-R10-07`). |
| Mixed-toolchain binaries | No ABI promise; unsupported and unverified, not a security boundary (`SD-R10-06`). |
