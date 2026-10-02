# KF-CORE-R03-PREP-001 — Install and Package Core Library

## 1. Task Information

- Task ID: `KF-CORE-R03-PREP-001`
- Milestone: `KF-CORE-R03 — Runtime Foundation` (preparation; does not change the accepted R0.2 baseline)
- Area: `CMakeLists.txt`, `cmake/`, `Makefile`, `tests/install/`
- Dependency: `KF-CORE-R02` (released as `kritva-core-r0.2`)
- Requirement: `CORE-BUILD-002`
- Implementer: Codex / Claude
- Reviewer: ChatGPT
- Origin: external audit finding — `make install` succeeded but installed zero files.

## 2. Objective

Make `kritva-core` a correctly installable CMake library that another Kritva component can consume.

## 3. Requirement

`CORE-BUILD-002` — Provide a CMake installation interface that installs the Kritva Core library and public headers for consumption by downstream projects.

## 4. Technical Requirements

- `cmake --install` and `make install` install the `kritva_core` library and all public headers.
- A CMake package (config, version, targets) lets a downstream project use `find_package` and link an imported target.
- The installed package must not reference the source or build tree.
- Build-tree usage remains unchanged.
- No third-party dependency may be introduced (ROS2/DDS/EtherCAT/vendor).
- No runtime implementation change.
- One installation/consumer integration test, not only a file-presence check.

## 5. Out of Scope

Debian/RPM packages, Conan, vcpkg, Docker, cross-compilation packages, SDK distribution infrastructure, Runtime Manager functionality, any public API change.

## 6. Tests

- Install into a scratch prefix; check library, representative headers and package files; check nothing test-related is installed.
- Configure, build and run a minimal consumer against the installed package only.
- Check version-compatibility behavior.
- Regression: the existing 17 tests.

## 7. Build / Test

```bash
rm -rf build
cmake -S . -B build
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
```

## 8. Deliverables

Implementation summary, exact files changed, build and test results, quality checks, commit hash, known limitations.
