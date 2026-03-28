# AWE CTRL: Overview


The component provides an interface for the communication between {{name.awe_mgr}} and {{name.awe_host}}. This communication is bi-directional, and one side is supposed to be the "primary" (or "server") while the other participant is the "secondary" ("client") - with {{name.awe_host}} being the "server".

The communication is used for 2 purposes:

- sending tuning commands **--** this follows a strict request/response pattern, with {{name.awe_mgr}} sending the requests and waiting for an immediate response - see [here](3_behavioral_view.md#tuning-commands)
- receiving event data **--** this requires listening on {{name.awe_mgr}} side for any event to happen - see [here](3_behavioral_view.md#event-data)

That communication is implemented in so-called backends. The backends are encapsulated by awe_CTRL's API and no backend internal data (eg offsets, addresses in ShMem or socket port numbers, etc) is used in {{name.awe_mgr}} API directly (yet). Those configuration data is set via backend specific helper functions (mainly used during development).

Different communication backends are supported:

- **Socket** - direct socket connection, e.g. to an AWE-Server executable or to a Tuning-Relay 
- **ShmMemory** - communication via (Carve Shared) memory 

The backend to be used is chosen at compile time via CMake configuration by the software integrator.

!!! note
    Especially the socket backend makes {{name.awe_mgr}} supported on various OS platforms. Sockets are also used during development process on CM/CI systems.
    
    On QC target the ShmMemory is currently hidden behind other components (awe_packet_client/awe_packet_server)

