import argparse
import logging
import os
from pathlib import Path
import shutil
import socket
import subprocess
import sys
import tempfile
import threading
import time

from awe_manager.tests.event_socket_simulator import TestServers

logger = logging.getLogger(__file__)


class AweServerExe(threading.Thread):
    """
    Wrapper around an "AWE Server" executable. Currently, it starts the LinuxApp executable
    with a blocksize compatible to the AWE Manager test designs.

    It also takes care that the executable, which is stored withing AWE Manager's source tree,
    is copied into the BINARY directory and confectioned there to be ready for execution.
    """
    this_dir = Path(__file__).parent
    exe_name = "LinuxApp"
    exe_dir = this_dir / "bin" / "linux_x86-64"
    exe_path = exe_dir / exe_name
    exe_params = ["-bsize:48"]

    def __init__(self):
        self.running = False
        self._proc = None

        if not os.access(AweServerExe.exe_path, os.X_OK):
            self.temp_dir = tempfile.TemporaryDirectory()
            self._exepath = Path(self.temp_dir.name) / AweServerExe.exe_name
            shutil.copy(AweServerExe.exe_path, self._exepath)
            os.chmod(self._exepath, 0o755)
        else:
            self.temp_dir = None
            self._exepath = AweServerExe.exe_path
        self.params = ""
        threading.Thread.__init__(self)

    def run(self):
        self.running = True
        logger.info(f"AweServerExe:: Calling EXE: {self._exepath}")
        try:
            self._proc = subprocess.Popen([self._exepath] + AweServerExe.exe_params, start_new_session=True)
            self._proc.communicate()
        except Exception:
            logger.error(f"AweServerExe:: Failed to start: {self._exepath}")
            self.running = False
        finally:
            self.running = False
            if self.temp_dir:
                self.temp_dir.cleanup()
                logger.info(f"AweServerExe:: Cleanup {self.temp_dir}")

    def stop(self):
        if self._proc is not None:
            self._proc.kill()
        logger.info(f"AweServerExe:: {self._exepath} stopped")


AWE_SERVER_PORT = 15002
CTEST_TIMEOUT = 600
PYTEST_TIMEOUT = 300


def _wait_for_port(host: str, port: int, timeout: float = 30.0) -> bool:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        try:
            with socket.create_connection((host, port), timeout=1.0):
                return True
        except OSError:
            time.sleep(0.25)
    return False


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Runs GTest suite with LinuxApp (and event socket simulator)')
    parser.add_argument("--cwd", default=".", help="Execute 'ctest' in this dir, rather than in current dir")
    parser.add_argument("--pytest-xml", default=None, help="If set, run pytest integration tests and write JUnit XML to this path")
    parser.add_argument("xml_output", help="XML output file for CTEST")
    args = parser.parse_args()

    logging.basicConfig(level=logging.INFO, format='CTEST-RUNNER: # %(levelname)-7s # %(module)-10s :: %(message)s')

    awe = AweServerExe()
    awe.start()

    ts = TestServers(single_shot=True)
    ts.start()

    if not _wait_for_port('127.0.0.1', AWE_SERVER_PORT, timeout=30.0):
        logger.error(f"LinuxApp not ready on port {AWE_SERVER_PORT} after 30s — aborting")
        awe.stop()
        ts.stop()
        sys.exit(1)

    ctest_cmd = ["ctest", "--output-on-failure", "--output-junit", args.xml_output]
    logger.info(f"Executing: {ctest_cmd} in dir: {args.cwd}")
    try:
        subprocess.run(ctest_cmd, cwd=args.cwd, timeout=CTEST_TIMEOUT)
    except subprocess.TimeoutExpired:
        logger.error(f"ctest timed out after {CTEST_TIMEOUT}s")

    if args.pytest_xml:
        src_root = Path(__file__).parent.parent.parent
        pytest_cmd = [
            "python", "-m", "pytest",
            str(src_root / "awe_manager" / "tests"),
            "-m", "needs_awe_server",
            f"--junitxml={args.pytest_xml}",
            "-v",
        ]
        logger.info(f"Executing pytest: {pytest_cmd}")
        env = os.environ.copy()
        env["AWEMGR_SERVER_EXTERNAL"] = "1"
        try:
            subprocess.run(pytest_cmd, cwd=src_root, env=env, timeout=PYTEST_TIMEOUT)
        except subprocess.TimeoutExpired:
            logger.error(f"pytest timed out after {PYTEST_TIMEOUT}s")

    awe.stop()

    ts.stop()
