#==============================================================================
# Copyright (c) 2026 KritvaOS
# SPDX-License-Identifier: Apache-2.0
#
# File        : CMakeLists.txt
# Description : Minimal downstream consumer of an installed Kritva Core
#
# Component   : Kritva Core
# Module      : Install Test
# Layer       : Development Infrastructure
#
# Requirements: CORE-BUILD-002, CORE-COMPAT-010
# API         : CMake consumer project
#
# Author      : KritvaOS Core Team
# Created     : 02-10-2026
#==============================================================================

cmake_minimum_required(VERSION 3.24)
project(kritva_core_install_consumer LANGUAGES CXX)

# Locates the *installed* package only (CMAKE_PREFIX_PATH is set by the test).
set(KRITVA_CORE_REQUIRE_VERSION 0.9 CACHE STRING "Kritva Core version to request")
set(KRITVA_CORE_EXPECT_VERSION "" CACHE STRING "Exact version the installed package must report")
set(KRITVA_CORE_REQUIRE_EXACT OFF CACHE BOOL "Request the version with EXACT")
if(KRITVA_CORE_REQUIRE_EXACT)
  find_package(kritva_core ${KRITVA_CORE_REQUIRE_VERSION} EXACT CONFIG REQUIRED)
else()
  find_package(kritva_core ${KRITVA_CORE_REQUIRE_VERSION} CONFIG REQUIRED)
endif()

# The installed package must report the intended version, not merely satisfy the request.
if(NOT KRITVA_CORE_EXPECT_VERSION STREQUAL "" AND NOT kritva_core_VERSION VERSION_EQUAL KRITVA_CORE_EXPECT_VERSION)
  message(FATAL_ERROR "installed kritva_core is ${kritva_core_VERSION}, expected ${KRITVA_CORE_EXPECT_VERSION}")
endif()

add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE kritva_core::kritva_core)
