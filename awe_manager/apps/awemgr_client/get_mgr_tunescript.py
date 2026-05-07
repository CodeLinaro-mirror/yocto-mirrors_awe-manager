"""

Tuning Script Generator - creates AWEMgr-Shell script for preset-AWB generation

Python script to read the values of all variables from AWE Core specified in an AWC file;
the script uses AWEMgr-Shell to enumerate all those variables,
then queries the values via an AWE tuning command; 
when querying the values it uses the `-show_tx` parameter of AWEMgr-Shell's 
read command to print out how a command would look like to transmit 
this value back to AWE Core. This information can also be stored
to create an AWEMgr-Shell script file. 

The generated AWEMgr-Shell shell script can be used in conjunction with 
AWEMgr-Shell's `comm-trace` command to create a preset-AWB.

## Example of Execution

This uses an AWEMgr-Shell script `prepare-asr-script.txt` which initializes AWE-Manager
and loads an AWC. This means it contains at least the following commands:

```
mgr_init -l 0x33333
load_awc -awc asr/awc_index.txt 
# optional: load_design -name Main
enum_controls
```

The command connects to a target / AWECore on a specific PC and starts
the executable `./awemgr_shell`. It then will print some info and 
will write a file `asr-vars.txt`.

```
python get_mgr_tunescript.py 
    --script prepare-asr-script.txt 
    --awe_socket "192.168.111.133@15092" 
    ./awemgr_shell 
    -o asr-vars.txt 
    -v
```

Then the final AWEMgr-Shell script is generated and AWE-Manager runs this script:
```
(echo "comm-trace -on -file tuned-asr.awb" && cat prepare-asr-script.txt asr-vars.txt)  > presetAWB-script.txt

rm presetAWB-script.txt  # note: AWEManager will append comm traces
./awemgr_shell -f presetAWB-script.txt
``` 
This will have created the preset AWB `tuned-asr.awb`.


"""
import argparse
import logging
from pathlib import Path
import re
from shellhandler import AweMgrShell


class TuneScriptGenerator:
    def __init__(self):
        self._top_level = 1
        self._dir_level = 2
        self._sec_level = 3

    def goto_controls(self, title: str):
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

    parser = argparse.ArgumentParser(description='Obtains variable values for tuning via AWEMgr-Shell')
    parser.add_argument("--verbose", "-v", help="Verbosity level", action="count", default=0)
    parser.add_argument("--script", help="AWEMgr-Shell script to execute before")
    parser.add_argument("--awe_socket", help="Socket communication to use.", default="172.18.16.1@15002")
    parser.add_argument("--output", "-o", help="Specifies script file to be written with TX commands")
    parser.add_argument("exefile", help="the AWEMgr-Shell executable")
    args = parser.parse_args()

    FORMAT = '%(name)s:%(levelname)7s: %(threadName)10s: %(asctime)-15s : %(message)s'
    levels = [logging.WARNING, logging.INFO, logging.DEBUG]
    level = levels[min(args.verbose, len(levels) - 1)]  # cap to last level index
    logging.basicConfig(format=FORMAT, level=level)

    logger = logging.getLogger(__name__)

    md = TuneScriptGenerator()

    exe_params = ["-i", "-f", args.script, "-awe_socket", args.awe_socket]
    with AweMgrShell(args.exefile, exe_params=exe_params) as shell:

        a = input("!!!! ENTER for starting... !!!")
        _dontcare = shell.exe_cmd(f"pwd")
        shell.exe_cmd(f"cd /")
        shell.exe_cmd(f"controls")
        entries = shell.get_dir()
        cmds = [e for e in entries if e.typ == "cmd"]

        print(f"Querying values of {len(cmds)} variables now ...")
        tx_cmds = []
        for l in cmds:
                cmd_w_h = f"{l.cmd} -show_tx"
                logger.info(cmd_w_h)
                cmd_response = shell.exe_cmd(cmd_w_h)
                for l in cmd_response.splitlines():
                    m = re.match(r'.*TX: (.*)', l)
                    if m:
                        tx_cmds.append(m.group(1))

        if not args.output:
              # this should print all "setting value" calls: XYZ -values "..."
            print("Those are the TX commands to put into a file:")
            print("\n".join(tx_cmds))
        else:
            print(f"Writing TX commands to script file: {args.output}")
            with open(args.output, "w") as f:
                f.write(f"# Auto generated with {Path(__file__).name}\n")
                f.write("# Prefix this content with appropriate commands to enumerate and select controls.\n")
                f.write("# Use with 'comm-trace -file xyz.awb' to execute and generate AWB preset file.\n")
                f.write("#\n")
                f.write("\n".join(tx_cmds))

