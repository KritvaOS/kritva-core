#==============================================================================
# Copyright (c) 2026 KritvaOS
# SPDX-License-Identifier: Apache-2.0
#
# File        : package_version_matrix.cmake
# Description : Package version-selection matrix test (cmake -P)
#
# Component   : Kritva Core
# Module      : Install Test
# Layer       : Development Infrastructure
#
# Requirements: CORE-COMPAT-010
# API         : CTest script
#
# Author      : KritvaOS Core Team
# Created     : 05-10-2026
#==============================================================================

# Checks the installed-package version-selection rule of docs/compatibility/VERSIONING_POLICY.md (section 6) against the
# behavior of CMake's find_package for the COMPATIBILITY mode the project actually uses:
#
#   a request is accepted only when it has the same MAJOR version as the installed package and the installed version is
#   greater than or equal to the requested one; a request for a newer MINOR or PATCH, or for a different MAJOR, is rejected;
#   a request with no version is accepted; EXACT is accepted only when the requested version string equals the installed
#   version string (CMake's EXACT is textual, so a partial request such as 1.2 never matches 1.2.0).
#
# For each installed version a fake package prefix is generated with the project's own COMPATIBILITY mode, and find_package
# is run for every request. Each result is compared with the model of the rule above; any disagreement fails the test.
#
# Inputs (-D): SOURCE_DIR (the repository root), WORK_DIR (scratch), optional MODE (override the mode read from
#              CMakeLists.txt, to show the matrix detects another mode) and EXPECT_MISMATCH (succeed only when the
#              matrix does detect a disagreement).

foreach(var SOURCE_DIR WORK_DIR)
  if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
    message(FATAL_ERROR "package_version_matrix.cmake: ${var} is required")
  endif()
endforeach()

if(DEFINED MODE AND NOT "${MODE}" STREQUAL "")
  set(mode "${MODE}")
else()
  file(READ "${SOURCE_DIR}/CMakeLists.txt" cmakelists)
  if(NOT cmakelists MATCHES "write_basic_package_version_file\\([^)]*COMPATIBILITY[ \t\r\n]+([A-Za-z]+)")
    message(FATAL_ERROR "no write_basic_package_version_file COMPATIBILITY mode found in CMakeLists.txt")
  endif()
  set(mode "${CMAKE_MATCH_1}")
endif()
message(STATUS "package version mode under test: ${mode}")

include(CMakePackageConfigHelpers)
file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}")

# Parse "a.b.c" (missing components are 0) into three variables in the caller's scope.
function(split_version text prefix)
  string(REPLACE "." ";" parts "${text}")
  list(LENGTH parts count)
  set(major 0)
  set(minor 0)
  set(patch 0)
  if(count GREATER 0)
    list(GET parts 0 major)
  endif()
  if(count GREATER 1)
    list(GET parts 1 minor)
  endif()
  if(count GREATER 2)
    list(GET parts 2 patch)
  endif()
  set(${prefix}_major ${major} PARENT_SCOPE)
  set(${prefix}_minor ${minor} PARENT_SCOPE)
  set(${prefix}_patch ${patch} PARENT_SCOPE)
endfunction()

# The documented rule: returns TRUE/FALSE in OUT.
function(policy_accepts installed request exact out)
  if("${request}" STREQUAL "")
    set(${out} TRUE PARENT_SCOPE)
    return()
  endif()
  split_version("${installed}" i)
  split_version("${request}" r)
  if(exact)
    # CMake's EXACT is textual: the requested version string must be the installed version string (all components spelled).
    if("${request}" STREQUAL "${installed}")
      set(${out} TRUE PARENT_SCOPE)
    else()
      set(${out} FALSE PARENT_SCOPE)
    endif()
    return()
  endif()
  if(NOT i_major EQUAL r_major)
    set(${out} FALSE PARENT_SCOPE)
    return()
  endif()
  if(i_minor GREATER r_minor OR (i_minor EQUAL r_minor AND NOT i_patch LESS r_patch))
    set(${out} TRUE PARENT_SCOPE)
  else()
    set(${out} FALSE PARENT_SCOPE)
  endif()
endfunction()

set(installed_versions 1.0.0 1.2.3 1.10.0 2.0.0 0.9.0)
set(requests "" 1 1.0 1.0.0 1.2 1.2.3 1.2.4 1.3 1.10 1.10.0 1.11 0.9 0.9.0 2 2.0 2.0.0 3.0)

file(WRITE "${WORK_DIR}/find.cmake" "list(APPEND CMAKE_PREFIX_PATH \"\${PREFIX}\")\nfind_package(kritva_core \${REQ} \${EXACT} CONFIG QUIET)\nif(kritva_core_FOUND)\n  message(STATUS \"RESULT found\")\nelse()\n  message(STATUS \"RESULT notfound\")\nendif()\n")

set(checked 0)
set(mismatches "")
foreach(installed IN LISTS installed_versions)
  set(prefix "${WORK_DIR}/prefix-${installed}")
  file(MAKE_DIRECTORY "${prefix}/lib/cmake/kritva_core")
  write_basic_package_version_file("${prefix}/lib/cmake/kritva_core/kritva_coreConfigVersion.cmake"
    VERSION ${installed} COMPATIBILITY ${mode})
  file(WRITE "${prefix}/lib/cmake/kritva_core/kritva_coreConfig.cmake" "set(kritva_core_FOUND TRUE)\n")
  foreach(request IN LISTS requests)
    foreach(exact_flag IN ITEMS FALSE TRUE)
      if(exact_flag AND "${request}" STREQUAL "")
        continue()
      endif()
      set(exact_arg "")
      if(exact_flag)
        set(exact_arg "EXACT")
      endif()
      execute_process(
        COMMAND "${CMAKE_COMMAND}" "-DPREFIX=${prefix}" "-DREQ=${request}" "-DEXACT=${exact_arg}" -P "${WORK_DIR}/find.cmake"
        OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE rc)
      set(actual FALSE)
      if("${out}${err}" MATCHES "RESULT found")
        set(actual TRUE)
      endif()
      policy_accepts("${installed}" "${request}" ${exact_flag} expected)
      math(EXPR checked "${checked} + 1")
      if(NOT actual STREQUAL expected)
        list(APPEND mismatches "installed ${installed}, request '${request}' ${exact_arg}: find_package ${actual}, policy ${expected}")
      endif()
    endforeach()
  endforeach()
endforeach()

list(LENGTH mismatches mismatch_count)
message(STATUS "package version matrix: ${checked} cases, ${mismatch_count} disagreement(s) with the policy")
if(EXPECT_MISMATCH)
  if(mismatch_count EQUAL 0)
    message(FATAL_ERROR "the matrix did not detect any disagreement for mode ${mode}")
  endif()
  message(STATUS "package version matrix: the disagreement of mode ${mode} was detected, as expected")
elseif(mismatch_count GREATER 0)
  string(REPLACE ";" "\n  " report "${mismatches}")
  message(FATAL_ERROR "find_package disagrees with docs/compatibility/VERSIONING_POLICY.md section 6:\n  ${report}")
endif()
message(STATUS "package version matrix: PASSED")
