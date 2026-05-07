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
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

add_executable(awemgr_service
    ${CMAKE_CURRENT_SOURCE_DIR}/src/awemgr_service.cpp
)

target_link_libraries(awemgr_service PRIVATE awe_manager)

if(AWEMGR_BUILD_SHELL)
    target_link_libraries(awemgr_service PRIVATE awemgr_shell_lib)
    target_compile_definitions(awemgr_service PRIVATE AWEMGR_SERVICE_WITH_SHELL)
endif()

if(AWEMGR_BUILD_TUNING_SERVER)
    target_link_libraries(awemgr_service PRIVATE awemgr_tuning_server_lib)
    target_compile_definitions(awemgr_service PRIVATE AWEMGR_SERVICE_WITH_TUNING_SERVER)
endif()

install(TARGETS awemgr_service)

# ---- systemd unit file ----
# Expand @CMAKE_INSTALL_FULL_BINDIR@ so ExecStart points to the real binary.
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/awemgr.service.in
    ${CMAKE_CURRENT_BINARY_DIR}/awemgr.service
    @ONLY
)

# installation of configured systemd config file into the DOC directory.
# It is up to the user to deploy the file correctly on the target system (at the moment).
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/awemgr.service TYPE DOC)