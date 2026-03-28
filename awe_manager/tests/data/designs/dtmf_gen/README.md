# README DTMF Generator

This signal flow implements a DTMF (https://en.wikipedia.org/wiki/DTMF) generator and shows the following:

A tone can be generated and modulated by AWE Manager:

- type of tone (freq column, freq row)
- volume of the tone
- muting of audio output


## Usage with AWEMgr-Shell on Linux-PC 

When using AWEMgr-Shell on Linux (WSL assumed currently!), the program tries to connect to a running AWE-Server executable on the Windows host machine.

Therefore, start AWE-Server before:

  - e.g. `C:\DSP Concepts\AWE Designer 8.D.2.3 Standard\Bin\win32-vc142-rel\AWE_Server.exe`

Then, start the shell with the given DTMF AWC, and also directly start the signal flow (Main):

- `./bin/awemgr_shell -awc ./data/designs/dtmf_gen/target_files/awc_index.txt -awb Main`
- Alternatively, run script: `./bin/awemgr_shell -f data/designs/dtmf_gen/awemgrshell_script.txt`

Note: the above command assumes that the `awe_manager-<version>-Linux.tar.bz2` has been used/unpacked.

Activity on the AWE-Server window should be observed and a DTMF tone (key "1") should be audible.

The next steps are all commands in the interactive shell after the `idbg>` prompt:

- `info` - getting target info
- `cd dtmf` - change to the "commands" for the specific DTMF generator signal flow
- `toggle_mute` - set/unset the mute module
- `freq -col <freq> -row <freq>` - modify the DTMF tone by setting the frequencies, e.g. `freq -col 1336 -row 770` (key "5")
- `vol -gain <value>` - adjust the output volume, value in dB


## Future Ideas

The signal flow further contains a DTMF detector which indicates which tone 
was generated, i.e. if the frequencies were set correctly.

The signal flow enables selection of freq col/row by using AWB-presets,
i.e. a control value is written (with the key index as input: 0..9, A..D).
This returns a value for the AWB preset name. The AWB preset file contains
the freq values for the corresponding tone.

