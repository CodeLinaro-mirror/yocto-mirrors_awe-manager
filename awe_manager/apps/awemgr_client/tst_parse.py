import argparse
from contextlib import contextmanager
from pathlib import Path

from shellhandler import AweMgrShell, SocketShell
from awe_parser import (
    parse_get_value, parse_version, parse_info, parse_info_cpu, parse_info_cpu_all, parse_module,
    parse_show_designs, parse_show_modules, parse_show_events, parse_show_controls, parse_event,
    parse_cfg, parse_comm, parse_comm_trace, parse_time_cmds, parse_awc,
)

TOPDIR = Path(__file__).parent / "../../../"
SHELL = TOPDIR / "build/Linux/debug/awe_manager/apps/awemgr_shell/awemgr_shell"
AWC_DIR = TOPDIR / "awe_manager/tests/data/designs/events/target_files/awc_index.txt"


def _parse_args():
    p = argparse.ArgumentParser(description="awemgr_shell parser smoke-test")
    p.add_argument(
        "--socket", metavar="HOST:PORT",
        help="Connect to a running awemgr_service shell socket instead of "
             "spawning a local subprocess (e.g. localhost:15100). "
             "When set, mgr_init / load_awc / load_design are skipped.",
    )
    return p.parse_args()


@contextmanager
def _open_shell(args):
    """Yield the appropriate transport based on *args*."""
    if args.socket:
        host, _, port = args.socket.rpartition(":")
        with SocketShell(host or "localhost", int(port)) as shell:
            yield shell, False   # (shell, needs_init)
    else:
        with AweMgrShell(SHELL) as shell:
            yield shell, True


def dbg(shell, cmd):
    """Run *cmd* and print its raw output before returning it."""
    raw = shell.exe_cmd(cmd)
    print(f"  [DBG] {cmd!r} -> {raw!r}")
    return raw


args = _parse_args()
with _open_shell(args) as (shell, needs_init):
    if needs_init:
        dbg(shell, "mgr_init")

    dbg(shell, f"load_awc -awc {AWC_DIR}")
    dbg(shell, "load_design -name Main")

    print("=== VERSION ===")
    print(f"  {parse_version(dbg(shell, 'version'))}")

    print("=== CFG ===")
    for entry in parse_cfg(dbg(shell, "cfg")):
        print(f"  {entry.key} = {entry.value}")

    print("=== COMM ===")
    comm = parse_comm(dbg(shell, "comm"))
    print(f"  {comm.host}:{comm.port}")

    print("=== COMM-TRACE ===")
    ct = parse_comm_trace(dbg(shell, "comm-trace"))
    print(f"  state={ct.state}  file={ct.file}")

    print("=== TIME-CMDS ===")
    print(f"  enabled={parse_time_cmds(dbg(shell, 'time-cmds'))}")

    print("=== AWC ===")
    awc = parse_awc(dbg(shell, "awc"))
    if awc.error:
        print(f"  error: {awc.error}")
    print(f"  endpoint_id={awc.endpoint_id} (max={awc.endpoint_id_max}) instance_id={awc.instance_id}")

    print("=== INFO ===")

    instances = parse_info(dbg(shell, "info"))
    print(f"  {len(instances)} instance(s) found")
    for inst in instances:
        print(f"  {inst.name}: {inst.awecore_version} - proc={inst.proc_type} block_size={inst.block_size} sample_rate={inst.sample_rate} ")

    print("=== CPU ===")
    cpu_all = parse_info_cpu_all(dbg(shell, "info -cpu"))
    for cpu_inst in cpu_all:
        load = f"{cpu_inst.cpu_load_percent:.2f}%" if cpu_inst.cpu_load_percent is not None else "n/a"
        layouts = ", ".join(f"{v:.2f}%" for v in cpu_inst.layouts)
        print(f"  {cpu_inst.name}: {load}  layouts=[{layouts}]")

    print("=== DESIGNS ===")
    for d in parse_show_designs(dbg(shell, "show -designs")):
        print(f"  {d.name}: {d.size_in_bytes} bytes, {len(d.plugins)} plugin(s)")

    print("=== EVENTS ===")
    events = parse_show_events(dbg(shell, "show -events"))
    print(f"  {len(events)} event module(s) found")
    for ev in events:
        print(f"  {ev.name}: state={ev.state} obj_id={ev.object_id} class_id={hex(ev.class_id)}")

    print("=== MODULES ===")
    modules = parse_show_modules(dbg(shell, "show -modules"))
    print(f"  {len(modules)} module(s) found")
    for mod in modules:
        ms = parse_module(dbg(shell, f"module -full -name {mod.name}"))
        alias = f" alias={mod.alias}" if mod.alias else ""
        print(f"  {mod.name}: state={ms.state} obj_id={mod.obj_id} class_id={hex(mod.class_id)}{alias}")

    print("=== CONTROLS ===")
    controls = parse_show_controls(dbg(shell, "show -controls"))
    print(f"  {len(controls)} control(s) found")
    for ctrl in controls:
        range_str = f"  range=[{ctrl.range.min}..{ctrl.range.max}]" if ctrl.range else ""
        alias_str = f"  alias={ctrl.alias}" if ctrl.alias else ""
        print(f"  {ctrl.name}: size={ctrl.size} type={ctrl.type}{alias_str}{range_str}")

    print("=== SAMPLE DATA ===")
    var = parse_get_value(dbg(shell, "get_value -var SourceV2ASCII.value"))
    print(f"  {var.name}: {var.data}")
    varf = parse_get_value(dbg(shell, "get_value -var EventTrigger.value"))
    print(f"  {varf.name}: {varf.data}")

    print("=== SAMPLE EVENT ===")
    # first call initializes the event subsystem, so we ignore its output and call it again to get actual events
    ev_data = parse_event(dbg(shell, "event"))

    # second call will still report nothing
    ev_data = parse_event(dbg(shell, "event"))
    if ev_data:
        print(f"  {ev_data.module_name}: cat={ev_data.category} type={ev_data.type} size={ev_data.size}")
        print(f"    payload: {[hex(x) for x in ev_data.payload]}")
    else:
        print("  (no event)")

    # setting value
    dbg(shell, 'set_value -var EventTrigger.value -values "1"')
    ev_data = parse_event(dbg(shell, "event"))
    if ev_data:
        print(f"  {ev_data.module_name}: cat={ev_data.category} type={ev_data.type} size={ev_data.size}")
        print(f"    payload: {[hex(x) for x in ev_data.payload]}")
    else:
        print("  (no event)")



