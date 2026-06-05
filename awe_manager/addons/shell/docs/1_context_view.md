# Shell Addon: Context View

The shell addon sits between the user or automation layer and {{name.awe_mgr}}.
It depends on the `idbg` addon for the underlying command-dispatch mechanism and on
`awe_manager` for all AWE operations.

```
┌─────────────────────────────────────────────┐
│  Callers                                    │
│  ┌──────────────┐  ┌────────────────────┐   │
│  │  awemgr_cli  │  │ awemgr_service     │   │
│  │  (cmdline)   │  │ (with shell flag)  │   │
│  └──────┬───────┘  └────────┬───────────┘   │
└─────────┼───────────────────┼───────────────┘
          │  awemgr_shell API │
          ▼                   ▼
   ┌─────────────────────────────┐
   │       Shell Addon           │
   │   (awemgr_shell_ctx)        │
   └────────┬────────────────────┘
            │
     ┌──────┴────────┐
     ▼               ▼
  idbg addon     awe_manager
```

### Interfaces consumed

| Interface | Provider | Purpose |
|-----------|----------|---------|
| `idbg` command dispatch | idbg addon | Command registration, parsing, help |
| `awemgr_*` API | awe_manager | AWE core operations |

### Interfaces provided

| Interface | Consumer |
|-----------|----------|
| `awemgr_shell_*` C API | awemgr_cli, awemgr_service, application code |
| TCP socket (optional) | Remote shell clients, `awemgr_client.py` |
