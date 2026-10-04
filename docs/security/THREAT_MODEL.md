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
