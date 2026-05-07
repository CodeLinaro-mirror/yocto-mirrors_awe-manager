# Info For Subcanvas Files

Deliberately, the intermediate files from AWE-TC are not all stored.

To generate new target_files

- open AWJ files and export AWC into ./awc
- install AWE-TC in proper version
- run eg `awetc signalflow=./awetc-cfg-tl1`
- use content of `_stages/deploy` and update target file data

How to use on target:

- adb push awe_manager\tests\data\designs\subcanvas\target_files_qctgt\ /tmp
- awemgr_shell  -awc /tmp/target_files_qctgt/awc_index.txt
