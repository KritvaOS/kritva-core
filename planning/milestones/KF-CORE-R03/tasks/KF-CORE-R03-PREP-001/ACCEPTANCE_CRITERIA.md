# KF-CORE-R03-PREP-001 — Acceptance Criteria

## 1. Task Information

- Task: Install and Package Core Library
- Milestone: `KF-CORE-R03` (preparation)
- Requirement: `CORE-BUILD-002`
- Expected commit: `build(core): install and package Core library`

## 2. Requirement Traceability

| Requirement ID | Artifact | Verification | Evidence |
|---|---|---|---|
| CORE-BUILD-002 | `CMakeLists.txt`, `cmake/kritva_coreConfig.cmake.in`, `Makefile` | `tests/install/run_install_test.cmake` (CTest `kritva_core_install_consumer`) | commit `29255d5`; 18/18 ctest |

## 3. Acceptance Criteria (from the task definition)

- [x] AC-1: `cmake --install build` succeeds.
- [x] AC-2: `make install` succeeds (`make install PREFIX=<dir>`).
- [x] AC-3: Public headers are installed under the install prefix.
- [x] AC-4: The `kritva_core` library is installed.
- [x] AC-5: A minimal external consumer locates and links Core using the installed package.
- [x] AC-6: Build-tree usage is unchanged.
- [x] AC-7: Existing tests remain passing (17 existing + 1 new = 18/18).
- [x] AC-8: No new compiler warnings.
- [x] AC-9: `git diff --check` passes.
- [x] AC-10: Traceability audit passes with `CORE-BUILD-002` (50 requirements, 49 traced, 1 reserved).
- [x] AC-11: No ROS2/DDS/EtherCAT/vendor or other third-party dependency introduced.
- [x] AC-12: No runtime implementation change (`include/` and `src/` untouched).
- [x] New installation/consumer integration test added.

## 4. Implementation Evidence (Claude)

- Commit: `29255d5` `build(core): install and package Core library` (R0.2 release tag `kritva-core-r0.2` = `46af52c`, unchanged).
- Files changed: `CMakeLists.txt`, `cmake/kritva_coreConfig.cmake.in` (new), `Makefile` (`PREFIX` override), `tests/install/run_install_test.cmake`, `tests/install/consumer/CMakeLists.txt`, `tests/install/consumer/main.cpp` (new), `REQUIREMENTS.md` (CORE-BUILD-002 + process row), `README.md`, `API.md` (section 19), `scripts/audit/check_traceability.py`.
- Root cause of the audit finding: `CMakeLists.txt` had `$<INSTALL_INTERFACE:include>` but no `install()` rules, so `cmake --install` exited 0 and installed 0 files.
- Design (names chosen here; please confirm):
  1. Package/target names: `find_package(kritva_core CONFIG)` and `kritva_core::kritva_core`, matching the existing library target `kritva_core` (the CMake project is named `kritva-core`, so `find_package(kritva-core)` is NOT provided). The same `kritva_core::kritva_core` alias exists in the build tree.
  2. Installed layout (GNUInstallDirs): `lib/libkritva_core.a`, `include/kritva/core/**/*.hpp` (35 headers), `lib/cmake/kritva_core/{kritva_coreConfig,kritva_coreConfigVersion,kritva_coreTargets,kritva_coreTargets-<config>}.cmake`. Tests, examples and scripts are not installed.
  3. Version policy: `SameMinorVersion` while pre-1.0 (0.2 accepts 0.2.x; 0.1, 0.3 and 9.0 are refused). To be reviewed when Core reaches 1.0.
  4. The library type follows `BUILD_SHARED_LIBS` (static by default); no change to that.
  5. `make install` gained an optional `PREFIX=<dir>`; without it the CMake default prefix (`/usr/local`) applies, as before.
- New test `kritva_core_install_consumer` (CTest, `cmake -P` driver): (a) installs the existing build tree into a scratch prefix; (b) checks the library, representative headers and the three package files, and that no `*_test*`/`*.cpp` file was installed; (c) checks the exported targets file does not reference the build tree; (d) configures, builds and runs a consumer that sees only the installed prefix and exercises header-only types plus library code (`Lifecycle`, `Version`); (e) checks version policy (0.2 accepted; 0.1, 0.3, 9.0 refused). Compiler and flags are propagated so sanitizer, strict-warning and coverage builds still link.
- Mutation evidence that the test can fail (each temporary edit reverted; file verified identical to the committed version): no header install → "installed header missing"; package files not installed → "installed CMake package files missing"; compatibility changed to `AnyNewerVersion` → "accepted incompatible version request 0.1"; a build-tree path in `INSTALL_INTERFACE` is rejected by CMake itself at generate time.
- Traceability audit change: the forbidden-dependency check (`CORE-GEN-003`) matched a `find_package(` inside a CMake comment. It now ignores comments; a real `find_package(Boost)` appended to `CMakeLists.txt` still produces an error (verified), and Core's own `find_package` is only in the *consumer test project* and package config template, which are not scanned as the library build.
- Build: `rm -rf build && cmake -S . -B build && cmake --build build -j$(nproc)` — 0 warnings.
- Tests: `ctest --test-dir build --output-on-failure` — 18/18 passed. Also Release 18/18, ASan+UBSan 18/18, strict (`-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`) 18/18.
- `make install PREFIX=<scratch>`: installed 40 files, including all 35 headers.
- `make check` passed (header-check; traceability-check 50/49/0/0; format-check and lint are stubs). `git diff --check` clean.
- Coverage: `make coverage` — 98% (160/163, unchanged; no source changed).
- Known limitations: not tested on other platforms/compilers or multi-config generators beyond the `--config` plumbing; shared-library builds are not exercised; no uninstall target; the install test needs the project's own build tree (it installs from it); `make install` without `PREFIX` writes to `/usr/local`.

## 5. Independent Reviewer Decision

Reviewer: ChatGPT

- [ ] PASS
- [ ] CHANGES REQUIRED
- [ ] BLOCKED

Review notes:

TBD
