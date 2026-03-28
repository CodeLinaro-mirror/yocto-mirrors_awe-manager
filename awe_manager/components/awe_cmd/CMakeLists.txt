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

option(AWE_CMD_BUILD_TESTS "Build the AWE_CMD tests" ON)

add_library (awe_cmd_lib
    "${CMAKE_CURRENT_SOURCE_DIR}/src/awe_cmd.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/awe_cmd_responses.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/awe_cmd_logging.h"

    "${CMAKE_CURRENT_SOURCE_DIR}/include/awe_cmd.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/awe_cmd_types.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/awe_cmd_responses.h"
)

target_include_directories(awe_cmd_lib
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/external
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(awe_cmd_lib PROPERTIES
    PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/external/Errors.h"
)

target_link_libraries(awe_cmd_lib awosal_lib awemgr_tracing_h)

if(AWEMGR_BUILD_TESTS AND AWE_CMD_BUILD_TESTS)
message(STATUS "AWE_CMD: Building Tests")
add_subdirectory(tests)
endif()

install(TARGETS awe_cmd_lib
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION include/externals
)
