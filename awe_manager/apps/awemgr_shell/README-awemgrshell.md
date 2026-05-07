# AWE Manager Shell

A simple test program to allow interactive debugging of AWEManager.

It was also created to test-drive and sanitize the API.

## Concept

The AWEMgr-Shell provides an interactive prompt to the user once it is started.
The user is supposed to enter commands (including parameters) just like on a command line
terminal of an OS. AWEMgr-Shell translates the entered "text" into the 
typical ARGC/ARGV structure and calls an appropriate callback function.

Functionality is therefore added by creating and registering those callback functions.

## General Usage

AWEMgr-Shell adheres to `-h` to get more info:

```
Use: awemgr_shell [-awc <awcfile>] [ [-s port] | -f <cli-cmd-file> ] 

   -awc <awcfile>    - load AWC on startup; Signalflow will be started automatically
   -awb <name>       - when load AWC on startup, loads this design (eg: Main)
   -s port           - Use socket mode for interactive commands
   -f <cli-cmd-file> - Execute script commands from file
   -i                - Start interactive console input; use this with '-f'

When no file is given, an interactive console is started.
``` 

Once in the interactive debug session, use built-in commands to "explore" the
available commands:

```
idbg> help
IDBG help
  internal cmds: ls/dir, cd, quit/exit
  entry types  : dir, cmd
For command specific help: try  <cmd> -h{elp}
``` 

Note: instead of the commands `quit` and `exit` also the keyboard <CTRL-D> can be used.

## AWEMgr-Shell on PC (Linux / Windows)

When compiling AWE-Manager for the PC (Linux-x64 or Windows), the system uses a socket connection
to an AWE-Server instead of shared memory connection to a DSP.

The AWE-Server is used to start and to control signal flows. 
When running AWEMgr-Shell under Linux (WSL is assumed), the target IP of the Windows host system
has to be specified, as the AWE-Server is running on the Windows host machine. See '-h' option. 

Simply start AWE-Server before AWEMgr-Shell usage:

  - e.g. under `C:\DSP Concepts\AWE Designer 8.D.2.3 Standard\Bin\win32-vc142-rel\AWE_Server.exe`
