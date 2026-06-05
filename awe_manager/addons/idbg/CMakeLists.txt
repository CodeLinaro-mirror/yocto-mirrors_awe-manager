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

option(IDBG_HAVE_ISOCLINE "Use ISOCline lib to handle user input" ON)

set (idbg_lib_sources
	${CMAKE_CURRENT_SOURCE_DIR}/src/idbg.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idbg_srv.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idbg_util.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idbg_util.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/idbg_arg.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idbg_types.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/idbg_internalcmds.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idbg_internalcmds.h
)
set (idbg_lib_incdirs_private
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/external/isocline/include
)

if(IDBG_HAVE_ISOCLINE)
	message(STATUS "IDBG: Command completion enabled via ISOCLINE package")
	list(APPEND idbg_lib_sources
		${CMAKE_CURRENT_SOURCE_DIR}/external/isocline/src/isocline.c
	)
	add_compile_definitions(IDBGSRV_USE_ISOCLINE=1)
else()
	message(STATUS "Command completion will not be available.")

endif()

add_library(idbg_lib ${idbg_lib_sources})

target_link_libraries(idbg_lib awosal_lib)

target_include_directories(idbg_lib
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
    ${idbg_lib_incdirs_private}
)

target_link_libraries(idbg_lib awosal_lib)

target_compile_options(idbg_lib PRIVATE
    $<$<COMPILE_LANG_AND_ID:C,GNU,Clang>:-Wno-all>
    $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-Wno-all>
)

install(TARGETS idbg_lib
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
