# AWE Manager Service: Overview

`awemgr_service` is a long-running Linux process that hosts {{name.awe_mgr}} and keeps
{{name.awe_host}} running continuously. It is designed to be managed by systemd and logs
to stderr so that journald captures every message with automatic metadata.

At startup the service initializes {{name.awe_mgr}}, optionally loads an {{name.awe_awc_db}}
file and a named design (like 'Main' or 'ProgressiveLoad'), then blocks until a stop signal
is received. On shutdown it unloads all resources and exits cleanly.

## Command-line options

| Option | Description |
|--------|-------------|
| `-awc <file>` | {{name.awe_awc_db}} file to load at startup |
| `-design <name>` | Design (AWB) to activate after `-awc` (e.g. `Main`) |
| `-cfg <string>` | AWE Manager config overrides (e.g. `mgr.api.log.level=debug;`) |
| `-shell_socket <port>` | Accept shell commands on a TCP socket — localhost only *(optional build)* |
| `-tuning_socket <port>` | Accept AWE tuning packets on a TCP socket *(optional build)* |
| `-version` | Print version string and exit |
| `-h`, `--help` | Print usage and exit |

## Environment variables

| Variable | Description |
|----------|-------------|
| `AWEMGR_CFG_OVERRIDE` | Semicolon-separated config overrides, applied after `-cfg` |

## Minimal systemd unit

```ini
[Service]
Type=simple
ExecStart=/usr/bin/awemgr_service -awc /etc/awe/awc_index.awc
Restart=on-failure
StandardOutput=journal
StandardError=journal
```
