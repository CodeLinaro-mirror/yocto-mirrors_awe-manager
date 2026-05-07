# AWE COMM: Requirements

These are the requirements specific to the communication handling component.

**List of `dsn` and `req` specification items **:

```yaml
- id: dsn~AWEMGR.ControlComm.Initialize~1
  needs: itest
  description: |
    The module needs an initialization routine which has to be called on "both sides"
    prior to exchange (control) information.

- id: dsn~AWEMGR.ControlComm.Role~1
  # needs: itest
  description: |
    The module can be used in different "roles", e.g., on a "server" side (ARM)
    or on a "client" side (aDSP).
    ** OBSOLETE. Item to be removed. **

- id: dsn~AWEMGR.ControlComm.SelectChannel~1
  needs: itest
  description: |
    It must be possible to select a specific "communication channel" for further
    write and read calls.
    Selecting a channel allows to use this API from several different components.
    The thread-safety must be guaranteed by the communication channel though,
    e.g. distinct areas in shared memory.

- id: dsn~AWEMGR.ControlComm.WriteData~1
  needs: itest
  description: |
    There must be a way that allows writing into the "communication" channel,
    i.e., for example into the shared memory.

- id: dsn~AWEMGR.ControlComm.NotifyWritten~1
  # needs: itest
  description: |
    A method must exist (internally) which informs the "other side" that data has been written
    to the communication channel (i.e. shared memory).
    ** OBSOLETE. Item to be removed. **

- id: dsn~AWEMGR.ControlComm.ReadData~1
  needs: itest
  description: |
    A function needs to exist to extract data from the "other side", i.e., to read
    data from the communication channel (shared memory). Reading operation is blocking.

- id: dsn~AWEMGR.ControlComm.Abstraction~1
  # needs: utest
  description: |
    This non-functional requirement states that no communication channel specific information shall
    be required above the API level of the component. The reasoning is that the underlying
    communication (i.e. shared memory) shall be substituted with other ways to exchange the data.
    This is to allow usage of this component on other testing platforms.
    Needs code review! :)

- id: dsn~AWEMGR.ControlComm.Protection~1
  needs: itest
  description: |
    It shall be possible that multiple threads access the transmission buffer without collision.

- id: dsn~AWEMGR.ControlComm.TimeOut~1
  needs: itest
  description: |
    It must be guaranteed that the communication calls return when other side is unresponsive.

- id: dsn~AWEMGR.ControlComm.Tracing~1
  needs: itest
  description: |
    The component must yield methods to provide a dump/trace of the data handled on the communication.


```


