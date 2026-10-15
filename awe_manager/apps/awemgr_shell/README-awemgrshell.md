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

## Repeating a command

The `repeat` command executes another command over and over again for a given
number of seconds and prints how many executions were performed. This is handy
for quick throughput or soak checks:

```
awemgr-shell > repeat -cmd "get_value -var Scaler1.gain" -sec 5
repeat:
  command: "get_value -var Scaler1.gain"
  duration_sec: 5.000431
  calls: 4271
  calls_per_sec: 854.20
```

Quote the repeated command when it has parameters of its own. The output of
the repeated command is suppressed, so that only the summary of the run is
printed; use `-verbose` to see the output of every single execution as well.
`-sec` defaults to 1.

Use `-count <number>` to end the run after a fixed number of executions instead
of after a time. This measures a known amount of work rather than a duration,
which keeps a check reproducible on machines of different speed:

```
awemgr-shell > repeat -cmd "get_value -var Scaler1.gain" -count 1000
repeat:
  command: "get_value -var Scaler1.gain"
  duration_sec: 1.170884
  calls: 1000
  calls_per_sec: 854.00
```

`-count` takes precedence over `-sec`, which is not used when a count is given.

By default the command is repeated as fast as possible. Use `-throttle <usec>`
to insert a delay between two executions, which keeps a soak run at a defined
load instead of saturating a core:

```
awemgr-shell > repeat -cmd "get_value -var Scaler1.gain" -sec 5 -throttle 10000
repeat:
  command: "get_value -var Scaler1.gain"
  duration_sec: 5.004219
  calls: 486
  calls_per_sec: 97.12
```

A `repeat` cannot repeat another `repeat` - the reported `calls` would count
the inner runs instead of the repeated command - and is rejected with an error.
Repeating a `script` that starts a `repeat` is rejected for the same reason;
starting a `repeat` from a script file is fine. Script files may include
further script files up to 8 levels deep, which stops a script that includes
itself from running until the stack is exhausted.

## AWEMgr-Shell on PC (Linux / Windows)

When compiling AWE-Manager for the PC (Linux-x64 or Windows), the system uses a socket connection
to an AWE-Server instead of shared memory connection to a DSP.

The AWE-Server is used to start and to control signal flows. 
When running AWEMgr-Shell under Linux (WSL is assumed), the target IP of the Windows host system
has to be specified, as the AWE-Server is running on the Windows host machine. See '-h' option. 

Simply start AWE-Server before AWEMgr-Shell usage:

  - e.g. under `C:\DSP Concepts\AWE Designer 8.D.2.3 Standard\Bin\win32-vc142-rel\AWE_Server.exe`
