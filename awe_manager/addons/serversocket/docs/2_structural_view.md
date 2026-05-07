# Tuning Server Addon: Internal Architecture

## Key types

| Type | Description |
|------|-------------|
| `awemgr_tuning_server` | Opaque handle; holds a reference to the `awemgr_data` instance |

## Source modules

| Module | Description |
|--------|-------------|
| `awemgr_tuning_server.c` | Full implementation: socket lifecycle, packet framing, transact forwarding |

## Internal structure

```
awemgr_tuning_server_run()
│
├── open_listen_socket()     bind + listen (SO_REUSEADDR)
│
├── accept()                 blocks until client connects or SIGTERM/SIGINT
│
└── serve_client()
    ├── recv_all()           read header word → derive packet length
    ├── recv_all()           read remaining packet words
    ├── awemgr_transact()    forward to AWE Core
    └── send_all()           write response words back to client
        (loop until COMM_TIMEOUT, RC_ERR, or client disconnect)
```

## Buffer sizing

The internal receive/send buffer is fixed at `TUNING_BUF_WORDS` (512 × 32-bit words).
Packets larger than this are rejected. This covers all standard AWE tuning packets.
