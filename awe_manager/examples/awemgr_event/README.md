# Example for awemgr_event Usage

This CMake package only contains a small sample program that help ssubscribing and unsubscribing from events - see `awemgr_event.c`.

Steps to run this executable:

- once built via CMake, there will be an executable `./awe_manager/examples/awemgr_event/awemgr_event` inside the build directory

- start LinuxApp executable or any program that will provide an event socket server port at 15010; alternatively, on embedded target, 
  the shared memory communication backend is supported too

- start `./awe_manager/examples/awemgr_event/awemgr_event` to get the help information

