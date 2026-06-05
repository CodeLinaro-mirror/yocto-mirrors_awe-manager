#!/usr/bin/env python3
"""
awemgr_client.py - Interactive socket client for awemgr_service with tab completion.

Connects to an ``awemgr_service`` instance started with ``-shell_socket <port>``,
provides a readline-style prompt with tab completion built from the server's own
``ls`` and ``<cmd> -h`` output, and forwards every line to the server.

Usage::

    python3 awemgr_client.py [-H HOST] [-p PORT]

Requirements::

    pip install prompt_toolkit

Completion is four-level, all fetched lazily from the server and cached:

1. Root commands  — queried via ``ls``
2. Sub-directory entries — queried via ``ls <dir>``
3. Command parameters — queried via ``<cmd> -h`` (or ``<dir> <cmd> -h``)
4. Flag values — queried via a provider command (e.g. ``show -controls`` for ``-var``)

Response framing relies on the server prompt ``"awemgr-shell > "`` as a
delimiter, which is more reliable than a fixed timeout.
"""

import argparse
import logging
import re
import socket
import sys
import yaml
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Optional

try:
    from prompt_toolkit import print_formatted_text, PromptSession
    from prompt_toolkit.completion import CompleteEvent, Completer, Completion
    from prompt_toolkit.document import Document
    from prompt_toolkit.formatted_text import FormattedText
    from prompt_toolkit.history import FileHistory
    from prompt_toolkit.styles import Style
except ImportError:
    sys.exit(
        "prompt_toolkit is required.  Install it with:  pip install prompt_toolkit"
    )

logger = logging.getLogger(__name__)

# The server emits this string after every response.
_SERVER_PROMPT = "awemgr-shell > "

# History file stored in the current directory.
_HISTORY_FILE = Path.cwd() / ".awemgrshell.history"

# ---------------------------------------------------------------------------
# Colour scheme
# ---------------------------------------------------------------------------

_STYLE = Style.from_dict({
    # Prompt
    "prompt.name":      "ansicyan bold",
    "prompt.arrow":     "ansiwhite",
    # Completion menu
    "completion-menu.completion":                "bg:#2d2d2d #d4d4d4",
    "completion-menu.completion.current":        "bg:#0078d4 #ffffff bold",
    "completion-menu.meta.completion":           "bg:#2d2d2d #888888",
    "completion-menu.meta.completion.current":   "bg:#0078d4 #cccccc",
})

_PROMPT_TOKENS = FormattedText([
    ("class:prompt.name",  "awemgr-shell"),
    ("class:prompt.arrow", " > "),
])


def _format_dir_line(line: str) -> Optional[List[tuple]]:
    """Return coloured segments for an idbg ``ls`` output line, or None if not a match.

    Example input:
        "    comm                 <cmd>   - Configuration of communication to AWEcore"
    """
    nl = "\n" if line.endswith("\n") else ""
    body = line.rstrip("\n")
    m = _LS_LINE.match(body)
    if not m:
        return None

    cmd_name, typ, desc = m.group(1), m.group(2), m.group(3) or ""
    indent_end = len(body) - len(body.lstrip())
    cmd_end    = indent_end + len(cmd_name)
    type_tag   = f"<{typ}>"
    type_start = body.index(type_tag, cmd_end)
    type_end   = type_start + len(type_tag)
    type_style = "ansiblue bold" if typ in ("dir", "sub_dir") else "ansigreen"

    parts: List[tuple] = [
        ("",            body[:indent_end]),   # leading whitespace
        ("ansicyan",    cmd_name),             # command / entry name
        ("",            body[cmd_end:type_start]),  # column padding
        (type_style,    type_tag),             # <cmd> / <dir>
    ]

    if desc:
        sep_start = body.index(" - ", type_end)
        parts.extend([
            ("ansibrightblack", body[type_end : sep_start + 3]),  # " - "
            ("",                desc + nl),
        ])
    else:
        parts.append(("", body[type_end:] + nl))

    return parts


def _colored_response(text: str) -> FormattedText:
    """Apply colour to server response lines based on content."""
    parts = []
    for line in text.splitlines(keepends=True):
        dir_parts = _format_dir_line(line)
        if dir_parts:
            parts.extend(dir_parts)
            continue
        low = line.lstrip().lower()
        if low.startswith("error") or "[error]" in low:
            style = "ansired bold"
        elif low.startswith("warn") or "[warn" in low:
            style = "ansiyellow"
        else:
            style = ""
        parts.append((style, line))
    return FormattedText(parts)

# Maps (root_command, flag) -> server command whose YAML output supplies candidate values.
# Extend this dict to add value completion for more flags.
_VALUE_PROVIDERS: Dict[tuple, str] = {
    ("get_value",   "-var"):  "show -controls",
    ("set_value",   "-var"):  "show -controls",
    ("load_design", "-name"): "show -designs",
    ("module", "-name"): "show -modules",
    ("event", "-enable"): "show -events",
    ("event", "-disable"): "show -events",
}

# Matches one idbg ``ls`` output line, e.g.:
#   "    version   <cmd>  - Prints AWE Manager version"
#   "    controls  <dir>  - Filled with commands from AWC by 'enum_controls'"
_LS_LINE = re.compile(r"^\s+(\S+)\s+<(\w+)>(?:\s+-\s+(.*))?$")

# Matches a parameter flag in ``<cmd> -h`` output, e.g.:
#   "  -designs              Show all designs"
#   "  [-modules]            Show all modules"
#   "  -awc <awcfile>        Load AWC configuration file"
#   "  -values <values>      Space-separated list of values ..."
#
# The arg-spec column (everything between the flag and the description) is
# consumed lazily with ``.*?`` so that any quoting or formatting style is
# handled without a strict pattern.  The description column is separated by
# two or more spaces (the idbg CMDUSAGE table always pads to a fixed width).
_HELP_LINE = re.compile(
    r"^\s+"
    r"\[?"                           # optional opening bracket
    r"(-[a-zA-Z_][\w-]*)"           # the flag itself, e.g. -designs
    r"\]?"                           # optional closing bracket
    r".*?"                           # arg-spec (lazy): <arg>, 'val ...', etc.
    r"(?:\s{2,}|\s*$)"              # separator: ≥2 spaces or end-of-line
    r"(.*?)\s*$"                     # description (may be empty)
)


@dataclass
class DirEntry:
    cmd: str
    typ: str   # "cmd" | "dir" | "sub_dir" | ...
    desc: str = ""


@dataclass
class HelpParam:
    flag: str  # e.g. "-designs"
    desc: str = ""


def _parse_help(text: str) -> List[HelpParam]:
    """Extract ``-flag`` entries from a command's ``-h`` output."""
    params: List[HelpParam] = []
    seen: set = set()
    for line in text.splitlines():
        m = _HELP_LINE.match(line)
        if m:
            flag = m.group(1)
            if flag not in seen:
                seen.add(flag)
                params.append(HelpParam(flag=flag, desc=(m.group(2) or "").strip()))
    return params


# ---------------------------------------------------------------------------
# Socket transport
# ---------------------------------------------------------------------------

class ShellSocket:
    """Thin wrapper around a TCP connection to awemgr_service."""

    def __init__(self, host: str, port: int) -> None:
        self._sock = socket.create_connection((host, port), timeout=10)
        self._sock.settimeout(None)  # switch to blocking after connect
        self._buf = ""
        # Consume the initial banner / prompt the server sends on connect.
        self._read_until_prompt()

    def send(self, cmd: str) -> str:
        """Send *cmd* and return everything the server writes before its next prompt."""
        logger.debug("send: %r", cmd)
        self._sock.sendall((cmd + "\n").encode())
        response = self._read_until_prompt()
        logger.debug("recv: %r", response)
        return response

    def _read_until_prompt(self) -> str:
        """Read from the socket until the server prompt is found."""
        while _SERVER_PROMPT not in self._buf:
            chunk = self._sock.recv(4096).decode(errors="replace")
            if not chunk:
                raise ConnectionError("Server closed the connection")
            self._buf += chunk
        idx = self._buf.index(_SERVER_PROMPT)
        response, self._buf = self._buf[:idx], self._buf[idx + len(_SERVER_PROMPT):]
        return response

    def close(self) -> None:
        try:
            self._sock.close()
        except OSError:
            pass

    def get_dir(self, path: Optional[str] = None) -> List[DirEntry]:
        """Fetch and parse ``ls [path]`` output into :class:`DirEntry` objects."""
        cmd = f"ls {path}" if path else "ls"
        output = self.send(cmd)
        entries: List[DirEntry] = []
        for line in output.splitlines():
            m = _LS_LINE.match(line)
            if m:
                entries.append(DirEntry(
                    cmd=m.group(1),
                    typ=m.group(2),
                    desc=(m.group(3) or "").strip(),
                ))
        return entries


# ---------------------------------------------------------------------------
# Tab completion
# ---------------------------------------------------------------------------

class ShellCompleter(Completer):
    """
    prompt_toolkit Completer with four-level lazy completion:

    1. Root command names  (via ``ls``)
    2. Sub-directory entry names  (via ``ls <dir>``)
    3. Command parameters  (via ``<cmd> -h`` or ``<dir> <subcmd> -h``)
    4. Flag values  (via ``_VALUE_PROVIDERS`` mapping, e.g. control/design names)

    All results are cached for the session lifetime.
    """

    def __init__(self, shell: ShellSocket) -> None:
        self._shell = shell
        self._dir_cache: Dict[Optional[str], List[DirEntry]] = {}
        self._help_cache: Dict[str, List[HelpParam]] = {}
        self._value_cache: Dict[str, List[str]] = {}

    def _entries(self, path: Optional[str]) -> List[DirEntry]:
        if path not in self._dir_cache:
            try:
                self._dir_cache[path] = self._shell.get_dir(path)
            except Exception as exc:
                logger.warning("ls %s failed: %s", path, exc)
                self._dir_cache[path] = []
        return self._dir_cache[path]

    def _values(self, provider_cmd: str) -> List[str]:
        """Fetch and cache candidate value names by running *provider_cmd* on the server."""
        if provider_cmd not in self._value_cache:
            try:
                resp = self._shell.send(provider_cmd)
                data = yaml.safe_load(resp)
                if isinstance(data, list):
                    names = [item["name"] for item in data if isinstance(item, dict) and "name" in item]
                elif isinstance(data, dict):
                    # Error responses wrap the list under a key (e.g. "controls").
                    inner = next((v for v in data.values() if isinstance(v, list)), [])
                    names = [item["name"] for item in inner if isinstance(item, dict) and "name" in item]
                else:
                    names = []
                self._value_cache[provider_cmd] = names
            except Exception as exc:
                logger.warning("value fetch %s failed: %s", provider_cmd, exc)
                self._value_cache[provider_cmd] = []
        return self._value_cache[provider_cmd]

    def _params(self, cmd_path: str) -> List[HelpParam]:
        """Return parameters for *cmd_path* by querying ``<cmd_path> -h``."""
        if cmd_path not in self._help_cache:
            try:
                resp = self._shell.send(f"{cmd_path} -h")
                self._help_cache[cmd_path] = _parse_help(resp)
            except Exception as exc:
                logger.warning("help %s failed: %s", cmd_path, exc)
                self._help_cache[cmd_path] = []
        return self._help_cache[cmd_path]

    def get_completions(
        self, document: Document, complete_event: CompleteEvent
    ) -> Iterable[Completion]:
        text = document.text_before_cursor.lstrip("/")
        parts = text.split()
        trailing_space = text.endswith(" ")

        root_entries = self._entries(None)
        root_by_name = {e.cmd: e for e in root_entries}

        # ---- Level 1: completing the first token ----------------------------
        if not parts or (len(parts) == 1 and not trailing_space):
            prefix = parts[0] if parts else ""
            for e in root_entries:
                if e.cmd.startswith(prefix):
                    yield Completion(e.cmd, start_position=-len(prefix), display_meta=e.desc)
            return

        first = parts[0]
        first_entry = root_by_name.get(first)
        first_type = first_entry.typ if first_entry else "cmd"

        if first_type in ("dir", "sub_dir"):
            sub_entries = self._entries(first)
            sub_by_name = {e.cmd: e for e in sub_entries}

            # ---- Level 2: completing a sub-command name ---------------------
            if len(parts) == 1 or (len(parts) == 2 and not trailing_space):
                prefix = parts[1] if len(parts) == 2 else ""
                for e in sub_entries:
                    if e.cmd.startswith(prefix):
                        yield Completion(e.cmd, start_position=-len(prefix), display_meta=e.desc)
                return

            # ---- Level 3: completing params of a sub-command ---------------
            subcmd = parts[1]
            subcmd_entry = sub_by_name.get(subcmd)
            if subcmd_entry and subcmd_entry.typ == "cmd":
                current = "" if trailing_space else parts[-1]
                if not current or current.startswith("-"):
                    for p in self._params(f"{first} {subcmd}"):
                        if p.flag.startswith(current):
                            yield Completion(p.flag, start_position=-len(current), display_meta=p.desc)

        else:
            # ---- Level 3 / 4: params or values of a root-level command ------
            current = "" if trailing_space else parts[-1]

            # Identify the flag immediately before the current word (if any).
            preceding_flag = None
            if trailing_space and parts and parts[-1].startswith("-"):
                preceding_flag = parts[-1]
            elif not trailing_space and len(parts) >= 2 and parts[-2].startswith("-"):
                preceding_flag = parts[-2]

            provider = _VALUE_PROVIDERS.get((first, preceding_flag)) if preceding_flag else None

            if provider and not current.startswith("-"):
                # Level 4: value completion (e.g. control names, design names)
                for name in self._values(provider):
                    if name.startswith(current):
                        yield Completion(name, start_position=-len(current))
            elif not current or current.startswith("-"):
                # Level 3: flag completion
                for p in self._params(first):
                    if p.flag.startswith(current):
                        yield Completion(p.flag, start_position=-len(current), display_meta=p.desc)


# ---------------------------------------------------------------------------
# Main REPL loop
# ---------------------------------------------------------------------------

def run(host: str, port: int) -> int:
    print(f"Connecting to {host}:{port} …", flush=True)
    try:
        shell = ShellSocket(host, port)
    except (ConnectionRefusedError, OSError) as exc:
        print(f"Connection failed: {exc}", file=sys.stderr)
        return 1

    print(f"Connected.  Type 'help' for commands, Ctrl-D or 'quit' to exit.\n")

    completer = ShellCompleter(shell)
    session: PromptSession = PromptSession(
        history=FileHistory(str(_HISTORY_FILE)),
        completer=completer,
        complete_while_typing=False,
        style=_STYLE,
    )

    try:
        while True:
            try:
                line = session.prompt(_PROMPT_TOKENS)
            except KeyboardInterrupt:
                continue
            except EOFError:
                break

            line = line.strip()
            if not line:
                continue
            if line.lower() in ("quit", "exit"):
                break

            try:
                response = shell.send(line)
            except ConnectionError as exc:
                print_formatted_text(FormattedText([("ansired bold", f"Connection lost: {exc}")]))
                return 1

            if response:
                print_formatted_text(_colored_response(response))
    finally:
        shell.close()

    return 0


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Interactive client for awemgr_service -shell_socket"
    )
    parser.add_argument(
        "-H", "--host", default="localhost", help="Server hostname (default: localhost)"
    )
    parser.add_argument(
        "-p", "--port", type=int, default=15100, help="Server port (default: 15100)"
    )
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable debug logging")
    args = parser.parse_args()

    if args.verbose:
        logging.basicConfig(level=logging.DEBUG, format="%(levelname)s %(message)s")

    sys.exit(run(args.host, args.port))


if __name__ == "__main__":
    main()
