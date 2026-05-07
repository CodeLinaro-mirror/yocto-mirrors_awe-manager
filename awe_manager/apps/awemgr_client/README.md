# Python Wrapper for AWE-Manager Shell

These scripts implement a wrapper around the AWE-Manager shell executable
`awemgr_shell`.  They start the shell as a subprocess, communicate with it
interactively, and parse its YAML-formatted output into typed Python
dataclasses.

## Files

| File | Purpose |
|------|---------|
| `shellhandler/__init__.py` | Subprocess wrapper — starts `awemgr_shell`, sends commands, strips log noise |
| `awe_parser.py` | Dataclasses and parse functions for every supported command |
| `tst_parse.py` | Sample implementation to run a special signal flow |
| `gen_mgr_help.py` | Call all commands with -h and store output in a markdown file |
---

## Testing Parse

- compile for debug via cmake presets, see toplevel README
- call LinuxApp as usual in one terminal
- change to this repo in another, call `python tst_parse.py`

## Quick Start

```python
from shellhandler import AweMgrShell
from awe_parser import parse_info, parse_show_designs, parse_get_value

SHELL = "/path/to/awemgr_shell"

with AweMgrShell(SHELL) as shell:
    # Connect to a running AWE-Manager server
    shell.exe_cmd("mgr_init")

    # Query target info
    for inst in parse_info(shell.exe_cmd("info")):
        print(inst.name, inst.sw_version)

    # List available designs
    for d in parse_show_designs(shell.exe_cmd("show -designs")):
        print(d.name, d.size_in_bytes)

    # Read a variable
    v = parse_get_value(shell.exe_cmd("get_value -var SinkFloat_10.value"))
    print(v.name, v.data)
```

If the `awemgr_shell` binary is at the default build location
(`build/Linux/debug/awe_manager/apps/awemgr_shell/awemgr_shell` relative to the repo root) the `exe_path` argument can be omitted.

---

## `shellhandler` — `AweMgrShell`

```python
class AweMgrShell:
    def __init__(self, exe_path=None, exe_params=None): ...
    def exe_cmd(self, cmd: str) -> str: ...
    def get_dir(self) -> List[DirEntry]: ...
```

Use as a context manager (`with` statement).  The shell process is killed on
exit.

### `exe_cmd(cmd)`

Sends `cmd` to the shell over stdin and reads the response until no new data
arrives for 250 ms.  AWE-Manager log lines (`[ERROR]`, `[WARN ]`, `[INFO ]`,
`[DEBUG]`, `[DUMP] `) that the library occasionally prints to stdout are
stripped automatically before the response is returned.

### `get_dir()`

Sends `ls` and parses the result into a list of `DirEntry(cmd, typ, desc)`
objects — one per idbg directory entry.

### Logging

`shellhandler` uses the standard `logging` module under the logger name
`shellhandler`.  Enable debug output with:

```python
import logging
logging.basicConfig(level=logging.DEBUG)
```

---

## `awe_parser` — Parse Functions

All functions accept the raw string returned by `exe_cmd()` and return typed
dataclasses.  The shell output is valid YAML throughout; parsing is done with
`yaml.safe_load()`.  PyYAML (YAML 1.1) natively handles `0xHEX` integer
literals.

| Command | Parse function | Return type |
|---------|---------------|-------------|
| `version` | `parse_version(text)` | `str` |
| `info` | `parse_info(text)` | `List[TargetInstance]` |
| `info -cpu [-core N] [-endpoint N]` | `parse_info_cpu(text)` | `CpuInfo` |
| `module -name <name> [-full]` | `parse_module(text)` | `ModuleState` |
| `show -designs` | `parse_show_designs(text)` | `List[DesignInfo]` |
| `show -modules` | `parse_show_modules(text)` | `List[ModuleInfo]` |
| `show -events` | `parse_show_events(text)` | `List[EventModuleInfo]` |
| `show -controls` | `parse_show_controls(text)` | `List[ControlInfo]` |
| `show -userdata` | `parse_show_userdata(text)` | `List[UserDataItem]` |
| `get_value -var <name>` | `parse_get_value(text)` | `VariableValue` |
| `cfg` | `parse_cfg(text)` | `List[CfgEntry]` |

### Dataclasses

```python
@dataclass
class TargetInstance:
    name: str          # e.g. "awe#0"
    sw_version: str    # e.g. "8.C.0.0"

@dataclass
class CpuInfo:
    cpu_load_percent: Optional[float]  # None when no signal flow is running
    average_cycles: int
    time_per_process: int
    overload: bool

@dataclass
class ModuleState:
    state: str                     # "ACTIVE", "BYPASS", "MUTE", …
    class_id: Optional[int]        # decimal; only with -full
    class_id_hex: Optional[int]    # same value, from hex literal; only with -full

@dataclass
class DesignInfo:
    name: str
    size_in_bytes: int
    plugins: List[PluginInfo]
    core_id: Optional[int]
    object_id: Optional[int]

@dataclass
class PluginInfo:
    name: str
    core: int

@dataclass
class ModuleInfo:
    name: str
    obj_id: int
    class_id: int      # use hex(m.class_id) for display
    alias: Optional[str]

@dataclass
class EventModuleInfo:
    name: str
    object_id: int
    class_id: int
    state: Optional[str]

@dataclass
class ControlInfo:
    name: str
    size: int          # number of array elements
    type: str          # "integer", "float", "fract32", "fract16", "unsigned integer"
    alias: Optional[str]
    range: Optional[ControlRange]

@dataclass
class ControlRange:
    min: float
    max: float
    step: float

@dataclass
class UserDataItem:
    name: str          # owning control or module name
    key: str
    type: str          # "int", "uint", "str", "float"
    value: Any         # int | float | str depending on type

@dataclass
class CfgEntry:
    key: str          # e.g. "mgr.api.log.level"
    value: str        # e.g. "warning"
    description: str  # human-readable description and allowed values

@dataclass
class VariableValue:
    name: str
    data: List[Union[int, float]]
    # Hex literals (0x0000dead) are stored as int.
    # Float arrays may contain a mix of int and float
    # depending on whether PyYAML can represent the value exactly.
```

---

## Notes

- **Server prerequisite** — `awemgr_shell` connects to a running AWE-Manager
  server process (e.g. `LinuxApp -bsize:48`).  Start the server before
  opening the shell.
- **Log noise** — The AWE library occasionally prints `[ERROR]` lines to
  stdout (e.g. when querying CPU info on an idle core).  These are filtered
  out automatically by `exe_cmd()` and never reach the parse functions.
- **Hex integers** — PyYAML (YAML 1.1) parses `0x0000dead` as `int(57005)`.
  YAML 1.2 parsers would reject this syntax.