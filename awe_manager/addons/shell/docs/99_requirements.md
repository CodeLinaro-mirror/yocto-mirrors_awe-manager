# Shell Addon: Requirements

```yaml
- id: dsn~AWEMGR.ADDON.SHELL.CommandDispatch~1
  needs: utest
  description: |
    The addon shall provide a command-dispatch engine that allows users to
    execute AWE Manager operations via named text commands.

- id: dsn~AWEMGR.ADDON.SHELL.InteractiveConsole~1
  needs: utest
  description: |
    The addon shall support an interactive session on stdin/stdout.

- id: dsn~AWEMGR.ADDON.SHELL.ScriptFile~1
  needs: utest
  description: |
    The addon shall support executing a sequence of commands from a text file.

- id: dsn~AWEMGR.ADDON.SHELL.SocketServer~1
  needs: utest
  description: |
    The addon shall support accepting commands over a TCP socket, enabling
    remote control from tooling or test scripts.

- id: dsn~AWEMGR.ADDON.SHELL.RepeatCommand~2
  needs: utest
  description: |
    The addon shall provide a command that executes another shell command
    repeatedly for a given number of seconds and reports how many executions
    were performed. The output of the repeated command shall be suppressed by
    default, so that only the summary of the run is printed, and shall be
    printed on request.

- id: dsn~AWEMGR.ADDON.SHELL.RepeatCount~1
  needs: utest
  description: |
    The repeat command shall support ending a run after a given number of
    executions instead of after a given time, so that a check can be defined
    by the work performed rather than by its duration. The number of
    executions shall take precedence over the repeat time when both are given.

- id: dsn~AWEMGR.ADDON.SHELL.RepeatThrottle~1
  needs: utest
  description: |
    The repeat command shall support an optional delay, given in microseconds,
    that is inserted between two executions of the repeated command, so that
    the load put on the system by a repeat run can be limited. Without the
    delay the command shall be repeated as fast as possible.

- id: dsn~AWEMGR.ADDON.SHELL.CommandNestingLimit~1
  needs: utest
  description: |
    Commands that dispatch further command lines on the same shell handle
    shall limit how deeply they may be nested inside themselves, so that a
    command which invokes itself - directly or through another such command -
    is rejected with an error message instead of producing a meaningless
    result or exhausting the stack. The repeat command shall not be nested at
    all. A script file shall be allowed to include further script files up to
    a fixed nesting depth, so that a recursive include is stopped while
    regular includes keep working.

- id: dsn~AWEMGR.ADDON.SHELL.AtomicCommandOutput~1
  needs: utest
  description: |
    A command handler shall be able to collect its output and have it written
    as one contiguous block when it returns, so that output produced while the
    command runs cannot break up the YAML document the command prints.

- id: dsn~AWEMGR.ADDON.SHELL.OutputSuppression~1
  needs: utest
  description: |
    A command handler shall be able to drop the shell output produced while it
    runs, so that output of a command it executes on behalf of the user cannot
    reach the console or socket. The output sink in use - stdout or socket -
    shall be restored when the handler stops dropping, also when the
    suppression is nested.

- id: dsn~AWEMGR.ADDON.SHELL.CommTraceStreaming~1
  needs: utest
  description: |
    Communication trace output of the shell comm tap shall bypass the output
    collected by a running command and be written to the output sink
    immediately.

- id: dsn~AWEMGR.ADDON.SHELL.EndpointValidation~1
  # needs: --- verified manually; requires a session without a loaded AWC
  description: |
    Target information commands shall reject an endpoint index outside the
    supported range - as is the case when no AWC has been loaded and no
    endpoint was given - with an error message instead of querying AWE Core.

- id: dsn~AWEMGR.ADDON.SHELL.ExternalMgrHandle~1
  needs: utest
  description: |
    When created with a pre-initialised AWE Manager handle, the addon shall
    default the active endpoint index to 0 so that commands work without
    requiring explicit endpoint selection.

- id: dsn~AWEMGR.ADDON.SHELL.OptionalBuild~1
  needs: utest
  description: |
    The addon shall be fully excludable from a build via a CMake option
    (AWEMGR_BUILD_SHELL=OFF) without affecting other components.

- id: dsn~AWEMGR.ADDON.SHELL.PublicHeaderInstall~1
  # needs: --- install concern, verifiable by inspecting the install tree
  description: |
    The addon shall install its public API header (awemgr_shell.h) into the
    package include directory.
```
