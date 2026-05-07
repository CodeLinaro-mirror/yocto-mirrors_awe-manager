#
# Simple checker to parse readelf output for forbidden functions
# The script just tells you THAT a function was used. Not where!
# Use Ctrl-Shift-F or other means to search your code.
#
# Script works under Linux mainly, unless Windows provides `readelf` in PATH.
#
# Use: python check_forbidden_functions.py <path_to_A_or_SO_or_EXE>
#
# can be used like:
# - cmake --build --preset debug
# - find ./build/Linux/debug/awe_manager/ -name "*.a" -exec python ./ci/check_forbidden_functions.py {} \; -print
# - python ./ci/check_forbidden_functions.py ./build/Linux/debug/awe_manager/apps/awemgr_shell/awemgr_shell
#

import subprocess
import sys
from typing import NamedTuple


banned_functions = """Banned function
Use instead
Data type
strcpy
strlcpy
char
wstrcpy
wstrlcpy
wchar
wcscpy
wcslcpy
wchar_t
strncpy
strlcpy
char
wstrncpy
wstrlcpy
wchar
wcsncpy
wcslcpy
wchar_t
strcat
strlcat
char
wstrcat
wstrlcat
wchar
wcscat
wcslcat
wchar_t
strncat
strlcat
char
wstrncat
wstrlcat
wchar
wcsncat
wcslcat
wchar_t
sprintf
snprintf (Use scnprintf() for the Linux Kernel)
char
vsprintf
vsnprintf (Use vscnprintf() for the Linux Kernel)
char
wsprintf
wsnprintf
wchar
gets
fgets
char
scanf
fgets (+strtoul/strtol as needed for ints)
char
strtok
strtok_r
char
"""

def chunks(lst, n):
    """Yield successive n-sized chunks from lst."""
    for i in range(0, len(lst), n):
        yield lst[i:i + n]

class BadFct(NamedTuple):
    fct: str
    subst: str
    type: str


class BadFunctions:
    def __init__(self) -> None:
        self._bfs = []
        lines = banned_functions.splitlines()
        for c in chunks(lines, 3):
            bf = BadFct(*c)
            self._bfs.append(bf)

    def check(self, readelf_line: str):
        try:
            elf_fct  = readelf_line.split()[-1]
            for bf in self._bfs:
                if bf.fct == elf_fct or f"{bf.fct}@" in elf_fct:
                    print(f"!!!!!!!!!!!!!!!! {str(bf)[:69]:70} in {elf_fct}")
        except IndexError:
            pass


if __name__ == "__main__":

    try:
        (script, exe_path) = sys.argv
    except ValueError:
        print("Use: check_forbidden_functions.py <outputfile>")
        sys.exit(-1)

    bf = BadFunctions()

    out_b = subprocess.check_output(['readelf', '-sW', exe_path])
    out = out_b.decode('utf-8')
    for o in out.splitlines():
        bf.check(o)



