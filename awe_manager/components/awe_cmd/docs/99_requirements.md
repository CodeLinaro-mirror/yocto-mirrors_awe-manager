# AWE CMD: Requirements

These are the requirements specific to the CMD handling component.

**List of `dsn` and `req` specification items **:

```yaml
- id: dsn~AWEMGR.AWECMD.MemoryHandle~1
  needs: utest
  description: |
    The component shall work on a single handle which encapsulates the read and write
    pointers and arithmetics.

- id: dsn~AWEMGR.AWECMD.TargetInfo~1
  needs: utest
  description: |
    An API for generating the PFID_GetTargetInfo command is required.

- id: dsn~AWEMGR.AWECMD.TargetInfo_Extended~1
  needs: utest
  description: |
    An API for getting extended target information is required. This includes obtaining number of cores supported
    on the system.

- id: dsn~AWEMGR.AWECMD.FromStream~1
  needs: utest
  description: |
    Instead of creating a tuning command given parameters, it shall be possible
    to read data from a file.

- id: dsn~AWEMGR.AWECMD.Destroy~1
  needs: utest
  description: |
    Encapsulation of PFID_Destroy; to stop/destroy audio processing.

- id: dsn~AWEMGR.AWECMD.SetGetValueOrValues~1
  needs: utest
  description: |
    Component requires a central call to create the command for setting a value or several
    values. This encapsulates severeal PFIDs. The same applies for reading a value or values.

- id: dsn~AWEMGR.AWECMD.ModuleOperationState~1
  needs: utest
  description: |
    Component requires a method to handle (modify/obtain) an AWE module's operating state,
    i.e., if the module is in active, bypass, muted or other state.

- id: dsn~AWEMGR.AWECMD.ModuleClass~1
  needs: utest
  description: |
    Component requires a method query an AWE module's class information.

- id: dsn~AWEMGR.AWECMD.MemInfo~1
  needs: utest
  description: |
    Component requires methods to query the heaps of AWE Core.

- id: dsn~AWEMGR.AWECMD.CpuInfo~1
  needs: utest
  description: |
    Component requires methods to query the CPU load from AWE Core.

- id: dsn~AWEMGR.AWECMD.LayoutProfilingInfo~1
  needs: utest
  description: |
    Component requires methods to query more detailed information about each signal flow layout from AWE Core.

- id: dsn~AWEMGR.AWECMD.CentralCheckError~1
  needs: utest
  description: |
    Component should encapsulate "first level" error checking.

- id: dsn~AWEMGR.AWECMD.ResetState~1
  needs: utest
  description: |
    Component requires methods to reset the state of every module and clear layout masks.

- id: dsn~AWEMGR.AWECMD.InitAllocationFailure~1
  # needs: --- requires a failing allocator; verified by inspection
  description: |
    Initialization of a command handle shall report an error to the caller when any of
    its buffers cannot be allocated, and shall not leave the handle partly initialized.
    Buffers already allocated shall be released and their pointers cleared, so that a
    handle from a failed initialization holds no dangling pointer.


```
