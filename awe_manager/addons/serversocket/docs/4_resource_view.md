# Tuning Server Addon: Resource View

## Memory

- One heap allocation for `awemgr_tuning_server` at creation time.
- Two stack-allocated packet buffers of `TUNING_BUF_WORDS` × 4 bytes = 2 KB each
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
