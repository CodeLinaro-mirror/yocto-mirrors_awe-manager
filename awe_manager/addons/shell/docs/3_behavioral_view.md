# Shell Addon: Behavioral View

## Lifecycle

```
awemgr_shell_create(mgr_p, cfg_p)
│  mgr_p = NULL  →  shell starts without AWE Manager;
│                   user calls "mgr-init" later
│  mgr_p ≠ NULL  →  shell uses provided handle;
│                   endpoint_id defaults to 0
▼
[optional] awemgr_shell_set_endpoint_id()

awemgr_shell_run_console()   ─┐
awemgr_shell_run_file()       ├─ blocks until done / EOF / "exit"
awemgr_shell_run_socket()    ─┘
awemgr_shell_execute()       ──  (single command, non-blocking)

awemgr_shell_destroy()
```

## Command execution flow

1. Input string is tokenised and matched against the idbg command tree.
2. The matching command handler is called with parsed arguments.
3. Output is written to the active output stream (stdout, socket, or internal buffer).
4. Return code (`IDBG_OK` / non-zero) is propagated to the caller.

## Repeating a command

`repeat -cmd <command> [-sec <seconds>] [-count <number>] [-verbose] [-throttle <usec>]`
runs another shell command over and over again until the given time has passed,
and then reports how many executions it managed:

```
repeat -cmd "get_value -var Scaler1.gain" -sec 5
│
└── repeat:
      command: "get_value -var Scaler1.gain"
      duration_sec: 5.000431
      calls: 4271
      calls_per_sec: 854.20
```

This makes the shell usable for simple throughput and soak checks without an
extra tool. Two properties of the idbg engine have to be observed by the
handler:

| Property | Consequence for `repeat` |
|----------|--------------------------|
| The repeated command is dispatched on the same idbg handle and therefore re-uses the handle's argument vector, splitting its command line in place | All arguments of `repeat` - including the command string itself - are copied into own memory before the first repetition; `argv` is invalid afterwards |
| The repeated command prints to the active output sink, which would flood the console or socket and would dominate the measured time | The output is dropped for the duration of the run (`IdbgOutputDrop`, see below) and the sink in use - stdout or socket - is back in place for the summary. `-verbose` keeps the output, e.g. to see what a single repetition returns |

The suppression covers the shell output only. Log messages of {{name.awe_mgr}} itself
go to the platform log, not through the shell output sink, and are silenced by
raising the log level instead, e.g. `cfg -key mgr.cmd.log.level -value error`.

The timing uses the monotonic clock of the OS abstraction
(`aweosal_measure_start()` / `aweosal_measure_elapsed()`). The command is
executed at least once, and the loop stops early when a repetition returns a
non-zero code, i.e. when it would stop the shell; that code is passed on.

A run can be ended by the work done instead of by the time it takes:
`-count <number>` performs exactly that many executions and then stops. It
takes precedence over `-sec`, which is not used at all when a count is given,
so `repeat -cmd "audio_stop" -count 100` is a fixed amount of work rather than
a fixed amount of time - useful when the interesting number is how long a known
workload takes, and to keep a test reproducible on a machine of a different
speed. Without `-count` (or with `-count 0`) `-sec` ends the run as before. The
summary is the same for both, i.e. `duration_sec` is the measured runtime in
either case.

Without `-throttle` the command is repeated as fast as the system allows, which
is what a throughput measurement needs but saturates one core. `-throttle
<usec>` inserts a delay (`aweosal_usleep()`) between two executions, so a soak
run can be kept at a defined load, e.g. `-throttle 10000` for roughly 100 calls
per second. The delay is skipped after the last repetition, so it does not
stretch the run beyond `-sec`. On Windows the OS abstraction maps the delay to
`Sleep()`, i.e. its resolution is one millisecond there.

## Nesting of commands that dispatch commands

`repeat` and `script` both run further command lines through the same idbg
handle, so both can end up invoking themselves - directly, or through each
other. Each of them counts its active invocations in the shell context
(`repeat_nesting`, `script_nesting`) using the `CmdNestingGuard` scope guard,
which releases the count on every return path:

| Case | Behavior |
|------|----------|
| `repeat` inside `repeat`, also when reached through a `script` | rejected with an error; the outer run would count the inner runs instead of the repeated command, so its `calls` and `calls_per_sec` would say nothing and its runtime would be stretched to the duration of the inner run |
| `repeat` inside a `script` that is not itself repeated | allowed - a soak run started from a script file is a regular use case |
| `script` including further script files | allowed up to `MAX_SCRIPT_NESTING` (8) levels |
| `script` including itself, directly or through a chain | rejected with an error once the limit is reached; without the limit the include recurses until the stack is exhausted and the shell dies |

A rejected command prints its error and returns `IDBG_OK`, i.e. it never stops
the shell. `script` keeps the script mode of the including file across an
included one, so the prompt stays suppressed for the remaining lines of the
outer file.

## Output handling with communication tracing

Command output is YAML, so it has to stay contiguous - a client such as
`awemgr_client.py` parses a response as one YAML document. With
`comm-trace -on` the shell installs a tap on the AWE Manager communication
(`awemgr_set_comm_observer()`), and that tap is called from *inside* the API
calls a command performs. Without further measures the trace blocks would be
printed in the middle of the command's YAML output, for example between two
entries of `info -cpu`.

Two mechanisms keep the two streams apart:

| Mechanism | Used by | Effect |
|-----------|---------|--------|
| `idbg_output_hold()` / `idbg_output_flush()`, wrapped in the `IdbgOutputHold` scope guard (`hlp_functions.h`) | command handlers that issue several API calls, e.g. `info` | The command's `idbg_print()` output is collected and written in one go when the handler returns - on every return path |
| `idbg_print_direct()` | the comm trace tap (`comm_observer_cb()`) | Trace output bypasses an active hold and is written immediately |
| `idbg_output_disable()` / `idbg_output_enable()`, wrapped in the `IdbgOutputDrop` scope guard (`hlp_functions.h`) | command handlers that execute a command on behalf of the user, e.g. `repeat` | The output printed while the guard is in scope is discarded, the sink in use is restored when it goes out of scope. Both mechanisms nest, also with each other |

The resulting order for a traced command is: trace blocks as they happen,
followed by the command's own output as one block.

```
info -cpu   with comm-trace -on
│
├── TX: [ ... ]      ── tap, printed directly while the command runs
├── RX: [ ... ]
├── ...
└── cpu_info:        ── command output, emitted as one block on return
      - name: awe#0
        cpu: 0.34
        layouts: [ 0.34, ]
```

Holds nest, and only the outermost flush emits. Because output sinks format
into fixed size buffers (`si_writef_va()` is limited to `MAX_LINE_LENGTH`), the
flush hands the collected block to the sink in chunks that fit into them;
nothing can appear in between, as the command has finished at that point.

The collecting buffer belongs to the idbg handle and is not thread-safe: only
the thread executing the command may hold/flush; any other thread has to use
`idbg_print_direct()`.

## Socket mode

```
awemgr_shell_run_socket(ctx, host, port)
│
├── bind + listen on host:port
├── accept() — blocks until client connects
│
└── per-connection loop:
    ├── recv line
    ├── awemgr_shell_execute()
    ├── send output + "awemgr-shell > " terminator
    └── repeat until client disconnects
```

The caller is responsible for re-invoking `awemgr_shell_run_socket()` to accept the next client.
