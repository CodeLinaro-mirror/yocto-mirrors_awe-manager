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
option(AWE_LOGGING_BUILD_TESTS "Build the AWE logging tests" ON)
option(AWE_LOGGING_AWEQ_MOCK
    "Replace awe_log_util.h with a mock when building with AWEMGR_LOGGING=AWEQ (enables full build+test without the real AWE-Q SDK)"
    OFF)

add_library(awemgr_tracing_h STATIC awemgr_logging.c)

target_include_directories(awemgr_tracing_h INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../awe_osal/include
)

target_link_libraries(awemgr_tracing_h awosal_lib)

if(AWEMGR_LOGGING STREQUAL "AWEQ")
    if(AWE_LOGGING_AWEQ_MOCK)
        message(STATUS "AWE Manager: Using mock for awe_log_util.h in AWE-Q logging mode")
        # Inject the mock header so every target that links awemgr_tracing_h
        # finds mocks/awe_log_util.h before any system path.
        # BEFORE + INTERFACE ensures propagation to all consumers.
        target_include_directories(awemgr_tracing_h BEFORE INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/mocks
        )
        # Provide a weak default implementation of mock_audio_base_log_impl so
        # every non-test binary links cleanly.  Test executables override it
        # with a strong (recording) definition of the same symbol.
        target_sources(awemgr_tracing_h PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/mocks/mock_audio_base_log.c
        )
    else()
        target_link_libraries(awemgr_tracing_h awe_utils)
    endif()
endif()

if(AWEMGR_BUILD_TESTS AND AWE_LOGGING_BUILD_TESTS)
    add_subdirectory(tests)
endif()
