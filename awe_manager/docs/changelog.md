# Changelog of AWE-Manager

This is a brief summary of changes to the AWE Manager release package.

## Version 1.0.1

Released: 2026-06-05

- **CHG**: CM/CI change: Distribution build takes compile warnings as errors now.
- **CHG**: Cleaner handling of CMake build flags (options); It is easier now to get a lean (library only) build by passing `AWEMGR_BUILD_TESTS=OFF` and `AWEMGR_BUILD_APPS=OFF` (new) to the cmake configuration. Script to run various combinations of build flags created, see ./scripts sub-directory. This directory also contains an overview of the build-matrix tested.
- **FIX**: Some customer's misra concerns addressed.
- **FIX**: Use of function awosal_strlcpy() strictly in strlcpy() macro
- **FIX**: AWEMgr-Shell and Tuning Server add-on public headers (`awemgr_shell.h`, `awemgr_tuning_server.h`) are now installed into the package include directory.
- **FIX**: Documentation updated regarding tuning server memory allocation.

## Version 1.0.0

Released: 2026-05-07

- **NEW**: Two new optional add-ons are available and can be enabled with the CMake option `AWEMGR_BUILD_ADDONS`:

    - **AWEMgr-Shell add-on** (`addons/shell`): packages the interactive AWEMgr-Shell as a reusable library so that it can be embedded into any host application, not just the standalone shell executable. Applications link against this add-on to gain a full command interpreter with tab completion.

    - **Tuning Server add-on** (`addons/serversocket`): exposes an AWE-Manager instance as a TCP tuning server, allowing AWE Designer to connect to a running target over the network. The add-on accepts one client at a time, proxies AWE tuning packets through AWE-Manager and forwards the responses back to the client.

- **NEW**: Applications for AWE-Manager are now packaged more prominently. Three applications are provided:

    - **NEW**: A new long-running **service application** (`awemgr_service`) has been added. It is designed to be managed by `systemd` (or equivalent), logs to `stderr`/`journald`, and optionally accepts an AWEMgr-Shell connection (`-shell_socket <port>`) and/or an AWE Designer tuning connection (`-tuning_socket <port>`) at startup. Both optional sockets run in detached background threads so the main service loop is never blocked.

    - **NEW**: A Python **interactive client** (`awemgr_client.py`) has been added for use with `awemgr_service -shell_socket`. It provides a `prompt_toolkit`-based REPL with four-level lazy tab completion (root commands, sub-directory entries, command parameters and flag values), colour-coded output, and persistent command history. Connect with `python3 awemgr_client.py [-H host] [-p port]`.

    - **CHG**: AWEMgr-Shell is now also shipped as a standalone **application** (`apps/awemgr_shell`), built on top of the new AWEMgr-Shell add-on library. The command-line interface and behavior are unchanged.

- **NEW**: The awe_COMM component, which talks to the BSP and AWE-Core, now supports protection of its transmission buffers to allow interleaved access from different threads. This means it is, for example, possible to load AWB data to AWE-Core (progressive load or into a Subcanvas), while access to control variables and modules remains unblocked.

- **NEW**: Tracing of the data exchanged with AWE-Core - for control and for events - can now be enabled/disabled by a configuration setting. The tracing is done to the platform's logging system. Previously, an awkward inclusion of internal function prototypes and code changes were required. This feature greatly helps to debug problems in an integrated system. See `mgr.comm.trace.state` and `mgr.comm.trace.file` in [Supported Configuration Items][supported-configuration-items].

- **NEW**: For communication data tracing, an application using AWE-Manager can also directly tap into the communication. The application can set a callback with [`awemgr_events_set_listener()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_events_set_listener). The callback allows the application to render the communication data in its own format. Note: this is used by the AWEMgr-Shell add-on to render the communication data inside the shell, e.g., over the socket.

- **NEW**: A list of plugins used by a design was added to [`struct awemgr_design_info`](/AweMgrHdrDocs/structawemgr__design__info). Applications can use this information to load the plugins in the BSP before loading a design using [`awemgr_send_command()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_send_command). This feature requires updated Designer and AWE-TC versions to support it.

- **NEW**: AWEMgr-Shell now produces improved dump output of event and communication data.
- **CHG**: AWEMgr-Shell `event` command logic has been cleaned up. Check with `event -help` for more details.
- **CHG**: AWEMgr-Shell `info` command supports the `-extended` flag to report more information about AWECore. Check with `info -help` for more details.
- **CHG**: AWEMgr-Shell `info -cpu` command now shows all CPU usage (in percent) on all CPU cores and in all layouts at once.
- **CHG**: For some API commands some AWE-Core return values are suppressed, namely for [awemgr_audio_stop()](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_audio_stop) and [awemgr_audio_start()](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_audio_start). These functions can be called repeatedly without triggering an error when AWE-Core returns a non-zero response code.
- **CHG**: The former awe_CTRL sub-component has been renamed in code and in documentation to **awe_COMM** to better reflect that this component communicates with BSP/AWE-Core.
- **CHG**: Code structure in awe_COMM component changed so that a communication backend can be managed more easily - possibly externally.
- **CHG**: The requirement traceability report is now auto-generated in the documentation as an HTML table.
- **CHG**: Internal improvement: Items can be added in batch mode in awe_CONTROL component.
- **CHG**: AWEMgr-Shell - the `cfg` command now outputs YAML format instead of a table. The reason for this change is that the table had problems with longer key names, and generally YAML output can be better handled in test scripting.
- **CHG**: Build config improvement: Windows uses CMake presets.
- **FIX**: Fixed potential memory access issues in awe_AWC component.
- **FIX**: Getting heap allocation info now works for every CPU core.
- **FIX**: AWEMgr-Shell - `transact` command now adheres to the configured package buffer size and also checks for zero-sized transmission buffers.
- **FIX**: Internal improvement: Code coverage improved.
- **FIX**: Memory leak and uninitialized pointer in AWC parser.

## Version 0.8.0

Released: 2025-11-07

- **NEW**: AWE-Manager now supports checking of the range information given in Designer. For scalars and arrays alike the values given in [`awemgr_control_write()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_control_write) are validated. The validation is enabled by default and can be switched off and on by the new configuration item `mgr.api.range_check`, see [Supported Configuration Items][supported-configuration-items].

- **CHG**: **Breaking API change** for [`awemgr_control_write()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_control_write) - Unused argument `enum awemgr_vartype type` has been removed and must be removed from any call to this method.

- **CHG**: **Breaking change** for [`enum awemgr_vartype`](/AweMgrHdrDocs/variables/#a/) - New types have been added to `enum awemgr_vartype` and `AWEMGR_VARTYPE_FRACT` is renamed to `AWEMGR_VARTYPE_FRACT32`.

- **CHG**: **Breaking change** for [`struct awemgr_ctl_elem_info`](/AweMgrHdrDocs/variables/#a/) - `value` is renamed to `range`, unused item `default_value` has been removed and `range` can now be accessed as unsigned integers.

- **CHG**: AWEMgr-Shell `comm` command now has to be used with a parameter like this `comm -uri host@1234`   .

- **CHG**: AWEMgr-Shell displays range information for controls supporting range information.

- **FIX**: Config API and setting of log levels now working correctly, also when updating via [`AWEMGR_CFG_OVERRIDE`](/AweMgrHdrDocs/awe__manager_8h/#define-awemgr_cfg_override) environment variable or from AWEMgr-Shell.

- **CHG**: CMake has been improved for Windows builds.

- **CHG**: Internal: CMake preset output structure modified.


## Version 0.7.7

Released: 2025-10-15

- **NEW**: API function [`awemgr_get_layout_info()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_layout_info) allows to retrieve detailed information about each layout/thread of the running signal flow.

- **NEW**: AWEMgr-Shell: command `info` has a new parameter `-layout` now which calls into `awemgr_get_layout_info()`

- **NEW**: New API [`awemgr_reset_state_all()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_reset_state_all) has been added to allow the resetting of AWE Core internal states for all AWE instances.

    Implementation note: This method is typically required when the system has to be put into a deep sleep mode. Then the client of AWE-Manager has to invoke [`awemgr_audio_stop()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_audio_stop) followed by a call to [`awemgr_reset_state_all()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_reset_state_all).

- **NEW**: AWEMgr-Shell: Command `reset` does reset the states in all AWE instances.

- **NEW**: AWEMgr-Shell: command `version` returns AWE-Manager's version string; useful in scripts

- **NEW**: AWEMgr-Shell: command `echo` can be used to print strings to the shell; useful in scripts, especially in conjunction of YML format output (see below); it is therefore recommended to print "comments", i.e. begin output with a '#' sign, like:

    `echo # !!!! this is a printout as a (YML) comment`

    Note that string delimiters `'` and `"` currently cannot be printed.

- **NEW**: AWEMgr-Shell: command `time-cmds` handles the reporting of time spent for a command.

    Example usage:

    ```
    awemgr-shell> time-cmds -on
    > Time commands: ON
    time_taken_ms: 0.009641
    awemgr-shell> sleep 100
    time_taken_ms: 100.203183
    awemgr-shell> time-cmds -off
    > Time commands: OFF
    awemgr-shell>
    ```

    The `awemgr_shell` executable has new parameter `-time` now as well which enables this timing report right from the start.

- **CHG**: AWEMgr-Shell: when some data buffers are printed, they are in YML format now. This makes it easier in test scripts to capture and analyze the output. Example:

    ```
    variable:
      name: SinkInt_10.value
      data: [
        0x0000000a, 0x000000aa, 0x0000dead, ...
        0x000000aa, 0x0000dead, ...
      ]
    ```

- **CHG**: code cleanup; flag for loading AWB not required in shared-mem communication anymore
- **FIX**: compile problems removed with compilation flags `-Wall -Werror` on AWE-Q Jenkins

## Version 0.7.6

Released: 2025-09-25

- **FIX**: Compile warning fix in aweCTRL component for communication with SHMEM


## Version 0.7.5

Released: 2025-08-13

- **FIX**: AWEMgr-Shell: strlcat C removed, using CPP string concatenation now for this debugging component (QAL 1465)


## Version 0.7.4

Released: 2025-08-12

- **FIX**: klocwork warnings fixed (QAL 1465)

## Version 0.7.3

Released: 2025-08-11

- **FIX**: removal of a strtok fct in favor of strtok_r
- **FIX**: removal of strncpy fct, implemented custom strlcpy function (awosal_strlcpy) when GLIB is not available
- **FIX**: CM/CI publication of PDF corrected
- **FIX**: lint fix for void fct returning a value

## Version 0.7.2

Released: 2025-07-04

- **CHG**: additional test cases for improved code coverage

## Version 0.7.1

Released: 2025-06-26

- **NEW**: The tuning buffer size used for the transmission of commands to AWECore can be adjusted via configuration setting `mgr.comm.buffersize` before awemgr_init() is called. Compile time default value for this config is 264 words for socket backend and 4096 words for shmem backend.

- **CHG**: The behavior of parsing the awc_index.txt file has changed. When encountering format errors for unknown variables, AWE-Manager now issues a warning to the tracing system instead of returning with an error. Please ensure tracing level for the AWC sub-component is at least at “warn” level.

- **FIX**: Set/Get module state for modules with instance != 0 works as expected now.

- **FIX**: The Subcanvas specific handling of transmission buffers has been fixed for the shmem-specific communication backend.

- **FIX**: AWEMgr-Shell `cfg` command works as expected now.


## Version 0.7.0

Released: 2025-06-19

- **NEW**: Handling of Subcanvases is now supported. AWE-Manager can be used to load and unload Subcanvas signal flows at runtime using existing API patterns (awemgr_load_design/awemgr_unload_design). This includes the usage of Subcanvas specific preset-AWB files. The Subcanvas AWB data configuration for AWE-Manager requires an updated AWE Target Configurator (awetc).

    Also supported is the reading and writing to exposed variables of the Subcanvas. This means an exposed module or variable of the Subcanvas can be handled like any other control variable and be used like, e.g., `awemgr_control_write(..., "Subcanvas_Inst0/DC.value", ...)`.

- **CHG**: **API change** for [`awemgr_unload_design()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_unload_design) to support unloading of Subcanvas signal flows.

- **CHG**: AWEMgr-Shell `unload_design` command has `-name` flag now

- **CHG**: AWEMgr-Shell `mgr_exit` command has `-detach` flag now

- **CHG**: AWEMgr-Shell `enum_controls` allows to use alias names now too in the `./controls` subdirectory

- **FIX**: Minor warning fixes for Windows platforms.


## Version 0.6.0

Released: 2024-05-19

This release contains a few new API calls extending {{name.awe_mgr}}'s functionality. Except for the special [`awemgr_transact()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_transact) call those are additional methods.

- **NEW**: Errors codes reported by AWECore itself are now being propagated by {{name.awe_mgr}}. An error code indicates to the caller that an AWECore error occurred. This error code may indicate to an application that further error (custom module) information may be retrieved using the new API [`awemgr_get_awe_error()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_awe_error).

- **NEW**: A new API [`awemgr_send_command()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_send_command) can be used to transmit a very specific tuning command to the software running AWECore. This "BSP" command may be used by the code implementing {{name.awe_mgr}} to communicate directly with this software rather with AWECore itself.

- **NEW**: Typically, {{name.awe_mgr}} stops any AWE Core processing on exit. The new API [`awemgr_skip_unload_design_on_exit()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_skip_unload_design_on_exit) can be used to change this behavior, so that AWE Core audio processing still commences when {{name.awe_mgr}} is stopped: _{{name.awe_mgr}} detaches from the target, so to say!_ This is useful in testing situations, but may also be valuable otherwise.

- **NEW**: A set of new API's are available to query AWE Core processing for information, like for CPU usage, available modules or usage of the available memory heaps.

    - [`awemgr_get_cpu_info()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_cpu_info)
    - [`awemgr_get_modulelist_info()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_modulelist_info)
    - [`awemgr_get_heap_info()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_get_heap_info)

- **NEW**: AWEMgr-Shell `info` command can now also be used to query the connected AWE Core target, like with `info -cpu`, `info -classes` or `info -mem`. Check out `info -h` to see how to direct the queries to the appropriate AWE Core instance or canvas.

- **NEW**: AWEMgr-Shell `script` command for including other command script files

- **NEW**: AWEMgr-Shell `cfg` command that allows to retrieve and change the configuration with which {{name.awe_mgr}} is configured (see fix entry below).

- **NEW**: AWEMgr-Shell will now persist its command history in a file `.awemgrshell.history` which is stored in the folder the application is started in.

- **CHG**: **API change** for [`awemgr_transact()`](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_transact) - it is not required to load an AWC file anymore to use this command. It can now directly be executed once {{name.awe_mgr}} is initialized.

- **CHG**: In AWEMgr-Shell the `-values` parameter for the commands `set_value` or `transact` now can also consume hex numbers, like `-values "0xdeadaffe 1234 0x1234"`

- **CHG**: AWEMgr-Shell `set_value` command does not require the awkward `-float` parameter anymore. The value parsing is automatically adapted depending on the variable type of the control.

- **FIX**: AWEMgr-Shell `comm-trace -file` creates a new (binary) file now (fopen with "wb"), instead of appending ("a+"). This also fixes the problem that running AWEmgr-Shell on Windows sometimes then created incorrect output.

- **FIX**: The known-issue of last release was fixed/changed. AWEMgr-Shell command `mgr_init` uses parameter `-cfg` now to adjust settings before initialization of {{name.awe_mgr}}. AWEMgr-Shell command line also supports this parameter. The format of that string is the same as for the environment variable [`AWEMGR_CFG_OVERRIDE`](/AweMgrHdrDocs/awe__manager_8h/#define-awemgr_cfg_override)

## Version 0.5.1

Released: 2025-03-18

- **FIX**: awe_CMD parsing of response data improved; more stable reaction to (corrupted) response data,
           resulting in additional test cases
- **FIX**: Internal fixes and improvements, like for test cases

Known Issues:

- The `-l` parameter of AWEmgr-Shell command `mgr_init` is not observed. The environment variable [`AWEMGR_CFG_OVERRIDE`](/AweMgrHdrDocs/awe__manager_8h/#define-awemgr_cfg_override) needs to be used to set e.g. log-levels per component for now.

## Version 0.5.0

Released: 2025-03-10

- **NEW**: AWEmgr-Shell `event` command for handling events and their subscription
- **NEW**: AWEmgr-Shell `user_data` command for printing info about (system integrator) data attached to AWE variables/modules
- **NEW**: AWEmgr-Shell `show` command for displaying configuration information of AWE-Manager (designs, modules, variables, events, user data)
- **NEW**: **!!! BREAKING CHANGE !!!** AWE-Manager now features a configuration module that can be used to adjust settings like log levels or the way how AWE-Manager is being initialized. This has an influence on the public API. The new component can be used with API functions prefixed with `awemgr_config_`. See [API documentation](/AweMgrHdrDocs/awe__manager_8h/#function-awemgr_config_create) and below for details.
- **CHG**: **!!! BREAKING CHANGE !!!** The `awemgr_load_awc` API now uses an endpointId instead of a pointer to an endpointId. This makes the usage of this API easier. See below for details.
- **CHG**: **!!! BREAKING CHANGE !!!** The `awemgr_get_nr_awcs` API is renamed to `awemgr_get_loaded_awc_count` for clarity.
- **CHG**: **!!! BREAKING CHANGE !!!** the event handling methods had to be renamed to be more consistent - for details see below.
- **CHG**: **!!! BREAKING CHANGE !!!** `awemgr_get_target_info` api now works with a pointer to awemgr_data instead of awemgr_ctx.
- **NEW**: the info structures for modules and variables obtained via the AWE-Manager API now also contain the alias name, which might have been added to the configuration data via AWE-TC tooling.
- **FIX**: event handling timeout values now; more robust against underlying communication backend problems

**Details On Event Handling API Changes:**

Calls working on or with the "event subsystem":

- `awemgr_events_start` (no change)
- `awemgr_events_stop` (no change)
- `awemgr_events_started` (new)
- `awemgr_events_process_next`  (was: `awemgr_process_next_event`)
- `awemgr_events_add_category_listener` (no change)
- `awemgr_events_remove_category_listener` (no change)

Calls handling a specific event (module)

- `awemgr_enable_event` (was: `awemgr_subscribe_event`)
- `awemgr_disable_event` (was: `awemgr_unsubscribe_event`)
- `awemgr_get_event_count` (no change)
- `awemgr_get_event_by_index` (no change)

**Details On The New AWE-Manager Configuration API:**

Before AWE-Manager was initialized with an integer value for the log-level settings. For the modification
of other parameters, like the settings for the socket communication backend, the access to internal
API functions was necessary. This has been changed now and the new configuration component can be used
prio to the initilization call.

Before:

- `awemgr_init(int loglevel, struct awemgr_data **mgr_pp)`

After:

- `awemgr_init(awe_config** cfg_pp, struct awemgr_data **mgr_pp)`

If no deviation from the defaults is required, then the first parameter can be NULL.
To set - for example - specific loglevels, use:

```
struct awemgr_data *my_awemgr_hdl;
awe_config* my_cfg_p;
awemgr_config_create(&my_cfg_p);
awemgr_config_set(my_cfg_p, "mgr.api.log.level", "debug");
awemgr_init(my_cfg_p, &my_awemgr_hdl);
```

For details on the available configuration keys, see [AWE-Manager documentation](/AweMgrHdrDocs/awe__manager_8h/).

**Details On awemgr_load_awc API Change:**

Before:
```c
  int endpoint = -1;
  enum awemgr_rc = awemgr_load_awc(mgr_p, awc_file, &endpoint);
```
After:
```c
  enum awemgr_rc = awemgr_load_awc(mgr_p, awc_file, 0);
```
In most cases, this parameter can simply be set to 0. It is only required for multi-canvas integrations.

## Version 0.4.0

Released: 2024-12-18

- **NEW**: Module and Control element aliases can be used on API level now;
           for example, it is possible to use 'Left Vol' instead of 'SubSys/VolCtrlSubSys/MainScaler.gain' now
- **NEW**: Custom data can be applied to the AWE-Manager configuration via AWE-Target-Configurator;
           this allows to attach any key=value pair to a module or control item. For example,
           a client of AWE-Manager can use a vmMask to limit the visibility of controllable items to certain user applications or certain virtual machines.
- **NEW**: Audio start and stop calls have been added to the API; this allows certain platforms
           to handle deep-power sleep modes.
- **CHG**: additional test cases; code coverage improvements
- **FIX**: Linux x64 binary distribution without gprof binaries :)
- **FIX**: Windows platform build fixes
- **FIX**: fract32 values handled in AWEmgr-Shell correctly
- **FIX**: AWEmgr-Shell handles problems with underlying communication backends (like awe_packet_client and awe_packet_server) better

Known issues:

- timeout handling for reading events not properly working


## Version 0.3.0

Released: 2024-11-12

- **NEW**: Event handling has been added (QAL-525).

  Note that the API for events proposed originally was changed slightly.

  AWE-Manager will not have an internal thread to read and report incoming event data.
  It is mandatory to call the `awemgr_process_next_event()` method from a thread spawn outside of AWE-Manager.
  AWE-Manager will dispatch incoming events via this method to the calling application.
  More information on this is provided in the documentation.

  The new generation of the `awc_index.txt` file (schema=2) is required to benefit from this functionality.
  Updated AWC-Tooling (0.2.0) is provided.

- **NEW**: A stress test application has been developed which can be used to measure the throughput of
  commands and events in AWE-Manager (only works on PC right now).

- **CHG**: awectrl_transact() does return error code when underlying communication backend had a failure (QAL-207)
  Note that this was tested only with socket backend so far and not with 'awe_packet_client' (which will be
  removed soon anyways)

- **CHG**: the debug/tracing level handling has been improved, new macros like AWEMGR_LOGGING_DEFAULT are provided
  to make usage of this feature cleaner

- **CHG**: CodeQuality and code complexity has been improved by adding more automated test cases;
  CM/CI changes are in place to check this quality consistently with every new code commit (pull request);
  metrics are stored on a central database to allow evaluation of those over time (QAL-367, EXTQCMOI-229)

- **CHG**: further internal improvements and code cleanups, like using proper error codes
  and more consistent logging/tracing

- **FIX**: handling of large data buffers corrected. The mask parameter was incorrectly set when chunking the
  data into separate tuning commands.

- **FIX**: fract32 type of modules are now correctly supported. Before this release that variable type was only
  tested on lower-level internal components, but not in integration tests.

- **FIX**: the coreId member inside the tuning command now correctly computed


## Version 0.2.0

Released: 2024-09-04

**!!Note!!** Please also observe the changes for 0.2.0-rc2 below.

- **NEW**: many more unit and integration test cases
- **NEW**: awemgr_read_partial() can be used to read only a subset of data from a variable
- **CHG**: awemgr_read() and awemgr_write() can handle big data buffers now which have a size
       greater than the underlying "tuning packet" buffer size (which typically is 264 words)
- **CHG**: design documentation updated and reviewed; the documentation also contains the
       requirement elicitation and traceability report
- **CHG**: renaming of API parameters to be more consistent

## Version 0.2.0-rc2

Released: 2024-07-23

ReleaseCandidate 2 containing a not yet finally reviewed version of the Design documentation.

**!!Note!!** AWC schema has changed. Older files will not be compatible anymore.
The AWC tooling has to be upgraded and AWC index files have to be re-generated.

- **NEW**: enumeration of controllable items (variables)
- **NEW**: Possibility to use correctly formatted AWE tuning buffers on AWE Manager
       interface ("raw access")
- **NEW**: API to control the state of a module (ACTIVE, BYPASS, MUTED, INACTIVE)
- **NEW**: AWEManager is able to consume several AWC files; this support is required
       once more than one AWE Core instance is running on the platform
- **NEW**: Google gtest applied and many more (unit/integration) test cases added (>=40 test cases)
- **NEW**: presetAWB test cases added
- **NEW**: Architecture documentation part of GIT repo; automatically built;
       will contain API documentation later too potentially
- **CHG**: AWC schema configuration is part of AWE-Manager repo/package
- **CHG**: cleanups and clearer separation of API calls, e.g. for awemgr_get_target_info()


## Version 0.1.1

Released: for QC R2.1

Unfortunately, there was not dedicated GIT tag applied for the release of this package.

- **CHG**: avoiding list of "forbidden" string handling functions
- **CHG**: centralized logging functionality
- **CHG**: internal reorg of code (CM); AWC component now inside AWE-Manager dir
- **CHG**: much improved and cleaned up AWE-Manager shell application
- **CHG**: removal of some OSS check violation complains
- **FIX**: cleanups and minor bugfixes; better parameter (sanity) checks


## Version 0.1.0-rc3

Released: for QC R2

- **NEW**: basic loading of AWC index and AWB design implemented
- **NEW**: basic writing/reading of variable data - "Named access"
