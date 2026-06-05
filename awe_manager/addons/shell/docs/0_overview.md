# Shell Addon: Overview

The shell addon provides an interactive command-dispatch engine on top of the `idbg` framework,
giving users and automated tools a text-based interface to {{name.awe_mgr}}.

Commands are organized in a virtual directory tree. Users can navigate the tree, query available
commands with tab completion or `-h` help, and execute them individually or from script files.

The addon is intentionally optional: production builds that do not need an interactive interface
can omit it by setting `AWEMGR_BUILD_SHELL=OFF` at CMake configure time.

## Entry points

| Function | Description |
|----------|-------------|
| `awemgr_shell_run_console()` | Interactive session on stdin/stdout |
| `awemgr_shell_run_file()` | Execute a script file |
| `awemgr_shell_run_socket()` | Accept commands over a TCP socket |
| `awemgr_shell_execute()` | Execute a single command string programmatically |
