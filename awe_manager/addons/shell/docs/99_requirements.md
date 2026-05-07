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
```
