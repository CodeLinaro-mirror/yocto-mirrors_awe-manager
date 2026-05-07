"""
awe_parser.py - Data models and parsers for awemgr_shell command output.

The shell output is valid YAML throughout and is parsed with
:func:`yaml.safe_load`.

Every top-level result type inherits :class:`ParseResult` which provides an
``error`` field.  When the C-side command emits ``error: <message>`` before
its normal YAML payload the parser stores it in ``result.error``; otherwise
``result.error`` is ``None``.

List-returning functions return container objects that also inherit
:class:`ParseResult`.  The containers are *iterable* and *sized*, so existing
``for x in parse_foo(...)`` / ``len(parse_foo(...))`` call-sites need no
changes.

Supported commands
------------------
* ``version``              → :func:`parse_version`        → ``str``
* ``info``                 → :func:`parse_info`           → ``InfoResult``
* ``module -name <n>``     → :func:`parse_module`         → ``ModuleState``
* ``info -cpu``            → :func:`parse_info_cpu_all`   → ``CpuInfoAllResult``
* ``info -cpu -endpoint X -core Y`` → :func:`parse_info_cpu` → ``CpuInfo``
* ``show -designs``        → :func:`parse_show_designs`   → ``ShowDesignsResult``
* ``show -modules``        → :func:`parse_show_modules`   → ``ShowModulesResult``
* ``show -events``         → :func:`parse_show_events`    → ``ShowEventsResult``
* ``show -controls``       → :func:`parse_show_controls`  → ``ShowControlsResult``
* ``show -userdata``       → :func:`parse_show_userdata`  → ``ShowUserdataResult``
* ``get_value -var <name>``→ :func:`parse_get_value`      → ``VariableValue``
* ``cfg``                  → :func:`parse_cfg`            → ``CfgResult``
* ``comm``                 → :func:`parse_comm`           → ``CommInfo``
* ``comm-trace``           → :func:`parse_comm_trace`     → ``CommTrace``
* ``time-cmds``            → :func:`parse_time_cmds`      → ``bool``
* ``event``                → :func:`parse_event`          → ``Optional[EventData]``
* ``awc``                  → :func:`parse_awc`            → ``AwcInfo``
"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any, Iterator, List, Optional, Union

import yaml


# ---------------------------------------------------------------------------
# Base
# ---------------------------------------------------------------------------

@dataclass(kw_only=True)
class ParseResult:
    """Base for every top-level parse-result type.

    ``error`` is ``None`` on success.  When the C command emits
    ``error: <message>`` in its YAML output the parser stores it here.
    """
    error: Optional[str] = None


# ---------------------------------------------------------------------------
# Item types  (nested inside list results — do not inherit ParseResult)
# ---------------------------------------------------------------------------

@dataclass
class ExtendedInfo:
    """Extended AWE instance data reported by ``info -extended``."""
    user_version: int
    hotfix_version: int
    nr_threads: int
    commbuffer_size: int
    alignment_size: int
    supports_module_reset: bool
    supports_fract16: bool


@dataclass
class PinsInfo:
    """Pin counts reported by ``info -extended``."""
    nr_in: int
    nr_out: int


@dataclass
class TargetInstance:
    """One AWE instance reported by ``info``."""
    name: str              # e.g. "awe#0"
    awecore_version: str   # e.g. "8.C.0.0"
    proc_type: Optional[str] = None
    instance_id: Optional[int] = None
    block_size: Optional[int] = None
    sample_rate: Optional[float] = None
    build_nr: Optional[int] = None        # None when target reports "-not set-"
    extended: Optional[ExtendedInfo] = None
    pins: Optional[PinsInfo] = None


@dataclass
class PluginInfo:
    """A plugin entry nested inside a :class:`DesignInfo`."""
    name: str
    core: int


@dataclass
class DesignInfo:
    """One design (AWB) reported by ``show -designs``."""
    name: str
    size_in_bytes: int
    plugins: List[PluginInfo] = field(default_factory=list)
    core_id: Optional[int] = None    # present when coreid_objectid != 0
    object_id: Optional[int] = None  # present when coreid_objectid != 0


@dataclass
class ControlRange:
    """Optional min/max/step constraint on a :class:`ControlInfo`."""
    min: float
    max: float
    step: float


@dataclass
class ControlInfo:
    """One controllable variable reported by ``show -controls``."""
    name: str           # e.g. "SourceInt_10.value"
    size: int           # number of array elements
    type: str           # "integer", "float", "fract32", "fract16", "unsigned integer"
    alias: Optional[str] = None
    range: Optional[ControlRange] = None


@dataclass
class ModuleInfo:
    """One module reported by ``show -modules``."""
    name: str
    obj_id: int
    class_id: int       # stored as int; use ``hex(m.class_id)`` for display
    alias: Optional[str] = None


@dataclass
class EventModuleInfo:
    """One event-capable module reported by ``show -events``."""
    name: str
    object_id: int
    class_id: int
    state: Optional[str] = None


@dataclass
class UserDataItem:
    """One user-data entry reported by ``show -userdata``."""
    name: str   # control or module name that owns this item
    key: str
    type: str   # "int", "uint", "str", "float"
    value: Any  # int | float | str, depending on type


@dataclass
class CfgEntry:
    """One configuration entry reported by ``cfg``."""
    key: str
    value: str
    description: str = ""


# ---------------------------------------------------------------------------
# Scalar result types  (inherit ParseResult)
# ---------------------------------------------------------------------------

@dataclass
class CpuInstanceInfo:
    """CPU load for one AWE instance from ``info -cpu`` (all-instances form)."""
    name: str
    cpu_load_percent: Optional[float]  # None when cpu is ~ (no signal flow)
    layouts: List[float] = field(default_factory=list)


@dataclass
class CpuInfo(ParseResult):
    """CPU load reported by ``info -cpu -endpoint <n> -core <n>``."""
    cpu_load_percent: Optional[float]  # None when no signal flow is running
    average_cycles: int
    time_per_process: int
    overload: bool


@dataclass
class ModuleState(ParseResult):
    """State of one module reported by ``module -name <name>`` (and ``-full``)."""
    state: str           # e.g. "ACTIVE", "BYPASS", "MUTE"
    class_id: Optional[int] = None      # decimal, present with -full
    class_id_hex: Optional[int] = None  # same value, hex notation, present with -full


@dataclass
class CommInfo(ParseResult):
    """Connection info reported by ``comm``."""
    host: str
    port: int


@dataclass
class CommTrace(ParseResult):
    """Trace state reported by ``comm-trace``."""
    state: str            # "on" or "off"
    file: Optional[str] = None   # None when output is ~ (YAML null)


@dataclass
class VariableValue(ParseResult):
    """Variable data reported by ``get_value -var <name>``.

    Hex integers (``0x0000dead``) are stored as plain ``int``.
    Float arrays (``3.5``) are stored as ``float``; integer literals
    (``1``, ``0``) in a float array remain ``int`` as parsed by PyYAML.
    """
    name: str
    data: List[Union[int, float]] = field(default_factory=list)


@dataclass
class EventData(ParseResult):
    """One event read by ``event``.

    ``payload`` hex literals are stored as plain ``int``.
    :func:`parse_event` returns ``None`` (not an ``EventData``) when no event
    arrived within the timeout.
    """
    category: int
    type: int
    size: int
    module_name: str
    object_id: int
    payload: List[int] = field(default_factory=list)


@dataclass
class AwcInfo(ParseResult):
    """Active AWC endpoint reported by ``awc``.

    ``instance_id`` is ``None`` when no AWC is loaded on the requested endpoint.
    """
    endpoint_id: int
    endpoint_id_max: int
    instance_id: Optional[int] = None


# ---------------------------------------------------------------------------
# Container result types for list-returning parsers  (inherit ParseResult)
#
# All containers are iterable and sized so that existing call-sites of the
# form ``for x in parse_foo(...)`` and ``len(parse_foo(...))`` work unchanged.
# ---------------------------------------------------------------------------

@dataclass
class CpuInfoAllResult(ParseResult):
    """Result of :func:`parse_info_cpu_all` (``info -cpu`` without -endpoint/-core)."""
    instances: List[CpuInstanceInfo] = field(default_factory=list)

    def __iter__(self) -> Iterator[CpuInstanceInfo]:
        return iter(self.instances)

    def __len__(self) -> int:
        return len(self.instances)

    def __getitem__(self, idx: int) -> CpuInstanceInfo:
        return self.instances[idx]


@dataclass
class InfoResult(ParseResult):
    """Result of :func:`parse_info`."""
    instances: List[TargetInstance] = field(default_factory=list)

    def __iter__(self) -> Iterator[TargetInstance]:
        return iter(self.instances)

    def __len__(self) -> int:
        return len(self.instances)

    def __getitem__(self, idx: int) -> TargetInstance:
        return self.instances[idx]


@dataclass
class ShowDesignsResult(ParseResult):
    """Result of :func:`parse_show_designs`."""
    designs: List[DesignInfo] = field(default_factory=list)

    def __iter__(self) -> Iterator[DesignInfo]:
        return iter(self.designs)

    def __len__(self) -> int:
        return len(self.designs)

    def __getitem__(self, idx: int) -> DesignInfo:
        return self.designs[idx]


@dataclass
class ShowModulesResult(ParseResult):
    """Result of :func:`parse_show_modules`."""
    modules: List[ModuleInfo] = field(default_factory=list)

    def __iter__(self) -> Iterator[ModuleInfo]:
        return iter(self.modules)

    def __len__(self) -> int:
        return len(self.modules)

    def __getitem__(self, idx: int) -> ModuleInfo:
        return self.modules[idx]


@dataclass
class ShowEventsResult(ParseResult):
    """Result of :func:`parse_show_events`."""
    event_modules: List[EventModuleInfo] = field(default_factory=list)

    def __iter__(self) -> Iterator[EventModuleInfo]:
        return iter(self.event_modules)

    def __len__(self) -> int:
        return len(self.event_modules)

    def __getitem__(self, idx: int) -> EventModuleInfo:
        return self.event_modules[idx]


@dataclass
class ShowControlsResult(ParseResult):
    """Result of :func:`parse_show_controls`."""
    controls: List[ControlInfo] = field(default_factory=list)

    def __iter__(self) -> Iterator[ControlInfo]:
        return iter(self.controls)

    def __len__(self) -> int:
        return len(self.controls)

    def __getitem__(self, idx: int) -> ControlInfo:
        return self.controls[idx]


@dataclass
class ShowUserdataResult(ParseResult):
    """Result of :func:`parse_show_userdata`."""
    userdata: List[UserDataItem] = field(default_factory=list)

    def __iter__(self) -> Iterator[UserDataItem]:
        return iter(self.userdata)

    def __len__(self) -> int:
        return len(self.userdata)

    def __getitem__(self, idx: int) -> UserDataItem:
        return self.userdata[idx]


@dataclass
class CfgResult(ParseResult):
    """Result of :func:`parse_cfg`."""
    entries: List[CfgEntry] = field(default_factory=list)

    def __iter__(self) -> Iterator[CfgEntry]:
        return iter(self.entries)

    def __len__(self) -> int:
        return len(self.entries)

    def __getitem__(self, idx: int) -> CfgEntry:
        return self.entries[idx]


# ---------------------------------------------------------------------------
# Public parse functions
# ---------------------------------------------------------------------------

def parse_version(text: str) -> str:
    """Parse the output of ``version``.

    Returns the version string (e.g. ``"0.8.0-172-g8d39e7b-dirty"``).

    Source: ``awemgr_cmdline.cpp::amgr_version``
    """
    data = yaml.safe_load(text) or {}
    return str(data.get("awemgr_version", ""))


def parse_info_cpu(text: str) -> CpuInfo:
    """Parse the output of ``info -cpu [-core <n>]``.

    ``cpu_load_percent`` is ``None`` when no signal flow is running
    (both ``AverageCycles`` and ``TimePerProcess`` are zero).

    Source: ``cmds_targetinfo.cpp``
    """
    data = yaml.safe_load(text) or {}
    return CpuInfo(
        cpu_load_percent=float(data["cpu_load_percent"]) if data.get("cpu_load_percent") is not None else None,
        average_cycles=int(data.get("average_cycles", 0)),
        time_per_process=int(data.get("time_per_process", 0)),
        overload=bool(data.get("overload", False)),
        error=data.get("error"),
    )


def parse_info_cpu_all(text: str) -> CpuInfoAllResult:
    """Parse the output of ``info -cpu`` (all-instances form).

    ``cpu_load_percent`` is ``None`` when the instance reports ``~``
    (no signal flow running on that core).

    Source: ``cmds_targetinfo.cpp::target_info_cpuload_all``
    """
    data = yaml.safe_load(text) or {}
    instances = []
    for d in data.get("cpu_info", []):
        cpu_raw = d.get("cpu")
        instances.append(CpuInstanceInfo(
            name=str(d["name"]),
            cpu_load_percent=float(cpu_raw) if cpu_raw is not None else None,
            layouts=[float(x) for x in (d.get("layouts") or [])],
        ))
    return CpuInfoAllResult(instances=instances, error=data.get("error"))


def parse_module(text: str) -> ModuleState:
    """Parse the output of ``module -name <name>`` (optionally with ``-full``).

    Source: ``cmds_control.cpp::module_generic``
    """
    data = yaml.safe_load(text) or {}
    return ModuleState(
        state=str(data.get("state", "")),
        class_id=int(data["class_id"]) if "class_id" in data else None,
        class_id_hex=int(data["class_id_hex"]) if "class_id_hex" in data else None,
        error=data.get("error"),
    )


def parse_info(text: str) -> InfoResult:
    """Parse the output of ``info`` and ``info -extended``.

    Source: ``cmds_targetinfo.cpp::targetinfo_software``
    """
    data = yaml.safe_load(text) or {}
    instances = []
    for d in data.get("target_info", []):
        raw_build = d.get("build_nr")
        try:
            build_nr = int(raw_build) if raw_build is not None else None
        except (ValueError, TypeError):
            build_nr = None

        ext_d = d.get("extended")
        extended = (
            ExtendedInfo(
                user_version=int(ext_d["user_version"]),
                hotfix_version=int(ext_d["hotfix_version"]),
                nr_threads=int(ext_d["nr_threads"]),
                commbuffer_size=int(ext_d["commbuffer_size"]),
                alignment_size=int(ext_d["alignment_size"]),
                supports_module_reset=bool(ext_d["supports_module_reset"]),
                supports_fract16=bool(ext_d["supports_fract16"]),
            )
            if ext_d else None
        )

        pins_d = d.get("pins")
        pins = (
            PinsInfo(nr_in=int(pins_d["nr_in"]), nr_out=int(pins_d["nr_out"]))
            if pins_d else None
        )

        instances.append(TargetInstance(
            name=str(d["name"]),
            awecore_version=str(d["awecore_version"]),
            proc_type=str(d["proc_type"]) if "proc_type" in d else None,
            instance_id=int(d["instance_id"]) if "instance_id" in d else None,
            block_size=int(d["block_size"]) if "block_size" in d else None,
            sample_rate=float(d["sample_rate"]) if "sample_rate" in d else None,
            build_nr=build_nr,
            extended=extended,
            pins=pins,
        ))
    return InfoResult(instances=instances, error=data.get("error"))


def parse_show_designs(text: str) -> ShowDesignsResult:
    """Parse the output of ``show -designs``.

    Source: ``cmds_showinfo.cpp::show_available_designs``
    """
    data = yaml.safe_load(text) or {}
    designs: List[DesignInfo] = []
    for d in data.get("designs", []):
        raw_plugins = d.get("plugins") or []
        plugins = [
            PluginInfo(name=str(p["name"]), core=int(p.get("core", 0)))
            for p in raw_plugins
            if isinstance(p, dict)
        ]
        designs.append(DesignInfo(
            name=str(d["name"]),
            size_in_bytes=int(d.get("size_in_bytes", 0)),
            plugins=plugins,
            core_id=int(d["core_id"]) if "core_id" in d else None,
            object_id=int(d["objectId"]) if "objectId" in d else None,
        ))
    return ShowDesignsResult(designs=designs, error=data.get("error"))


def parse_show_modules(text: str) -> ShowModulesResult:
    """Parse the output of ``show -modules``.

    Source: ``cmds_showinfo.cpp::show_available_modules``
    """
    data = yaml.safe_load(text) or {}
    return ShowModulesResult(
        modules=[
            ModuleInfo(
                name=str(d["name"]),
                obj_id=int(d["obj_id"]),
                class_id=int(d["class_id"]),
                alias=str(d["alias"]) if "alias" in d else None,
            )
            for d in data.get("modules", [])
        ],
        error=data.get("error"),
    )


def parse_show_events(text: str) -> ShowEventsResult:
    """Parse the output of ``show -events``.

    Source: ``cmds_showinfo.cpp::show_available_event_modules``
    """
    data = yaml.safe_load(text) or {}
    return ShowEventsResult(
        event_modules=[
            EventModuleInfo(
                name=str(d["name"]),
                object_id=int(d["object_id"]),
                class_id=int(d["class_id"]),
                state=str(d["state"]) if "state" in d else None,
            )
            for d in data.get("event_modules", [])
        ],
        error=data.get("error"),
    )


def parse_show_controls(text: str) -> ShowControlsResult:
    """Parse the output of ``show -controls``.

    The output has no root key — PyYAML parses it directly as a list.
    If an ``error:`` key is present the output is a mapping instead; the
    parser handles both forms.

    Source: ``cmds_showinfo.cpp::show_available_controls``
    """
    raw = yaml.safe_load(text)
    if isinstance(raw, dict):
        # error case: YAML is a mapping with at least an "error" key
        items = raw.get("controls") or []
        err: Optional[str] = raw.get("error")
    else:
        items = raw or []
        err = None
    controls: List[ControlInfo] = []
    for d in items:
        raw_range = d.get("range")
        ctrl_range = (
            ControlRange(
                min=float(raw_range["min"]),
                max=float(raw_range["max"]),
                step=float(raw_range["step"]),
            )
            if raw_range
            else None
        )
        controls.append(ControlInfo(
            name=str(d["name"]),
            size=int(d["size"]),
            type=str(d["type"]),
            alias=str(d["alias"]) if "alias" in d else None,
            range=ctrl_range,
        ))
    return ShowControlsResult(controls=controls, error=err)


def parse_show_userdata(text: str) -> ShowUserdataResult:
    """Parse the output of ``show -userdata``.

    Source: ``cmds_showinfo.cpp::show_available_userdata``
    """
    data = yaml.safe_load(text) or {}
    return ShowUserdataResult(
        userdata=[
            UserDataItem(
                name=str(d["name"]),
                key=str(d["key"]),
                type=str(d["type"]),
                value=d.get("value"),
            )
            for d in data.get("userdata", [])
        ],
        error=data.get("error"),
    )


def parse_event(text: str) -> Optional[EventData]:
    """Parse the output of ``event``.

    Returns ``None`` when the shell reports no event was received within the
    timeout (``status: No event received within 250ms``).

    Payload hex literals (``0x74736554``) are stored as plain ``int``.

    Source: ``cmds_event.cpp``
    """
    data = yaml.safe_load(text) or {}
    if "event" not in data:
        return None
    ev = data["event"]
    return EventData(
        category=int(ev["category"]),
        type=int(ev["type"]),
        size=int(ev["size"]),
        module_name=str(ev["module_name"]),
        object_id=int(ev["object_id"]),
        payload=[int(x) for x in ev.get("payload") or []],
        error=data.get("error"),
    )


def parse_awc(text: str) -> AwcInfo:
    """Parse the output of ``awc`` / ``awc -endpoint <n>``.

    Valid endpoint::

        awc:
          endpoint_id: 0
          endpoint_id_max: 15
          instance_id: 0

    Invalid endpoint (no AWC loaded on that endpoint)::

        error: Invalid endpointId. Endpoint should be between 0 and 15
        awc:
          endpoint_id: 0
          endpoint_id_max: 15
          instance_id: ~

    ``instance_id`` is ``None`` when ``~`` (no AWC loaded).
    ``error`` is ``None`` on success.
    """
    data = yaml.safe_load(text) or {}
    awc = data.get("awc", {})
    raw_instance = awc.get("instance_id")
    return AwcInfo(
        endpoint_id=int(awc["endpoint_id"]),
        endpoint_id_max=int(awc["endpoint_id_max"]),
        instance_id=None if raw_instance is None else int(raw_instance),
        error=data.get("error"),
    )


def parse_time_cmds(text: str) -> bool:
    """Parse the output of ``time-cmds``.

    Returns ``True`` when command timing is enabled.

    Source: ``cmds_base.cpp``
    """
    data = yaml.safe_load(text) or {}
    return bool(data.get("time_commands", False))


def parse_comm_trace(text: str) -> CommTrace:
    """Parse the output of ``comm-trace``.

    ``file`` is ``None`` when the YAML value is ``~`` (null).

    Source: ``cmds_comm.cpp``
    """
    data = yaml.safe_load(text) or {}
    traces = data.get("traces", {})
    return CommTrace(
        state=str(traces.get("state", "off")),
        file=str(traces["file"]) if traces.get("file") is not None else None,
        error=data.get("error"),
    )


def parse_comm(text: str) -> CommInfo:
    """Parse the output of ``comm``.

    The YAML ``ctrl_socket`` value is a list of single-key dicts
    (``[{host: ...}, {port: ...}]``); these are merged into one object.

    Source: ``cmds_comm.cpp``
    """
    data = yaml.safe_load(text) or {}
    flat: dict = {}
    for item in data.get("ctrl_socket", []):
        flat.update(item)
    return CommInfo(
        host=str(flat.get("host", "")),
        port=int(flat.get("port", 0)),
        error=data.get("error"),
    )


def parse_cfg(text: str) -> CfgResult:
    """Parse the output of ``cfg``.

    Returns all configuration key/value pairs with their descriptions.

    Source: ``cmds_config.cpp``
    """
    data = yaml.safe_load(text) or {}
    return CfgResult(
        entries=[
            CfgEntry(
                key=str(d["key"]),
                value=str(d["value"]),
                description=str(d.get("description", "")),
            )
            for d in data.get("cfg", [])
        ],
        error=data.get("error"),
    )


def parse_get_value(text: str) -> VariableValue:
    """Parse the output of ``get_value -var <name>``.

    Hex integers (``0x0000dead``) are parsed natively by PyYAML (YAML 1.1)
    and stored as plain ``int``.

    Source: ``cmds_control.cpp``
    """
    data = yaml.safe_load(text) or {}
    var = data.get("variable", {})
    return VariableValue(
        name=str(var.get("name", "")),
        data=list(var.get("data") or []),
        error=data.get("error"),
    )
