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


# todo: add gtest deps etc and potentially remove/convert t1

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data" DESTINATION ${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME} )

add_executable(awe_mgr_test
    src/test_awe_mgr.cpp
    src/test_detach.cpp
    src/test_fixtures.cpp
    src/test_multi_instance.cpp
    src/test_designs.cpp
    src/test_enum.cpp
    src/test_sendcommand.cpp
    src/test_module.cpp
    src/test_variable.cpp
    src/test_events.cpp
    src/test_multiinstance.cpp
    src/test_sleep_resume.cpp
    src/test_userdata.cpp
    src/test_awecore_info.cpp
    src/test_subcanvas.cpp
    )
target_link_libraries(awe_mgr_test awe_manager gtest gtest_main gmock)
target_compile_features(awe_mgr_test PRIVATE cxx_std_14)
target_include_directories(awe_mgr_test PRIVATE ../src)

gtest_discover_tests(awe_mgr_test)

add_subdirectory(stress_test)