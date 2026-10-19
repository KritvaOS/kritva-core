//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : README.md
// Description : Repository overview and navigation map (Kritva README standard)
//
// Component   : Kritva Core
// Module      : Documentation
// Layer       : Core Foundation
//
// Requirements: CORE-DOC-001
// API         : CORE-API-README
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================

# Kritva Core

> Platform-independent runtime foundation for the Kritva Open Robotic Computing Platform: an API-first C++ library of types, lifecycle, status, health, errors, events, capabilities, configuration and runtime and platform contracts. The visibility class of this repository is not yet decided (D-039 recommends private).

| Property | Value |
|---|---|
| Project | KritvaOS |
| Repository | `KritvaOS/kritva-core` |
| Repository Type | Library |
| Visibility Class | Not yet decided (D-039 in kritva-program recommends PRIVATE; the Apache-2.0 licence would permit a public release, so a public release needs an owner exception and a release review) |
| GitHub Setting | PUBLIC (temporary, D-028 and D-037 in kritva-program; the setting is attested by the owner and was not verified from this README) |
| Lifecycle Status | ACTIVE |
| Repository Owner | Project owner (role; see section 15) |
| Security Owner | Security Owner role ([SECURITY.md](SECURITY.md)) |
| Architecture | [ARCHITECTURE.md](ARCHITECTURE.md) and [docs/architecture/](docs/architecture/) |
| Milestones | [planning/MILESTONE.md](planning/MILESTONE.md) and [planning/MILESTONE_STATUS.md](planning/MILESTONE_STATUS.md) |
| Tasks | [planning/TASKS.md](planning/TASKS.md) and [planning/MASTER_TRACKER.md](planning/MASTER_TRACKER.md) |
| Acceptance Criteria | Per milestone, under [planning/milestones/](planning/milestones/) (each milestone folder holds its own acceptance criteria) |
| Status Reporting | [status/README.md](status/README.md) and [status/status.json](status/status.json) (generated) |
| License | [LICENSE](LICENSE) (Apache License 2.0) |

This README follows the Kritva README standard (`v2/planning/standards/README_STANDARD.md` in [kritva-program](https://github.com/KritvaOS/kritva-program)). It is a navigation map; the documents it links own the detail.

## 1. Purpose and Scope

Core is an API-first, platform-independent library. Platform implementations are supplied by the parent `kritvaos-community` repository.

**Core owns** types, lifecycle, status, health, errors, events, capabilities, configuration, runtime contracts, messaging contracts, time abstractions and platform contracts.

**Core does not own** Linux/PREEMPT_RT, FreeRTOS, STM32/TI BSPs, ROS 2/DDS, EtherCAT, vendor drivers or robot algorithms. Do not add them here; propose the boundary first ([AGENTS.md](AGENTS.md)).

**Implementation state.** IMPLEMENTED: the library at version 1.0.0 (see [VERSION](VERSION)): 49 public headers under `include/kritva/core/` (inventoried in [docs/compatibility/API_INVENTORY.md](docs/compatibility/API_INVENTORY.md)), eight source files under `src/`, and unit, contract, integration, platform-conformance, install and compatibility tests under `tests/`. PROPOSED or planning-only: the later phases listed as P1 and beyond in [REQUIREMENTS.md](REQUIREMENTS.md). NOT IMPLEMENTED as real checks: source formatting and static analysis (`make format-check` and `make lint` are placeholders; see section 11). A published tag or a "RELEASED" line in the planning records is the repository's own record; it is not central acceptance by kritva-program and does not make the library safe or fit for any robot (see section 13).

## 2. Position in the KritvaOS Ecosystem

Core is the foundation layer that other Kritva repositories build on. Its documented layering (from [ARCHITECTURE.md](ARCHITECTURE.md)) is Application, SDK, Skill, Mind and Motion, Sense, Core, hardware abstraction, then the Nexus and Edge. Core defines reusable software contracts and does not own physical hardware implementation. It may be integrated into KritvaOS as a Git submodule; the parent references a deliberate Core commit or tag, and this repository never changes the parent's pointer. The roster and order of repositories are owned by kritva-program (`v2/planning/repo-pipeline.json`), not by this README.

## 3. Architecture

The architecture, requirements and public API are defined in [ARCHITECTURE.md](ARCHITECTURE.md), [REQUIREMENTS.md](REQUIREMENTS.md) and [API.md](API.md), with the maintained pages under [docs/](docs/). The foundation covers identity, version, timestamp, duration and metadata; lifecycle; status and health; statistics; error and result; events; capabilities; configuration; and the runtime, messaging, time and platform contracts. Public headers belong under `include/kritva/core/`, implementation under `src/`, tests under `tests/`. The compatibility contract for Core 1.x (source and semantic compatibility, versioning, deprecation) is under [docs/compatibility/](docs/compatibility/). That contract states that **no ABI or binary compatibility is promised** ([docs/compatibility/ABI_POLICY.md](docs/compatibility/ABI_POLICY.md)). Security architecture, trust boundaries and the threat model are under [docs/security/](docs/security/).

## 4. Documents to Read First

Role first, then the status the document declares. A label is a role, not an approval; where a document does not declare a status in its header, none is shown here.

| # | Document | Label, status | What it answers |
|---|---|---|---|
| 1 | [AGENTS.md](AGENTS.md) | AUTHORITATIVE for contributor and agent rules; in force | How to work in this repository: authority order, boundaries, API-first sequence, governance rules |
| 2 | [ARCHITECTURE.md](ARCHITECTURE.md) | AUTHORITATIVE for the architecture; status per the file | The architectural role and the foundation domains |
| 3 | [REQUIREMENTS.md](REQUIREMENTS.md) | AUTHORITATIVE for the requirements (R1.0); later phases are not yet implemented | What Core must do |
| 4 | [API.md](API.md) and [docs/api/](docs/api/) | AUTHORITATIVE for the public API contract (the reviewed headers and the maintained pages) | The public API |
| 5 | [TESTING.md](TESTING.md) | AUTHORITATIVE for the test levels and strategy | What is tested and how |
| 6 | [docs/README.md](docs/README.md) | REFERENCE | The documentation layers and which is authoritative |
| 7 | [docs/compatibility/](docs/compatibility/) | AUTHORITATIVE for the Core 1.x compatibility contract; "defined by" the R10 tasks and normative once the R10 gate is accepted (read each page's own status line) | What stays compatible |
| 8 | [docs/security/](docs/security/) | AUTHORITATIVE for security architecture and decisions; status per file | Trust boundaries, threat model, security decisions |
| 9 | [planning/README.md](planning/README.md) | AUTHORITATIVE for planning and execution control (per its own header) | How milestones and tasks are run |
| 10 | [planning/MASTER_TRACKER.md](planning/MASTER_TRACKER.md) | REFERENCE: a consolidated view; it states that it is not an acceptance authority | Milestone and task progress |
| 11 | [CHANGELOG.md](CHANGELOG.md) | REFERENCE | What changed in each release |
| 12 | [status/README.md](status/README.md) | REFERENCE | How the status report is generated and what it means |
| 13 | [implementation.md](implementation.md) | REFERENCE (a hand-off note; may lag the repository) | A handoff summary of the repository |

## 5. Repository Structure and File Responsibilities

Labels: INPUT is edited by hand and read by a tool or review; GENERATED is written by a tool and never edited; EVIDENCE supports a claim; AUTHORITATIVE owns a rule; REFERENCE governs nothing. A label is a role, not an approval.

| Path | Content | Label, status | Notes |
|---|---|---|---|
| [include/kritva/core/](include/kritva/core/) | The public headers | AUTHORITATIVE for the public API; IMPLEMENTED | Changes need the compatibility process |
| [src/](src/) | Implementation | INPUT; IMPLEMENTED | |
| [tests/](tests/) | Unit, contract, integration, platform conformance, install, compatibility, audit and commit-checker tests | EVIDENCE; IMPLEMENTED | `tests/compat/api_surface.snapshot` is a snapshot of the public declaration surface |
| [docs/](docs/) | Architecture, API, compatibility, security and development documentation | MIXED: AUTHORITATIVE and REFERENCE per document | `docs/architecture/archive/` is historical |
| [planning/](planning/) | Milestones, tasks, review checklists, change logs | MIXED: planning records and views | Governs execution and acceptance of milestones |
| [scripts/](scripts/) | Source-header lint, audit checkers, status generator, commit checkers | INPUT; IMPLEMENTED | `scripts/audit/` checks the docs, inventory, compatibility and security records |
| [status/](status/) | Status report, vendored schema, evidence file | GENERATED (`status.json`); SNAPSHOT (schema) | Never hand-edit `status.json` |
| [cmake/](cmake/), [CMakeLists.txt](CMakeLists.txt), [CMakePresets.json](CMakePresets.json) | Build and install package configuration | INPUT; IMPLEMENTED | |
| [config/](config/) | Source-header lint configuration | INPUT | |
| [examples/](examples/) | Examples | NOT YET IMPLEMENTED (empty) | |
| [Makefile](Makefile) | Local commands (`make help`) | INPUT | Some targets are placeholders (section 11) |
| `.github/` | CODEOWNERS, pull request template, workflows | AUTHORITATIVE | |
| [VERSION](VERSION), [CHANGELOG.md](CHANGELOG.md), [LICENSE](LICENSE), [SECURITY.md](SECURITY.md), [AGENTS.md](AGENTS.md) | Version, release notes, licence, security policy, agent rules | AUTHORITATIVE | |

## 6. Toolchain and Development Workflow

| Tool | Purpose | Version | Check |
|---|---|---|---|
| CMake | Configure, build, test, install | 3.24 or later | `cmake --version` |
| A C++ compiler | Build the library and tests | one supported by `CMakeLists.txt`; CI uses the default compiler of an `ubuntu-latest` runner | `c++ --version` |
| Python 3 | Lint, audit and status scripts | 3.12 in CI | `python3 --version` |
| GNU make | Convenience wrapper | any | `make --version` |
| gcov-based coverage tools | `make coverage` | see the Makefile | not applicable unless you run coverage |

Run `make help` for the commands. Development follows the API-first sequence in [AGENTS.md](AGENTS.md): identify the requirement and owning domain, define the API, behaviour and error behaviour, define the tests, implement, verify. Any API intended for real-time use documents allocation, blocking, synchronisation, complexity, thread-safety and failure behaviour.

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

### Install and use as a library

```bash
cmake -S . -B build
cmake --build build
cmake --install build --prefix <prefix>      # or: make install PREFIX=<prefix>
```

Downstream CMake projects consume the installed package:

```cmake
find_package(kritva_core 1.0 CONFIG REQUIRED)   # add <prefix> to CMAKE_PREFIX_PATH
target_link_libraries(my_target PRIVATE kritva_core::kritva_core)
```

The installed package provides the `kritva_core` library and the public headers under `include/kritva/core/`. It has no third-party dependencies. A requested version is accepted only when it has the same major version as the installed package and the installed version is greater than or equal to the requested one (`docs/compatibility/VERSIONING_POLICY.md`); a newer minor or patch and a different major are refused. In a source tree that embeds Core with `add_subdirectory`, the same `kritva_core::kritva_core` target is available.

## 7. Dependencies and Interfaces

| Dependency | Direction | Kind | Contract | State |
|---|---|---|---|---|
| kritvaos-community | downstream | platform implementations of the platform contracts | `include/kritva/core/platform/` and the platform conformance suite in `tests/platform/` | The contracts are IMPLEMENTED here; the adapters are supplied by that repository and are not verified here |
| Third-party libraries | upstream | none | none | The installed package has no third-party dependencies |
| kritva-program | upstream | governance and status | status schema 2.0 ([status/README.md](status/README.md)) | planning-only reporting; no program credit |
| Other Kritva repositories (sense, motion, mind, skill, raas, actuator) | downstream consumers | library use | the public API and compatibility contract | PLANNED or PROPOSED in the program's repository plan; no consumer is verified here |

Five kinds of relationship are kept apart: program ordering, Git submodule, source or build dependency, runtime integration, and governance or status reporting. A PROPOSED or PLANNED dependency is not an integration that exists.

## 8. Milestones and Implementation Plan

Work is organised in milestones KF-CORE-R01 to KF-CORE-R10, each with a folder under [planning/milestones/](planning/milestones/). The consolidated view is [planning/MILESTONE_STATUS.md](planning/MILESTONE_STATUS.md); the plan is [planning/MILESTONE.md](planning/MILESTONE.md). Those records are the repository's own planning records. A milestone listed as released there is the repository's own statement: central acceptance and baseline membership are decided in kritva-program, and dates in these records are planning information, not delivery commitments.

## 9. Tasks and Acceptance Criteria

- **Where tasks live.** [planning/TASKS.md](planning/TASKS.md) and each milestone's folder; [planning/MASTER_TRACKER.md](planning/MASTER_TRACKER.md) is a consolidated view and states that it is not an acceptance authority.
- **Acceptance.** Task acceptance is governed by the task's own acceptance criteria and an independent reviewer decision, recorded in the milestone folder; [planning/REVIEW_CHECKLIST.md](planning/REVIEW_CHECKLIST.md) is the review list.
- Work is not accepted because code compiles, a commit or a merged pull request exists, or a test passes locally. Public API compatibility changes, architectural boundary changes, safety and security behaviour, real-time guarantees and major dependency changes need human review ([AGENTS.md](AGENTS.md)).

## 10. Status Reporting and Program Integration

`status/status.json` (schema 2.0) is generated by `scripts/status/gen_status.py`; never edit it by hand. Its scope is the nine KF-CORE-R10 tasks; earlier milestones are not reported, and including them in the program baseline is a central decision ([status/README.md](status/README.md)). This repository reports facts only and earns no program credit.

| Dimension | Source record | Authority |
|---|---|---|
| Execution | the planning records, reported in `status/status.json` (`COMPLETE` means the planning records list the task as accepted or released) | The task owner records it |
| Evidence | [status/evidence.json](status/evidence.json) and the milestone evidence in `planning/milestones/`; each report entry carries a criteria reference with no commit, CI run or hash, so it is never proof | The author records it; a reviewer other than the author checks it |
| Acceptance | the milestone's acceptance criteria and the independent reviewer decision | The independent reviewer, never a status report |
| Release readiness | the release gate in the milestone records and the tags (`kritva-core-r*`) | The owner, through the release gate |

Evidence has types that are not interchangeable (a software test result, an audit script result, a review decision). A generated view is a snapshot that can lag its inputs, cannot approve its own work, and does not establish program credit.

## 11. Build, Test, Regression and Validation

CI runs five separate workflows (below). `make check` is the local pre-PR command; it is not the same as CI.

| Workflow | Trigger | What it runs |
|---|---|---|
| `Kritva Core CI` (`kritva-core-ci.yml`) | every push; pull requests | CMake configure (Debug), build, `ctest --output-on-failure` |
| `source-header-check` | every push; pull requests | `python3 scripts/lint/check_source_headers.py --mode tracked --strict` |
| `status-report` | every push; pull requests | the status generator tests, `gen_status.py --check-facts`, and the schema checksum |
| `commit-message-check` | pull requests | the commit checker's unit tests and the checker on the commits in the pull request (rejects session trailers and links only) |
| `secret-scan` | push to `main`; pull requests; manual dispatch | gitleaks over the git history |

| Command | Effect | Notes |
|---|---|---|
| `make build`, `make test`, `make test-verbose` | generate (writes `build/`) | configure, build and run the CTest suite |
| `make header-check` | check | source-header lint |
| `make check-readme` | check | structural check of this README only (metadata, topics, relative links, placeholders); a PASS does not show the content is accurate; not part of `make check` or CI |
| `make traceability-check`, `make api-docs-check`, `make api-inventory-check`, `make compat-policy-check`, `make api-surface-check`, `make security-docs-check` | check | audit scripts over the requirements, API pages, inventory, compatibility pages, the API surface snapshot and the security records |
| `make format-check`, `make lint` | placeholder | print a to-do message and do nothing: they are not real checks even though `make check` runs them |
| `make format` | placeholder | prints a to-do message; it formats nothing |
| `make check` | check | runs the header, audit, format-check and lint targets; the last two are placeholders |
| `make coverage`, `make coverage-check` | generate (writes a coverage build and report) | `coverage-check` enforces a minimum line coverage; not in CI |
| `make api-surface-update` | modify (regenerates the API surface snapshot) | a deliberate change; needs an evolution review |
| `make clean`, `make coverage-clean`, `make rebuild` | modify (deletes build artefacts) | local only |
| `make install` | modify (writes to the install prefix) | `PREFIX=<dir>` overrides the prefix |

A passing check shows that the repository's documents, inventory and tests agree with each other. It does not show that a design is right, that a gate is closed or that the library is fit for a robot. `make test` and CI run on the host compiler only: there is no hardware or real-time test here.

## 12. Git Workflow and Review

Repository convention: `type(scope): summary` with small logical commits (examples in [AGENTS.md](AGENTS.md): `feat(core): add identity foundation`). CI enforces only that a commit carries no `Claude-Session:` trailer or session link (`commit-message-check`); the subject form is not checked by CI. Local style checks (`scripts/security/check_commit_format.py`) are advisory. The Kritva commit standard in kritva-program (D-053) is NOT formally adopted here (PD-019). Work on a branch and open a pull request; do not push to `main`, force-push a shared branch or rewrite history without the owner's explicit approval. `main` is protected by a ruleset that requires `secret-scan`, `commit-message-check` and Code Owner review (owner statement, D-040; not independently verified); with one owner the PR author cannot self-approve, so use the owner bypass or a second reviewer. Human review is required for the items listed in section 9.

## 13. Safety and Security

Core is a software library with no actuators or hardware. It is not a safety-certified component and no safety case is made for it: its tests are host tests, and passing them does not show that a robot using Core is safe. Safety and security behaviour changes need human review. Real-time guarantees are documented per API and are not verified by any hardware or real-time test here.

**Reporting a vulnerability:** follow [SECURITY.md](SECURITY.md), which asks for GitHub private vulnerability reporting and, if that is not available, a direct request to the owner for a private channel. Whether private vulnerability reporting is enabled has not been verified from this README. Never put details of an unfixed vulnerability in a public issue, pull request or commit message.

- Never commit secrets, private keys, tokens, passwords or production data. Use obviously fake values and mark them as fake. The `secret-scan` workflow runs on every push and pull request; do not add allowlist entries to silence it.
- If a secret is committed by mistake, tell the Security Owner at once; removing it in a later commit does not make it safe.

## 14. Release and Versioning

The version is in [VERSION](VERSION) and the release notes in [CHANGELOG.md](CHANGELOG.md). Versioning, compatibility and deprecation rules are in [docs/compatibility/VERSIONING_POLICY.md](docs/compatibility/VERSIONING_POLICY.md) and the related policy pages; releases are tagged `kritva-core-r*`. A release is complete only when the milestone's release gate is met, not when code is merged. This README records no release state beyond pointing to those records.

## 15. Ownership and Escalation

| Role | Responsibility |
|---|---|
| Repository owner (Dinesh; `.github/CODEOWNERS`) | Directs the work, decides, approves and merges |
| Security Owner | Handles vulnerability reports and committed secrets |
| Independent reviewer | Reviews and accepts tasks against the milestone acceptance criteria |
| Implementers (people and agents) | Implement approved scope, write tests, record evidence and follow `AGENTS.md` |

## 16. Related Repositories and References

| Repository | Relationship |
|---|---|
| [kritva-program](https://github.com/KritvaOS/kritva-program) | Program plan, baseline, decisions, status consolidation, README standard |
| [kritvaos-community](https://github.com/KritvaOS/kritvaos-community) | Supplies platform implementations of the platform contracts |

## 17. License

Apache License 2.0. Copyright (c) 2026 KritvaOS. See [LICENSE](LICENSE).
