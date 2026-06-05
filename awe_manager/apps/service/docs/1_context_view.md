# AWE Manager Service: Context View

`awemgr_service` is the integration point between the host system and {{name.awe_mgr}}.
It ties together the core library, the optional shell and tuning server addons, and the
operating system process lifecycle.

```
┌───────────────────────────────────────────────────────────┐
│  System / Operator                                        │
│  ┌───────────┐  ┌──────────────┐  ┌────────────────────┐  │
│  │ systemd   │  │ shell client │  │  AWE tuning tool   │  │
│  │(lifecycle)│  │(awemgr_client│  │ (AWE Designer, ...)│  │
│  └────┬──────┘  └──────┬───────┘  └────────┬───────────┘  │
└───────┼────────────────┼───────────────────┼──────────────┘
        │ SIGTERM/SIGINT │ TCP (text cmds)   │ TCP (AWE packets)
        ▼                ▼                   ▼
┌────────────────────────────────────────────────────────┐
│                  awemgr_service                        │
│  ┌─────────────────┐  ┌──────────────────────────────┐ │
│  │   awe_manager   │  │ Shell Addon  (optional)      │ │
│  │   (core)        │  │ Tuning Server Addon(optional)│ │
│  └────────┬────────┘  └──────────────────────────────┘ │
└───────────┼────────────────────────────────────────────┘
            │
            ▼
      AWE Core (DSP)
```

### Interfaces consumed

| Interface | Provider | Purpose |
|-----------|----------|---------|
| `awemgr_*` API | awe_manager | Init, AWC load, design load, exit |
| `awemgr_shell_*` API | Shell addon | Optional runtime command interface |
| `awemgr_tuning_server_*` API | Tuning Server addon | Optional AWE packet forwarding |
| `SIGTERM` / `SIGINT` | OS / systemd | Clean shutdown trigger |

### Interfaces provided

| Interface | Consumer |
|-----------|----------|
| TCP shell socket (localhost) | `awemgr_client.py`, scripts |
| TCP tuning socket (all interfaces) | AWE Designer, others |
| journald log stream (stderr) | systemd / journald |
| Process exit code | systemd `Restart=on-failure` |
