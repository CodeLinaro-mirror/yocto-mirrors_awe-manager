# cross compilation toolchain file for AARCH systems
# assuming to be used under WSL, with the following packages being installed:
#
#  `sudo apt install gcc make gcc-aarch64-linux-gnu g++-aarch64-linux-gnu binutils-aarch64-linux-gnu`
#  `sudo apt install qemu-user qemu-user-static`
#

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_SYSTEM_HOST aarch64-linux-gnu) # used to pass to autotools ./configure scripts for external libraries

# Specify the compiler and flags
set(CMAKE_C_COMPILER ${CMAKE_SYSTEM_HOST}-gcc)
set(CMAKE_CXX_COMPILER ${CMAKE_SYSTEM_HOST}-g++)
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER}-as)

# ensure that correct emulator is used when running generated exes on host system
set(CMAKE_CROSSCOMPILING_EMULATOR "/usr/bin/qemu-aarch64;-L;/usr/aarch64-linux-gnu")
