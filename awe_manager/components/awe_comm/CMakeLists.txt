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
option(AWE_COMM_BUILD_TESTS "Build the AWE_COMM tests" ON)

set(SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/awe_comm.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/awe_comm_backend_selector.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/awe_comm_trace.c"

    "${CMAKE_CURRENT_SOURCE_DIR}/src/awe_event.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/awe_event_backend_selector.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/awe_event_trace.c"
)

add_library (awe_comm_lib ${SOURCES})

target_include_directories(awe_comm_lib
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/backends/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(DEFINED AWEMGR_AWECORE_CONNECTION AND AWEMGR_AWECORE_CONNECTION STREQUAL "CSHMEM")
    add_subdirectory(backends/aweq)
    target_link_libraries(awe_comm_lib awe_comm_backend_aweq_lib)
endif()
if(DEFINED AWEMGR_AWECORE_CONNECTION AND AWEMGR_AWECORE_CONNECTION STREQUAL "SOCKET")
    add_subdirectory(backends/tcp_socket)
    target_link_libraries(awe_comm_lib awe_comm_backend_socket_lib)
endif()


if(AWEMGR_BUILD_TESTS AND AWE_COMM_BUILD_TESTS)
message(STATUS "AWE_COMM: Building Tests")
add_subdirectory(tests)
endif()

# temp disabled. should be enabled via CMake flag if lib is to be installed or not
# install(TARGETS awe_comm_lib
#     RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
# )
