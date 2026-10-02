#==============================================================================
# Copyright (c) 2026 KritvaOS
# SPDX-License-Identifier: Apache-2.0
#
# File        : run_install_test.cmake
# Description : Install-and-consume integration test driver (cmake -P)
#
# Component   : Kritva Core
# Module      : Install Test
# Layer       : Development Infrastructure
#
# Requirements: CORE-BUILD-002
# API         : CTest script
#
# Author      : KritvaOS Core Team
# Created     : 02-10-2026
#==============================================================================

# Installs Kritva Core from an existing build tree into a scratch prefix, checks
# the installed layout, then configures, builds, and runs a minimal consumer
# against the installed package only (the source tree is never on its path).
#
# Inputs (-D): CORE_BUILD_DIR, CONSUMER_SOURCE_DIR, WORK_DIR, CXX_COMPILER,
#              CXX_FLAGS (propagated so sanitizer/coverage builds still link),
#              CONFIG (build configuration; empty for single-config).

foreach(var CORE_BUILD_DIR CONSUMER_SOURCE_DIR WORK_DIR CXX_COMPILER)
  if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
    message(FATAL_ERROR "run_install_test.cmake: ${var} is required")
  endif()
endforeach()

set(prefix "${WORK_DIR}/prefix")
set(consumer_build "${WORK_DIR}/consumer-build")
file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}")

set(config_args "")
if(NOT "${CONFIG}" STREQUAL "")
  set(config_args --config "${CONFIG}")
endif()

function(run_step name)
  execute_process(COMMAND ${ARGN} RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
  if(NOT rc EQUAL 0)
    message(FATAL_ERROR "install test step '${name}' failed (rc=${rc})\n${out}\n${err}")
  endif()
endfunction()

# 1. Install.
run_step(install "${CMAKE_COMMAND}" --install "${CORE_BUILD_DIR}" --prefix "${prefix}" ${config_args})

# 2. Installed layout: library, public headers, package files; no test artifacts.
file(GLOB_RECURSE libs "${prefix}/lib*/libkritva_core.*")
if(NOT libs)
  message(FATAL_ERROR "installed library libkritva_core not found under ${prefix}")
endif()
foreach(header
    include/kritva/core/core.hpp
    include/kritva/core/error/result.hpp
    include/kritva/core/lifecycle/lifecycle.hpp
    include/kritva/core/platform/scheduler.hpp)
  if(NOT EXISTS "${prefix}/${header}")
    message(FATAL_ERROR "installed header missing: ${header}")
  endif()
endforeach()
file(GLOB_RECURSE config_files "${prefix}/lib*/cmake/kritva_core/kritva_coreConfig.cmake")
file(GLOB_RECURSE version_files "${prefix}/lib*/cmake/kritva_core/kritva_coreConfigVersion.cmake")
file(GLOB_RECURSE target_files "${prefix}/lib*/cmake/kritva_core/kritva_coreTargets.cmake")
if(NOT config_files OR NOT version_files OR NOT target_files)
  message(FATAL_ERROR "installed CMake package files missing under ${prefix}")
endif()
file(GLOB_RECURSE stray "${prefix}/*_test*" "${prefix}/*.cpp")
if(stray)
  message(FATAL_ERROR "unexpected files installed: ${stray}")
endif()

# 3. Exported targets must not leak the source or build tree.
file(READ "${target_files}" targets_text)
string(FIND "${targets_text}" "${CORE_BUILD_DIR}" leak_build)
if(NOT leak_build EQUAL -1)
  message(FATAL_ERROR "installed targets file references the build tree")
endif()

# 4. Configure, build and run a consumer against the installed package only.
run_step(configure "${CMAKE_COMMAND}" -S "${CONSUMER_SOURCE_DIR}" -B "${consumer_build}"
  "-DCMAKE_PREFIX_PATH=${prefix}" "-DCMAKE_CXX_COMPILER=${CXX_COMPILER}"
  "-DCMAKE_CXX_FLAGS=${CXX_FLAGS}")
run_step(build "${CMAKE_COMMAND}" --build "${consumer_build}" ${config_args})
file(GLOB_RECURSE consumer_exe "${consumer_build}/consumer")
if(NOT consumer_exe)
  file(GLOB_RECURSE consumer_exe "${consumer_build}/consumer.exe")
endif()
run_step(run "${consumer_exe}")

# 5. Version compatibility: same minor (0.2) is accepted; a different minor
#    (0.1, 0.3) or major (9.0) is refused (SameMinorVersion, pre-1.0 policy).
foreach(request 0.2 0.1 0.3 9.0)
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -S "${CONSUMER_SOURCE_DIR}" -B "${WORK_DIR}/consumer-version-${request}"
            "-DCMAKE_PREFIX_PATH=${prefix}" "-DCMAKE_CXX_COMPILER=${CXX_COMPILER}"
            "-DCMAKE_CXX_FLAGS=${CXX_FLAGS}" "-DKRITVA_CORE_REQUIRE_VERSION=${request}"
    RESULT_VARIABLE version_rc OUTPUT_QUIET ERROR_QUIET)
  if(request STREQUAL "0.2" AND NOT version_rc EQUAL 0)
    message(FATAL_ERROR "find_package refused compatible version request ${request}")
  elseif(NOT request STREQUAL "0.2" AND version_rc EQUAL 0)
    message(FATAL_ERROR "find_package accepted incompatible version request ${request}")
  endif()
endforeach()

message(STATUS "kritva_core install test: PASSED")
