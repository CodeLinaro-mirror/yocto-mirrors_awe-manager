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

if(NOT GIT_EXECUTABLE)
  set(GIT_EXECUTABLE git)
endif()

if(GIT_EXECUTABLE)
  message(STATUS "AWE Manager: Trying to obtain GIT version ...")
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags --dirty
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_DESCRIBE_VERSION
    RESULT_VARIABLE GIT_DESCRIBE_ERROR_CODE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  if(NOT GIT_DESCRIBE_ERROR_CODE)
    set(COMPONENT_VERSION ${GIT_DESCRIBE_VERSION})
  endif()
endif()

# fallback
if(NOT DEFINED COMPONENT_VERSION)
  set(COMPONENT_VERSION 0.0.0-unknown)
  message(WARNING "Failed to determine COMPONENT_VERSION from Git tags. Using default version \"${COMPONENT_VERSION}\".")
endif()

