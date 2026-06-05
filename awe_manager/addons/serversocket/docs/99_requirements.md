# Tuning Server Addon: Requirements

```yaml
- id: dsn~AWEMGR.ADDON.TUNINGSERVER.PacketForwarding~1
  needs: itest
  description: |
    The addon shall accept AWE tuning packets over TCP, forward them to
    awemgr_transact(), and return the AWE Core response to the client.

- id: dsn~AWEMGR.ADDON.TUNINGSERVER.AWEWireFormat~1
  needs: itest
  description: |
    The addon shall use the AWE wire format for framing: the high 16 bits of
    the first 32-bit word encode the total packet length in words.

- id: dsn~AWEMGR.ADDON.TUNINGSERVER.Respawn~1
  needs: itest
  description: |
    After a client disconnects, the server shall be able to accept a new
    client connection without requiring a restart.

- id: dsn~AWEMGR.ADDON.TUNINGSERVER.CleanShutdown~1
  needs: itest
  description: |
    A SIGTERM or SIGINT received while waiting for a client connection shall
    cause the server to return cleanly so the calling service can shut down.

- id: dsn~AWEMGR.ADDON.TUNINGSERVER.OptionalBuild~1
  # needs: --- cannot be tested except possibly by cmake-logfile analysis
  description: |
    The addon shall be fully excludable from a build via a CMake option
    without affecting other components.

- id: dsn~AWEMGR.ADDON.TUNINGSERVER.PublicHeaderInstall~1
  # needs: --- install concern, verifiable by inspecting the install tree
  description: |
    The addon shall install its public API header (awemgr_tuning_server.h)
    into the package include directory.
```
