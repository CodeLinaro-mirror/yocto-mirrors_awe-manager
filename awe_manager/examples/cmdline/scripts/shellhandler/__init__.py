
import logging
import re
import select
import subprocess
from typing import NamedTuple
from pathlib import Path


logger = logging.getLogger(__name__)


class AweMgrShell:

    def __init__(self, exe_path=None, exe_params=[]) -> None:
        if not exe_path:
            self.exe_path = Path(__file__).parent / "../../../_build-x86_64/awe_manager/examples/cmdline/awemgr_shell"
        else:
            self.exe_path = exe_path
        self.exe_params = exe_params
        self._proc = None

    def __enter__(self):
        logger.debug("Starting subprocess:")
        logger.debug(f" exe - {self.exe_path}")
        logger.debug(f" arg - {self.exe_params}")
        self._proc = subprocess.Popen([self.exe_path] + self.exe_params, stdout=subprocess.PIPE, stdin=subprocess.PIPE)
        return self
    
    def __exit__(self, exc_type, exc_value, exc_tb):
        if self._proc:
            self._proc.kill()

    def exe_cmd(self, cmd):
        logger.debug(f"PYTX: {cmd}")
        self._proc.stdin.write(f"{cmd}\n".encode('utf-8'))
        self._proc.stdin.flush()
        content = bytearray()
        while True:
            if select.select([self._proc.stdout,],[],[],0.25)[0]:
                line = self._proc.stdout.read1()
                content += line
            else:
                # No data for 0.25 secs"
                break
        try:
            txt = content.decode("utf-8")
            logger.debug(f"PYRX: {txt}")
        except UnicodeDecodeError:
            print(f"ERROR UNICODE: {content} for '{cmd}'")
            txt = "BAD TEXT"

        return txt
    
    def get_dir(self):
        class ParseResult(NamedTuple):
            cmd: str
            typ: str
            desc: str

        c = self.exe_cmd("ls")
        entries = []
        for l in c.splitlines():
            m = re.match(r'\s+([.\|\w-]+)\s+<(\w+)>(.*)', l)
            if m:
                entries.append(ParseResult(m.group(1), m.group(2), m.group(3)))
        # print(f"----- {entries}")
        return entries
