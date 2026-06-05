# Tuning Server Addon: Overview

The tuning server addon accepts AWE tuning packets over a TCP socket, forwards them to
`awemgr_transact()`, and returns the {{name.awe_lib}} response to the client.

This allows tools such as {{name.awctool}} or {{name.awetc}} to connect to a running
{{name.awe_mgr}} service over the network and tune {{name.awe_host}} at runtime without
requiring a dedicated hardware communication interface.

The addon is intentionally optional and is enabled via `AWEMGR_BUILD_ADDONS` at CMake
configure time.

## Packet framing

Both request and response packets use the AWE wire format: the high 16 bits of the first
32-bit word encode the total packet length in words (including the header word itself).
The server reads exactly that many words from the socket, passes them to `awemgr_transact()`,
and writes the response words back.

## Typical usage

```c
awemgr_tuning_server *srv = awemgr_tuning_server_create(mgr_p);
while (!stop_requested)
    awemgr_tuning_server_run(srv, "7200");
awemgr_tuning_server_destroy(srv);
```

Each call to `awemgr_tuning_server_run()` blocks until the connected client disconnects,
then returns so the caller can re-enter the accept cycle. A `SIGTERM` or `SIGINT` received
while waiting causes the call to return immediately.

## Limitations

It is (currently) not supported to allow the simultaneous connection of more than one client to the provided socket port.

The method `awemgr_tuning_server_run()` shall not be called from different threads with the same
port number.
