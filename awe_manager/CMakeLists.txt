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

set(TEST_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests/data")
add_compile_options(-DTEST_DATA_DIR="${TEST_DATA_DIR}")

add_subdirectory(components/logging)
add_subdirectory(components/awe_config)
add_subdirectory(components/awe_osal)
add_subdirectory(components/awe_ctrl)
add_subdirectory(components/awe_cmd)
add_subdirectory(components/awe_awc)

set(_sources
    src/awe_manager.c
    src/awemgr_awc.c
    src/awemgr_config.c
    src/awemgr_designs.c
    src/awemgr_sendcommand.c
    src/awemgr_modules.c
    src/awemgr_controls.c
    src/awemgr_transact.c
    src/awemgr_audio.c
    src/awemgr_events.c
    src/awemgr_targetinfo.c
    src/awemgr_classinfo.c
    src/awemgr_heaps.c
    src/awemgr_cpuinfo.c

    src/types/awemgr_data.h
    src/types/awemgr_ctx.h
    src/types/awemgr_limits.h

    src/awemgr_util.c
    src/awemgr_util.h
    src/awemgr_api_logging.h
)
if(AWEMGR_BUILD_SINGLE_LIB)
    set(_libname awe_manager_lib)
else()
    set(_libname awe_manager)
endif()

add_library (${_libname} ${AWEMGR_BUILD_TYPE} ${_sources})
set_target_properties(${_libname} PROPERTIES PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include/awe_manager.h)

set(_HAVE_CUST_VERSION 0)
if(AWEMGR_CUSTOM_VERSION)
set(_HAVE_CUST_VERSION 1)
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/versioninfo.h.in ${CMAKE_CURRENT_BINARY_DIR}/versioninfo.h @ONLY)

target_include_directories(${_libname}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(${_libname} awe_config_lib awe_ctrl_lib awe_cmd_lib awe_awc awemgr_tracing_h)

if(AWEMGR_BUILD_SINGLE_LIB)
    bundle_static_library(awe_manager_lib awe_manager)

    # installing files separately instead of using install(TARGETS...)
    # as properties of combined awe_manager archive cannot be resolved
    get_target_property(_lib awe_manager IMPORTED_LOCATION)
    install(FILES
        ${_lib}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
    get_target_property(_header awe_manager_lib PUBLIC_HEADER)
    install(FILES
        ${_header}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
else()
    install(TARGETS
        awe_manager
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()

if(AWEMGR_BUILD_ADDONS)
add_subdirectory(examples/cmdline)
add_subdirectory(examples/minimal)
add_subdirectory(examples/events)
add_subdirectory(examples/awemgr_event)
endif()

if(AWEMGR_BUILD_TESTS)
add_subdirectory(tests)
endif()
#
# Packaging related configuration
#

set(CPACK_PACKAGE_VERSION ${COMPONENT_VERSION})
set(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})
set(CPACK_GENERATOR "TBZ2")

set(CPACK_SOURCE_GENERATOR "TBZ2")
set(CPACK_SOURCE_IGNORE_FILES "/\.git/;/.*venv/;/_build.*/;/ci/;/\.vscode/;/\.pytest.*/;/\.bitbucket/;/build/")

if(PROJECT_IS_TOP_LEVEL)
include(CPack)
endif()
