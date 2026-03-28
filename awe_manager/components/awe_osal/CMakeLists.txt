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

option(AWE_OSAL_BUILD_EXAMPLES "Build the AWE OSAL example programs" ON)
option(AWE_OSAL_BUILD_TESTS "Build the AWE OSAL tests" ON)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/awosal_socket.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/awosal_logging.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/awosal_time.c
)

if(NOT AWEMGR_BUILD_USE_GLIB)
    list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/awosal_string.c)
endif()

find_package(Threads REQUIRED)

if (WIN32)
    list(APPEND SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Windows/win_thread.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Windows/win_mutex.c
    )
else()
    list(APPEND SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Linux/posix_thread.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Linux/posix_mutex.c
    )
    add_definitions(-DPLATFORM_POSIX)
endif()

add_library (awosal_lib
    ${SOURCES}
)

target_include_directories(awosal_lib
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(awosal_lib awemgr_tracing_h Threads::Threads)
if(WIN32)
    target_link_libraries(awosal_lib ws2_32)
endif()
if(AWEMGR_BUILD_USE_GLIB)
    target_link_libraries(awosal_lib glib-2.0)
endif()

# temp disabled. should be enabled via CMake flag if lib is to be installed or not
# install(TARGETS
#     awosal_lib
#     PUBLIC_HEADER
#     RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
# )

if(AWEMGR_BUILD_TESTS AND AWE_OSAL_BUILD_TESTS)
message(STATUS "AWOSAL: Building Tests")
add_subdirectory(tests)
endif()

if(AWEMGR_BUILD_ADDONS AND AWE_OSAL_BUILD_EXAMPLES)
message(STATUS "AWOSAL: Building Examples")
add_executable(example ${CMAKE_CURRENT_SOURCE_DIR}/example/example.c)
target_link_libraries(example awosal_lib)
endif()
