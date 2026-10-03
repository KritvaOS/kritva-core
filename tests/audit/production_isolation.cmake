# Requirements: CORE-PLAT-016
#
# Test-target isolation (CORE-PLAT-016): the production library must not depend on, include or
# install any test-only support such as the reference platform and its fakes.
#
# Inputs (-D): SOURCE_DIR, CXX_COMPILER, WORK_DIR (scratch directory for generated files)
#
# Three layers:
#   1. static: no production file includes anything under tests/ or a reference/fake/test double;
#   2. build isolation: every production translation unit and the umbrella header compile with
#      ONLY include/ on the include path (so tests/ is not reachable at all);
#   3. build description: the production library target lists no test source and no install()
#      rule installs test support.

foreach(var SOURCE_DIR CXX_COMPILER WORK_DIR)
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "production_isolation.cmake: -D${var}=... is required")
  endif()
endforeach()

file(GLOB_RECURSE production_files "${SOURCE_DIR}/include/*.hpp" "${SOURCE_DIR}/src/*.cpp" "${SOURCE_DIR}/src/*.hpp")
list(LENGTH production_files production_count)
if(production_count LESS 20)
  message(FATAL_ERROR "isolation test found only ${production_count} production files: wrong SOURCE_DIR?")
endif()

# 1. static include scan
foreach(file IN LISTS production_files)
  file(STRINGS "${file}" include_lines REGEX "^[ \t]*#[ \t]*include")
  foreach(line IN LISTS include_lines)
    if(line MATCHES "tests/|reference_|fake_|test_double|conformance")
      message(FATAL_ERROR "production file ${file} includes test-only support: ${line}")
    endif()
  endforeach()
endforeach()

# 2. build isolation: only include/ is visible
file(GLOB production_sources "${SOURCE_DIR}/src/*.cpp")
set(umbrella "${WORK_DIR}/isolation_umbrella.cpp")
file(WRITE "${umbrella}" "#include <kritva/core/core.hpp>\nint main() { return 0; }\n")
foreach(source IN LISTS production_sources umbrella)
  execute_process(
    COMMAND "${CXX_COMPILER}" -std=c++20 -fsyntax-only "-I${SOURCE_DIR}/include" "${source}"
    RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
  if(NOT rc EQUAL 0)
    message(FATAL_ERROR "${source} does not compile with only include/ on the include path:\n${err}")
  endif()
endforeach()

# 3. build description
file(READ "${SOURCE_DIR}/CMakeLists.txt" cmake_text)
string(REGEX MATCH "add_library\\(kritva_core[^)]*\\)" production_target "${cmake_text}")
if(production_target STREQUAL "")
  message(FATAL_ERROR "could not find the production library target in CMakeLists.txt")
endif()
if(production_target MATCHES "tests/|reference_|fake_")
  message(FATAL_ERROR "the production library target lists a test source: ${production_target}")
endif()
string(REGEX MATCHALL "install\\([^)]*\\)" install_rules "${cmake_text}")
foreach(rule IN LISTS install_rules)
  if(rule MATCHES "tests/|reference_|fake_")
    message(FATAL_ERROR "an install() rule installs test support: ${rule}")
  endif()
endforeach()

message(STATUS "production isolation: ${production_count} production files, ${CXX_COMPILER}")
