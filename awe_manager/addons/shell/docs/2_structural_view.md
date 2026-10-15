# Shell Addon: Internal Architecture

## Key types

| Type | Description |
|------|-------------|
| `awemgr_shell_ctx` | Opaque handle; owns the idbg server instance, the AWE Manager handle (if created internally), and session state |

## Source modules

| Module | Description |
|--------|-------------|
| `awemgr_shell.cpp` | Public API implementation: create/destroy, run modes, execute |
| `cmds_base.cpp` | Base command set: `mgr-init`, `awc-load`, `design-load`, `show`, `script`, `repeat`, … and the comm trace tap |
| `cmds_control.cpp` | Control commands: `set_value`, `get_value`, `transact`, … |
| `hlp_functions.h` | Shared command helpers: error printing macros and the output scope guards `IdbgOutputHold` (collect) and `IdbgOutputDrop` (discard) |

## Command tree layout

Commands are registered in a virtual directory tree rooted at `/`:

Example:

```
/
├── <SOMEDIR>/     # DIR: `cd <SOMEDIR>` as command to enter this virtual directory
├── <CMD>          # CMD: simply type command with optional parameters, like `<CMD> -h`
└── <CMD>          # ...
```

Users navigate the tree with `ls` and `cd`; tab completion queries it at runtime.

## Optional socket server

When `awemgr_shell_run_socket()` is used, the addon listens on a TCP port and
dispatches commands received as newline-delimited text strings. Responses are
terminated with the prompt string `awemgr-shell > ` as a framing delimiter.
