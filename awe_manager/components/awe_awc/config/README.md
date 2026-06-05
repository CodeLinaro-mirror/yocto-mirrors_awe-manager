# AWC Schema Configuration

The AWC schema is defined as a template file (Jinja2). 

This template file is input to the AWC-Tooling which uses this file to generate an AWC index file.

The AWC-Tooling is part of another package/repo and currently consists of a set of Python scripts. Those scripts analyse AudioWeaver target files and generate the AWC index. 

The configuration directory here is included and used by AWC-Tooling CM/CI.

Basically, the generator configuration YML file has to be used with AWC-Tooling, like:

`awe-awcgen --gen_spec ./gencfg_awc/gen_cfg.yml ... `

The detailed usage of the `awe-awcgen` command is given in AWC-Tooling. 
