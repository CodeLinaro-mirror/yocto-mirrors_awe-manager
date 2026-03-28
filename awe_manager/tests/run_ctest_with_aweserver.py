import argparse
import logging
import os
from pathlib import Path
import shutil
import subprocess
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
    DEFAULT_TIMEOUT = 30

    this_dir = Path(__file__).parent
    exe_name = "LinuxApp"
    exe_dir = this_dir / "bin" / "linux_x86-64"
    exe_path = exe_dir / exe_name
    exe_params = ["-bsize:48"]

    def __init__(self):
        self.running = False
        self.timeout = AweServerExe.DEFAULT_TIMEOUT

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
        if self.running:
            logger.info(f"AweServerExe:: Calling EXE: {self._exepath}")
            self._proc = subprocess.Popen([self._exepath] + AweServerExe.exe_params, start_new_session=True)
            try:
                _outs, _errs = self._proc.communicate(timeout=self.timeout)
            except subprocess.TimeoutExpired:
                self._proc.kill()
                _outs, _errs = self._proc.communicate()
                logger.error(f"AweServerExe:: Timed out after {self.timeout}")
                # self.running = False
            except:
                logger.error(f"AweServerExe:: Failed to communicate with / start: {self._exepath}")
                self.running = False

        if self.temp_dir:
            self.temp_dir.cleanup()
            logger.error(f"AweServerExe:: Cleanup {self.temp_dir}")

    def stop(self):
        self.running = False
        self._proc.kill()
        logger.info(f"AweServerExe:: {self._exepath} stopped")


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Runs GTest suite with LinuxApp (and event socket simulator)')
    parser.add_argument("--cwd", default=".", help="Execute 'ctest' in this dir, rather than in current dir")
    parser.add_argument("xml_output", help="XML output file for CTEST")
    args = parser.parse_args()

    logging.basicConfig(level=logging.INFO, format='CTEST-RUNNER: # %(levelname)-7s # %(module)-10s :: %(message)s')

    awe = AweServerExe()
    awe.start()  

    ts = TestServers(single_shot=True)
    ts.start()

    # give it a second ;-)
    time.sleep(1)
    # a = input("WARTE HIER")

    ctest_cmd = ["ctest", "--output-junit", args.xml_output]
    logger.info(f"Executing: {ctest_cmd} in dir: {args.cwd}")
    subprocess.run(["ctest", "--output-junit", args.xml_output], cwd=args.cwd)
    
    awe.stop()

    ts.stop()
