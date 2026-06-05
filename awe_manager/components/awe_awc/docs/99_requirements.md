# AWE AWC: Requirements

These are the requirements specific to the AWC handling component.

**List of `dsn` and `req` specification items **:

```yaml
- id: dsn~AWEMGR.AWC.NameToAweData~1
  needs: utest
  description: |
    The module shall convert a textual description into AWE module variable handle/size information.

- id: dsn~AWEMGR.AWC.AliasToAweData~1
  needs: utest
  description: |
    It shall be possible to access the modules or controls by an optional Alias.

- id: dsn~AWEMGR.AWC.AweDataByIndex~1
  needs: utest
  description: |
    The module shall also allow to "enumerate" and get address to AWE module or variable handle/size
    information by using an index value.

- id: dsn~AWEMGR.AWC.AwbDataInfo~1
  needs: utest
  description: |
    It shall be possible to address all AWB data in the system via an API, i.e., main AWB 
    and preset AWB data. It shall also be possible to query/enumerate the AWB data stored.

- id: dsn~AWEMGR.AWC.CheckConsistency~1
  needs: utest
  description: |
    It shall be possible to check the consistency of AWC data content. Errors shall be reported.

- id: dsn~AWEMGR.AWC.Compatibility~1
  needs: utest
  description: |
    It shall be possible to check if the AWC content is compatible with the platform software,
    e.g. in terms of available AWE modules or schema/syntax.

- id: dsn~AWEMGR.AWC.VersionInfo~1
  needs: utest
  description: |
    All version information embedded into AWC can be returned.

- id: dsn~AWEMGR.AWC.Efficiency~1
  # needs: utest
  description: |
    The lookup of strings (module names) shall be efficient and not require repeated lookup in "file".

- id: dsn~AWEMGR.AWC.Events~1
  needs: utest
  description: |
    The module shall handle EventModules specifically. To avoid long searching for those modules they
    shall be treated in a separate list so that their type can be stored as well (eventType).

- id: dsn~AWEMGR.AWC.OwnVarTypes~1
  needs: utest
  description: |
    When storing information about the type of a variable, the module uses an own data type
    and data representation.

- id: dsn~AWEMGR.AWC.UserData~1
  needs: utest
  description: |
    Application specific data for Modules and Controls can be queried via awe_awc public API's.

- id: dsn~AWEMGR.AWC.TopData~1
  needs: utest
  description: |
    Application specific Toplevel data can be queried via awe_awc public API's.
```

In the future, also these items could be activated:

```
- id: dsn~FUTURE.AWEMGR.AWC.EnumeratePcmSourceModules~1
  needs: utest
  description: |
    It shall be possible to obtain a list of AWE HLOS Audio Data source modules ("AlsaSource").

- id: dsn~FUTURE.AWEMGR.AWC.EnumeratePcmSinkModules~1
  needs: utest
  description: |
    It shall be possible to obtain a list of AWE HLOS Audio Data sink modules ("AlsaSink").

```

