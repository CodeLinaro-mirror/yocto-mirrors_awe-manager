# MIT License
#
# Copyright (c) 2024 DSP Concepts, Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#


cmake_minimum_required (VERSION 3.8)

# -----------------------------------------------------------------
#
# global build options
#
# -----------------------------------------------------------------
option(AWEMGR_BUILD_SHARED_LIB "Build AWE Manager as a shared library" OFF)  # not: not usable for now!
option(AWEMGR_BUILD_SINGLE_LIB "Combines all AWE-Manager libraries into one big archive" OFF)
option(AWEMGR_BUILD_USE_GLIB "Include glib library for string functions" OFF)
option(AWEMGR_BUILD_TESTS "Build the awe_manager tests" ON)
option(AWEMGR_BUILD_ADDONS "Build additional targets, like the AWE-MgrShell program" ON)
option(AWEMGR_TESTS_COVERAGE "When building tests include coverage (gcov)" OFF)
option(AWEMGR_STATIC_ANALYSIS "Enable targets for running static analysis." OFF)
option(AWEMGR_ENABLE_ALL_WARNINGS "Compile with '-Wall' flag to enable all warnings" OFF)

set(AWEMGR_CUSTOM_VERSION "" CACHE STRING "Allows to use an additional custom version to be included in AWE Manager code")

if(AWEMGR_ENABLE_ALL_WARNINGS)
    add_compile_options(-Wall)
endif()
# -----------------------------------------------------------------

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/bundle_static_library.cmake)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateVersionHeader.cmake)
message(STATUS "AWE Manager: Setting component version to " ${COMPONENT_VERSION})
if(AWEMGR_CUSTOM_VERSION)
message(STATUS "AWE Manager: Setting custom version to " ${AWEMGR_CUSTOM_VERSION})
endif()

# communication type to AWECore
set(AWEMGR_CTRL_COMM_TYPES SOCKET CSHMEM)
set(AWEMGR_AWECORE_CONNECTION SOCKET CACHE STRING "Selects how AWE Manager connects to AWECore: SOCKET or CSHMEM")
set_property(CACHE AWEMGR_AWECORE_CONNECTION PROPERTY STRINGS ${AWEMGR_CTRL_COMM_TYPES})
if(NOT AWEMGR_AWECORE_CONNECTION IN_LIST AWEMGR_CTRL_COMM_TYPES)
    message(FATAL_ERROR "AWEMGR_AWECORE_CONNECTION may only be: ${AWEMGR_CTRL_COMM_TYPES}")
endif()
message(STATUS "AWE Manager: Communication backend to AWECore is ${AWEMGR_AWECORE_CONNECTION} (-DAWEMGR_AWECORE_CONNECTION_${AWEMGR_AWECORE_CONNECTION})")
add_compile_options(-DAWEMGR_AWECORE_CONNECTION_${AWEMGR_AWECORE_CONNECTION})

# tracer backend for logging
set(AWEMGR_LOG_TYPES STDIO SYSLOG)
set(AWEMGR_LOGGING STDIO CACHE STRING "Select logging backend: STDIO or SYSLOG")
set_property(CACHE AWEMGR_LOGGING PROPERTY STRINGS ${AWEMGR_LOG_TYPES})
if(NOT AWEMGR_LOGGING IN_LIST AWEMGR_LOG_TYPES)
    message(FATAL_ERROR "AWEMGR_LOGGING may only be: ${AWEMGR_LOG_TYPES}")
endif()
message(STATUS "AWE Manager: Tracer backend is ${AWEMGR_LOGGING} (-DAWEMGR_LOGGING_${AWEMGR_LOGGING})")
add_compile_options(-DAWEMGR_LOGGING_${AWEMGR_LOGGING})

# -----------------------------------------------------------------

if(AWEMGR_BUILD_TESTS)
    CPMAddPackage(
    NAME googletest
    GITHUB_REPOSITORY google/googletest
    GIT_TAG release-1.12.1
    VERSION 1.12.1
    OPTIONS "INSTALL_GTEST OFF" "gtest_force_shared_crt"
    )

    include(GoogleTest)
endif()

project("awe_manager")

include(GNUInstallDirs)

enable_testing()

if(CMAKE_HOST_WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)

    if(AWEMGR_BUILD_SINGLE_LIB)
        message(FATAL_ERROR "Single big static library not supported on Windows")
    endif()
endif()

if(AWEMGR_BUILD_SHARED_LIB)
    if(AWEMGR_BUILD_SINGLE_LIB)
        message(FATAL_ERROR "Single big static library as shared lib is (yet) supported")
    endif()
    add_definitions (-fPIC)
    set(AWEMGR_BUILD_TYPE SHARED)
    message(STATUS "AWE Manager: Will be build as SHARED library")
else()
    set(AWEMGR_BUILD_TYPE STATIC)
    message(STATUS "AWE Manager: Will be build as STATIC library")
endif()

if(AWEMGR_BUILD_USE_GLIB)
    message(STATUS "AWEOSAL: Using GLIB library for some string functions (-DAWEMGR_BUILD_USE_GLIB)")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLIB REQUIRED glib-2.0)
    include_directories (${GLIB_INCLUDE_DIRS})
    add_compile_options(-DAWEMGR_BUILD_USE_GLIB)
endif()

if (AWEMGR_BUILD_TESTS AND AWEMGR_TESTS_COVERAGE)
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        # Enabling coverage will decrease optimization level to 0
        # so coverage reports are as accurate as possible
        set(cov_flags "--coverage -fprofile-abs-path -O0 -g")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${cov_flags}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${cov_flags}")
        message(STATUS "AWE Manager: Building Tests - with coverage, adding flags: ${cov_flags}")

        # set(covAnalysisFile ${CMAKE_SOURCE_DIR}/scripts/run_coverage_analysis.sh)
        # file(COPY ${covAnalysisFile} DESTINATION ${CMAKE_BINARY_DIR})
    else()
        message(FATAL_ERROR "ENABLE_COVERAGE only supported for GCC/GNU compilers. Cannot support ${CMAKE_C_COMPILER_ID}")
    endif()
endif()

add_subdirectory(awe_manager)

if(AWEMGR_BUILD_TESTS)
    include(cmake/testing.cmake)
endif()

if(AWEMGR_STATIC_ANALYSIS AND NOT WIN32)
    include(cmake/cppcheck.cmake)
endif()

install(FILES README.md TYPE DOC)
