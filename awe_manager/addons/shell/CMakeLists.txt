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

add_library(awemgr_shell_lib STATIC
    src/awemgr_shell.cpp
    src/cmds_base.cpp
    src/cmds_control.cpp
    src/cmds_events.cpp
    src/cmds_reset.cpp
    src/cmds_sendcommand.cpp
    src/cmds_showinfo.cpp
    src/cmds_targetinfo.cpp
    src/cmds_userdata.cpp
    src/cmds_signalflow_dtmf.cpp
    src/hlp_dump_response.cpp
)

target_include_directories(awemgr_shell_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src   # for awemgr_util.h
)

set_target_properties(awemgr_shell_lib PROPERTIES
    # Promote the addon API header so install(... PUBLIC_HEADER ...) actually
    # copies it into the package include directory.
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include/awemgr_shell.h
)

target_link_libraries(awemgr_shell_lib
    PUBLIC  awe_manager
    PRIVATE idbg_lib
)

install(TARGETS awemgr_shell_lib
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
