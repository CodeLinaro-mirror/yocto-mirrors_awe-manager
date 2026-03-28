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
option(AWE_CTRL_BUILD_TESTS "Build the AWE_CTRL tests" ON)

set(SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/awe_ctrl.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/awe_event.c"
)

if(DEFINED AWEMGR_AWECORE_CONNECTION AND AWEMGR_AWECORE_CONNECTION STREQUAL "CSHMEM")
    list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/awe_evt_shm.c")
else()
    list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/awe_evt_socket.c")
endif()

add_library (awe_ctrl_lib ${SOURCES})
target_include_directories(awe_ctrl_lib PUBLIC include)

target_link_libraries(awe_ctrl_lib awe_cmd_lib awosal_lib awe_config_lib awemgr_tracing_h)
if(DEFINED AWEMGR_AWECORE_CONNECTION AND AWEMGR_AWECORE_CONNECTION STREQUAL "CSHMEM")
add_definitions (-DLINUX)
target_link_libraries(awe_ctrl_lib awe_alsa awe_packetclient awe_audiolite_osal)
endif()

if(AWEMGR_BUILD_TESTS AND AWE_CTRL_BUILD_TESTS)
message(STATUS "AWE_CTRL: Building Tests")
add_subdirectory(tests)
endif()

# temp disabled. should be enabled via CMake flag if lib is to be installed or not
# install(TARGETS awe_ctrl_lib
#     RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
# )
