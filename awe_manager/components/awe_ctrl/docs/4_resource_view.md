# AWE CTRL: Resource View

awe_CTRL does not contain any resources that need to be stored in persistent memory.

It is re-initialized after every boot cycle.

## Protocols

### Tuning Commands

The protocol implemented between awe_CTRL, i.e. between {{name.awe_mgr}} and {{name.awe_host}} uses the AWE tuning command syntax, see [public documentation](https://w.dspconcepts.com/hubfs/Docs-AWECoreOS/AWECoreOS_UserGuide/a00075.html).

### Event Data

Data exchanged for events (see [call sequence](3_behavioral_view.md#event-data)) contains the event payload itself as well as information about the event itself.

The following data items are used:

  - _instanceId_ **--** which AWE Core instance the event module is running on
  - _objectId_ **--** event module objectId
  - _classId_ **--** event module classid, 0 or don't care for system events (eventCategory != 0)
  - _eventType_ **--** type as defined in event module; can be any arbitrary number defined by system integrators
  - _eventCategory_ **--** ID to classify the event source; typically, **0** means the event comes from an event module, **1** indicates a "system event", i.e., the BSP has indicated a state change
  - _timeStamp_ **--** systick or CPU clock, depends on the target platform; it indicates the time when the BSP has either received an event from a module, or when it has triggered an event on its own
  - _dataSize_ **--** size of payload data in number of bytes
  - _payload_ **--** the data of this event

The detailed structure (C-struct) of such a package depends on the chosen backend. The reason why the structure is a bit different is that, for example for the **Socket** backend, a magic identifier is prefixed, allowing clients reading from this socket to understand that this is conveying event data.

Also, for the **ShmMemory** backend the _payload_ member is merely a pointer to memory to provide a copy-free operation and transmission of the payload data.
