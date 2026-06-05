# AWE Manager Service: Resource View

## Memory

All significant memory is owned by `awe_manager` and the addon libraries.
The service itself allocates only a small number of stack and global variables.

## Threads

| Thread | Condition | Purpose |
|--------|-----------|---------|
| Main thread | always | Startup, main loop, shutdown |
| Shell thread | `AWEMGR_SERVICE_WITH_SHELL` + `-shell_socket` | Runs `awemgr_shell_run_socket()` accept loop |
| Tuning thread | `AWEMGR_SERVICE_WITH_TUNING_SERVER` + `-tuning_socket` | Runs `awemgr_tuning_server_run()` accept loop |

Both addon threads are detached. They observe the `g_stop_requested` flag to terminate
their loops; the process exit then tears down any remaining sockets.

## Network

| Socket | Bind address | Consumer |
|--------|-------------|----------|
| Shell socket | `localhost:<port>` | Local clients only (`awemgr_client.py`, scripts) |
| Tuning socket | `0.0.0.0:<port>` | Any network client (AWE Designer, etc) |

## Logging

Output goes to `stderr` only. No log files are written by the service itself.
Log rotation, persistence, and forwarding are delegated to journald / systemd.

## Signals

| Signal | Effect |
|--------|--------|
| `SIGTERM` | Sets `g_stop_requested = 1`; triggers clean shutdown |
| `SIGINT` | Same as `SIGTERM` |
| Other | Wakes `pause()` but does not affect the stop flag |
