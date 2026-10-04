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
# Requirements: CORE-BUILD-001, CORE-BUILD-002
# API         : CMake
#
# Author      : KritvaOS Core Team
# Created     : 26-09-2026
#==============================================================================

cmake_minimum_required(VERSION 3.24)
project(kritva-core VERSION 0.8.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(KRITVA_CORE_BUILD_TESTS "Build Core tests" ON)
option(KRITVA_CORE_BUILD_EXAMPLES "Build Core examples" OFF)

add_library(kritva_core
    src/version.cpp
    src/metadata.cpp
    src/lifecycle.cpp
    src/capability_set.cpp
    src/configuration.cpp
    src/component_registry.cpp
    src/dependency_graph.cpp
    src/runtime_manager.cpp
)

target_compile_features(kritva_core PUBLIC cxx_std_20)
target_include_directories(kritva_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# The same namespaced name is used in the build tree and by installed consumers
# (find_package(kritva_core) -> kritva_core::kritva_core).
add_library(kritva_core::kritva_core ALIAS kritva_core)

# Installation (CORE-BUILD-002): library, public headers, CMake package.
set(KRITVA_CORE_CMAKE_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/kritva_core)

install(TARGETS kritva_core
    EXPORT kritva_coreTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
)

install(EXPORT kritva_coreTargets
    NAMESPACE kritva_core::
    DESTINATION ${KRITVA_CORE_CMAKE_INSTALL_DIR}
)

configure_package_config_file(
    cmake/kritva_coreConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/kritva_coreConfig.cmake
    INSTALL_DESTINATION ${KRITVA_CORE_CMAKE_INSTALL_DIR}
)

# Pre-1.0: a different minor version may change the API, so match the minor.
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/kritva_coreConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMinorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kritva_coreConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/kritva_coreConfigVersion.cmake
    DESTINATION ${KRITVA_CORE_CMAKE_INSTALL_DIR}
)

if(KRITVA_CORE_BUILD_TESTS)
    enable_testing()

    # Every public header must compile on its own (self-containment): one generated
    # translation unit per header, built with the library's flags and include path.
    file(GLOB_RECURSE KRITVA_PUBLIC_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include
         ${CMAKE_CURRENT_SOURCE_DIR}/include/kritva/core/*.hpp)
    set(KRITVA_HEADER_CHECK_SOURCES "")
    foreach(header IN LISTS KRITVA_PUBLIC_HEADERS)
        string(REPLACE "/" "_" header_name "${header}")
        set(check_source ${CMAKE_BINARY_DIR}/header_checks/${header_name}.cpp)
        file(GENERATE OUTPUT ${check_source} CONTENT "#include <${header}>\n")
        list(APPEND KRITVA_HEADER_CHECK_SOURCES ${check_source})
    endforeach()
    add_library(kritva_core_header_checks OBJECT ${KRITVA_HEADER_CHECK_SOURCES})
    target_link_libraries(kritva_core_header_checks PRIVATE kritva_core)

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

    #Add Test: tests/contract/foundation_contract_test.cpp
    add_executable(kritva_core_foundation_contract_test tests/contract/foundation_contract_test.cpp)
    target_link_libraries(kritva_core_foundation_contract_test PRIVATE kritva_core)
    add_test(NAME kritva_core_foundation_contract COMMAND kritva_core_foundation_contract_test)

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

    #Add Test: tests/unit/component_test.cpp
    add_executable(kritva_core_component_test tests/unit/component_test.cpp)
    target_link_libraries(kritva_core_component_test PRIVATE kritva_core)
    add_test(NAME kritva_core_component COMMAND kritva_core_component_test)

    #Add Test: tests/unit/component_registry_test.cpp
    add_executable(kritva_core_component_registry_test tests/unit/component_registry_test.cpp)
    target_link_libraries(kritva_core_component_registry_test PRIVATE kritva_core)
    add_test(NAME kritva_core_component_registry COMMAND kritva_core_component_registry_test)

    #Add Test: tests/unit/dependency_graph_test.cpp
    add_executable(kritva_core_dependency_graph_test tests/unit/dependency_graph_test.cpp)
    target_link_libraries(kritva_core_dependency_graph_test PRIVATE kritva_core)
    add_test(NAME kritva_core_dependency_graph COMMAND kritva_core_dependency_graph_test)

    #Add Test: tests/unit/runtime_manager_test.cpp
    add_executable(kritva_core_runtime_manager_test tests/unit/runtime_manager_test.cpp)
    target_link_libraries(kritva_core_runtime_manager_test PRIVATE kritva_core)
    add_test(NAME kritva_core_runtime_manager COMMAND kritva_core_runtime_manager_test)

    #Add Test: tests/unit/runtime_lifecycle_test.cpp
    add_executable(kritva_core_runtime_lifecycle_test tests/unit/runtime_lifecycle_test.cpp)
    target_link_libraries(kritva_core_runtime_lifecycle_test PRIVATE kritva_core)
    add_test(NAME kritva_core_runtime_lifecycle COMMAND kritva_core_runtime_lifecycle_test)

    #Add Test: tests/unit/runtime_failure_test.cpp
    add_executable(kritva_core_runtime_failure_test tests/unit/runtime_failure_test.cpp)
    target_link_libraries(kritva_core_runtime_failure_test PRIVATE kritva_core)
    add_test(NAME kritva_core_runtime_failure COMMAND kritva_core_runtime_failure_test)

    #Add Test: tests/integration/runtime_integration_test.cpp
    add_executable(kritva_core_runtime_integration_test tests/integration/runtime_integration_test.cpp)
    target_link_libraries(kritva_core_runtime_integration_test PRIVATE kritva_core)
    add_test(NAME kritva_core_runtime_integration COMMAND kritva_core_runtime_integration_test)

    #Add Test: tests/integration/runtime_platform_test.cpp
    add_executable(kritva_core_runtime_platform_test tests/integration/runtime_platform_test.cpp)
    target_link_libraries(kritva_core_runtime_platform_test PRIVATE kritva_core)
    add_test(NAME kritva_core_runtime_platform COMMAND kritva_core_runtime_platform_test)

    #Add Test: tests/integration/runtime_platform_integration_test.cpp (Runtime + PlatformContext + reference platform)
    add_executable(kritva_core_runtime_platform_integration_test tests/integration/runtime_platform_integration_test.cpp)
    target_link_libraries(kritva_core_runtime_platform_integration_test PRIVATE kritva_core)
    add_test(NAME kritva_core_runtime_platform_integration COMMAND kritva_core_runtime_platform_integration_test)

    #Add Test: tests/integration/component_context_integration_test.cpp (Runtime + Component + ComponentContext + reference platform)
    add_executable(kritva_core_component_context_integration_test tests/integration/component_context_integration_test.cpp)
    target_link_libraries(kritva_core_component_context_integration_test PRIVATE kritva_core)
    add_test(NAME kritva_core_component_context_integration COMMAND kritva_core_component_context_integration_test)

    #Add Test: tests/integration/component_operational_integration_test.cpp (Runtime + operational components + sink + reference platform)
    add_executable(kritva_core_component_operational_integration_test tests/integration/component_operational_integration_test.cpp)
    target_link_libraries(kritva_core_component_operational_integration_test PRIVATE kritva_core)
    add_test(NAME kritva_core_component_operational_integration COMMAND kritva_core_component_operational_integration_test)

    #Add Test: tests/integration/component_configuration_integration_test.cpp (Runtime + configuration-aware components + reference platform)
    add_executable(kritva_core_component_configuration_integration_test tests/integration/component_configuration_integration_test.cpp)
    target_link_libraries(kritva_core_component_configuration_integration_test PRIVATE kritva_core)
    add_test(NAME kritva_core_component_configuration_integration COMMAND kritva_core_component_configuration_integration_test)

    #Add Test: tests/unit/platform_context_test.cpp
    add_executable(kritva_core_platform_context_test tests/unit/platform_context_test.cpp)
    target_link_libraries(kritva_core_platform_context_test PRIVATE kritva_core)
    add_test(NAME kritva_core_platform_context COMMAND kritva_core_platform_context_test)

    #Add Test: tests/unit/platform_requirements_test.cpp
    add_executable(kritva_core_platform_requirements_test tests/unit/platform_requirements_test.cpp)
    target_link_libraries(kritva_core_platform_requirements_test PRIVATE kritva_core)
    add_test(NAME kritva_core_platform_requirements COMMAND kritva_core_platform_requirements_test)

    #Add Test: tests/unit/platform_service_test.cpp
    add_executable(kritva_core_platform_service_test tests/unit/platform_service_test.cpp)
    target_link_libraries(kritva_core_platform_service_test PRIVATE kritva_core)
    add_test(NAME kritva_core_platform_service COMMAND kritva_core_platform_service_test)

    #Add Test: tests/unit/runtime_platform_lifecycle_test.cpp
    add_executable(kritva_core_runtime_platform_lifecycle_test tests/unit/runtime_platform_lifecycle_test.cpp)
    target_link_libraries(kritva_core_runtime_platform_lifecycle_test PRIVATE kritva_core)
    add_test(NAME kritva_core_runtime_platform_lifecycle COMMAND kritva_core_runtime_platform_lifecycle_test)

    #Add Test: tests/unit/component_context_test.cpp
    add_executable(kritva_core_component_context_test tests/unit/component_context_test.cpp)
    target_link_libraries(kritva_core_component_context_test PRIVATE kritva_core)
    add_test(NAME kritva_core_component_context COMMAND kritva_core_component_context_test)

    #Add Test: tests/unit/component_context_access_test.cpp
    add_executable(kritva_core_component_context_access_test tests/unit/component_context_access_test.cpp)
    target_link_libraries(kritva_core_component_context_access_test PRIVATE kritva_core)
    add_test(NAME kritva_core_component_context_access COMMAND kritva_core_component_context_access_test)

    #Add Test: tests/unit/component_context_injection_test.cpp
    add_executable(kritva_core_component_context_injection_test tests/unit/component_context_injection_test.cpp)
    target_link_libraries(kritva_core_component_context_injection_test PRIVATE kritva_core)
    add_test(NAME kritva_core_component_context_injection COMMAND kritva_core_component_context_injection_test)

    #Add Test: tests/unit/component_context_requirements_test.cpp
    add_executable(kritva_core_component_context_requirements_test tests/unit/component_context_requirements_test.cpp)
    target_link_libraries(kritva_core_component_context_requirements_test PRIVATE kritva_core)
    add_test(NAME kritva_core_component_context_requirements COMMAND kritva_core_component_context_requirements_test)

    #Add Test: tests/unit/component_observation_test.cpp
    add_executable(kritva_core_component_observation_test tests/unit/component_observation_test.cpp)
    target_link_libraries(kritva_core_component_observation_test PRIVATE kritva_core)
    add_test(NAME kritva_core_component_observation COMMAND kritva_core_component_observation_test)

    #Add Test: tests/unit/component_status_health_test.cpp
    add_executable(kritva_core_component_status_health_test tests/unit/component_status_health_test.cpp)
    target_link_libraries(kritva_core_component_status_health_test PRIVATE kritva_core)
    add_test(NAME kritva_core_component_status_health COMMAND kritva_core_component_status_health_test)

    #Add Test: tests/unit/component_events_test.cpp
    add_executable(kritva_core_component_events_test tests/unit/component_events_test.cpp)
    target_link_libraries(kritva_core_component_events_test PRIVATE kritva_core)
    add_test(NAME kritva_core_component_events COMMAND kritva_core_component_events_test)

    #Add Test: tests/unit/component_statistics_test.cpp
    add_executable(kritva_core_component_statistics_test tests/unit/component_statistics_test.cpp)
    target_link_libraries(kritva_core_component_statistics_test PRIVATE kritva_core)
    add_test(NAME kritva_core_component_statistics COMMAND kritva_core_component_statistics_test)

    #Add Test: tests/unit/reference_operational_test.cpp (the test-only operational harness in tests/runtime/)
    add_executable(kritva_core_reference_operational_test tests/unit/reference_operational_test.cpp)
    target_link_libraries(kritva_core_reference_operational_test PRIVATE kritva_core)
    add_test(NAME kritva_core_reference_operational COMMAND kritva_core_reference_operational_test)

    #Add Test: tests/unit/component_configuration_test.cpp
    add_executable(kritva_core_component_configuration_test tests/unit/component_configuration_test.cpp)
    target_link_libraries(kritva_core_component_configuration_test PRIVATE kritva_core)
    add_test(NAME kritva_core_component_configuration COMMAND kritva_core_component_configuration_test)

    #Add Test: tests/unit/configuration_ownership_test.cpp
    add_executable(kritva_core_configuration_ownership_test tests/unit/configuration_ownership_test.cpp)
    target_link_libraries(kritva_core_configuration_ownership_test PRIVATE kritva_core)
    add_test(NAME kritva_core_configuration_ownership COMMAND kritva_core_configuration_ownership_test)

    #Add Test: tests/unit/configuration_validation_test.cpp
    add_executable(kritva_core_configuration_validation_test tests/unit/configuration_validation_test.cpp)
    target_link_libraries(kritva_core_configuration_validation_test PRIVATE kritva_core)
    add_test(NAME kritva_core_configuration_validation COMMAND kritva_core_configuration_validation_test)

    #Add Test: tests/unit/reference_configuration_test.cpp (the test-only configuration harness in tests/runtime/)
    add_executable(kritva_core_reference_configuration_test tests/unit/reference_configuration_test.cpp)
    target_link_libraries(kritva_core_reference_configuration_test PRIVATE kritva_core)
    add_test(NAME kritva_core_reference_configuration COMMAND kritva_core_reference_configuration_test)

    #Add Test: tests/unit/configuration_boundary_test.cpp (compile-time snapshot of the frozen configuration boundary)
    add_executable(kritva_core_configuration_boundary_test tests/unit/configuration_boundary_test.cpp)
    target_link_libraries(kritva_core_configuration_boundary_test PRIVATE kritva_core)
    add_test(NAME kritva_core_configuration_boundary COMMAND kritva_core_configuration_boundary_test)

    #Add Test: tests/unit/capability_contract_test.cpp
    add_executable(kritva_core_capability_contract_test tests/unit/capability_contract_test.cpp)
    target_link_libraries(kritva_core_capability_contract_test PRIVATE kritva_core)
    add_test(NAME kritva_core_capability_contract COMMAND kritva_core_capability_contract_test)

    #Add Test: tests/unit/capability_set_contract_test.cpp
    add_executable(kritva_core_capability_set_contract_test tests/unit/capability_set_contract_test.cpp)
    target_link_libraries(kritva_core_capability_set_contract_test PRIVATE kritva_core)
    add_test(NAME kritva_core_capability_set_contract COMMAND kritva_core_capability_set_contract_test)

    #Add Test: tests/unit/capability_requirement_boundary_test.cpp
    add_executable(kritva_core_capability_requirement_boundary_test tests/unit/capability_requirement_boundary_test.cpp)
    target_link_libraries(kritva_core_capability_requirement_boundary_test PRIVATE kritva_core)
    add_test(NAME kritva_core_capability_requirement_boundary COMMAND kritva_core_capability_requirement_boundary_test)

    #Add Test: tests/unit/reference_context_test.cpp (the test-only context harness in tests/runtime/ and tests/platform/)
    add_executable(kritva_core_reference_context_test tests/unit/reference_context_test.cpp)
    target_link_libraries(kritva_core_reference_context_test PRIVATE kritva_core)
    add_test(NAME kritva_core_reference_context COMMAND kritva_core_reference_context_test)

    #Add Test: tests/unit/reference_platform_test.cpp (the test-only reference platform in tests/platform/)
    add_executable(kritva_core_reference_platform_test tests/unit/reference_platform_test.cpp)
    target_link_libraries(kritva_core_reference_platform_test PRIVATE kritva_core)
    add_test(NAME kritva_core_reference_platform COMMAND kritva_core_reference_platform_test)

    #Add Test: tests/audit/production_isolation.cmake (production code never depends on test-only support)
    add_test(NAME kritva_core_test_isolation
        COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -DCXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DWORK_DIR=${CMAKE_BINARY_DIR}/isolation_test
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/audit/production_isolation.cmake)

    #Add Test: tests/unit/platform_boundary_test.cpp
    add_executable(kritva_core_platform_boundary_test tests/unit/platform_boundary_test.cpp)
    target_link_libraries(kritva_core_platform_boundary_test PRIVATE kritva_core)
    add_test(NAME kritva_core_platform_boundary COMMAND kritva_core_platform_boundary_test)

    #Add Test: tests/unit/scheduler_contract_test.cpp
    add_executable(kritva_core_scheduler_contract_test tests/unit/scheduler_contract_test.cpp)
    target_link_libraries(kritva_core_scheduler_contract_test PRIVATE kritva_core)
    add_test(NAME kritva_core_scheduler_contract COMMAND kritva_core_scheduler_contract_test)

    #Add Test: tests/unit/timer_contract_test.cpp
    add_executable(kritva_core_timer_contract_test tests/unit/timer_contract_test.cpp)
    target_link_libraries(kritva_core_timer_contract_test PRIVATE kritva_core)
    add_test(NAME kritva_core_timer_contract COMMAND kritva_core_timer_contract_test)

    #Add Test: tests/unit/watchdog_contract_test.cpp
    add_executable(kritva_core_watchdog_contract_test tests/unit/watchdog_contract_test.cpp)
    target_link_libraries(kritva_core_watchdog_contract_test PRIVATE kritva_core)
    add_test(NAME kritva_core_watchdog_contract COMMAND kritva_core_watchdog_contract_test)

    #Add Test: tests/unit/platform_adapter_test.cpp
    add_executable(kritva_core_platform_adapter_test tests/unit/platform_adapter_test.cpp)
    target_link_libraries(kritva_core_platform_adapter_test PRIVATE kritva_core)
    add_test(NAME kritva_core_platform_adapter COMMAND kritva_core_platform_adapter_test)

    #Add Test: tests/unit/platform_conformance_test.cpp (runs the reusable suite in tests/platform/)
    add_executable(kritva_core_platform_conformance_test tests/unit/platform_conformance_test.cpp)
    target_link_libraries(kritva_core_platform_conformance_test PRIVATE kritva_core)
    add_test(NAME kritva_core_platform_conformance COMMAND kritva_core_platform_conformance_test)

    #Add Test: tests/unit/messaging_test.cpp
    add_executable(kritva_core_messaging_test tests/unit/messaging_test.cpp)
    target_link_libraries(kritva_core_messaging_test PRIVATE kritva_core)
    add_test(NAME kritva_core_messaging COMMAND kritva_core_messaging_test)

    #Add Test: tests/unit/platform_test.cpp
    add_executable(kritva_core_platform_test tests/unit/platform_test.cpp)
    target_link_libraries(kritva_core_platform_test PRIVATE kritva_core)
    add_test(NAME kritva_core_platform COMMAND kritva_core_platform_test)

    #Add Test: tests/install (install the library and build a consumer against it)
    add_test(NAME kritva_core_install_consumer
        COMMAND ${CMAKE_COMMAND}
            -DCORE_BUILD_DIR=${CMAKE_BINARY_DIR}
            -DCONSUMER_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/tests/install/consumer
            -DWORK_DIR=${CMAKE_BINARY_DIR}/install_test
            -DCXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DEXPECTED_VERSION=${PROJECT_VERSION}
            "-DCXX_FLAGS=${CMAKE_CXX_FLAGS}"
            -DCONFIG=$<CONFIG>
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/install/run_install_test.cmake
    )

endif()
