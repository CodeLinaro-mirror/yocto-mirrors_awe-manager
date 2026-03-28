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


add_subdirectory(idbg)

add_executable(awemgr_shell
	${CMAKE_CURRENT_SOURCE_DIR}/src/awemgr_cmdline.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/cmds_base.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/cmds_base.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/cmds_control.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/cmds_control.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/cmds_events.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/cmds_events.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/cmds_sendcommand.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/cmds_sendcommand.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/cmds_userdata.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/cmds_userdata.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/cmds_showinfo.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/cmds_showinfo.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/cmds_targetinfo.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/cmds_targetinfo.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/hlp_dump_response.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/hlp_dump_response.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/cmds_signalflow_dtmf.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/cmds_signalflow_dtmf.h
)

target_link_libraries(awemgr_shell awe_manager idbg_lib)

install(TARGETS awemgr_shell)
install(FILES README-awemgrshell.md TYPE DOC)
