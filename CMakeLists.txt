cmake_minimum_required(VERSION 3.24)

project(kritva_core
    VERSION 0.1.0
    DESCRIPTION "Kritva Core Foundation"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(KRITVA_CORE_BUILD_TESTS "Build Kritva Core tests" ON)
option(KRITVA_CORE_BUILD_EXAMPLES "Build Kritva Core examples" OFF)

add_library(kritva_core INTERFACE)

target_include_directories(kritva_core
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(kritva_core INTERFACE cxx_std_20)

if(KRITVA_CORE_BUILD_TESTS)
    enable_testing()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
        add_subdirectory(tests)
    endif()
endif()

if(KRITVA_CORE_BUILD_EXAMPLES)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt")
        add_subdirectory(examples)
    endif()
endif()
