# List of TODOs

## From Discussions with QC

- API: get_nr_controls()/get_control_info(idx) - enumeration to be implemented
    - info struct to be defined (union, enum)
    - copy from AWC internal structure into this
    - CHECK AWC: access by index possible?
- API: first step: get_nr_designs()/get_design_info() for handling AWB and preset-AWBs
    - enumerate designs separately from CTLs
    - next step: decide: AWB-preset part of get_nr_controls() 

- API: write_control()/read_control() methods to be renamed with _by_name()
    - lookup by char* name possibly not used later by QC, rather than by idx

- API: new write_control()/read_control() work by CTL index


## General "Plans"

- code cleanup - remove dead code - maybe in conjunction with DOC update? (see below)
- go through TODOs in *c/*h files - derive more "tickets" or simply work on branches!

- main: add more fine granular error codes for AWE-Manager API

- awe_CMD: return structures instead of sclar values: #345 in awe_cmd.c 
- check bytes/nr_words again: is it consistent?

- AWCTooling: feature : value ranges, min/max, enums!
- AWC: value ranges, etc... if not already existing
- AWC: also include indication of variable is read-only or writable

- TEST: have DTMF detector integrated, read from ctrl module; create (Idbg shell?)script to run fully automated
- TEST: use abspath via CMake define to use data in src dir; avoid cmake copy into binary dir!

- IDbg/Shell: multi-line data input (QAL-168-IDBG-multiline)
