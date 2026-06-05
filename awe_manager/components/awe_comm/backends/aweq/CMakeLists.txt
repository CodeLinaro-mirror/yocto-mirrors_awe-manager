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

set(SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/awe_event_aweq.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/awe_event_aweq_cfg.c"

    "${CMAKE_CURRENT_SOURCE_DIR}/src/awe_comm_aweq.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/awe_comm_aweq_cfg.c"
)

add_definitions (-DLINUX)

add_library (awe_comm_backend_aweq_lib ${SOURCES})

target_include_directories(awe_comm_backend_aweq_lib
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include     # the general backend include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include  # the component include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src      # pointing to SRC dir for internal headers
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(awe_comm_backend_aweq_lib awe_cmd_lib awosal_lib awe_config_lib awemgr_tracing_h)

# rest is QC/Ease code - ideally we would like to avoid linking to these directly, but for now we need them for the aweq backend
target_link_libraries(awe_comm_backend_aweq_lib awe_alsa awe_packetclient awe_audiolite_osal)
