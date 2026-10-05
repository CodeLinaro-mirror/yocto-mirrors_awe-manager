## High Level Requirements

This page lists specification objects as YML formatted objects. Tooling exists to extract the list of those items from this documentation as well as from other sources (e.g. the test code) to form the requirement traceability report. This report helps identifying potential gaps in specification or testing. See [Traceability Introduction][traceability-introduction] for more information.

The list here is separated into a "high level" or customer originated list (feature), and into an internally (broken down) list of requirements or design decisions.

**List of `feat` specification items **:

Belows section lists "high level requirements" extracted from JIRA and/or from Feature Parity Doc.

- [CUSTPR-779](https://dspconcepts.atlassian.net/browse/CUSTPR-779)


```yaml
- id: feat~AWEMGR.VOLUME~1
  needs: req
  description: |
    The user can control the volume via a named interface from an HLOS application.
    See [CUSTPR-779](https://dspconcepts.atlassian.net/browse/CUSTPR-779)

- id: feat~AWEMGR.PRESET~1
  needs: req
  description: |
    The user can apply a tuning preset file to change the parameters in a module
    (e.g. biquad sparse module can demostrate changing from a HP filter to a LP filer with
    pre-calculated coefficients.)
    See [CUSTPR-779](https://dspconcepts.atlassian.net/browse/CUSTPR-779)

- id: feat~AWEMGR.AUDIO_PROPERTIES~1
  needs: req
  description: |
    The user can control an arbitrary module via the control interface
    (e.g. bass/mid/treble/balance/fade can all be demonstrated)
    See [CUSTPR-779](https://dspconcepts.atlassian.net/browse/CUSTPR-779)

```


## Requirements

Here the derived items (`req` - user requirement and/or `dsn` - design requirement) are mentioned.
Those have been added on top of the (customer) `feat` items above.

!!! important
    There are further requirement sections for each of the sub-components.

      - [AWE AWC: Requirements][awe-awc-requirements]
      - [AWE CMD: Requirements][awe-cmd-requirements]
      - [AWE COMM: Requirements][awe-comm-requirements]
      - [AWE CONFIG: Requirements][awe-config-requirements]
      - [AWE OSAL: Requirements][awe-osal-requirements]

    As mentioned above, the requirements on this page as well as all of these sub-component requirements,
    together with the test cases and other "specification objects" are retrieved from code to generate the traceability report.
    This is not part of this document though.

**List of `dsn` and `req` specification items **:

```yaml
- id: req~AWEMGR.Raw_Access~1
  needs: itest
  description: |
    An application shall be able to use correctly formed AWE tuning messages. AWE-Manager
    is supposed to "simply" pass on this command message to the Audio Processors.
    It's the application's responsibility to construct the AWE tuning message and to handle
    all possible return values or errors.

- id: req~AWEMGR.Named_Access~1
  needs: itest
  covers:
    - feat~AWEMGR.AUDIO_PROPERTIES~1
    - feat~AWEMGR.VOLUME~1
  description: |
    It shall be possible to select an AWE controllable module or variable by name. Data can then be
    written to or read from this item.

- id: req~AWEMGR.ModuleAdministration~1
  needs: itest
  description: |
    An application shall have an AIP which allows to change or query the state of an AWE controllable
    module. This is a little bit more specific than writing/reading data.

- id: req~AWEMGR.AweCoreAdministration~1
  needs: itest
  description: |
    An application shall have the possibility to query information from AWE Core instances on the platform.
    AWE-Manager shall provide an API to return this information.

- id: req~AWEMGR.LoadDesign~1
  needs: itest
  description: |
    It shall be possible to instruct AWE-Manager to load a complete signal flow and start audio
    processing.

- id: req~AWEMGR.UnloadDesign~1
  needs: itest
  description: |
    It shall be possible to instruct AWE-Manager to destroy a complete signal flow and stop audio
    processing.

- id: req~AWEMGR.PluginInfo~1
  needs: itest
  description: |
    It shall be possible to iterate through the list of plugins in a design. Applications can use this information to load the plugins before loading the design or unload plugins after destroying the design.

- id: req~AWEMGR.Preset_Selection~1
  needs: itest
  covers: feat~AWEMGR.PRESET~1
  description: |
    Depending on the current use-case (or target configuration) an application may apply one
    or several preset configurations.

- id: req~AWEMGR.SharedMem~1
  # needs: itest
  description: |
    It shall be possible for an application to inform the AWE design about a shared memory location.
    This shared mem can be used by modules in the AWE design to exchange data directly with
    the application.
    ** OBSOLETE ** The shared memory mapping is done not via AWE Manager.

- id: req~AWEMGR.Event_Notification~1
  needs: itest
  description: |
    An application can be informed when events inside the AWE design happen.

- id: req~AWEMGR.ControlEnumeration~1
  needs: itest
  description: |
    An application can query the controllable items from AWE-Manager. It is the application's task
    to further filter or use this list of items.

- id: req~AWEMGR.MultipleDesignSupport~1
  needs: itest
  description: |
    AWE-Manager shall be a central frontend to all AWE Core instances on the platform.
    When several of these instances exist (multi-canvas integration), AWE-Manager shall be
    able to route application requests to the appropriate AWE Core instance.

- id: req~AWEMGR.SoftwareComponent~1
  needs: itest
  description: |
    AWE-Manager, as a SW component being linked into an application, needs to handle (system) resources.
    It shall be possible for the application to initialized and release AWE-Manager.

- id: req~AWEMGR.DesignEnumeration~1
  needs: itest
  description: |
    It shall be possible to query AWE-Manager for the designs supported on the platform.
    The list of designs contains the "boot" or "main" AWB, as well as the preset AWBs.
    The design info must also contain the list of the plugins required to load this design.

- id: req~AWEMGR.DesignLookupByName~1
  needs: itest
  description: |
    Besides the index based enumeration, it shall be possible to query the information
    of a single design by its name, so that an application knowing the name does not
    have to iterate over all designs. The name shall be resolved as it is for loading a
    design, and a name that is not known in the AWC shall be reported as such, so that
    the call can be used to check whether a design exists.

- id: req~AWEMGR.Sleep~1
  needs: itest
  description: |
    It shall be possible to halt Audio Weaver Audio Processing via AWE-Manager in case the system enters sleep mode.

- id: req~AWEMGR.Resume~1
  needs: itest
  description: |
    It shall be possible to resume Audio Weaver Audio Processing via AWE-Manager in case the system exits sleep mode.

- id: req~AWEMGR.UserData~1
  needs: itest
  description: |
    It shall be possible to query the control and module user data with key or with index.

- id: req~AWEMGR.AWECoreErrors~1
  needs: itest
  description: |
    It shall be possible to query the AWECore error code and corresponding error string.

- id: req~AWEMGR.CommFailErrorCode~1
  needs: itest
  description: |
    A failed communication with AWECore (connection not available or lost, send or receive
    error) shall be reported with its own error code awemgr_RC_COMM_FAIL, distinct from a
    communication timeout, an AWECore error and the generic error code. A malformed response or
    malformed event data is not a failed communication and shall be reported with the generic
    error code. AWE-Manager shall not perform any recovery on such a failure; this is left to
    the client.

- id: req~AWEMGR.SupressionAWECoreReturnValues~1
  needs: itest
  description: |
    Certain return values of AWECore may require some special handling at clients, like for
    starting audio twice a code is returned to indicate that audio was already started.
    This may require special handling at the client if this shall be ignored and not count as error.
    It shall therefore be possible to suppress returning those values to the client application
    and return OK to the client.

- id: req~AWEMGR.DetachFromRunningTarget~1
  needs: itest
  description: |
    It must be possible to shut down AWE-Manager (library), i.e. call the exit function, without tearing
    down a running AWE core system. This is like a "detach from a running system" for Designer.

- id: req~AWEMGR.SendBSPCommand~1
  needs: itest
  description: |
    It shall be possible to use AWE-Manager API to send special commands to the software
    encapsulating AWECore processing ("BSP"). Those commands are targeted to that software
    rather than to AWECore itself. For example, such commands can be used to
    load a plugin into the BSP.

- id: req~AWEMGR.AWECoreInformationQuery~2
  needs: itest
  description: |
    It shall be possible to query information from AWE Core, like CPU, memory, modules and layout profiling info.

- id: req~AWEMGR.ApiArgumentValidation~1
  needs: itest
  description: |
    API functions taking an endpoint or core index shall reject an index outside the
    supported range (0 to MAX_AWE_ENDPOINTS-1 respectively 0 to MAX_AWE_CORES-1) and
    report an error to the caller. Both indices are encoded in the tuning message
    header, so an out of range index shall not be passed on to message construction.

- id: req~AWEMGR.SubcanvasLoadUnloadDesign~1
  needs: itest
  description: |
    It shall be possible to load/unload designs in to Subcanvas AWE Instance.

- id: req~AWEMGR.SubcanvasAccess~1
  needs: itest
  description: |
    It shall be possible to perform operations such control set/get, audio start, audio stop on a subcanvas AWE Instance.

- id: req~AWEMGR.Resetting~1
  needs: itest
  description: |
    It must be supported to reset AWE Core internal states.

- id: req~AWEMGR.ControlRangeCheck~1
  needs: itest
  description: |
    When an application uses values for named control (see `req~AWEMGR.Named_Access~1`), which are outside of
    the range information specified by Designer, no write access to AWE Core shall happen and an error
    shall be returned. The check applies to scalar values and arrays.

- id: req~AWEMGR.ControlRangeCheckArraysSetting~1
  needs: itest
  description: |
    It must be possible to turn off the range check information for arrays. The bypassing of the check
    may increase processing performance and reduce possible delay.

- id: req~AWEMGR.ConcurrentLoadingAndControl~1
  needs: itest
  description: |
    While AWE-Manager loads a design (Subcanvas or progressive load), it shall still be possible
    to use other API functions to control or query the system.

- id: req~AWEMGR.Logging~1
  needs: utest
  description: |
    AWE-Manager, as a SW component being linked into an application, shall support logging
    and tracing. This logging shall be configurable.


- id: req~AWEMGR.Addon.TuningSocket~1
  needs:
    - utest
    - itest
  description: |
    When built with AWEMGR_SERVICE_WITH_TUNING_SERVER AWE-Manager shall provide a component which
    can be used to send AWE tuning commands from external tuning tools, such as Designer on a PC.

```

