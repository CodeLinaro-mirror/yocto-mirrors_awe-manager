"""
conftest.py — shared pytest fixtures for awemgr_shell integration tests.

Running
-------
From the project root, after building the debug target:

    pytest awe_manager/tests/

To point at a different build:

    AWEMGR_SHELL_EXE=/path/to/awemgr_shell pytest awe_manager/tests/

Tests marked ``needs_awe_server`` require a running AWE Server (LinuxApp).
They are skipped automatically when the binary is absent or the server
fails to start.  To run them:

    pytest awe_manager/tests/ -m needs_awe_server
"""

import os
import sys
import time
from pathlib import Path

import pytest

# ---------------------------------------------------------------------------
# Ensure all local packages are importable regardless of how pytest is invoked.
#
# awe_manager  — project root must be on sys.path so that
#                "from awe_manager.tests.X import Y" works.
# shellhandler — lives inside apps/awemgr_client/, not installed as a package.
# ---------------------------------------------------------------------------
_PROJECT_ROOT = Path(__file__).parent.parent.parent  # …/qc-audiolite-integration-awemanager
_CLIENT_DIR   = Path(__file__).parent.parent / "apps" / "awemgr_client"

for _p in (_PROJECT_ROOT, _CLIENT_DIR):
    if str(_p) not in sys.path:
        sys.path.insert(0, str(_p))

from shellhandler import AweMgrShell, SocketShell  # noqa: E402

# ---------------------------------------------------------------------------
# Path constants
# ---------------------------------------------------------------------------
TESTS_DIR = Path(__file__).parent
PROJECT_ROOT = TESTS_DIR.parent.parent

# Default: debug build produced by CMake on Linux
_DEFAULT_SHELL_EXE = (
    PROJECT_ROOT / "build" / "Linux" / "testing"
    / "awe_manager" / "apps" / "awemgr_shell" / "awemgr_shell"
)

SHELL_EXE = Path(os.environ.get("AWEMGR_SHELL_EXE", str(_DEFAULT_SHELL_EXE)))

AWC_MINIMAL = TESTS_DIR / "data" / "designs" / "set_get" / "target_files" / "awc_index.txt"

# ---------------------------------------------------------------------------
# Custom marks
# ---------------------------------------------------------------------------
def pytest_configure(config):
    config.addinivalue_line(
        "markers",
        "needs_awe_server: test requires LinuxApp (AWE Server) to be running",
    )


# ---------------------------------------------------------------------------
# Session-scoped AWE Server fixture
# ---------------------------------------------------------------------------
@pytest.fixture(scope="session")
def awe_server():
    """Start LinuxApp once per session; skip if the binary is not executable.

    When invoked from run_ctest_with_aweserver.py the server is already
    running.  In that case AWEMGR_SERVER_EXTERNAL=1 is set in the environment
    and this fixture simply yields without starting or stopping anything.
    """
    if os.environ.get("AWEMGR_SERVER_EXTERNAL") == "1":
        yield None
        return

    from awe_manager.tests.run_ctest_with_aweserver import AweServerExe

    if not os.access(AweServerExe.exe_path, os.R_OK):
        pytest.skip(f"AWE Server binary not found: {AweServerExe.exe_path}")

    srv = AweServerExe()
    srv.start()
    time.sleep(1)  # let the server bind its socket
    yield srv
    srv.stop()


# ---------------------------------------------------------------------------
# Per-test shell fixtures (subprocess transport)
# ---------------------------------------------------------------------------
@pytest.fixture
def shell():
    """Fresh awemgr_shell subprocess for each test (no AWE Server needed)."""
    if not SHELL_EXE.exists():
        pytest.skip(f"awemgr_shell binary not found: {SHELL_EXE}")
    with AweMgrShell(str(SHELL_EXE)) as sh:
        yield sh


@pytest.fixture
def loaded_shell(awe_server, shell):
    """Shell with a manager initialised and the 'minimal' design loaded."""
    shell.exe_cmd("mgr_init")
    shell.exe_cmd(f"load_awc -awc {AWC_MINIMAL}")
    shell.exe_cmd("load_design -name Main")
    yield shell


# ---------------------------------------------------------------------------
# Per-test shell fixture (socket transport)
# ---------------------------------------------------------------------------
SHELL_SOCKET_PORT = int(os.environ.get("AWEMGR_SHELL_SOCKET_PORT", "7100"))


@pytest.fixture
def socket_shell(awe_server, tmp_path):
    """
    Start awemgr_shell in socket mode, connect via SocketShell.

    This exercises the TCP code-path in both the shell addon and
    the shellhandler transport.
    """
    import subprocess

    if not SHELL_EXE.exists():
        pytest.skip(f"awemgr_shell binary not found: {SHELL_EXE}")

    proc = subprocess.Popen(
        [str(SHELL_EXE), "-s", str(SHELL_SOCKET_PORT)],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    time.sleep(0.5)  # let the listener bind

    try:
        with SocketShell("localhost", SHELL_SOCKET_PORT) as sh:
            yield sh
    finally:
        proc.kill()
        proc.wait()
