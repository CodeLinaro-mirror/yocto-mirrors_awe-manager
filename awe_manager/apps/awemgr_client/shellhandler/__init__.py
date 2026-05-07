"""
shellhandler - Transport wrappers for programmatic interaction with awemgr_shell.

Two transports are provided, both exposing the same interface
(``exe_cmd(cmd) -> str``, ``get_dir() -> List[DirEntry]``, context manager):

* :class:`AweMgrShell`  — spawns ``awemgr_shell`` as a local subprocess.
* :class:`SocketShell`  — connects to a running ``awemgr_service`` (or any
  ``awemgr_shell_run_socket`` listener) over TCP.

Both transports strip AWE-Manager log lines from responses automatically, so
:mod:`awe_parser` functions receive clean YAML regardless of which transport
is used.

Typical usage::

    from shellhandler import AweMgrShell, SocketShell

    # Local subprocess
    with AweMgrShell("/path/to/awemgr_shell") as shell:
        print(shell.exe_cmd("info"))

    # Remote service
    with SocketShell("localhost", 7100) as shell:
        print(shell.exe_cmd("info"))
        for entry in shell.get_dir():
            print(entry.typ, entry.cmd, entry.desc)
"""

import logging
import re
import select
import socket
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import List, Optional

logger = logging.getLogger(__name__)

# Log-line prefixes emitted by awemgr_logging.h macros (AWEMGR_LOGE/W/I/D/LOG)
_LOG_PREFIXES = ("[ERROR]", "[WARN ]", "[INFO ]", "[DEBUG]", "[DUMP] ")

# Prompt string used by awemgr_shell as a response terminator
_PROMPT = "awemgr-shell > "


@dataclass
class DirEntry:
    """One entry from awemgr_shell's ``ls`` / ``dir`` output."""

    cmd: str
    typ: str
    desc: str = ""


class AweMgrShell:
    """Subprocess wrapper around ``awemgr_shell`` for programmatic control."""

    def __init__(
        self,
        exe_path: Optional[str] = None,
        exe_params: Optional[List[str]] = None,
    ) -> None:
        if not exe_path:
            self.exe_path = Path(__file__).parent / "../../../../build/Linux/debug/awe_manager/apps/awemgr_shell/awemgr_shell"
        else:
            self.exe_path = exe_path
        self.exe_params = exe_params or []
        self._proc = None

    def __enter__(self) -> "AweMgrShell":
        logger.debug("Starting subprocess: %s %s", self.exe_path, self.exe_params)
        self._proc = subprocess.Popen(
            [self.exe_path] + self.exe_params,
            stdout=subprocess.PIPE,
            stdin=subprocess.PIPE,
        )
        self._buf = ""
        # Consume the initial prompt emitted before the first fgets
        self._recv_until_prompt()
        return self

    def __exit__(self, exc_type, exc_value, exc_tb) -> bool:
        if self._proc:
            # Close stdin so the shell's read loop hits EOF and returns from
            # main() normally.  A normal exit() call is required for libgcov
            # to flush .gcda files; SIGTERM/SIGKILL both bypass atexit handlers.
            try:
                self._proc.stdin.close()
            except OSError:
                pass
            try:
                self._proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self._proc.kill()
                self._proc.wait()
        return False

    def exe_cmd(self, cmd: str) -> str:
        """Send *cmd* and return its response (prompt and log lines stripped)."""
        logger.debug("send: %r", cmd)
        self._proc.stdin.write(f"{cmd}\n".encode("utf-8"))
        self._proc.stdin.flush()
        raw = self._recv_until_prompt()
        result = _strip_log_lines(raw)
        logger.debug("recv: %r", result)
        return result

    def _recv_until_prompt(self) -> str:
        """Read from subprocess stdout until the shell prompt appears."""
        while _PROMPT not in self._buf:
            if select.select([self._proc.stdout], [], [], 5.0)[0]:
                chunk = self._proc.stdout.read1().decode("utf-8", errors="replace")
                if not chunk:  # EOF — process exited
                    break
                self._buf += chunk
            else:
                logger.warning("Timed out waiting for shell prompt")
                break
        idx = self._buf.find(_PROMPT)
        if idx >= 0:
            result = self._buf[:idx]
            self._buf = self._buf[idx + len(_PROMPT):]
        else:
            result = self._buf
            self._buf = ""
        return result

    def get_dir(self) -> List[DirEntry]:
        """Return the entries in the current idbg directory."""
        output = self.exe_cmd("ls")
        entries: List[DirEntry] = []
        pattern = re.compile(r"^\s+(\S+)\s+<(\w+)>(?:\s+-\s+(.*))?$")
        for line in output.splitlines():
            m = pattern.match(line)
            if m:
                entries.append(
                    DirEntry(
                        cmd=m.group(1),
                        typ=m.group(2),
                        desc=(m.group(3) or "").strip(),
                    )
                )
        return entries


class SocketShell:
    """TCP socket transport for programmatic control of a remote awemgr_shell.

    Connects to a host running ``awemgr_shell_run_socket()`` (e.g.
    ``awemgr_service -shell_socket <port>``) and exposes the same interface
    as :class:`AweMgrShell` so that :mod:`awe_parser` call-sites are
    transport-agnostic.

    Responses are framed by the ``awemgr-shell > `` prompt that the shell
    emits after every command.  The prompt is consumed internally and never
    appears in the return value of :meth:`exe_cmd`.
    """

    def __init__(self, host: str = "localhost", port: int = 7100,
                 timeout: float = 5.0) -> None:
        self.host = host
        self.port = port
        self.timeout = timeout
        self._sock: Optional[socket.socket] = None
        self._buf: str = ""

    def __enter__(self) -> "SocketShell":
        logger.debug("Connecting to %s:%d", self.host, self.port)
        self._sock = socket.create_connection((self.host, self.port),
                                              timeout=self.timeout)
        self._buf = ""
        # Consume the initial prompt sent by the server on connect
        self._recv_until_prompt()
        return self

    def __exit__(self, exc_type, exc_value, exc_tb) -> bool:
        if self._sock:
            self._sock.close()
            self._sock = None
        return False

    def exe_cmd(self, cmd: str) -> str:
        """Send *cmd* and return its response (prompt and log lines stripped)."""
        logger.debug("send: %r", cmd)
        self._sock.sendall(f"{cmd}\n".encode("utf-8"))
        raw = self._recv_until_prompt()
        result = _strip_log_lines(raw)
        logger.debug("recv: %r", result)
        return result

    def get_dir(self) -> List[DirEntry]:
        """Return the entries in the current idbg directory."""
        output = self.exe_cmd("ls")
        entries: List[DirEntry] = []
        pattern = re.compile(r"^\s+(\S+)\s+<(\w+)>(?:\s+-\s+(.*))?$")
        for line in output.splitlines():
            m = pattern.match(line)
            if m:
                entries.append(DirEntry(
                    cmd=m.group(1),
                    typ=m.group(2),
                    desc=(m.group(3) or "").strip(),
                ))
        return entries

    def _recv_until_prompt(self) -> str:
        """Receive data until ``_PROMPT`` appears; return content before it."""
        while _PROMPT not in self._buf:
            try:
                chunk = self._sock.recv(4096).decode("utf-8", errors="replace")
            except socket.timeout:
                logger.warning("Timed out waiting for shell prompt")
                break
            if not chunk:
                break
            self._buf += chunk
        idx = self._buf.find(_PROMPT)
        if idx >= 0:
            result = self._buf[:idx]
            self._buf = self._buf[idx + len(_PROMPT):]
        else:
            result = self._buf
            self._buf = ""
        return result


def _strip_log_lines(text: str) -> str:
    """Remove AWE-Manager log lines from *text*.

    Lines prefixed with ``[ERROR]``, ``[WARN ]``, ``[INFO ]``, ``[DEBUG]``,
    or ``[DUMP] `` are dropped; all other lines are kept unchanged.
    """
    clean = []
    for line in text.splitlines(keepends=True):
        stripped = line.lstrip("\r")
        if any(stripped.startswith(p) for p in _LOG_PREFIXES):
            logger.debug("log: %s", stripped.rstrip())
        else:
            clean.append(line)
    return "".join(clean)
