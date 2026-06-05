# AWE Manager Service: Internal Architecture

## Source modules

| Module | Description |
|--------|-------------|
| `awemgr_service.cpp` | Entire service implementation — argument parsing, lifecycle, signal handling, main loop |

The service is intentionally a single translation unit with no internal sub-components.
All domain logic lives in the libraries it calls (`awe_manager`, shell addon, tuning server addon).

## Compile-time feature flags

| Flag | Effect |
|------|--------|
| `AWEMGR_SERVICE_WITH_SHELL` | Includes shell addon and `-shell_socket` argument |
| `AWEMGR_SERVICE_WITH_TUNING_SERVER` | Includes tuning server addon and `-tuning_socket` argument |

Both flags are set by the CMake build when the corresponding addon libraries are included.

## Logging

All log output goes to `stderr` using journald severity prefixes (`SD_INFO`, `SD_ERR`, etc.).
This allows journald to attach log level metadata without requiring the systemd development
headers as a build dependency — the prefix strings are reproduced inline.

## Signal handling

`SIGTERM` and `SIGINT` are caught by a minimal `sigaction` handler that sets a
`volatile sig_atomic_t` flag. The main loop polls this flag via `pause()`.
The same flag is read by the optional addon threads to stop their accept loops.
