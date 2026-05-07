# MIT License
#
# Copyright (c) 2026 DSP Concepts, Inc.
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


cmake_minimum_required(VERSION 3.10)

project(awe_awc)

option(AWE_AWC_BUILD_EXAMPLES "Build the AWC example programs" ON)
option(AWE_AWC_BUILD_TESTS "Build the AWC tests" ON)

add_library(awe_awc
   ${CMAKE_CURRENT_SOURCE_DIR}/src/awe_awc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/src/awc_internal.c
   ${CMAKE_CURRENT_SOURCE_DIR}/src/awc_map.c )

target_include_directories(awe_awc
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../awe_osal/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../awe_cmd/include
)

target_link_libraries(awe_awc awosal_lib awemgr_tracing_h)
if(NOT CMAKE_HOST_WIN32)
    target_link_libraries(awe_awc m)
endif()

set(AWC_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests/data")
add_compile_options(-DAWC_DATA_DIR="${AWC_DATA_DIR}")

if(AWEMGR_BUILD_TESTS AND AWE_AWC_BUILD_TESTS)
message(STATUS "AWC: Building Tests")
add_subdirectory(tests)
endif()

if(AWEMGR_BUILD_ADDONS OR AWE_AWC_BUILD_EXAMPLES)
message(STATUS "AWC: Building Examples")
add_subdirectory(examples)
endif()
