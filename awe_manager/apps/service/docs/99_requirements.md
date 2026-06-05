# AWE Manager Service: Requirements

```yaml
- id: dsn~AWEMGR.APP.SERVICE.Lifecycle~1
  needs: utest
  description: |
    The service shall initialise AWE Manager at startup and perform a clean
    shutdown (unload AWC, call awemgr_exit) when SIGTERM or SIGINT is received.

- id: dsn~AWEMGR.APP.SERVICE.AwcLoad~1
  needs: utest
  description: |
    The service shall optionally load an AWC file and a named design at startup
    when the -awc and -design arguments are provided.

- id: dsn~AWEMGR.APP.SERVICE.ConfigOverride~1
  needs: utest
  description: |
    The service shall apply AWE Manager configuration overrides from the -cfg
    argument and from the AWEMGR_CFG_OVERRIDE environment variable, with the
    environment variable taking precedence.

- id: dsn~AWEMGR.APP.SERVICE.ShellSocket~1
  needs: utest
  description: |
    When built with AWEMGR_SERVICE_WITH_SHELL and started with -shell_socket,
    the service shall accept shell commands on a localhost TCP socket in a
    dedicated thread without blocking the main loop.

- id: dsn~AWEMGR.APP.SERVICE.TuningSocket~1
  needs: utest
  description: |
    When built with AWEMGR_SERVICE_WITH_TUNING_SERVER and started with
    -tuning_socket, the service shall forward AWE tuning packets received on
    a TCP socket to AWE Core via awemgr_transact() in a dedicated thread.

- id: dsn~AWEMGR.APP.SERVICE.Logging~1
  needs: utest
  description: |
    The service shall write all log output to stderr using journald severity
    prefixes so that systemd/journald captures log levels without requiring
    the systemd development headers as a build dependency.
```
