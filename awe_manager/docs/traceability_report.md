# Traceability Report

In total the project contains 212 items.



* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWC.AliasToAweData - version: 1

    It shall be possible to access the modules or controls by an optional Alias.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.AWC.ControlNameAndAlias-1
    * Covered by: utest-AWEMGR.AWC.ModuleNameAndAlias-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWC.AwbDataInfo - version: 1

    It shall be possible to address all AWB data in the system via an API, i.e., main AWB 
and preset AWB data. It shall also be possible to query/enumerate the AWB data stored.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.AWC.DesignAccessByName-1
    * Covered by: utest-AWEMGR.AWC.ForEachDesign-2

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWC.AweDataByIndex - version: 1

    The module shall also allow to "enumerate" and get address to AWE module or variable handle/size
information by using an index value.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.AWC.ControlAccessByIndex-2
    * Covered by: utest-AWEMGR.AWC.DesignAccessByIndex-2
    * Covered by: utest-AWEMGR.AWC.ForEachModuleAndControl-1
    * Covered by: utest-AWEMGR.AWC.ModuleAccessByIndex-2

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWC.CheckConsistency - version: 1

    It shall be possible to check the consistency of AWC data content. Errors shall be reported.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.AWC.ControlCountError-2
    * Covered by: utest-AWEMGR.AWC.DesignCountError-1
    * Covered by: utest-AWEMGR.AWC.InValidLines-1
    * Covered by: utest-AWEMGR.AWC.InvalidFile-1
    * Covered by: utest-AWEMGR.AWC.LineEndings-1
    * Covered by: utest-AWEMGR.AWC.MapCreateAndFree-1
    * Covered by: utest-AWEMGR.AWC.MapDelete-1
    * Covered by: utest-AWEMGR.AWC.MapDuplicateKeys-1
    * Covered by: utest-AWEMGR.AWC.MapHashConsistency-1
    * Covered by: utest-AWEMGR.AWC.MapInsert-1
    * Covered by: utest-AWEMGR.AWC.MapResize-1
    * Covered by: utest-AWEMGR.AWC.MapSearch-1
    * Covered by: utest-AWEMGR.AWC.ModuleCountError-1
    * Covered by: utest-AWEMGR.AWC.TestInvalidControlType-2
    * Covered by: utest-AWEMGR.AWC.ValidLines-1
    * Covered by: utest-AWEMGR.AWC.parseNumericToken-1
    * Covered by: utest-AWEMGR.AWC.parseStringToken-1
    * Covered by: utest-AWEMGR.AWC.parseType-1
    * Covered by: utest-AWEMGR.AWC.stringToNumber-1
    * Covered by: utest-AWEMGR.AWC.validateControlParsing-1
    * Covered by: utest-AWEMGR.AWC.validateModuleParsing-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWC.Compatibility - version: 1

    It shall be possible to check if the AWC content is compatible with the platform software,
e.g. in terms of available AWE modules or schema/syntax.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.AWC.NewSchemaUpgrade-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWC.Efficiency - version: 1

    The lookup of strings (module names) shall be efficient and not require repeated lookup in "file".



    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWC.Events - version: 1

    The module shall handle EventModules specifically. To avoid long searching for those modules they
shall be treated in a separate list so that their type can be stored as well (eventType).


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.AWC.TestEvents-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWC.NameToAweData - version: 1

    The module shall convert a textual description into AWE module variable handle/size information.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.AWC.ControlAccessByName-2
    * Covered by: utest-AWEMGR.AWC.ControlNameAndAlias-1
    * Covered by: utest-AWEMGR.AWC.DesignAccessByName-1
    * Covered by: utest-AWEMGR.AWC.ModuleAccessByName-1
    * Covered by: utest-AWEMGR.AWC.ModuleNameAndAlias-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWC.OwnVarTypes - version: 1

    When storing information about the type of a variable, the module uses an own data type
and data representation.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.AWC.CtlTypeName-2

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWC.TopData - version: 1

    Application specific Toplevel data can be queried via awe_awc public API's.

    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.AWC.TopDataAccess-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWC.UserData - version: 1

    Application specific data for Modules and Controls can be queried via awe_awc public API's.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.AWC.UserDataIndexAccess-1
    * Covered by: utest-AWEMGR.AWC.UserDataNamedAccess-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWC.VersionInfo - version: 1

    All version information embedded into AWC can be returned.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.AWC.TestInfo-2

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWECMD.CentralCheckError - version: 1

    Component should encapsulate "first level" error checking.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.Cmd.GetResponseFailBufSize-1
    * Covered by: utest-AWEMGR.Cmd.GetResponseFailErrorCode-1
    * Covered by: utest-AWEMGR.Cmd.GetResponse-1
    * Covered by: utest-AWEMGR.Cmd.ResponseHandlingCpuCores-1
    * Covered by: utest-AWEMGR.Cmd.ResponseHandling-2
    * Covered by: utest-AWEMGR.Cmd.ResponseTwiceError-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWECMD.CpuInfo - version: 1

    Component requires methods to query the CPU load from AWE Core.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.Cmd.CpuInfo-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWECMD.Destroy - version: 1

    Encapsulation of PFID_Destroy; to stop/destroy audio processing.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.Cmd.Destroy-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWECMD.FromStream - version: 1

    Instead of creating a tuning command given parameters, it shall be possible
to read data from a file.


    * Needs coverage by: ['utest']


    * Status: <span style='color:red'>ERROR</span>
    * Errors:
        * No coverage provided by requested type


* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWECMD.LayoutProfilingInfo - version: 1

    Component requires methods to query more detailed information about each signal flow layout from AWE Core.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.Cmd.LayoutProfilingInfo-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWECMD.MemInfo - version: 1

    Component requires methods to query the heaps of AWE Core.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.Cmd.MemInfo-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWECMD.MemoryHandle - version: 1

    The component shall work on a single handle which encapsulates the read and write
pointers and arithmetics.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.Cmd.CheckBufferSizes-1
    * Covered by: utest-AWEMGR.Cmd.Init_Exit-1
    * Covered by: utest-AWEMGR.Cmd.InvalidInputs-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWECMD.ModuleClass - version: 1

    Component requires a method query an AWE module's class information.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.Cmd.Classes-1
    * Covered by: utest-AWEMGR.Cmd.ModuleClass-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWECMD.ModuleOperationState - version: 1

    Component requires a method to handle (modify/obtain) an AWE module's operating state,
i.e., if the module is in active, bypass, muted or other state.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.Cmd.ModuleOperationState-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWECMD.ResetState - version: 1

    Component requires methods to reset the state of every module and clear layout masks.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.Cmd.ResetState-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWECMD.SetGetValueOrValues - version: 1

    Component requires a central call to create the command for setting a value or several
values. This encapsulates severeal PFIDs. The same applies for reading a value or values.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.Cmd.GetCmds-1
    * Covered by: utest-AWEMGR.Cmd.SetCmds-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWECMD.TargetInfo_Extended - version: 1

    An API for getting extended target information is required. This includes obtaining number of cores supported
on the system.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.Cmd.GetExtInfo-1
    * Covered by: utest-AWEMGR.Cmd.GetNrCores2-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWECMD.TargetInfo - version: 1

    An API for generating the PFID_GetTargetInfo command is required.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.Cmd.TargetInfo-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWECONFIG.Add - version: 1

    There must be an API to add "default" values.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.AWECONFIG.add_set_get-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWECONFIG.BatchAdd - version: 1

    There must be an API to set/configure multiple values at once.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.AWECONFIG.add_string_set-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWECONFIG.EnvAdd - version: 1

    There must be an API to add values via an environment variable.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.AWECONFIG.add_string_get-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWECONFIG.Errorhandling - version: 1

    Component should handle erros such as conversion errors, key not found, invalid handles


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.AWECONFIG.ErrorCases-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWECONFIG.Get - version: 1

    The Component shouw allow to get the value by the key,
either as string or apropriate type.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.AWECONFIG.add_string_get-1
    * Covered by: utest-AWEMGR.AWECONFIG.add_string_set-1
    * Covered by: utest-AWEMGR.AWECONFIG.bool_test-1
    * Covered by: utest-AWEMGR.AWECONFIG.float_test-1
    * Covered by: utest-AWEMGR.AWECONFIG.int_test-1
    * Covered by: utest-AWEMGR.AWECONFIG.uint_test-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWOSAL.Mutex - version: 1

    The component shall provide the api to create, lock, unlock and destroy a mutex in platform independent way.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.AWOSAL.Mutex.Basic-1
    * Covered by: utest-AWEMGR.AWOSAL.Mutex.Multithread-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.AWOSAL.Threads - version: 1

    The component shall provide the api to create, start, join and destroy a thread in platform independent way.


    * Needs coverage by: ['utest']


    * Covered by: utest-AWEMGR.AWOSAL.Threads.Basic-1
    * Covered by: utest-AWEMGR.AWOSAL.Threads.Multithread-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.ControlComm.Abstraction - version: 1

    This non-functional requirement states that no communication channel specific information shall
be required above the API level of the component. The reasoning is that the underlying
communication (i.e. shared memory) shall be substituted with other ways to exchange the data.
This is to allow usage of this component on other testing platforms.
Needs code review! :)



    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.ControlComm.Initialize - version: 1

    The module needs an initialization routine which has to be called on "both sides"
prior to exchange (control) information. 


    * Needs coverage by: ['itest']


    * Covered by: itest-AWEMGR.ControlComm.InitExit_Fail-1
    * Covered by: itest-AWEMGR.ControlComm.InitExit-1
    * Covered by: itest-AWEMGR.ControlComm.TuningBufferSize-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.ControlComm.NotifyWritten - version: 1

    A method must exist (internally) which informs the "other side" that data has been written
to the communication channel (i.e. shared memory).
** OBSOLETE. Item to be removed. **



    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.ControlComm.ReadData - version: 1

    A function needs to exist to extract data from the "other side", i.e., to read
data from the communication channel (shared memory). Reading operation is blocking.


    * Needs coverage by: ['itest']


    * Covered by: itest-AWEMGR.ControlComm.SendReceive-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.ControlComm.Role - version: 1

    The module can be used in different "roles", e.g., on a "server" side (ARM) 
or on a "client" side (aDSP). 
** OBSOLETE. Item to be removed. **



    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.ControlComm.SelectChannel - version: 1

    It must be possible to select a specific "communication channel" for further 
write and read calls. 
Selecting a channel allows to use this API from several different components.
The thread-safety must be guaranteed by the communication channel though,
e.g. distinct areas in shared memory.


    * Needs coverage by: ['itest']


    * Covered by: itest-AWEMGR.ControlComm.GetBuf_Fail-1
    * Covered by: itest-AWEMGR.ControlComm.TransAct_Fail-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#3CC639'>dsn</span> - AWEMGR.ControlComm.WriteData - version: 1

    There must be a way that allows writing into the "communication" channel,
i.e., for example into the shared memory. 


    * Needs coverage by: ['itest']


    * Covered by: itest-AWEMGR.ControlComm.SendReceive-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#6B3A9F'>feat</span> - AWEMGR.AUDIO_PROPERTIES - version: 1

    The user can control an arbitrary module via the control interface
(e.g. bass/mid/treble/balance/fade can all be demonstrated)
See [CUSTPR-779](https://dspconcepts.atlassian.net/browse/CUSTPR-779)


    * Needs coverage by: ['req']


    * Covered by: req-AWEMGR.Named_Access-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#6B3A9F'>feat</span> - AWEMGR.PRESET - version: 1

    The user can apply a tuning preset file to change the parameters in a module
(e.g. biquad sparse module can demostrate changing from a HP filter to a LP filer with
pre-calculated coefficients.)
See [CUSTPR-779](https://dspconcepts.atlassian.net/browse/CUSTPR-779)


    * Needs coverage by: ['req']


    * Covered by: req-AWEMGR.Preset_Selection-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#6B3A9F'>feat</span> - AWEMGR.VOLUME - version: 1

    The user can control the volume via a named interface from an HLOS application.
See [CUSTPR-779](https://dspconcepts.atlassian.net/browse/CUSTPR-779)


    * Needs coverage by: ['req']


    * Covered by: req-AWEMGR.Named_Access-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.AWC.UserData - version: 1

    Ensures the user data can be accessed by index


    * covers: req-AWEMGR.UserData-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.AWECoreErrors - version: 1

    When AWE Manager returns error code awemgr_RC_AWECORE_ERROR, API awemgr_get_awe_error can be used to get the structure containing error code and description string.


    * covers: req-AWEMGR.AWECoreErrors-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.AweCoreInfo.AweCoreLayoutProfiling_Fails - version: 1

    Checks incorrect parameter handling of method.


    * covers: req-AWEMGR.AWECoreInformationQuery-2

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.AweCoreInfo.Cpu - version: 1

    Checks that CPU load can be retrieved.


    * covers: req-AWEMGR.AWECoreInformationQuery-2

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.AweCoreInfo.Fails - version: 1

    Checks capture of incorrect command usages.


    * covers: req-AWEMGR.AWECoreInformationQuery-2

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.AweCoreInfo.LayoutProfiling - version: 1

    Checks that profiling info of layouts can be retrieved.


    * covers: req-AWEMGR.AWECoreInformationQuery-2

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.AweCoreInfo.Memory - version: 2

    Checks that memory info can be retrieved


    * covers: req-AWEMGR.AWECoreInformationQuery-2

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.AweCoreInfo.Modules - version: 1

    Checks that a module list can be retrieved.


    * covers: req-AWEMGR.AWECoreInformationQuery-2

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.CommTimeout - version: 1

    Checks that the communication timeout errors are propagated correctly


    * covers: req-AWEMGR.SoftwareComponent-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.ControlComm.GetBuf_Fail - version: 1

    Checks errors when trying to get a CMD buffer object.


    * covers: dsn-AWEMGR.ControlComm.SelectChannel-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.ControlComm.InitExit_Fail - version: 1

    Checks incorrect context handle parameters for init and exit methods.


    * covers: dsn-AWEMGR.ControlComm.Initialize-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.ControlComm.InitExit - version: 1

    Checks that component can be created and safely destructed.


    * covers: dsn-AWEMGR.ControlComm.Initialize-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.ControlComm.SendReceive - version: 1

    Transmits a get-target-info message to AWE Core/Server and retrieves the response.


    * covers: dsn-AWEMGR.ControlComm.ReadData-1
    * covers: dsn-AWEMGR.ControlComm.WriteData-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.ControlComm.TransAct_Fail - version: 1

    Checks errors when trying to specify incorrect transact parameters.


    * covers: dsn-AWEMGR.ControlComm.SelectChannel-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.ControlComm.TuningBufferSize - version: 1

    Checks that the default tuning buffer size can be modified via awe_config 


    * covers: dsn-AWEMGR.ControlComm.Initialize-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.ControlInfoByName - version: 1

    Checks if control info can also be retrieved by name.


    * covers: req-AWEMGR.ControlEnumeration-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Design_And_Preset_Load - version: 1

    Loads the SetGet AWB; then it reads the values of the integer SINK modules
and compares them to the know values (from the design);
a first presetAWB is applied which will change the values of the corresponding
integer SOURCE modules, and the SINK modules are queried again,
this time for a different set of values; finally, a presetAWB with original
values is restored.



    * covers: req-AWEMGR.LoadDesign-1
    * covers: req-AWEMGR.Preset_Selection-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.DetachFromSystemFail - version: 1

    Checks incorrect usage of API. Call with incorrect parameter.


    * covers: req-AWEMGR.DetachFromRunningTarget-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.DetachFromSystem - version: 1

    Ensures that AWE Manager detaches from a system without stopping the system.
This includes the steps: a) Init and Run system, b) Modify something on system,
c) Detach from system, d) Init AWE Manager only,
e) Check if modifications are still present.


    * covers: req-AWEMGR.DetachFromRunningTarget-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Enum.DesignEnumFail - version: 1

    Checks that incorrect design enumerate API calls return error.


    * covers: req-AWEMGR.DesignEnumeration-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Enum.Fail - version: 1

    Checks that incorrect control enumerate API calls return error.


    * covers: req-AWEMGR.ControlEnumeration-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.EnumControls - version: 1

    Checks that information on all control items are provided and in line with the
information in the AWC file.


    * covers: req-AWEMGR.ControlEnumeration-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.EnumDesigns - version: 2

    Checks if all information about designs inside an AWC can be retireved.


    * covers: req-AWEMGR.DesignEnumeration-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Events.AddRemoveListener - version: 1

    Test API's specific to add/remove/replace category listener callbacks.


    * covers: req-AWEMGR.Event_Notification-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Events.AsyncEvents - version: 1

    The event module (Event2), triggers an event whenever the RMS of a source signal is greater than the certain threshold.
The test sets the gain of the signal, causing the RMS to exceed the threshold, thus triggering events Asynchronously.


    * covers: req-AWEMGR.Event_Notification-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Events.AsyncTestEventCallbacks - version: 1

    The test counts the number of event callbacks to ensure all the callbacks are recieved.
This test is configured to run for 2 seconds to get significant number of event callbacks.


    * covers: req-AWEMGR.Event_Notification-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Events.DisableEnableAllEvents - version: 1

    Checks if the event modules in the test design can be disabled, then enabled again.


    * covers: req-AWEMGR.Event_Notification-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Events.ErrorProtocol_IncorrectEventHdr - version: 1

    When reading from socket, there is no suitable/enough data for an event header.


    * covers: req-AWEMGR.Event_Notification-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Events.ErrorProtocol_IncorrectMagicW - version: 1

    Reading from a socket, we do not get the right magic word.


    * covers: req-AWEMGR.Event_Notification-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Events.ErrorProtocol_IncorrectPayload - version: 1

    When reading from socket, there is just enough data for the header, but not for the payload.


    * covers: req-AWEMGR.Event_Notification-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Events.ErrorProtocol_PayloadResize - version: 1

    When reading from socket, we receive a bigger payload size than expected and we need to resize the internal buffer.


    * covers: req-AWEMGR.Event_Notification-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Events.Errors - version: 2

    Checks if the Errors in Events API's are handled correctly


    * covers: req-AWEMGR.SoftwareComponent-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Events.ReadEvents - version: 2

    Manually triggers an event, and reads the event data.


    * covers: req-AWEMGR.Event_Notification-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Events.TimeOut - version: 1

    Check that checking events with various timeout values works.


    * covers: req-AWEMGR.Event_Notification-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.EventsAbstraction.Api - version: 1

    Tests the event abstraction interface API's


    * covers: dsn-AWEMGR.ControlComm.Abstraction-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.GetCtx.Fail - version: 1

    Checks incorrect get context API calls.


    * covers: req-AWEMGR.MultipleDesignSupport-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.GetModuleClass - version: 1

    Ensures a module's class ID can be retrieved.


    * covers: req-AWEMGR.ModuleAdministration-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.GetModuleHandleByIndex_Fail - version: 1

    Ensures that incorrect parameters are handled ok.


    * covers: req-AWEMGR.ModuleAdministration-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.GetModuleHandleByIndex - version: 1

    Ensures that a module handle is returned correctly at a given index.


    * covers: req-AWEMGR.ModuleAdministration-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.GetModuleHandle_Fail - version: 1

    Checks incorrect usage of module handle retrival.


    * covers: req-AWEMGR.ModuleAdministration-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.GetModuleHandle - version: 1

    Ensures that a module handle is returned correctly.


    * covers: req-AWEMGR.ModuleAdministration-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Init.Fail - version: 1

    Makes sure init returns error when called with incorrect params.


    * covers: req-AWEMGR.SoftwareComponent-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Init_MultiInstance.Auto - version: 1

    Loads AWC files for multiple instances; assignes instanceIds sequentially.


    * covers: req-AWEMGR.MultipleDesignSupport-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Init_MultiInstance.Fail - version: 1

    Checks various incorrect API usages specific for setting up multi-instance AWC


    * covers: req-AWEMGR.MultipleDesignSupport-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Init_MultiInstance.Specific - version: 1

    Loads AWC files for multiple instances given specific endpointId


    * covers: req-AWEMGR.MultipleDesignSupport-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Init_deInit - version: 1

    Checks that AWEMGR can be initialized and uninitialized


    * covers: req-AWEMGR.SoftwareComponent-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.LoadDesign_Fail - version: 1

    Checks incorrect usage of loading an incorrect design. 
Also assumes a case (with "PresetWithAProblem") in which user has a correct AWC
file format, but forgot to add the AWB data file to the AWC directory.


    * covers: req-AWEMGR.LoadDesign-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Miscelleneous.LogBuffer - version: 1

    Checks that a buffer can be logged


    * covers: req-AWEMGR.SoftwareComponent-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.ModuleOperationStateDownStream - version: 1

    Checks if operational state change really has influence on downstream SINK module


    * covers: req-AWEMGR.ModuleAdministration-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.ModuleOperationState - version: 1

    Checks if runtime state can be set and queried again.


    * covers: req-AWEMGR.ModuleAdministration-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Multiinstance.set_get_events - version: 1

    This test shows that events are propogated from second instance and the
module parameters can be read and written to in a multiinstance design.


    * covers: req-AWEMGR.Event_Notification-1
    * covers: req-AWEMGR.Named_Access-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Multiinstance.subscribe_unsubscribe - version: 1

    This test subscribes/unsubscribe the events running on 2 different Instances. 
Event1 is running on InstanceId 0 and Event2 is running on InstanceID 1.


    * covers: req-AWEMGR.Event_Notification-1
    * covers: req-AWEMGR.Named_Access-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.ResetAllStates - version: 1

    Ensures that all module states are cleared.


    * covers: req-AWEMGR.Resetting-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.SendSpecialCommand_Fail_onAweCore - version: 1

    Sends an arbitrary (unknown) command to AWECore and detects that it failed.


    * covers: req-AWEMGR.SendBSPCommand-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.SendSpecialCommand_Fail - version: 1

    Checks incorrect usage of API is handled correctly.


    * covers: req-AWEMGR.SendBSPCommand-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.SkipUnloadTwice - version: 1

    Checks that double skip-marking is not returned as error.


    * covers: req-AWEMGR.DetachFromRunningTarget-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.SleepResume.Fail - version: 1

    This test shows that awemgr_audio_stop and awemgr_audio_start API's return error code when called with NULL Handle.


    * covers: req-AWEMGR.Sleep-1
    * covers: req-AWEMGR.Resume-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.SleepResume.Success - version: 1

    This test shows:
  - Audio pumping is stopped on calling awemgr_audio_stop by checking that the event count does not increase when pumping is stopped. 
  - Audio pumping is resumed again on calling awemgr_audio_start by checking that the event count increase during pumping.


    * covers: req-AWEMGR.Sleep-1
    * covers: req-AWEMGR.Resume-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.SubCanvasPresets - version: 1

    This test shows a preset can be loaded in a subcanvas


    * covers: req-AWEMGR.SubcanvasLoadUnloadDesign-1
    * covers: req-AWEMGR.SubcanvasAccess-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Subcanvas - version: 1

    This test shows:
  - A multi-instance design with 1 subcanvas per core can be loaded.
  - A design can be loaded to a subcanvas using tunneling mechanism
  - Controls from the subcanvas can be accessed (Set/Get) via the main canvas
  - Designs can be unloaded


    * covers: req-AWEMGR.SubcanvasLoadUnloadDesign-1
    * covers: req-AWEMGR.SubcanvasAccess-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.TargetInfo - version: 2

    Proves that information about target can be retrieved.


    * covers: req-AWEMGR.AweCoreAdministration-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Transact_Fail - version: 1

    Checks if transact function now returns an error code of the underlying transact method (QAL-207).


    * covers: req-AWEMGR.Raw_Access-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Transact - version: 1

    Check whether the raw-access method transact works correctly and returns the correct target info.


    * covers: req-AWEMGR.Raw_Access-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.UnloadDesign_Fail - version: 1

    Checks failure cases of unloading the design. 


    * covers: req-AWEMGR.UnloadDesign-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.UserDataNamedAccess - version: 1

    Ensures the user data can be accessed by key


    * covers: req-AWEMGR.UserData-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Util_VarTypeString - version: 1

    Checks the string output of the varType conversion.


    * covers: req-AWEMGR.SoftwareComponent-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Util_VersionString - version: 1

    Checks if version string can be retrieved.


    * covers: req-AWEMGR.SoftwareComponent-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.Utils - version: 2

    Checks the utility functions of AweMgr


    * covers: req-AWEMGR.SoftwareComponent-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.VariablePartialWriteFail - version: 1

    Checks error handling for incorrect partial reads.


    * covers: req-AWEMGR.Named_Access-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.VariableReadBigBuffer - version: 1

    Reads and Writes a variable array of a size bigger than the tuning message buffer.
It checks how the partial write/reads happen.


    * covers: req-AWEMGR.Named_Access-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.VariableReadPartialFail - version: 1

    Checks error handling for incorrect partial reads.


    * covers: req-AWEMGR.Named_Access-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.VariableReadPartial - version: 1

    Reads only partial sections of a variable.


    * covers: req-AWEMGR.Named_Access-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.VariableReadWriteSetMask - version: 1

    Checks that variables of modules that are written to, do not change the output until the last
buffer content has been sent.


    * covers: req-AWEMGR.Named_Access-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.VariableReadWrite - version: 1

    Reads and Writes variable values, for scaler and array


    * covers: req-AWEMGR.Named_Access-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#ff5900'>itest</span> - AWEMGR.VariableWritePartial - version: 1

    Writes parts of the source variable and checks modified buffer in sink variable.


    * covers: req-AWEMGR.Named_Access-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#21911E'>req</span> - AWEMGR.AWECoreErrors - version: 1

    It shall be possible to query the AWECore error code and corresponding error string.


    * Needs coverage by: ['itest']


    * Covered by: itest-AWEMGR.AWECoreErrors-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#21911E'>req</span> - AWEMGR.AWECoreInformationQuery - version: 2

    It shall be possible to query information from AWE Core, like CPU, memory, modules and layout profiling info.


    * Needs coverage by: ['itest']


    * Covered by: itest-AWEMGR.AweCoreInfo.AweCoreLayoutProfiling_Fails-1
    * Covered by: itest-AWEMGR.AweCoreInfo.Cpu-1
    * Covered by: itest-AWEMGR.AweCoreInfo.Fails-1
    * Covered by: itest-AWEMGR.AweCoreInfo.LayoutProfiling-1
    * Covered by: itest-AWEMGR.AweCoreInfo.Memory-2
    * Covered by: itest-AWEMGR.AweCoreInfo.Modules-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#21911E'>req</span> - AWEMGR.AweCoreAdministration - version: 1

    An application shall have the possibility to query information from AWE Core instances on the platform.
AWE-Manager shall provide an API to return this information.


    * Needs coverage by: ['itest']


    * Covered by: itest-AWEMGR.TargetInfo-2

    * Status: <span style='color:green'>OK</span>

* <span style='color:#21911E'>req</span> - AWEMGR.ControlEnumeration - version: 1

    An application can query the controllable items from AWE-Manager. It is the application's task
to further filter or use this list of items.


    * Needs coverage by: ['itest']


    * Covered by: itest-AWEMGR.ControlInfoByName-1
    * Covered by: itest-AWEMGR.Enum.Fail-1
    * Covered by: itest-AWEMGR.EnumControls-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#21911E'>req</span> - AWEMGR.DesignEnumeration - version: 1

    It shall be possible to query AWE-Manager for the designs supported on the platform.
The list of designs contains the "boot" or "main" AWB, as well as the preset AWBs.


    * Needs coverage by: ['itest']


    * Covered by: itest-AWEMGR.Enum.DesignEnumFail-1
    * Covered by: itest-AWEMGR.EnumDesigns-2

    * Status: <span style='color:green'>OK</span>

* <span style='color:#21911E'>req</span> - AWEMGR.DetachFromRunningTarget - version: 1

    It must be possible to shut down AWE-Manager (library), i.e. call the exit function, without tearing
down a running AWE core system. This is like a "detach from a running system" for Designer.


    * Needs coverage by: ['itest']


    * Covered by: itest-AWEMGR.DetachFromSystemFail-1
    * Covered by: itest-AWEMGR.DetachFromSystem-1
    * Covered by: itest-AWEMGR.SkipUnloadTwice-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#21911E'>req</span> - AWEMGR.Event_Notification - version: 1

    An application can be informed when events inside the AWE design happen.


    * Needs coverage by: ['itest']


    * Covered by: itest-AWEMGR.Events.AddRemoveListener-1
    * Covered by: itest-AWEMGR.Events.AsyncEvents-1
    * Covered by: itest-AWEMGR.Events.AsyncTestEventCallbacks-1
    * Covered by: itest-AWEMGR.Events.DisableEnableAllEvents-1
    * Covered by: itest-AWEMGR.Events.ErrorProtocol_IncorrectEventHdr-1
    * Covered by: itest-AWEMGR.Events.ErrorProtocol_IncorrectMagicW-1
    * Covered by: itest-AWEMGR.Events.ErrorProtocol_IncorrectPayload-1
    * Covered by: itest-AWEMGR.Events.ErrorProtocol_PayloadResize-1
    * Covered by: itest-AWEMGR.Events.ReadEvents-2
    * Covered by: itest-AWEMGR.Events.TimeOut-1
    * Covered by: itest-AWEMGR.Multiinstance.set_get_events-1
    * Covered by: itest-AWEMGR.Multiinstance.subscribe_unsubscribe-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#21911E'>req</span> - AWEMGR.LoadDesign - version: 1

    It shall be possible to instruct AWE-Manager to load a complete signal flow and start audio
processing.


    * Needs coverage by: ['itest']


    * Covered by: itest-AWEMGR.Design_And_Preset_Load-1
    * Covered by: itest-AWEMGR.LoadDesign_Fail-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#21911E'>req</span> - AWEMGR.ModuleAdministration - version: 1

    An application shall have an AIP which allows to change or query the state of an AWE controllable
module. This is a little bit more specific than writing/reading data.


    * Needs coverage by: ['itest']


    * Covered by: itest-AWEMGR.GetModuleClass-1
    * Covered by: itest-AWEMGR.GetModuleHandleByIndex_Fail-1
    * Covered by: itest-AWEMGR.GetModuleHandleByIndex-1
    * Covered by: itest-AWEMGR.GetModuleHandle_Fail-1
    * Covered by: itest-AWEMGR.GetModuleHandle-1
    * Covered by: itest-AWEMGR.ModuleOperationStateDownStream-1
    * Covered by: itest-AWEMGR.ModuleOperationState-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#21911E'>req</span> - AWEMGR.MultipleDesignSupport - version: 1

    AWE-Manager shall be a central frontend to all AWE Core instances on the platform.
When several of these instances exist (multi-canvas integration), AWE-Manager shall be
able to route application requests to the appropriate AWE Core instance.


    * Needs coverage by: ['itest']


    * Covered by: itest-AWEMGR.GetCtx.Fail-1
    * Covered by: itest-AWEMGR.Init_MultiInstance.Auto-1
    * Covered by: itest-AWEMGR.Init_MultiInstance.Fail-1
    * Covered by: itest-AWEMGR.Init_MultiInstance.Specific-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#21911E'>req</span> - AWEMGR.Named_Access - version: 1

    It shall be possible to select an AWE controllable module or variable by name. Data can then be
written to or read from this item.


    * Needs coverage by: ['itest']


    * covers: feat-AWEMGR.AUDIO_PROPERTIES-1
    * covers: feat-AWEMGR.VOLUME-1

    * Covered by: itest-AWEMGR.Multiinstance.set_get_events-1
    * Covered by: itest-AWEMGR.Multiinstance.subscribe_unsubscribe-1
    * Covered by: itest-AWEMGR.VariablePartialWriteFail-1
    * Covered by: itest-AWEMGR.VariableReadBigBuffer-1
    * Covered by: itest-AWEMGR.VariableReadPartialFail-1
    * Covered by: itest-AWEMGR.VariableReadPartial-1
    * Covered by: itest-AWEMGR.VariableReadWriteSetMask-1
    * Covered by: itest-AWEMGR.VariableReadWrite-1
    * Covered by: itest-AWEMGR.VariableWritePartial-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#21911E'>req</span> - AWEMGR.Preset_Selection - version: 1

    Depending on the current use-case (or target configuration) an application may apply one
or several preset configurations.


    * Needs coverage by: ['itest']


    * covers: feat-AWEMGR.PRESET-1

    * Covered by: itest-AWEMGR.Design_And_Preset_Load-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#21911E'>req</span> - AWEMGR.Raw_Access - version: 1

    An application shall be able to use correctly formed AWE tuning messages. AWE-Manager
is supposed to "simply" pass on this command message to the Audio Processors.
It's the application's responsibility to construct the AWE tuning message and to handle
all possible return values or errors.


    * Needs coverage by: ['itest']


    * Covered by: itest-AWEMGR.Transact_Fail-1
    * Covered by: itest-AWEMGR.Transact-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#21911E'>req</span> - AWEMGR.Resetting - version: 1

    It must be supported to reset AWE Core internal states.


    * Needs coverage by: ['itest']


    * Covered by: itest-AWEMGR.ResetAllStates-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#21911E'>req</span> - AWEMGR.Resume - version: 1

    It shall be possible to resume Audio Weaver Audio Processing via AWE-Manager in case the system exits sleep mode.


    * Needs coverage by: ['itest']


    * Covered by: itest-AWEMGR.SleepResume.Fail-1
    * Covered by: itest-AWEMGR.SleepResume.Success-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#21911E'>req</span> - AWEMGR.SendBSPCommand - version: 1

    It shall be possible to use AWE-Manager API to send special commands to the software
encapsulating AWECore processing ("BSP"). Those commands are targeted to that software
rather than to AWECore itself. For example, such commands can be used to
load a plugin into the BSP.


    * Needs coverage by: ['itest']


    * Covered by: itest-AWEMGR.SendSpecialCommand_Fail_onAweCore-1
    * Covered by: itest-AWEMGR.SendSpecialCommand_Fail-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#21911E'>req</span> - AWEMGR.SharedMem - version: 1

    It shall be possible for an application to inform the AWE design about a shared memory location.
This shared mem can be used by modules in the AWE design to exchange data directly with
the application.
** OBSOLETE ** The shared memory mapping is done not via AWE Manager.



    * Status: <span style='color:green'>OK</span>

* <span style='color:#21911E'>req</span> - AWEMGR.Sleep - version: 1

    It shall be possible to halt Audio Weaver Audio Processing via AWE-Manager in case the system enters sleep mode.


    * Needs coverage by: ['itest']


    * Covered by: itest-AWEMGR.SleepResume.Fail-1
    * Covered by: itest-AWEMGR.SleepResume.Success-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#21911E'>req</span> - AWEMGR.SoftwareComponent - version: 1

    AWE-Manager, as a SW component being linked into an application, needs to handle (system) resources.
It shall be possible for the application to initialized and release AWE-Manager.


    * Needs coverage by: ['itest']


    * Covered by: itest-AWEMGR.CommTimeout-1
    * Covered by: itest-AWEMGR.Events.Errors-2
    * Covered by: itest-AWEMGR.Init.Fail-1
    * Covered by: itest-AWEMGR.Init_deInit-1
    * Covered by: itest-AWEMGR.Miscelleneous.LogBuffer-1
    * Covered by: itest-AWEMGR.Util_VarTypeString-1
    * Covered by: itest-AWEMGR.Util_VersionString-1
    * Covered by: itest-AWEMGR.Utils-2

    * Status: <span style='color:green'>OK</span>

* <span style='color:#21911E'>req</span> - AWEMGR.SubcanvasAccess - version: 1

    It shall be possible to perform operations such control set/get, audio start, audio stop on a subcanvas AWE Instance.


    * Needs coverage by: ['itest']


    * Covered by: itest-AWEMGR.SubCanvasPresets-1
    * Covered by: itest-AWEMGR.Subcanvas-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#21911E'>req</span> - AWEMGR.SubcanvasLoadUnloadDesign - version: 1

    It shall be possible to load/unload designs in to Subcanvas AWE Instance.


    * Needs coverage by: ['itest']


    * Covered by: itest-AWEMGR.SubCanvasPresets-1
    * Covered by: itest-AWEMGR.Subcanvas-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#21911E'>req</span> - AWEMGR.UnloadDesign - version: 1

    It shall be possible to instruct AWE-Manager to destroy a complete signal flow and stop audio
processing.


    * Needs coverage by: ['itest']


    * Covered by: itest-AWEMGR.UnloadDesign_Fail-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#21911E'>req</span> - AWEMGR.UserData - version: 1

    It shall be possible to query the control and module user data with key or with index.


    * Needs coverage by: ['itest']


    * Covered by: itest-AWEMGR.AWC.UserData-1
    * Covered by: itest-AWEMGR.UserDataNamedAccess-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.ControlAccessByIndex - version: 2

    Ensures a control object can be retrieved by index value.


    * covers: dsn-AWEMGR.AWC.AweDataByIndex-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.ControlAccessByName - version: 2

    Ensures a control object can be retrieved by a name.


    * covers: dsn-AWEMGR.AWC.NameToAweData-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.ControlCountError - version: 2

    Checks that the awc_init succeeds when the nr_ctl is not equal to Control count(_CTL_ Enteries)


    * covers: dsn-AWEMGR.AWC.CheckConsistency-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.ControlNameAndAlias - version: 1

    Ensures that an AWC Control can be searched based on the its name and alias


    * covers: dsn-AWEMGR.AWC.NameToAweData-1
    * covers: dsn-AWEMGR.AWC.AliasToAweData-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.CtlTypeName - version: 2

    Checks the name of the awc_ctl_type_t


    * covers: dsn-AWEMGR.AWC.OwnVarTypes-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.DesignAccessByIndex - version: 2

    Ensures that an AWC design object can be retrieved given an
index value (rather than a name).


    * covers: dsn-AWEMGR.AWC.AweDataByIndex-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.DesignAccessByName - version: 1

    Ensures that an AWC design object can be retrieved given a
name (rather than an index value).


    * covers: dsn-AWEMGR.AWC.NameToAweData-1
    * covers: dsn-AWEMGR.AWC.AwbDataInfo-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.DesignCountError - version: 1

    Checks that the awc_init fails when the nr_awb is not equal to design count(_AWB_ Enteries)


    * covers: dsn-AWEMGR.AWC.CheckConsistency-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.ForEachDesign - version: 2

    Checks the looping method by counting the designs.
It also checks if the generic user data pointer is consumed.


    * covers: dsn-AWEMGR.AWC.AwbDataInfo-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.ForEachModuleAndControl - version: 1

    Checks the looping method by counting the modules and controls.


    * covers: dsn-AWEMGR.AWC.AweDataByIndex-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.InValidLines - version: 1

    Checks the parser with incorrect usage.


    * covers: dsn-AWEMGR.AWC.CheckConsistency-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.InvalidFile - version: 1

    Checks that incorrect file names are handled.


    * covers: dsn-AWEMGR.AWC.CheckConsistency-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.LineEndings - version: 1

    Verifies the parser works with different line trailings.


    * covers: dsn-AWEMGR.AWC.CheckConsistency-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.MapCreateAndFree - version: 1

    Checks that map can be created (in Fixture Setup) and freed(in Fixture Destroy) .


    * covers: dsn-AWEMGR.AWC.CheckConsistency-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.MapDelete - version: 1

    Checks that elements can be deleted from the map.


    * covers: dsn-AWEMGR.AWC.CheckConsistency-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.MapDuplicateKeys - version: 1

    Checks that duplicate keys are rejected.


    * covers: dsn-AWEMGR.AWC.CheckConsistency-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.MapHashConsistency - version: 1

    Checks that hash algorithm produces same hash for same input and different hash for different input.


    * covers: dsn-AWEMGR.AWC.CheckConsistency-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.MapInsert - version: 1

    Checks that elements can be inserted in the map.


    * covers: dsn-AWEMGR.AWC.CheckConsistency-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.MapResize - version: 1

    Checks that table can resize itself when the number of elements increase map size


    * covers: dsn-AWEMGR.AWC.CheckConsistency-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.MapSearch - version: 1

    Checks that elements can be retreived from the map.


    * covers: dsn-AWEMGR.AWC.CheckConsistency-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.ModuleAccessByIndex - version: 2

    Ensures a module object can be retrieved by index value.


    * covers: dsn-AWEMGR.AWC.AweDataByIndex-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.ModuleAccessByName - version: 1

    Ensures a module object can be retrieved by a name.


    * covers: dsn-AWEMGR.AWC.NameToAweData-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.ModuleCountError - version: 1

    Checks that the awc_init fails when the nr_mod is not equal to Module count(_MOD_ Enteries)


    * covers: dsn-AWEMGR.AWC.CheckConsistency-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.ModuleNameAndAlias - version: 1

    Ensures that an AWC Control can be searched based on the its name and alias


    * covers: dsn-AWEMGR.AWC.NameToAweData-1
    * covers: dsn-AWEMGR.AWC.AliasToAweData-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.NewSchemaUpgrade - version: 1

    Loads an AWC with a new schema information. This contains
information about classId and objectId of a module.


    * covers: dsn-AWEMGR.AWC.Compatibility-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.TestEvents - version: 1

    Ensures an events are correctly parsed


    * covers: dsn-AWEMGR.AWC.Events-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.TestInfo - version: 2

    Ensures an information structure is correctly parsed.


    * covers: dsn-AWEMGR.AWC.VersionInfo-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.TestInvalidControlType - version: 2

    Checks a selection of incorrectly formatted lines when readsing a control object.


    * covers: dsn-AWEMGR.AWC.CheckConsistency-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.TopDataAccess - version: 1

    Ensures the Top data can be accessed by key


    * covers: dsn-AWEMGR.AWC.TopData-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.UserDataIndexAccess - version: 1

    Ensures the user data can be accessed by index


    * covers: dsn-AWEMGR.AWC.UserData-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.UserDataNamedAccess - version: 1

    Ensures the user data can be accessed by key


    * covers: dsn-AWEMGR.AWC.UserData-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.ValidLines - version: 1

    Checks the parser and feeds it a bunch of valid lines.


    * covers: dsn-AWEMGR.AWC.CheckConsistency-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.parseNumericToken - version: 1

    Checks the parser internal functions with incorrect usage.


    * covers: dsn-AWEMGR.AWC.CheckConsistency-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.parseStringToken - version: 1

    Checks the parser internal functions with incorrect usage.


    * covers: dsn-AWEMGR.AWC.CheckConsistency-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.parseType - version: 1

    Checks the parser internal functions with incorrect usage.


    * covers: dsn-AWEMGR.AWC.CheckConsistency-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.stringToNumber - version: 1

    Checks the parser internal functions with incorrect usage.


    * covers: dsn-AWEMGR.AWC.CheckConsistency-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.validateControlParsing - version: 1

    Checks the parser internal functions with incorrect usage.


    * covers: dsn-AWEMGR.AWC.CheckConsistency-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWC.validateModuleParsing - version: 1

    Checks the parser internal functions with incorrect usage.


    * covers: dsn-AWEMGR.AWC.CheckConsistency-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWECONFIG.ErrorCases - version: 1

    Checks that the component correctly handles the error cases


    * covers: dsn-AWEMGR.AWECONFIG.Errorhandling-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWECONFIG.add_set_get - version: 1

    Checks that the configs can be added and set / get can be performed on the config.


    * covers: dsn-AWEMGR.AWECONFIG.Add-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWECONFIG.add_string_get - version: 1

    Checks that the configs can be added and set via a config string, and then can be retrieved.


    * covers: dsn-AWEMGR.AWECONFIG.EnvAdd-1
    * covers: dsn-AWEMGR.AWECONFIG.Get-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWECONFIG.add_string_set - version: 1

    Checks that the configs can be added and set via a config string, and then can be retrieved.


    * covers: dsn-AWEMGR.AWECONFIG.BatchAdd-1
    * covers: dsn-AWEMGR.AWECONFIG.Get-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWECONFIG.bool_test - version: 1

    Checks that the bool values are correctly handled.


    * covers: dsn-AWEMGR.AWECONFIG.Get-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWECONFIG.float_test - version: 1

    Checks that the float values are correctly handled.


    * covers: dsn-AWEMGR.AWECONFIG.Get-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWECONFIG.int_test - version: 1

    Checks that the int values are correctly handled.


    * covers: dsn-AWEMGR.AWECONFIG.Get-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWECONFIG.uint_test - version: 1

    Checks that the unsigned int values are correctly handled.


    * covers: dsn-AWEMGR.AWECONFIG.Get-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWOSAL.Mutex.Basic - version: 1

    Ensures that a mutex can be created, locked, unlocked and destroyed


    * covers: dsn-AWEMGR.AWOSAL.Mutex-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWOSAL.Mutex.Multithread - version: 1

    Ensures that a mutex can effectively work when 2 threads are trying to lock/unlock a mutex


    * covers: dsn-AWEMGR.AWOSAL.Mutex-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWOSAL.Threads.Basic - version: 1

    Ensures that a thread can be created, started, joined and destroyed


    * covers: dsn-AWEMGR.AWOSAL.Threads-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.AWOSAL.Threads.Multithread - version: 1

    Ensures that component can manage multiple threads independently.


    * covers: dsn-AWEMGR.AWOSAL.Threads-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.Cmd.CheckBufferSizes - version: 1

    Checks if buffer sizes (in nr of words) can be retrieved from initialized structure.


    * covers: dsn-AWEMGR.AWECMD.MemoryHandle-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.Cmd.Classes - version: 1

    Checks if (installed) module information can be retrieved.


    * covers: dsn-AWEMGR.AWECMD.ModuleClass-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.Cmd.CpuInfo - version: 1

    Checks commands to retrieve CPU load information.


    * covers: dsn-AWEMGR.AWECMD.CpuInfo-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.Cmd.Destroy - version: 1

    Checks that a correct destroy command is created.


    * covers: dsn-AWEMGR.AWECMD.Destroy-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.Cmd.GetCmds - version: 1

    Checks the tune commands for getting values are correctly formatted.


    * covers: dsn-AWEMGR.AWECMD.SetGetValueOrValues-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.Cmd.GetExtInfo - version: 1

    Checks that a command to get extended an info structure.


    * covers: dsn-AWEMGR.AWECMD.TargetInfo_Extended-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.Cmd.GetNrCores2 - version: 1

    Checks that a command to get the number of cores is created.


    * covers: dsn-AWEMGR.AWECMD.TargetInfo_Extended-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.Cmd.GetResponseFailBufSize - version: 1

    Checks that too small output buffer is handled correctly and only that amount is copied out
which fits into the output buffer.


    * covers: dsn-AWEMGR.AWECMD.CentralCheckError-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.Cmd.GetResponseFailErrorCode - version: 1

    Checks that a buffer with AWE error is handled.


    * covers: dsn-AWEMGR.AWECMD.CentralCheckError-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.Cmd.GetResponse - version: 1

    Checks whether an AWE response can be parsed and extracted from buffer correctly.


    * covers: dsn-AWEMGR.AWECMD.CentralCheckError-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.Cmd.Init_Exit - version: 1

    Ensures the awe_CMD component can be initialized and torn down.


    * covers: dsn-AWEMGR.AWECMD.MemoryHandle-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.Cmd.InvalidInputs - version: 1

    Ensures the awe_CMD correctly handles Invalid Inputs such as NULL context


    * covers: dsn-AWEMGR.AWECMD.MemoryHandle-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.Cmd.LayoutProfilingInfo - version: 1

    Checks commands to retrieve information about layouts.


    * covers: dsn-AWEMGR.AWECMD.LayoutProfilingInfo-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.Cmd.MemInfo - version: 1

    Checks commands to retrieve (heap) memory size information.


    * covers: dsn-AWEMGR.AWECMD.MemInfo-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.Cmd.ModuleClass - version: 1

    Checks that correct commands are created to obtaining the module class information.


    * covers: dsn-AWEMGR.AWECMD.ModuleClass-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.Cmd.ModuleOperationState - version: 1

    Checks that correct commands for getting the module state are generated.


    * covers: dsn-AWEMGR.AWECMD.ModuleOperationState-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.Cmd.ResetState - version: 1

    Checks that reset command is properly created.


    * covers: dsn-AWEMGR.AWECMD.ResetState-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.Cmd.ResponseHandlingCpuCores - version: 1

    Checks reception of PFID_GetCores2 responses


    * covers: dsn-AWEMGR.AWECMD.CentralCheckError-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.Cmd.ResponseHandling - version: 2

    Checks the central response handling method, tries various cases to retrieve data without causing an error response


    * covers: dsn-AWEMGR.AWECMD.CentralCheckError-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.Cmd.ResponseTwiceError - version: 1

    Checks that it is not possible to call the response buffer parsing routine twice. Also checks that buffer size is smaller than 3.


    * covers: dsn-AWEMGR.AWECMD.CentralCheckError-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.Cmd.SetCmds - version: 1

    Checks the tune commands for setting values are correctly formatted.


    * covers: dsn-AWEMGR.AWECMD.SetGetValueOrValues-1

    * Status: <span style='color:green'>OK</span>

* <span style='color:#CF6511'>utest</span> - AWEMGR.Cmd.TargetInfo - version: 1

    Checks a correct cmd buffer for getting target info.


    * covers: dsn-AWEMGR.AWECMD.TargetInfo-1

    * Status: <span style='color:green'>OK</span>