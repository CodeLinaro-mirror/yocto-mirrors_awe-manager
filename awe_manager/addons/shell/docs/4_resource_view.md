# Shell Addon: Resource View

## Memory

- One heap allocation for `awemgr_shell_ctx` at creation time.
- If `cfg_p == NULL`, one additional heap allocation for an `awe_config` object.
- No dynamic allocations during command execution.

## Network (socket mode only)

| Resource | Value |
|----------|-------|
| Protocol | TCP |
| Default port | configurable by caller |
| Connections | one client at a time |
| Framing | newline-delimited text commands; `awemgr-shell > ` response terminator |

## Threads

The shell itself is single-threaded. When used in `awemgr_service` with the
`-shell_socket` flag, the service launches the shell run loop in a dedicated
detached thread.

## Build-time footprint

The addon is excluded from the build when `AWEMGR_BUILD_SHELL=OFF`, adding
zero overhead to production binaries.
