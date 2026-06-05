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
cmake_minimum_required(VERSION 3.10)

# Compile awemgr_logging.c with -DAWEMGR_LOGGING_AWEQ so the AWEQ macro branch
# is active.  We cannot reuse awemgr_tracing_h because that library is built
# without this define.
#
# The mock directory is placed BEFORE all other include directories so the
# preprocessor finds mocks/awe_log_util.h instead of the real customer header.

add_library(awemgr_logging_aweq_lib STATIC
    ../awemgr_logging.c
)
target_compile_definitions(awemgr_logging_aweq_lib PRIVATE
    AWEMGR_LOGGING_AWEQ
)
target_include_directories(awemgr_logging_aweq_lib BEFORE PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks          # provides mock awe_log_util.h
)
target_include_directories(awemgr_logging_aweq_lib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..             # awemgr_logging.h
    ${CMAKE_SOURCE_DIR}/awe_manager/components/awe_osal/include  # awosal_string.h
)

# ---------------------------------------------------------------------------

add_executable(logging_aweq_test test_logging.cpp)
target_compile_definitions(logging_aweq_test PRIVATE
    AWEMGR_LOGGING_AWEQ
)
target_include_directories(logging_aweq_test BEFORE PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
)
target_include_directories(logging_aweq_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(logging_aweq_test
    awemgr_logging_aweq_lib
    gtest
    gtest_main
)
target_compile_features(logging_aweq_test PRIVATE cxx_std_14)

gtest_discover_tests(logging_aweq_test)
