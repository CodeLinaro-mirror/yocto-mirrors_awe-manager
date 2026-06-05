# AWE Manager Service: Behavioral View

## Startup sequence

```
parse arguments
│
├── -h / --help   → print usage, exit 0
├── -version      → print version, exit 0
│
├── awemgr_config_create()
├── aweconfig_from_string()    [if -cfg]
├── aweconfig_from_envvar()    [AWEMGR_CFG_OVERRIDE]
│
├── awemgr_init()
│
├── awemgr_load_awc()          [if -awc]
│   └── awemgr_load_design()   [if -design]
│
├── [AWEMGR_SERVICE_WITH_SHELL]
│   ├── awemgr_shell_create()
│   └── detached thread: while (!stop) awemgr_shell_run_socket()
│
└── [AWEMGR_SERVICE_WITH_TUNING_SERVER]
    ├── awemgr_tuning_server_create()
    └── detached thread: while (!stop) awemgr_tuning_server_run()
```

## Main loop

```
while (!g_stop_requested)
    pause()           ← sleeps until any signal wakes the process
```

`pause()` returns on every signal. Only `SIGTERM`/`SIGINT` set `g_stop_requested`;
other signals (e.g. `SIGCHLD`) cause `pause()` to return but the loop continues.

## Shutdown sequence

```
SIGTERM / SIGINT received
│
├── [AWEMGR_SERVICE_WITH_SHELL]
│   ├── awemgr_shell_execute("/event -stop")
│   └── awemgr_shell_destroy()
│
├── [AWEMGR_SERVICE_WITH_TUNING_SERVER]
│   └── awemgr_tuning_server_destroy()
│
└── awemgr_exit()
```

The detached addon threads observe `g_stop_requested` and stop accepting new clients.
In-progress connections are torn down when the process exits.
