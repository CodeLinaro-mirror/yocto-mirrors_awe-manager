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
