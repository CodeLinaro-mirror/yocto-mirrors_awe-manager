
import sys
from shellhandler import AweMgrShell


class MarkdownWriter:
    def __init__(self):
        self._top_level = 1
        self._dir_level = 2
        self._sec_level = 3

    def gen_page(self, title: str):
        print(f"{self._top_level * '#'} {title}")

    def gen_dir_section(self, dir: str, path: str, cmds: list, desc: str):
        print(f"{self._dir_level * '#'} Directory: {dir} ({path.replace('> ', '').strip()})")
        print()
        print(desc.replace('   - ', ''))
        print()
        if cmds:
            print("This directory yields the following commands")
        print()

    def gen_cmd_section(self, cmd: str, desc: str, help: str):
        print(f"{self._sec_level * '#'} Command: {cmd}")
        print()
        if desc:
            print(desc.replace('   - ', ''))
        print()
        if help:
            print("```")
            print(help)
            print("```")
        else:
            print("Command has no help text")
        print()


if __name__ == "__main__":
    exe_path = sys.argv[1]
    if not exe_path:
        print("Use: script.py <exefile> [> file.md]")
        sys.exit(-1)
    
    md = MarkdownWriter()
    md.gen_page("AWEManager Shell Commands")

    with AweMgrShell(exe_path) as shell:

        def recurse_dir(title, desc):
            entries = shell.get_dir()
            cmds = [e for e in entries if e.typ == "cmd"]
            dirs = [e for e in entries if e.typ == "dir"]

            path = shell.exe_cmd("pwd")
            md.gen_dir_section(title, path, cmds, desc)

            for l in cmds:
                cmd_w_h = f"{l.cmd} -h"
                c = shell.exe_cmd(cmd_w_h)
                md.gen_cmd_section(l.cmd, l.desc, c)

            for d in dirs:
                shell.exe_cmd(f"cd {d.cmd}")
                recurse_dir(d.cmd, d.desc)
                shell.exe_cmd("..")

        recurse_dir("TopLevel", "")
