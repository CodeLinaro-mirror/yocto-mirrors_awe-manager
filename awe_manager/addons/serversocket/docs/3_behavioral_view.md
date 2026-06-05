# Tuning Server Addon: Behavioral View

## Lifecycle

```
awemgr_tuning_server_create(mgr_p)

while (!stop_requested)
    awemgr_tuning_server_run(srv, host, port)
    │
    ├── 0  →  client disconnected cleanly, or SIGTERM/SIGINT received
    └── -1 →  fatal socket error (bind/listen failed)

awemgr_tuning_server_destroy(srv)
```

## Per-call flow

```
awemgr_tuning_server_run()
│
├── open_listen_socket()
│   SO_REUSEADDR set → rapid rebind after disconnect
│
├── accept()
│   EINTR (signal) → return 0 immediately
│
└── serve_client() loop:
    │
    ├── recv header word (4 bytes)
    │   length = header >> 16  (total words incl. header)
    │
    ├── recv remaining (length-1) words
    │
    ├── awemgr_transact(mgr_p, req_buf, length, resp_buf, BUF_WORDS)
    │
    ├── send response words
    │
    └── repeat until:
        ├── COMM_TIMEOUT  →  client gone, break
        ├── RC_ERR        →  fatal AWE error, break
        └── send failure  →  client disconnected, break
```

## Signal handling

`SIGTERM` or `SIGINT` during `accept()` causes `errno == EINTR`. The server
treats this as a clean exit signal and returns `0` so the enclosing service
loop can evaluate its stop condition.
