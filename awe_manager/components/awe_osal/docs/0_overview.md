# AWE OSAL: Overview

This is a "helper" component, used by all other software (components) in {{name.awe_mgr}}.

It abstracts platform specific function calls and provides a common platform agnostic API for other components to use, e.g.:

- sockets
- threads
- mutexes
- timeing related functions
- string handling functions

Currently, the following platforms are supported:

- Linux/Posix system
- Windows
