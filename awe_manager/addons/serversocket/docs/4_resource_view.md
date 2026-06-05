# Tuning Server Addon: Resource View


## Configuration Items

The Tuning Server Addon uses settings in [awe_CONFIG][awe-config-overview] component:

- `mgr.tuning_server.buf_words` - Size of the AWE tuning packet buffer in 32-bit words (default: 4096);
                                  **Note:** This size should not exceed the buffer size `mgr.comm.buffersize`
                                  configured for {{name.awe_mgr}} - see [Supported Configuration Items][supported-configuration-items].
- `mgr.tuning_server.timeoutms` - Client socket connection timeout in milliseconds; -1 = wait indefinitely
                                  (default: -1, reserved for future use).

!!! note
    In future releases these configuration settings may potentially be removed in favor of more centralized settings.

## Memory

- One heap allocation for `awemgr_tuning_server` at creation time.
- Two heap allocations for packet buffers of `mgr.tuning_server.buf_words` words each
  (request and response), allocated inside `serve_client()` per connection.

## Network

| Resource | Value |
|----------|-------|
| Protocol | TCP |
| Default port | 7200 (configurable by caller) |
| Connections | one client at a time; re-listens after disconnect |
| Packet format | AWE wire format — 32-bit words, length in `header >> 16` |
| Max packet size | 512 words (2048 bytes) |

## Socket options

| Option | Purpose |
|--------|---------|
| `SO_REUSEADDR` | Allows immediate rebind after client disconnect without waiting for `TIME_WAIT` |

## Threads

The tuning server is single-threaded. When used in `awemgr_service` with the
`-tuning_socket` flag, the service launches the run loop in a dedicated detached thread.

## Build-time footprint

The addon is compiled as a separate static library (`awemgr_tuning_server_lib`) and
excluded when `AWEMGR_BUILD_ADDONS=OFF`.
