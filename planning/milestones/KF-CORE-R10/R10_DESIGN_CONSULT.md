# KF-CORE-R10 — Design Consult

## Decision

**APPROVED — proceed to R10 Scope Confirmation.**

## Core Decision

R1.0 shall be primarily an **API maturity and compatibility foundation** milestone, not another functional subsystem milestone.

The R0.2–R0.9 contracts form the functional baseline. R1.0 shall define how those contracts evolve safely across 1.x releases.

## Baseline Findings

The R0.9 baseline already provides:

- explicit public Core headers under `include/kritva/core/`;
- lifecycle, Runtime, platform, context, configuration and capability contracts;
- canonical Markdown API documentation and a mechanical documentation audit;
- requirements traceability;
- reference/conformance and boundary tests;
- install-consumer validation;
- security architecture and trust-boundary documentation.

The principal maturity gap is the lack of one authoritative project-level policy for source compatibility, semantic compatibility, ABI scope, deprecation, version selection and API evolution.

## Architecture Questions and Decisions

### Q1 — What is the principal purpose of R1.0?

**Decision:** Establish the Core 1.x compatibility/evolution contract around the already released functional foundation.

### Q2 — Should R1.0 add a new Runtime or dependency framework?

**Decision:** No. The R0.9 architecture remains authoritative. New functional frameworks require a separate architecture justification.

### Q3 — What kinds of compatibility matter?

**Decision:** R1.0 treats source compatibility, semantic compatibility and ABI/binary compatibility as distinct dimensions.

### Q4 — Should ABI stability be universally guaranteed?

**Decision:** Not by default. Any ABI guarantee must be explicitly bounded by a supported compiler, standard-library, platform and build configuration matrix. A universal C++ ABI promise is not inferred from reaching 1.0.

### Q5 — What does SemVer mean for Kritva Core?

**Decision:** Major/minor/patch release impact shall be defined specifically for public API, semantics, ownership/lifetime, enums/error values, virtual interfaces, package behavior and documented guarantees.

### Q6 — Should semantic-only changes count as compatibility events?

**Decision:** Yes. A signature-preserving behavioral contract change is a semantic API change and shall be classified and reviewed accordingly.

### Q7 — How should public enums and error values be treated?

**Decision:** Externally observable numeric values and their meanings are compatibility-sensitive. Additions require explicit review because application switch/default behavior may change even when existing declarations remain present.

### Q8 — Should stable APIs be removable in a minor release?

**Decision:** No, except for an explicit, documented emergency policy approved by architecture review. Normal removal requires a major release.

### Q9 — What is the role of deprecation?

**Decision:** Deprecation is a supported migration mechanism. A deprecated API requires documented rationale, replacement/migration guidance where applicable, and a declared compatibility window or removal plan.

### Q10 — How should CMake package compatibility work?

**Decision:** R1.0 shall specify and test package version-selection semantics. Pre-1.0 exact-major/minor behavior must not silently become the 1.x policy.

### Q11 — What is the scope of compatibility?

**Decision:** Compatibility applies to explicitly classified public installed API and package contracts. Internal implementation, tests, generated documentation and unlisted helper artifacts are not automatically contractual.

### Q12 — Should all R0.9 stub API pages become maintained for R1.0?

**Decision:** No blanket conversion. R1.0 shall use the R0.9 stub policy unless compatibility classification demonstrates that a particular domain needs a maintained page.

### Q13 — Does R1.0 authorize security mechanisms?

**Decision:** No. Only security-impact assessment of package/compatibility/release boundaries is in scope.

## Architectural Invariants

1. R0.9 functional semantics remain the starting baseline.
2. Core remains platform-independent.
3. Compatibility and evolution are explicit contracts, not implicit repository behavior.
4. Source, semantic and ABI compatibility remain distinct.
5. No universal ABI guarantee is implied by version 1.0.
6. Stable public API removal normally requires a major release.
7. Public enum/error values and ownership/lifetime semantics are compatibility-sensitive.
8. Package/version-selection behavior is part of the supported installation contract.
9. Compatibility policy does not authorize new runtime frameworks.
10. Compatibility claims do not imply security, authenticity or provenance guarantees.
11. Canonical documentation remains Markdown.
12. R0.9 release evidence is historical and is not retroactively rewritten.

## Review Outcome

**APPROVED — suitable for R10 Scope Confirmation.**
