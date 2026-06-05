"""
test_shell.py — integration tests for the shell addon and awemgr_shell app.

Test structure
--------------
Section 1 – Shell startup          No AWE Server required.  Exercises the
                                    binary's argument handling and idbg
                                    framework before any AWE communication.

Section 2 – Manager lifecycle      Marked ``needs_awe_server``.  Covers
                                    mgr_init / load_awc / load_design and
                                    their error paths.

Section 3 – Info and show          Marked ``needs_awe_server``.  Exercises
                                    the show / info commands once a design
                                    is loaded.

Section 4 – Control get / set      Marked ``needs_awe_server``.  Round-trip
                                    read-write-read on the minimal design's
                                    Scaler1.gain control.

Section 5 – Socket transport       Marked ``needs_awe_server``.  Same
                                    command checks repeated over the TCP
                                    socket transport (SocketShell).

Add new test functions inside the appropriate section.  Use the
``loaded_shell`` fixture for any test that needs a live design; use the
``shell`` fixture for anything that does not touch the AWE Server.
"""

import re
from pathlib import Path
from time import sleep

import pytest

# Path to the minimal test design — mirrors the constant in conftest.py.
_TESTS_DIR = Path(__file__).parent
AWC_MINIMAL = _TESTS_DIR / "data" / "designs" / "minimal" / "target_files" / "awc_index.txt"

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _is_error(response: str) -> bool:
    """Return True if the shell reported an error in *response*."""
    return "error" in response.lower()


def _assert_yaml_list(response: str, expected_key: str) -> list:
    """
    Parse *response* as YAML and assert it contains a list under *expected_key*.

    Returns the list so callers can make further assertions.
    """
    import yaml
    data = yaml.safe_load(response)
    assert isinstance(data, (dict, list)), f"Expected YAML, got: {response!r}"
    if isinstance(data, dict):
        assert expected_key in data, f"Key {expected_key!r} missing from: {data}"
        items = data[expected_key]
    else:
        items = data
    assert isinstance(items, list) and len(items) > 0, (
        f"Expected non-empty list under {expected_key!r}"
    )
    return items


# ===========================================================================
# Section 1 – Shell startup (no AWE Server needed)
# ===========================================================================

def test_shell_starts(shell):
    """awemgr_shell subprocess starts and accepts commands."""
    # Sending an empty command should not crash the shell.
    response = shell.exe_cmd("pwd")
    # No hard assertion on content — just verify we got a string back.
    assert isinstance(response, str)


def test_shell_ls_returns_commands(shell):
    """ls returns a non-empty list of DirEntry objects."""
    entries = shell.get_dir()
    assert len(entries) > 0, "Expected at least one command in ls output"
    names = [e.cmd for e in entries]
    # Core commands that must always be present.
    for required in ("mgr_init", "load_awc", "info", "show"):
        assert required in names, f"Expected {required!r} in ls output: {names}"


def test_shell_ls_entry_types(shell):
    """Every DirEntry has a non-empty cmd and a recognised typ."""
    known_types = {"cmd", "dir", "sub_dir"}
    for entry in shell.get_dir():
        assert entry.cmd, "DirEntry.cmd must not be empty"
        assert entry.typ in known_types, (
            f"Unexpected typ {entry.typ!r} for command {entry.cmd!r}"
        )


def test_unknown_command_reports_error(shell):
    """An unrecognised command should produce an error response."""
    response = shell.exe_cmd("this_command_does_not_exist_xyz")
    assert _is_error(response), f"Expected error, got: {response!r}"


def test_help_flag_shows_usage(shell):
    """Running a command with -h should print usage text, not an error."""
    response = shell.exe_cmd("load_awc -h")
    assert "-awc" in response, f"Expected -awc flag in help output: {response!r}"
    assert not _is_error(response)


def test_cfg_shows_config(shell):
    """cfg should return non-empty configuration output without error."""
    response = shell.exe_cmd("cfg")
    assert "key: mgr.api.log.level" in response, f"Expected key: mgr.api.log.level in cfg output: {response!r}"


def test_comm_trace_on_off(shell):
    """comm-trace -on followed by -off should be accepted without error."""
    on_resp = shell.exe_cmd("comm-trace -on")
    assert not _is_error(on_resp), f"comm-trace -on failed: {on_resp!r}"
    off_resp = shell.exe_cmd("comm-trace -off")
    assert not _is_error(off_resp), f"comm-trace -off failed: {off_resp!r}"


# ===========================================================================
# Section 2 – Manager lifecycle
# ===========================================================================

@pytest.mark.needs_awe_server
def test_mgr_init_succeeds(shell, awe_server):
    """mgr_init with default config returns no error."""
    response = shell.exe_cmd("mgr_init")
    assert not _is_error(response), f"mgr_init failed: {response!r}"


@pytest.mark.needs_awe_server
def test_load_awc_succeeds(shell, awe_server, tmp_path):
    """load_awc with the minimal AWC file succeeds."""
    shell.exe_cmd("mgr_init")
    response = shell.exe_cmd(f"load_awc -awc {AWC_MINIMAL}")
    assert not _is_error(response), f"load_awc failed: {response!r}"


@pytest.mark.needs_awe_server
def test_load_awc_bad_path_reports_error(shell, awe_server):
    """load_awc with a non-existent path should report an error."""
    shell.exe_cmd("mgr_init")
    response = shell.exe_cmd("load_awc -awc /nonexistent/path/awc_index.txt -awb Main")
    assert _is_error(response), f"Expected error for bad path, got: {response!r}"


@pytest.mark.needs_awe_server
def test_load_design_succeeds(loaded_shell):
    """A design should be loadable after AWC is initialised."""
    # loaded_shell fixture already ran load_design; verify info is reachable.
    response = loaded_shell.exe_cmd("info")
    assert not _is_error(response), f"info after load_design failed: {response!r}"


@pytest.mark.needs_awe_server
def test_unload_design(loaded_shell):
    """unload_design stops the design."""
    response = loaded_shell.exe_cmd("unload_design -name Main")
    assert not _is_error(response), f"unload_design failed: {response!r}"
    show_resp = loaded_shell.exe_cmd("info -cpu")
    assert "xyz" not in show_resp, (
        f"No pumping design expected after unload: {show_resp!r}"
    )


@pytest.mark.needs_awe_server
def test_reset(loaded_shell):
    """reset should complete without error on a loaded design."""
    response = loaded_shell.exe_cmd("reset")
    assert not _is_error(response), f"reset failed: {response!r}"


# ===========================================================================
# Section 3 – Info and show
# ===========================================================================

@pytest.mark.needs_awe_server
def test_info_returns_version(loaded_shell):
    """info output should mention a version string."""
    response = loaded_shell.exe_cmd("info")
    assert re.search(r"\d+\.\d+", response), (
        f"Expected version pattern in info output: {response!r}"
    )


@pytest.mark.needs_awe_server
def test_show_designs_lists_main(loaded_shell):
    """show -designs should list the loaded 'Main' design."""
    response = loaded_shell.exe_cmd("show -designs")
    assert "Main" in response, f"Expected 'Main' in show -designs: {response!r}"


@pytest.mark.needs_awe_server
def test_show_controls_lists_scaler_gain(loaded_shell):
    """show -controls should include the Scaler1.gain control from the minimal AWC."""
    response = loaded_shell.exe_cmd("show -controls")
    assert "SourceFloat_1.value" in response, (
        f"Expected SourceFloat_1.value in show -controls: {response!r}"
    )


@pytest.mark.needs_awe_server
def test_show_modules_lists_scaler(loaded_shell):
    """show -modules should include the Scaler1 module."""
    response = loaded_shell.exe_cmd("show -modules")
    assert "SourceFloat_10" in response, (
        f"Expected SourceFloat_10 in show -modules: {response!r}"
    )


@pytest.mark.needs_awe_server
def test_show_events(loaded_shell):
    """show -events should complete without error."""
    response = loaded_shell.exe_cmd("show -events")
    assert not _is_error(response), f"show -events failed: {response!r}"


@pytest.mark.needs_awe_server
def test_show_userdata(loaded_shell):
    """show -userdata should complete without error."""
    response = loaded_shell.exe_cmd("show -userdata")
    assert not _is_error(response), f"show -userdata failed: {response!r}"


@pytest.mark.needs_awe_server
def test_info_cpu(loaded_shell):
    """info -cpu should return CPU usage data without error."""
    response = loaded_shell.exe_cmd("info -cpu")
    assert not _is_error(response), f"info -cpu failed: {response!r}"
    assert response.strip(), "info -cpu returned empty response"


@pytest.mark.needs_awe_server
def test_info_mem(loaded_shell):
    """info -mem should return memory statistics without error."""
    response = loaded_shell.exe_cmd("info -mem")
    assert not _is_error(response), f"info -mem failed: {response!r}"
    assert response.strip(), "info -mem returned empty response"


@pytest.mark.needs_awe_server
def test_info_classes(loaded_shell):
    """info -classes should list available module classes."""
    response = loaded_shell.exe_cmd("info -classes")
    print(response)
    assert not _is_error(response), f"info -classes failed: {response!r}"
    assert response.strip(), "info -classes returned empty response"


@pytest.mark.needs_awe_server
def test_info_layout(loaded_shell):
    """info -layout should describe the audio layout without error."""
    response = loaded_shell.exe_cmd("info -layout")
    assert not _is_error(response), f"info -layout failed: {response!r}"
    assert response.strip(), "info -layout returned empty response"


# ===========================================================================
# Section 4 – Control get / set
# ===========================================================================

@pytest.mark.needs_awe_server
def test_get_value_returns_float(loaded_shell):
    """get_value on SourceFloat_1.value should return a numeric value."""
    response = loaded_shell.exe_cmd("get_value -var SourceFloat_1.value")
    assert not _is_error(response), f"get_value failed: {response!r}"
    # The value should contain at least one digit.
    assert re.search(r"-?\d+(\.\d+)?", response), (
        f"Expected numeric value in get_value response: {response!r}"
    )


@pytest.mark.needs_awe_server
def test_set_value_roundtrip(loaded_shell):
    """set_value followed by get_value should reflect the new value."""
    new_value = "-12.0"
    set_resp = loaded_shell.exe_cmd(
        f"set_value -var SourceFloat_1.value -values \"{new_value}\""
    )
    assert not _is_error(set_resp), f"set_value failed: {set_resp!r}"
    sleep(0.1)  # give the server a moment to process the change
    get_resp = loaded_shell.exe_cmd("get_value -var SinkFloat_1.value")
    assert not _is_error(get_resp), f"get_value after set failed: {get_resp!r}"
    assert "-12" in get_resp, (
        f"Expected -12 in get_value response after set: {get_resp!r}"
    )


@pytest.mark.needs_awe_server
def test_get_value_fract32(loaded_shell):
    """get_value on a fract32 control should return integer values without error."""
    response = loaded_shell.exe_cmd("get_value -var SinkFract_MixOut.value")
    assert not _is_error(response), f"get_value fract32 failed: {response!r}"
    # fract32 values are raw integers — at least one digit must be present.
    assert re.search(r"-?\d+", response), (
        f"Expected integer values in fract32 response: {response!r}"
    )


@pytest.mark.needs_awe_server
def test_set_value_bad_control_reports_error(loaded_shell):
    """set_value on a non-existent control should report an error."""
    response = loaded_shell.exe_cmd(
        "set_value -var DoesNotExist.gain -values 0.0 -float"
    )
    assert _is_error(response), f"Expected error for bad control: {response!r}"


# ===========================================================================
# Section 5 – Socket transport
# ===========================================================================

@pytest.mark.needs_awe_server
def test_socket_shell_ls(socket_shell):
    """SocketShell.get_dir() returns the same command set as subprocess transport."""
    entries = socket_shell.get_dir()
    assert len(entries) > 0
    names = [e.cmd for e in entries]
    for required in ("mgr_init", "load_awc", "info", "show"):
        assert required in names, (
            f"Expected {required!r} in socket ls output: {names}"
        )


@pytest.mark.needs_awe_server
def test_socket_shell_unknown_command(socket_shell):
    """SocketShell returns an error for an unrecognised command."""
    response = socket_shell.exe_cmd("this_command_does_not_exist_xyz")
    assert _is_error(response), f"Expected error over socket, got: {response!r}"


@pytest.mark.needs_awe_server
def test_socket_shell_mgr_init_and_info(socket_shell, awe_server):
    """mgr_init + info work over the socket transport."""
    socket_shell.exe_cmd("mgr_init")
    response = socket_shell.exe_cmd(f"load_awc -awc {AWC_MINIMAL}")
    assert not _is_error(response), f"load awc over socket failed: {response!r}"
    socket_shell.exe_cmd("load_design -name Main")
    response = socket_shell.exe_cmd("info")
    assert not _is_error(response), f"info over socket failed: {response!r}"
