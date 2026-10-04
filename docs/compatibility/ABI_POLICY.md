# Kritva Core ABI / Binary Compatibility Policy (R1.0)

**Status:** defined by KF-CORE-R10-003 (CORE-COMPAT-004). Normative for the Core 1.x line once KF-CORE-R10-001..005 are accepted and the R10 API / Compatibility Review has frozen it.
**Authority:** this page is the authoritative statement of what Core 1.x promises about binary compatibility. Source and semantic compatibility are defined by `COMPATIBILITY_POLICY.md` and are independent of this page.

## 1. Purpose

To state, explicitly and without implication, what Kritva Core 1.x does and does not promise about binary (ABI) compatibility, why, and what would have to be decided before any promise could be made.

## 2. Decision

**Kritva Core 1.x does not promise ABI or binary compatibility.** Reaching version 1.0, a stable classification in `API_INVENTORY.md`, or source and semantic compatibility under `COMPATIBILITY_POLICY.md` implies no binary promise. A program that uses Core must be compiled and linked against the same Core build it runs with, using one toolchain and one set of build settings for Core and the program.

This is a decision, not an omission: no ABI mechanism is introduced by R1.0 and none may be added without a separate architecture decision (section 6).

## 3. Rationale

| Fact about Core | Why it prevents an unqualified promise |
|---|---|
| The public interface is C++20 and header-based; much of it is inline or templated, so Core code is compiled into the client | Client binaries embed Core's layouts and code; changing either changes the binary contract. |
| Public classes expose their data layout, size and alignment to clients (values, aggregates, members) | Any added or reordered member changes `sizeof`, offsets and, for aggregates, the source form. |
| Public interfaces are virtual (for example `Component` in `runtime/component.hpp`, `IPlatformAdapter` in `platform/adapter.hpp` and `Runtime` in `runtime/runtime.hpp`) | Adding or reordering a virtual changes the vtable layout that clients and implementers were compiled against. |
| The interface uses standard-library types by value and by reference (`std::string`, `std::vector` and `std::optional`) | Their layout and behavior depend on the standard library implementation, its version and its configuration macros. |
| Names are C++-mangled and the mangling, exception model, calling convention and standard-library ABI vary by compiler, version and flags | There is no single binary interface to promise across compilers. |
| The CMake target is built with the default library type (a static library) and has no symbol-visibility or export control, no shared-library version (SOVERSION) and no ABI-checking tooling | Core is not built, versioned or checked as a shared binary interface. |

## 4. What is not promised

| Item | Promise |
|---|---|
| Linking a program built against one Core release with a different Core release without rebuilding the program | None. |
| Mixing compilers, compiler versions, standard libraries, `-std` modes, ABI-affecting flags (`_GLIBCXX_USE_CXX11_ABI`, `-fno-rtti`, `-fno-exceptions`, sanitizers, LTO) between Core and its clients | None. |
| Stable object layout, `sizeof`, alignment, member offsets or vtable layout of any public type | None. |
| Stable symbol names, inline-function bodies or template instantiations | None. |
| Building Core as a shared library or loading it as a plug-in | Not a supported configuration: no export control or shared-library versioning exists. |
| Compatibility of one Core build with a client binary built from different headers of the same major version | None. |
| Compatibility across operating systems, architectures or endiannesses | None. |

A combination in this table may happen to work. It is not verified, not tested and not a commitment.

## 5. What clients may rely on

- Rebuilding a program against a later Core 1.x release is governed by `COMPATIBILITY_POLICY.md` (source and semantic compatibility).
- The installed package consumed with the toolchain that produced it is the verified configuration (validated by the installed-consumer test, R10-007).
- Core code that is released together (one install tree, one version) is internally consistent.

## 6. Conditions for any future ABI promise

An ABI guarantee requires a separate architecture decision that must at least define: a supported matrix of compiler, standard library, operating system, architecture and build settings; the public types and interfaces covered and the rules that keep their layout and vtables stable; symbol visibility and export control; shared-library versioning; automated ABI checking in validation; and the interaction with deprecation and removal. Until such a decision is accepted and recorded, this policy stands, and the audit of this page fails if ABI machinery appears in the build (section 7).

## 7. Guard

`scripts/audit/check_compat_policy.py` fails when `CMakeLists.txt` introduces ABI machinery that this policy says does not exist (`SOVERSION`, symbol-visibility settings, an export-header generator, or a shared library target), so that adding any of them forces this page to be revisited.

## 8. Compatibility is not security

A binary-compatibility statement, or its absence, says nothing about the authenticity or provenance of an artifact. This policy introduces no security mechanism.

## 9. Relationship to other R1.0 policy

| Concern | Owner |
|---|---|
| Source and semantic compatibility | `COMPATIBILITY_POLICY.md` (R10-002) |
| Release-impact classification and evolution review | R10-004 |
| Deprecation and migration | R10-005 |
| Package version selection and installed-consumer validation | R10-004 / R10-007 |

## 10. Requirements traceability

- `CORE-COMPAT-004` — the ABI posture defined here.
- `CORE-COMPAT-002`, `CORE-COMPAT-003` — the source and semantic policy this page is independent of.

## 11. Explicit exclusions

This page introduces no ABI mechanism, no export or visibility control, no shared-library support and no ABI-checking tool, and it changes no header, source or behavior. It defines no version-number mapping, deprecation rule or package rule.
