# Tuning Server Addon: Context View

The tuning server addon connects AWE tuning tools to a running {{name.awe_mgr}} instance
over TCP. It sits between the network and the `awe_manager` transact interface.

```
┌─────────────────────┐
│  Tuning Tools (PC)  │
│  ┌─────────────┐    │
│  │ AWE Designer│    │
│  └─────┬───────┘    │
└────────┼────────────┘
         │  TCP (AWE wire format)
         ▼
  ┌─────────────────────────────┐
  │    Tuning Server Addon      │
  │  (awemgr_tuning_server)     │
  └──────────────┬──────────────┘
                 │  awemgr_transact()
                 ▼
           awe_manager
                 │
                 ▼
           AWE Core (DSP)
```

### Interfaces consumed

| Interface | Provider | Purpose |
|-----------|----------|---------|
| `awemgr_transact()` | awe_manager | Forward tuning packets to AWE Core |

### Interfaces provided

| Interface | Consumer |
|-----------|----------|
| TCP socket (AWE wire format) | AWE Designer, {{name.awctool}}, {{name.awetc}}, custom clients |
| `awemgr_tuning_server_*` C API | awemgr_service, application code |
