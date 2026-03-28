# README For Script Commands

This lists the most important script commands for the interactive
AWEMgr-Shell program.

The commands are implemented in corresponding functions in code, see `./src` folder.

## Command Overview

```
idbg> ls
 mgr_init                       <cmd>
 mgr_exit                       <cmd>
 load_awc                       <cmd>
 stop_awc                       <cmd>
 load_design                    <cmd>
 info                           <cmd>
 get_value                      <cmd>
 set_value                      <cmd>
 raw                            <cmd>
 comm                           <cmd>
 dtmf                           <dir>
```

## Initialization of AWE-Manager

```
idbg> mgr_init -h
Use: mgr_init -l <loglevel>
  l <loglevel>         - loglevel from 0-4
```

## Loading of AWC Content

With this step AWE-Manager knows about the design.
The signal flow might not yet run though.

```
idbg> load_awc -h
Use: load_awc -awc <fname>
  awc <fname>          - name of AWC to load
```

## Applying AWB Data

This loads the AWB which is referred to by <name> in the AWC file.

```
idbg> load_design -h
Use: load_design -name <name>
  name <name>          - name as given in AWC index file
```

## Reading of Values

`<varname>` is given as e.g. "Scaler1/gain", just like in the AWC file.

```
idbg> get_value -h
Use: get_value -var <varname>
  -var <varname>       - var name as in AWC file
```

## Writing of Values

Note that the value type must be correctly specified, as the type cannot be determined
(yet) on command line. See AWC file entry for that variable to obtain if it is a `float`
value or not.

```
idbg> set_value -h
Use: set_value -var <varname> [-offset <offset>] -values '<val1> <val2> ...'
  -var <varname>       - var name as in AWC file
  -offset <offset>     - Offset into variable's array. default: 0
  -values '<val1> <val2> ...'  - space separated list of values (encoded in string)
```

## Communication Setup to AWE Core

Use e.g. `comm -trace` to enable/disable traces of the binary packages.

```
idbg> comm -h
Use: comm [-host <ip>] [-port <port>] [-trace]
  host <ip>            - host IP of AWE Server or target
  port <port>          - Port to connect to
  trace                - enable COMM traces
```
