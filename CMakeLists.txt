#==============================================================================
# Copyright (c) 2026 KritvaOS
# SPDX-License-Identifier: Apache-2.0
#
# File        : CMakeLists.txt
# Description : Kritva Core build definition
#
# Component   : Kritva Core
# Module      : Build
# Layer       : Development Infrastructure
#
# Requirements: CORE-BUILD-001
# API         : CMake
#
# Author      : KritvaOS Core Team
# Created     : 26-09-2026
#==============================================================================

cmake_minimum_required(VERSION 3.24)
project(kritva-core VERSION 0.2.0 LANGUAGES CXX)

option(KRITVA_CORE_BUILD_TESTS "Build Core tests" ON)
option(KRITVA_CORE_BUILD_EXAMPLES "Build Core examples" OFF)

add_library(kritva_core
    src/version.cpp
    src/metadata.cpp
    src/lifecycle.cpp
    src/capability_set.cpp
    src/configuration.cpp
)

target_compile_features(kritva_core PUBLIC cxx_std_20)
target_include_directories(kritva_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(KRITVA_CORE_BUILD_TESTS)
    enable_testing()

    # Tests use assert(); keep them active in Release/RelWithDebInfo builds.
    # Applies only to targets created below, not to the kritva_core library.
    if(MSVC)
        add_compile_options(/UNDEBUG)
    else()
        add_compile_options(-UNDEBUG)
    endif()

    #Add Test: tests/unit/version_test.cpp
    add_executable(kritva_core_version_test tests/unit/version_test.cpp)
    target_link_libraries(kritva_core_version_test PRIVATE kritva_core)
    add_test(NAME kritva_core_version COMMAND kritva_core_version_test)

    #Add Test: tests/unit/lifecycle_test.cpp
    add_executable(kritva_core_lifecycle_test tests/unit/lifecycle_test.cpp)
    target_link_libraries(kritva_core_lifecycle_test PRIVATE kritva_core)
    add_test(NAME kritva_core_lifecycle COMMAND kritva_core_lifecycle_test)

    #Add Test: tests/contract/core_contract_test.cpp
    add_executable(kritva_core_contract_test tests/contract/core_contract_test.cpp)
    target_link_libraries(kritva_core_contract_test PRIVATE kritva_core)
    add_test(NAME kritva_core_contract COMMAND kritva_core_contract_test)

    #Add Test: tests/unit/status_test.cpp
    add_executable(kritva_core_status_test tests/unit/status_test.cpp)
    target_link_libraries(kritva_core_status_test PRIVATE kritva_core)
    add_test( NAME kritva_core_status COMMAND kritva_core_status_test)

    #Add Test: tests/unit/health_test.cpp
    add_executable(kritva_core_health_test tests/unit/health_test.cpp)
    target_link_libraries(kritva_core_health_test PRIVATE kritva_core)
    add_test( NAME kritva_core_health COMMAND kritva_core_health_test)

    #Add Test: tests/unit/error_test.cpp
    add_executable(kritva_core_error_test tests/unit/error_test.cpp)
    target_link_libraries(kritva_core_error_test PRIVATE kritva_core)
    add_test(NAME kritva_core_error COMMAND kritva_core_error_test)

    #Add Test: tests/unit/result_test.cpp
    add_executable(kritva_core_result_test tests/unit/result_test.cpp)
    target_link_libraries(kritva_core_result_test PRIVATE kritva_core)
    add_test(NAME kritva_core_result COMMAND kritva_core_result_test)

    #Add Test: tests/unit/capability_test.cpp
    add_executable(kritva_core_capability_test tests/unit/capability_test.cpp)
    target_link_libraries(kritva_core_capability_test PRIVATE kritva_core)
    add_test(NAME kritva_core_capability COMMAND kritva_core_capability_test)

    #Add Test: tests/unit/configuration_test.cpp
    add_executable(kritva_core_configuration_test tests/unit/configuration_test.cpp)
    target_link_libraries(kritva_core_configuration_test PRIVATE kritva_core)
    add_test(NAME kritva_core_configuration COMMAND kritva_core_configuration_test)

    #Add Test: tests/unit/event_test.cpp
    add_executable(kritva_core_event_test tests/unit/event_test.cpp)
    target_link_libraries(kritva_core_event_test PRIVATE kritva_core)
    add_test(NAME kritva_core_event COMMAND kritva_core_event_test)

    #Add Test: tests/unit/statistics_test.cpp
    add_executable(kritva_core_statistics_test tests/unit/statistics_test.cpp)
    target_link_libraries(kritva_core_statistics_test PRIVATE kritva_core)
    add_test(NAME kritva_core_statistics COMMAND kritva_core_statistics_test)

    #Add Test: tests/unit/types_test.cpp
    add_executable(kritva_core_types_test tests/unit/types_test.cpp)
    target_link_libraries(kritva_core_types_test PRIVATE kritva_core)
    add_test(NAME kritva_core_types COMMAND kritva_core_types_test)

    #Add Test: tests/unit/time_test.cpp
    add_executable(kritva_core_time_test tests/unit/time_test.cpp)
    target_link_libraries(kritva_core_time_test PRIVATE kritva_core)
    add_test(NAME kritva_core_time COMMAND kritva_core_time_test)

    #Add Test: tests/unit/runtime_test.cpp
    add_executable(kritva_core_runtime_test tests/unit/runtime_test.cpp)
    target_link_libraries(kritva_core_runtime_test PRIVATE kritva_core)
    add_test(NAME kritva_core_runtime COMMAND kritva_core_runtime_test)

    #Add Test: tests/unit/messaging_test.cpp
    add_executable(kritva_core_messaging_test tests/unit/messaging_test.cpp)
    target_link_libraries(kritva_core_messaging_test PRIVATE kritva_core)
    add_test(NAME kritva_core_messaging COMMAND kritva_core_messaging_test)

    #Add Test: tests/unit/platform_test.cpp
    add_executable(kritva_core_platform_test tests/unit/platform_test.cpp)
    target_link_libraries(kritva_core_platform_test PRIVATE kritva_core)
    add_test(NAME kritva_core_platform COMMAND kritva_core_platform_test)

endif()
