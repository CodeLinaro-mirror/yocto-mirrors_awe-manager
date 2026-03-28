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

add_executable(awosal_tst_socket_server socket_server.c)
add_executable(awosal_tst_socket_client socket_client.c)

target_link_libraries(awosal_tst_socket_server awosal_lib)
target_link_libraries(awosal_tst_socket_client awosal_lib)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/test_osal.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/test_mutex.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/test_thread.cpp
)

if(NOT AWEMGR_BUILD_USE_GLIB)
    list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/test_string.cpp)
endif()

add_executable(awe_osal_test 
    ${SOURCES}
)

target_include_directories(awe_osal_test PRIVATE ../src)

target_link_libraries(awe_osal_test awosal_lib gtest gtest_main gmock)

gtest_discover_tests(awe_osal_test)
