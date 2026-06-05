# Calls By Functionality

The main include file of {{name.awe_mgr}} is the [awe_manager.h](/AweMgrHdrDocs/awe__manager_8h) include file.


## API Overview

All public functions are prefixed `awemgr_`.  They operate on two opaque handle types:

| Handle | Meaning |
|--------|---------|
| `struct awemgr_data *` | Top-level manager instance (one per process) |
| `struct awemgr_ctx *`  | Per-endpoint (canvas) context; obtained via [`awemgr_get_awc_context()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_awc_context) |

---

### Lifecycle

| Function | Description |
|----------|-------------|
| [`awemgr_get_version()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_version) | Return the library version string |
| [`awemgr_config_create()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_config_create) | Allocate a config object with defaults (optional — pass NULL to `awemgr_init` to use built-in defaults) |
| [`awemgr_config_destroy()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_config_destroy) | Free a config object |
| [`awemgr_config_set()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_config_set) | Override a config key before or after init |
| [`awemgr_config_get()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_config_get) | Read the current value of a config key |
| [`awemgr_init()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_init) | Initialise the manager and connect to AWE Core |
| [`awemgr_exit()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_exit) | Disconnect and free all resources |

---

### AWC (endpoint) management

An *AWC* file groups one or more signal-flow designs together with their metadata.
Each loaded AWC occupies one *endpoint* slot (identified by an integer index).

| Function | Description |
|----------|-------------|
| [`awemgr_load_awc()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_load_awc) | Load an AWC file into an endpoint slot |
| [`awemgr_unload_awc()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_unload_awc) | Unload the AWC from an endpoint slot |
| [`awemgr_get_loaded_awc_count()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_loaded_awc_count) | Number of currently loaded AWC endpoints |
| [`awemgr_get_max_awcs()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_max_awcs) | Maximum number of simultaneous endpoints supported |
| [`awemgr_get_awc_context()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_awc_context) | Obtain the `awemgr_ctx *` for a given endpoint index |

---

### Design (signal flow) management

A *design* is a compiled signal flow, aka, AWB file. The AWC file contains the information which designs are available.

| Function | Description |
|----------|-------------|
| [`awemgr_load_design()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_load_design) | Load and start a named design from the current AWC |
| [`awemgr_unload_design()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_unload_design) | Stop and unload a design |
| [`awemgr_skip_unload_design_on_exit()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_skip_unload_design_on_exit) | Mark a context so the design is NOT unloaded on [`awemgr_exit()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_exit) (detach mode) |
| [`awemgr_get_design_count()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_design_count) | Number of designs available in the loaded AWC |
| [`awemgr_get_design_info()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_design_info) | Name and metadata for a design by index |

---

### Audio control

| Function | Description |
|----------|-------------|
| [`awemgr_audio_start()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_audio_start) | Start audio processing on an endpoint |
| [`awemgr_audio_stop()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_audio_stop) | Stop audio processing on an endpoint |
| [`awemgr_reset_state_all()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_reset_state_all) | Reset all internal AWE state (use before deep sleep) |

---

### Control (parameter) access

*Controls* are named, typed parameters exposed by a loaded design.

| Function | Description |
|----------|-------------|
| [`awemgr_get_controls_count()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_controls_count) | Number of controls in the loaded design |
| [`awemgr_get_control_info()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_control_info) | Info (name, type, range) for a control by index |
| [`awemgr_get_control_info_by_name()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_control_info_by_name) | Info for a control looked up by name |
| [`awemgr_control_write()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_control_write) | Write values to a named control |
| [`awemgr_control_read()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_control_read) | Read all values from a named control |
| [`awemgr_control_read_partial()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_control_read_partial) | Read a subset of values (offset + count) |

---

### Module access

| Function | Description |
|----------|-------------|
| [`awemgr_get_modules_count()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_modules_count) | Number of modules in the loaded design |
| [`awemgr_get_module_by_name()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_module_by_name) | Look up a module handle by name |
| [`awemgr_get_module_by_index()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_module_by_index) | Look up a module handle by index |
| [`awemgr_module_set_state()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_module_set_state) | Set module runtime state (active / bypass / mute) |
| [`awemgr_module_get_state()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_module_get_state) | Read module runtime state |
| [`awemgr_module_get_class()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_module_get_class) | Read the AWE class ID of a module |

---

### User data (metadata attached to modules/controls in the AWC)

| Function | Description |
|----------|-------------|
| [`awemgr_get_module_userdata_count()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_module_userdata_count) | Number of user-data entries on a named module |
| [`awemgr_get_control_userdata_count()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_control_userdata_count) | Number of user-data entries on a named control |
| [`awemgr_get_module_userdata_by_index()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_module_userdata_by_index) | Read a user-data entry by index |
| [`awemgr_get_control_userdata_by_index()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_control_userdata_by_index) | Read a control user-data entry by index |
| [`awemgr_get_module_userdata_by_key()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_module_userdata_by_key) | Read a user-data entry by key string |
| [`awemgr_get_control_userdata_by_key()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_control_userdata_by_key) | Read a control user-data entry by key string |

---

### Events

*Events* are asynchronous notifications fired by AWE modules at runtime
(e.g. level detection, clipping, state changes).

| Function | Description |
|----------|-------------|
| [`awemgr_events_start()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_events_start) | Open the event channel and register a default catch-all callback |
| [`awemgr_events_stop()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_events_stop) | Close the event channel |
| [`awemgr_events_process_next()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_events_process_next) | Block-wait for the next event (poll/thread model) |
| [`awemgr_events_add_category_listener()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_events_add_category_listener) | Register a callback for a specific event category |
| [`awemgr_events_remove_category_listener()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_events_remove_category_listener) | Remove a category listener |
| [`awemgr_events_set_inspector()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_events_set_inspector) | Register a callback that sees *all* events before category dispatch |
| [`awemgr_events_clear_inspector()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_events_clear_inspector) | Remove the inspector callback |
| [`awemgr_enable_event()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_enable_event) | Enable event firing on a named module |
| [`awemgr_disable_event()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_disable_event) | Disable event firing on a named module |
| [`awemgr_enable_event_type()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_enable_event_type) | Enable all events of a given type ID |
| [`awemgr_disable_event_type()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_disable_event_type) | Disable all events of a given type ID |
| [`awemgr_get_event_count()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_event_count) | Number of event-capable modules in the loaded design |
| [`awemgr_get_event_by_index()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_event_by_index) | Module info for an event source by index |

---

### Target information & diagnostics

| Function | Description |
|----------|-------------|
| [`awemgr_get_target_info()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_target_info) | AWE instance names and software versions |
| [`awemgr_get_cpu_info()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_cpu_info) | CPU load per core |
| [`awemgr_get_heap_info()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_heap_info) | Heap memory usage |
| [`awemgr_get_modulelist_info()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_modulelist_info) | All available module class IDs on the target |
| [`awemgr_get_layout_info()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_layout_info) | Per-layout profiling data (cycles, overflow counts) |
| [`awemgr_get_awe_error()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_awe_error) | Last AWE Core error code and description |

---

### Raw communication

Normally the manager handles all protocol framing.  These entry points are for
tooling or testing that needs to bypass the high-level API.

| Function | Description |
|----------|-------------|
| [`awemgr_send_command()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_send_command) | Send a pre-built AWE tuning command to the BSP. |
| [`awemgr_transact()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_transact) | Send a raw word buffer and receive a raw response |
| [`awemgr_set_comm_trace_tap()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_set_comm_trace_tap) | Install a callback that receives raw TX/RX buffers for every transaction |
