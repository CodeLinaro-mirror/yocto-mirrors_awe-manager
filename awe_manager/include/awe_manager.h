/* MIT License
**
** Copyright (c) 2026 DSP Concepts, Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**/


#ifndef INCLUSION_GUARD_AWE_MANAGER_H
#define INCLUSION_GUARD_AWE_MANAGER_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include "awemgr_targetinfo.h"
#include "awemgr_classinfo.h"
#include "awemgr_heapinfo.h"
#include "awemgr_cpuinfo.h"
#include "awemgr_layoutinfo.h"

#if defined(__cplusplus)
extern "C" {
#endif


/** @brief defines which schema the AWC index file needs to have (at minimum) */
#define AWEMGR_MAX_AWC_SCHEMA 3

/** @brief AWE Core can handle up to 16 Multicore endpoints/canvases
 * Each multicore canvas can support up to 16 cores
*/
#define MAX_AWE_ENDPOINTS  16

/** @brief Number of cores/instances supported per endpoint/canvas.
 * Endpoint and core index share one byte of the tuning message header - four
 * bits each - so both are limited to 0 .. 15. Passing a larger index would
 * corrupt the message header, therefore the API rejects it.
*/
#define MAX_AWE_CORES      16

/** @brief defines the maximum length (in chars) of the plugin/library name including the null termination */
#define AWEMGR_MAX_PLUGIN_NAME_LEN 64

/** @brief defines the maximum number of plugins/libraries supported per design */
#define AWEMGR_MAX_PLUGINS_PER_DESIGN 32

/**
 * @section awecfg AWE Manager Configuration API
 *
 * AWE Manager has a configuration API, which allows applications to create,
 * modify, and manage configuration settings for the AWE Manager.
 * The configuration settings control various aspects of the AWE Manager's behavior,
 * such as logging levels, network settings, and timeouts.
 *
 * The API provides functions to create and destroy configuration objects,
 * set configuration values, and retrieve configuration values.
 *
 * @subsection cfg_supported Supported Configuration Items
 *
 * The configuration keys, default values, and descriptions are documented below.
 *
 * \attention All configuration values are given as strings.
 * \attention Socket-related configurations are applied once during the initialization. Setting these configurations **after** the call to awemgr_init() will have no affect.
 *
 * | Key               | Description           | Default Value | Values                       |
 * |-------------------|-----------------------|---------------|------------------------------|
 * | `mgr.api.log.level` | API log level options | `info`          | `error, warning, info, debug` |
 * | `mgr.awc.log.level` | AWC component log level | `info`        | `error, warning, info, debug` |
 * | `mgr.cmd.log.level` | CMD component log level | `info`        | `error, warning, info, debug` |
 * | `mgr.comm.log.level` | COMM component log level | `info`      | `error, warning, info, debug` |
 * | `mgr.osal.log.level` | OSAL component log level | `info`      | `error, warning, info, debug` |
 * | `mgr.cfg.log.level` | CONFIG component log level | `info`     | `error, warning, info, debug` |
 * | `mgr.comm.socket.ip` | IP address for socket-based tuning/control | `127.0.0.1` | IP address string |
 * | `mgr.comm.socket.port` | Port for socket-based tuning/control | `15002` | <integer value> |
 * | `mgr.comm.buffersize` | Size of communication buffer | `264` or `4096` - depending on platform | <integer value> |
 * | `mgr.comm.timeoutms` | Number of milliseconds to wait for response | `2000` | <integer value> |
 * | `mgr.comm.trace.state` | Enable or disable tracing of tuning messages and events | `false` | `true`, `false`, `1`, `0` |
 * | `mgr.comm.trace.file` | When tracing is enabled, write (binary) trace data to files derived from this basename (TX to `<name>.tx`, RX to `<name>.rx`). Note that data may be buffered by the OS. The buffers are flushed when the trace is disabled. | `~` | `~` for no file, basename otherwise |
 * | `mgr.event.socket.ip` | IP address for socket-based event transmission | `127.0.0.1` | IP address string |
 * | `mgr.event.socket.port` | Port for socket-based event transmission | `15010` | <integer value> |
 * | `mgr.event.trace.state` | Enable or disable tracing of event messages | `false` | `true`, `false`, `1`, `0` |
 * | `mgr.event.trace.file` | When tracing is enabled write (binary) event trace data to this file | `~` | `~` for no file, path otherwise |
 * | `mgr.api.range_check` | Controls whether a range checking of values shall be done | `true` | `true`, `false`, `1`, `0` |
 *
 *
 * @subsection awecfg_logging AWE Manager Logging Configuration
 *
 * AWE Manager allows independent control of the logging levels for the api
 * and its various sub-components. Logging level can be adjusted for any component by
 * calling `awemgr_config_set` with the corresponding key and value strings For example:
 *
 * @code
 * awemgr_config_set(cfg_p, "mgr.api.log.level", "info");
 * @endcode
 *
 * Convenience macros are provided for the logging configuration keys (e.g.,
 * CFG_MGR_LOGLEVEL, CFG_AWC_LOGLEVEL, etc.) and for standard log level strings
 * (e.g., LOG_LEVEL_ERROR, LOG_LEVEL_INFO, LOG_LEVEL_WARN, and LOG_LEVEL_DEBUG).
 *
 * @code
 * awemgr_config_set(cfg_p, CFG_MGR_LOGLEVEL, LOG_LEVEL_INFO);
 * @endcode
 */


// AWE Manager logging components
#define CFG_MGR_LOGLEVEL                "mgr.api.log.level"
#define CFG_AWC_LOGLEVEL                "mgr.awc.log.level"
#define CFG_CMD_LOGLEVEL                "mgr.cmd.log.level"
#define CFG_COMM_LOGLEVEL               "mgr.comm.log.level"
#define CFG_OSAL_LOGLEVEL               "mgr.osal.log.level"
#define CFG_CFG_LOGLEVEL                "mgr.cfg.log.level"
#define CFG_MGR_RANGECHECK_ENABLED      "mgr.api.range_check"


// AWE Manager logging levels strings
#define LOG_LEVEL_ERROR                 "error"
#define LOG_LEVEL_INFO                  "info"
#define LOG_LEVEL_WARN                  "warning"
#define LOG_LEVEL_DEBUG                 "debug"
#define CFG_VAL_TRUE                    "true"
#define CFG_VAL_FALSE                   "false"


/**
 * @brief AWE Manager Configuration Override Using an Environment Variable
 *
 * AWE Manager allows you to override the configurations set by the application via the
 * environment variable "AWEMGR_CFG_OVERRIDE". To override one or more keys, assign the variable
 * a value formatted as "key1=value1;key2=value2;...". For example, to override certain logging
 * configurations on Linux, you can execute the following command:
 *
 *   `export AWEMGR_CFG_OVERRIDE="mgr.api.log.level=warning;mgr.awc.log.level=debug;"`
 */
 #define AWEMGR_CFG_OVERRIDE "AWEMGR_CFG_OVERRIDE"

/* ****************************************************************************
 * TYPE DEFINITIONS (PUBLIC)
 * ***************************************************************************/
/**
 * @brief Error codes returned by _AWE-Manager_ methods.
 */
enum awemgr_rc {
    awemgr_RC_COMM_FAIL = -6,       /**< the communication with AWECore failed (connection not
                                         available or lost, send or receive error) */
    awemgr_RC_ERR_DESIGN_NOTFOUND = -5, /**< the design was not found in the AWC; use this error code to check for existence of a design name */
    awemgr_RC_ERR_INVALID_VAL = -4, /**< a value for awemgr_control_write() was outside the
                                         allowed range */
    awemgr_RC_AWECORE_ERROR = -3,   /**< AWECore has returned an error */
    awemgr_RC_COMM_TIMEOUT = -2,    /**< this error code is returned when a timeout occurs while
                                         receiving data from bsp */
    awemgr_RC_ERR = -1,             /**< generic error code, e.g. an invalid argument; a failed
                                         communication is reported as awemgr_RC_COMM_FAIL */
    awemgr_RC_OK = 0,               /**< all good */
    awemgr_RC_MAX
};

/**
 * @brief Variable types exchanged when reading/writing data
 */
enum awemgr_vartype {
    AWEMGR_VARTYPE_UNSIGNED_INTEGER,    /**< 32bit unsigned integer value */
    AWEMGR_VARTYPE_INTEGER,     /**< 32bit integer value */
    AWEMGR_VARTYPE_BOOL,    /**< 32 bit integer 0 = false, 1=true */
    AWEMGR_VARTYPE_FLOAT,   /**< 32bit float */
    AWEMGR_VARTYPE_FRACT32,   /**< 32bit integer Q1.31 */
    AWEMGR_VARTYPE_FRACT16,   /**< 16bit integer Q1.15 */
    AWEMGR_VARTYPE_ENUM,

    AWEMGR_VARTYPE_UNDEF
};

struct awemgr_data;

/**
 * @brief AWE Manager internal data. (forward declaration)
 */
typedef struct awe_config awe_config;

/**
 * Data related to a selected AWC / context. (forward declaration)
 */
struct awemgr_ctx;

/**
 * @brief Attributes stored for each controllable item / variable
 *
 * Identifier of a controllable item
 * Used in `struct awemgr_ctl_elem_info`.
 */
struct awemgr_ctl_elem_id {
    unsigned int         instanceId; /**< instance/canvas ID */
    const char*          name;       /**< name of the control element */
    enum awemgr_vartype  type;       /**< specifies which variable type this is */
    unsigned int         nr_items;   /**< how many data points does this control element have;
                                          is it a scalar variable or an array */
    const char*          parentName; /**< pointer to the name of the module's name */
    const char* alias;               /**< in case the control was given an alias name by the integrator,
                                          this is the name; otherwise this value is NULL */
};

/**
 * @brief Meta-information of a controllable item/variable
 *
 * Information of an AWE controllable item; similar to the ASLA mixer control
 * data it will contain the information about value ranges, enumerations, etc.
 */
struct awemgr_ctl_elem_info {
    struct awemgr_ctl_elem_id id;
    //Set to false if the range information is not provided
    //or not supported for the control type (e.g AWEMGR_VARTYPE_ENUM)
    bool range_enabled;
    // store for range/default value specification
    union {
        struct {
            uint32_t min;
            uint32_t max;
            uint32_t step;
        } u32;
        struct {
            int32_t min;
            int32_t max;
            int32_t step;
        } i32;
        struct {
            float min;
            float max;
            float step;
        } f32;
        // todo: enums!
    } range;
    // todo: add reserved etc...
};

/**
 * @brief Plugin Structure
 *
 * Defines the name and the core of a plugin
 *
 */
struct awemgr_plugin
{
    char name[AWEMGR_MAX_PLUGIN_NAME_LEN];                              /**< This is the basename of the library file to be loaded by the BSP. It is up to the platform
                                                                             to define how this name has to be formatted, like “libXYZ.so” or “XYZ”. */
    uint8_t core;                                                       /**< core id where the plugin needs to be loaded */
};

/**
 * @brief Design Information Structure
 *
 * Information of a specific design inside an AWC.
 *
 * Typically, there is a "main" or boot design. But there might be
 * several additional preset-AWB or Subcanvas files.
 *
 */
struct awemgr_design_info
{
    const char*             name;                                       /**< name of the design */
    unsigned int            coreid_objectid;                            /**< packs the core and object id for the sub canvas (coreid << 16 | objectid), 0 otherwise */
    unsigned int            size;                                       /**< size of the file in bytes */
    const char              *md5sum;                                    /**< md5 checksum string */
    unsigned int            plugin_count;                               /**< number of required plugins for the design */
    struct awemgr_plugin    plugins[AWEMGR_MAX_PLUGINS_PER_DESIGN];     /**< array of plugins for the design */
};

/**
 * @brief AWE Manager Module Location Structure

 * Defines a "location" of a module an AWE design.
 *
 * This information is returned in calls to awemgr_get_module() or awemgr_get_module_by_name().
 *
 * @todo NAMING! clarify forCore = (endPointID << 4) | ((handle & 0x780) >> 7);
 */
typedef struct awemgr_module {
    const char  *name;        /**< name as in module's name property (see AWE Modules);
                                   points to an internally allocated string; may be NULL */
    const char *alias;        /**< in case the module was given an alias name by the integrator,
                                   this is the name; otherwise this value is NULL */
    unsigned int instanceId;  /**< instance/canvas ID */
    unsigned int objectId;    /**< object ID of the module - same as in ControlInterface.h */
    unsigned int classId; /**< type of module */
} awemgr_module;

/**
 * @brief AWE Core Module Runtime States

 * Querying or setting the runtime state of a module uses these
 * module state values; please refer to AWE Core documentation
 * for more details.
 */
enum awemgr_module_runtimestate {
    MODULE_ACTIVE = 0,
    MODULE_BYPASS,
    MODULE_MUTED,
    MODULE_INACTIVE,

    MODULE_RUNTIME_UNKOWN
};

/**
 * Structure to return texts for the awemgr_module_runtimestate values.
 */
extern const char* awemgr_module_runtimestate_as_string[MODULE_RUNTIME_UNKOWN];

/**
 * @brief AWE Manager Variable Location Structure
 *
 * Defines a "location" of a variable inside an AWE design. This struct contains all information
 * to route specific information to that variable.
 * This information is returned in calls to awemgr_get_variable() or awemgr_get_variable_by_name().
 *
 * @todo NAMING! clarify forCore = (endPointID << 4) | ((handle & 0x780) >> 7);
 */
struct awemgr_variable {
    int    instanceId; ///< instance/canvas ID
    int    handle;     ///< module handle/address - same as in ControlInterface.h
    char  *name;       ///< name as in variables's name property (see AWE Modules); points to an internally allocated string; may be NULL
};

/**
 * @brief Event structure returned from AWE sub-system
 *
 * Structure returned upon an event originating from the AWE sub-system;
 * also used to configure the subscription of events, i.e., the client can use this structure
 * to update a list of event filters (masks) to report on.
 */
typedef struct awemgr_event {
    awemgr_module   module;        ///< origin of the event when it comes from the signal flow (EventModule)
    uint32_t    eventType;         ///< type of event as returned from EventModule in signal flow
    uint32_t    eventCategory;     ///< ID or category of the eventType; definition is up to the system integration
    uint64_t    timeStamp;         ///< system specific time stamp or sysTick() count
    uint32_t    sizeInBytes;       ///< size of payload
    const char* payload;           ///< pointer to memory containing the event data; the calling application may not assume ownership of this memory!
} awemgr_event;

/**
 * @brief Prototype of a callback invoked when an AWE EventModule has triggered during processing/pumping.
 *
 * Note: it is up to the client code to dispatch the events to specific listeners or to enqueue the event data
 *
 * @param ev - Carries the event information
 * @param userdata - Pointer to user specific data
 *
 * @return void
 */
typedef void (*event_cb_t)(const awemgr_event* ev, void* userdata);


/**
 * Callback type for an observer to the .
 *
 * Called once for TX (before the packet is sent) and once for RX (after the
 * response is received).  Both calls happen synchronously inside the same
 * manager API call, so the shell can print TX/RX blocks in order.
 *
 * @param direction  "TX" or "RX"
 * @param data       Pointer to the raw word buffer
 * @param data_sz_words  Number of 32-bit words in @p data
 * @param ctx        Opaque context supplied to awecomm_set_observer()
 */
typedef void (*comm_observer_cb_t)(const char *direction,
                                   const void *data,
                                   int         data_sz_words,
                                   void       *ctx);

/**
 * @brief User Data Types
 *
 * Type of the Module/Control User data type.
 * Supported types: int, uint, str, float
 */
typedef enum
{
    AWEMGR_USRDATA_INT = 0,
    AWEMGR_USRDATA_UINT,
    AWEMGR_USRDATA_STR,
    AWEMGR_USRDATA_FLOAT,
    AWEMGR_USRDATA_TYPE_MAX,
} awemgr_userdata_type;

/**
 * @brief Union representing Module/Control user data values.
 *
 * The value can be accessed as one of the following types: uint32_t, int32_t, float, or const char*.
 *
 * Note:
 * - If accessed as `const char*`, the string's memory is managed internally by `AWE-Manager`.
 * - Do not free or modify the memory pointed to by the `const char*`.
 */
typedef union {
    uint32_t u32;
    int32_t i32;
    float f32;
    const char* str;
} awemgr_userdata_value;

/**
 * @brief Holds arbitrary information an application can attach to modules, variables or designs.
 *
 * Structure encapsulating the user data.
 * Contains the key, type and value of the control/module userdata.
 */
typedef struct
{
    const char* key;
    awemgr_userdata_type type;
    awemgr_userdata_value value;
} awemgr_userdata;


/* ****************************************************************************
 * PUBLIC VERSION
 * ***************************************************************************/
/**
 * @brief Retrieves the version information for AWE-Manager library
 *
 * @return A character pointer to a (git) version string.
 *         Semantic versioning is used. See changelog for more details.
 */
const char *awemgr_get_version(void);

/* ****************************************************************************
 * CONSTRUCT / DESTRUCT
 * ***************************************************************************/

/**
 * @brief Initializes _AWE-Manager_ component; allocates internally needed memory
 * and the communication to "carvedoutShMem" (DSP IF) (or the Audio Processors)
 *
 * Note: on PC the communication via "ShMem" is substituted by a socket to
 * AWE-Server instead
 *
 * @param cfg_pp[in/out] - Address of a pointer to an initialized AWE manager configuration. This has to be
 *                         created with awemgr_config_create() before. If NULL is passed, then AWE-Manager will
 *                         internally allocate such structure and will be initialized with default configurations.
 * @param mgr_pp[in/out] - Address of a pointer to an AWE Manager handle, this ptr
 *                         may be used in further API calls.
 *
 * @return error code
 */
enum awemgr_rc awemgr_init(awe_config** cfg_pp, struct awemgr_data **mgr_pp);

/**
 * @brief De-initialize AWE-Manager; frees internally allocation memory and other resources.
 *
 * @param mgr_pp[out] - Address of a pointer to an AWE Manager handle
 *
 * @return error code
 */
enum awemgr_rc awemgr_exit(struct awemgr_data **mgr_pp);

/* ****************************************************************************
 * ADMINISTRATIVE
 * ***************************************************************************/
/**
 * @brief Calling this API stops audio processing(Stop Audio Interrupts/Halts Audio pumping)
 *
 * @param ctx_p[in] - Handle of an AWC context
 *
 * @return error code
 */
enum awemgr_rc awemgr_audio_stop(struct awemgr_ctx *ctx_p);

/**
 * @brief Calling this API starts Audio processing(Start Audio Interrupts/Resumes Audio pumping)
 *
 * @param ctx_p[in] - Handle of an AWC context
 *
 * @return error code
 */
enum awemgr_rc awemgr_audio_start(struct awemgr_ctx *ctx_p);

/**
 * @brief Loads AWC "meta" data into an endpointId.
 *
 * After this call AWE Manager is able to return information about the control or device
 * elements of the design included in the AWC data.
 *
 * Loading returns an endpoint ID in the variable pointed to by `pEndpointId`.
 * This ID may later be used with awemgr_get_awc_context() to obtain an AWC context handle .
 * The index is 0 based and corresponds to the endpoint value typically used
 * in AWE integration.
 *
 * The AWC context handle is then used to finally set or get information from the
 * running AWE Core instances.
 *
 * @param mgr_p[in] - AWE Manager handle
 * @param awcFileName[in] - Path to an AWC index file; support for Windows and Un*x path delimiters available
 * @param endpointId[in] - endpointId to load the AWC, valid values are between 0 - 15
 *
 * @return error code
 *
 * @see awemgr_get_awc_context()
 */
enum awemgr_rc awemgr_load_awc(struct awemgr_data *mgr_p, const char *awcFileName, int endpointId);

/**
 * @brief Removes specific AWC data from AWE Manager
 *
 * After this call the available slot inside AWE Manager is free to load another AWC context.
 *
 * @param mgr_p[in] - AWE Manager handle
 * @param endpointId[out] - endpointId of the AWC to unload
 *
 * @return error code
 */
enum awemgr_rc awemgr_unload_awc(struct awemgr_data *mgr_p, int endpointId);

/**
 * @brief Reports how many AWC contexts (or AWE endpoints) are loaded/initialized in AWE-Manager.
 *
 * The AWE Manager interface foresees that several endpoints may be handled in parallel.
 *
 * The deployment of the designs are handled by integrator and by AWE Manager.
 * They are addressed by an application simply by index.
 *
 * @param mgr_p[in] - Handle of an AWE Manager
 *
 * @returns Number of endpoints into which an AWC is already loaded
 *
 * @see awemgr_get_awc_context
 */
int awemgr_get_loaded_awc_count(struct awemgr_data *mgr_p);

/**
* @brief Reports how many AWC contexts can be supported on the platform

* @returns Number of max designs on the platform
*
*/
int awemgr_get_max_awcs();

/**
 * @brief Returns a handle to an AWC context that was loaded before with awemgr_load_awc().
 *
 * Returns a handle to an AWC context that was loaded before with
 * awemgr_load_awc(). Use this handle in all other administrative or
 * data controlling API calls.
 *
 * @param mgr_p[in] - Handle of an AWE Manager
 * @param endpointId[in] - Index of an AWC context (= endpointId)
 *
 * @returns A handle to an AWC context
 *
 * @see awemgr_load_awc()
 */
struct awemgr_ctx * awemgr_get_awc_context(struct awemgr_data *mgr_p, int endpointId);

/**
 * @brief Selects an AWE design or signal flow data. Those are stored as AWB data.
 *
 * The AWB data is obtained from the AWC and sent to the DSPs.
 *
 * This call is used for both, the "main" or functional signal flow which provide the audio features
 * as well as for the tuning-AWB or preset-AWBs which fine tune already loaded functional
 * signal flows.
 *
 * Loading a (main) AWE signal flow typically also starts the audio processing.
 * This kind of signal flow is typically applied early on. After the call to awemgr_load_design()
 * the applications can use the other runtime control functions.
 *
 * Typically Preset-AWBs are applied after the main AWE signal flow has been loaded
 * to fine-tune the main signal flow to a specific car-line.
 * But Preset-AWBs may also be applied often, for example on use-case switches.
 *
 * Preset-AWBs are selected by an application by name. It is the task of the system
 * integrator to store the list of preset AWB files in AWC. The integrator also has
 * to ensure there is only one AWB connected to `designName` (in case there are
 * several AWCs on the platform).
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param designName[in] - Identifies which AWB to load from any of the AWC,
 *                         if not given (NULL) the main AWB(s) will be loaded.
 *
 * @return error code
 *
 */
enum awemgr_rc awemgr_load_design(struct awemgr_ctx *ctx_p, const char *designName);

/**
 * @brief Stops audio processing for the given AWE design so that no CPU is consumed anymore.
 *
 * Note that the AWE core instance running the design is NOT destroyed.
 *
 * It is possible to unload specific Subcanvas designs by supplying the design's name.
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param designName[in] - Name of the design to unload; if NULL, the main design will be unloaded.
 *
 * @return error code
 */
enum awemgr_rc awemgr_unload_design(struct awemgr_ctx *ctx_p, const char *designName);

/**
 * @brief Marks that upon awemgr_exit() the design shall NOT be destroyed.
 *
 * This marks the AWC context so that that upon call to awemgr_exit()
 * AWE-Manager will not shut-down the design (i.e. not send a PFID-Destroy to AWECore)
 * and will only remove itself from memory. The signal flow running in AWE Core
 * will continue to process audio.
 *
 * When AWE-Manager exits this way (without destroy) the behavior is
 * similar to AWE Designer's "Tool->Detach from target".
 *
 * @param ctx_p[in] - Handle of an AWC context
 *
 * @return error code
 */
enum awemgr_rc awemgr_skip_unload_design_on_exit(struct awemgr_ctx *ctx_p);


/**
 * @brief This method sends a special command to AWE-Core processing which is
 * interpreted by the implemented software (BSP) rather than AWE-Core itself.
 *
 * Typical commands include the instruction to load a (shared) library to extends
 * the capabilities of AWE-Core processing at runtime, like getting access to
 * additional (third-party) modules without the need to re-compile (DSP) software.
 *
 * This method should typically be used before awemgr_load_design() is invoked.
 *
 * NOTE: The payload size of this command is (currently) limited to the size of the
 * communication buffer on the target (typically 264 words).
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param cmdId[in] - the command ID of the command - platform/integration specific
 * @param coreId[in] - Selects the core on which the AWE-Core instance runs, like
 *                     0=ADSP, 1=GPDSP0, ..., 3=ARM (those IDs are defined in the system configuration)
 * @param payload[in] - Pointer to a payload structure for the cmdId - platform/integration specific
 * @param data_sz_in_words[in] - Size of the payload structure
 * @param response_buffer[in] - address of a buffer to fill with the response of the command (can be NULL)
 * @param response_buffer_size_in_words[in] - maximum size of the response buffer
 * @param response_buffer_filled_p[out] - Pointer to a variable that will receive the number of word copied into the response buffer
 *
 * @return error code
 */
enum awemgr_rc awemgr_send_command(struct awemgr_ctx *ctx_p, int cmdId, unsigned int coreId,
    void *payload, unsigned int data_sz_in_words,
    void *response_buffer, unsigned int response_buffer_size_in_words, unsigned int *response_buffer_filled_p);


/**
 * @brief Returns the information of an AWE Instance.
 *
 * This fills the response buffer with binary encoded information about the AWE Instances on the target
 *
 * @param mgr_p[in] - Handle of awe manager
 * @param info_buffer[out] - will be updated to contain information structures of all AWE instances
 *
 * @return error code
 *
 */
enum awemgr_rc  awemgr_get_target_info(struct awemgr_data* mgr_p, awemgr_targetinfo* info_buffer);


/**
 * @brief Returns the information about the current CPU load.
 *
 * @param mgr_p[in] - Handle of awe manager
 * @param endpointId[in] - Query a specific endpoint/canvas, `0` to `MAX_AWE_ENDPOINTS-1`
 * @param coreId[in] - Selects the core on which the AWE-Core instance runs (on a multi-core system), `0` to `MAX_AWE_CORES-1`
 * @param info_buffer[out] - will be updated to contain information structures of all AWE instances
 *
 * @return error code
 *
 */
enum awemgr_rc  awemgr_get_cpu_info(struct awemgr_data* mgr_p, int endpointId, int coreId, awemgr_cpuinfo* info_buffer_p);

/**
 * @brief Returns the information about which modules are available for AWE-Core.
 *
 * This method can be used to query which AWE modules have been compiled in or are available
 * at the AWE Core instance.
 *
 * @param mgr_p[in] - Handle of awe manager
 * @param endpointId[in] - Query a specific endpoint/canvas, `0` to `MAX_AWE_ENDPOINTS-1`
 * @param coreId[in] - Selects the core on which the AWE-Core instance runs (on a multi-core system), `0` to `MAX_AWE_CORES-1`
 * @param info_buffer[out] - will be updated to contain information structures of all AWE instances
 *
 * @return error code
 *
 */
enum awemgr_rc  awemgr_get_modulelist_info(struct awemgr_data* mgr_p, int endpointId, int coreId, awemgr_modulelist_info* info_buffer_p);

/**
 * @brief Returns information about the memory (heap) used by AWE Core processing.
 *
 * When this method is called it queries AWE Core processing to retrieve the memory used by the
 * AWE processing system. The returned structure is filled with the information about fast-A, fast-B
 * slow and shared heap memories.
 *
 * @param mgr_p[in] - Handle of awe manager
 * @param endpointId[in] - Query a specific endpoint/canvas, `0` to `MAX_AWE_ENDPOINTS-1`
 * @param coreId[in] - Selects the core on which the AWE-Core instance runs (on a multi-core system), `0` to `MAX_AWE_CORES-1`
 * @param info_buffer[out] - will be updated with information about the (heap) memory allocation
 *
 * @return error code
 *
 */
enum awemgr_rc awemgr_get_heap_info(struct awemgr_data* mgr_p, int endpointId, int coreId, awemgr_heapinfo* info_buffer_p);

/**
 * @brief Returns information about a specific or about all layouts.
 *
 * @param mgr_p[in] - Handle of awe manager
 * @param endpointId[in] - Query a specific endpoint/canvas, `0` to `MAX_AWE_ENDPOINTS-1`
 * @param coreId[in] - Selects the core on which the AWE-Core instance runs (on a multi-core system), `0` to `MAX_AWE_CORES-1`
 * @param info_buffer[out] - will be updated to contain information structures of all AWE instances
 *
 * @return error code
 *
 */
enum awemgr_rc  awemgr_get_layout_info(struct awemgr_data* mgr_p, int endpointId, int coreId, awemgr_layoutinfo* info_buffer_p);


/**
 * This is the most basic method to control a signal flow or control a module.
 * It expects a fully compatible AWE command buffer as described in
 * https://w.dspconcepts.com/hubfs/Docs-AWECore/AWECore_API_Doc/a00085.html
 *
 * @param mgr_p[in] - Handle of awe manager
 * @param request_buffer[in] - Pointer to an AWE message tuning buffer; this buffer is supposed to contain a fully
 *                             and properly encoded AWE command message
 * @param request_buffer_size[in] - Number of words in the request_buffer
 * @param response_buffer[in] - Pointer to a data buffer which will be filled with information about the AWE Instance
 * @param response_buffer_size[in] - Size of the response buffer (in words)
 */

enum awemgr_rc  awemgr_transact(struct awemgr_data* mgr_p, void* request_buffer, int request_buffer_size, void* response_buffer, int response_buffer_size);

/**
 * @brief Install (or remove) a raw-buffer tap on all comm transactions.
 *
 * When set, @p cb is called once for TX (after sending) and once for RX
 * (after receiving) on every transaction, in addition to any existing trace
 * log / file output.  The calls happen synchronously inside the manager API
 * call that triggered the transaction, so a command handler can print TX/RX
 * blocks in order before printing its own result.
 *
 * Pass @p cb = NULL to remove the tap.
 *
 * @param mgr_p[in]  Handle of awe manager
 * @param cb[in]     Tap callback, or NULL to clear
 * @param ctx[in]    Opaque context forwarded to @p cb
 */
void awemgr_set_comm_observer(struct awemgr_data *mgr_p, comm_observer_cb_t cb, void *ctx);

/**
 * @brief Sends a command to AWE-Core to reset all internal state variables and masks.
 *
 * Issue this command when the system has stopped processing audio and is supposed
 * to go into a deep sleep mode.
 *
 * Note that the reset instruction is passed on to any Subcanvas signal flow
 * currently loaded in this AWE instance.
 *
 * @param mgr_p[in] - Handle of awe manager
 *
 * @return error code
 */
enum awemgr_rc awemgr_reset_state_all(struct awemgr_data* mgr_p);


/* ****************************************************************************
 * ENUMERATION OF DEVICES/CONTROLS/DESIGNS/EVENTS
 * ***************************************************************************/
/**
 * @brief Returns the number of controllable items for a specific AWE design.
 *
 * @param ctx_p[in] - Handle of an AWC context
 *
 * @return number of controls for this specific AWE design (context).
 *         In case there is an (internal) error, -1 is returned.
 */
int  awemgr_get_controls_count(struct awemgr_ctx *ctx_p);

/**
 * @brief Retrieves information about a specific control item
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param index[in] - Index (0 based) which is used to get a specific control item
 * @param info[out] - pointer to a structure to be filled
 *
 * @return error code
 */
enum awemgr_rc  awemgr_get_control_info(struct awemgr_ctx *ctx_p, int index, struct awemgr_ctl_elem_info *info);

/**
 * @brief Returns the number of designes the AWC handles.
 *
 * This returns the number of designs ("AWB files") the AWC knows about.
 *
 * @param ctx_p[in] - Handle of an AWC context
 *
 * @return number of designs; values is >= 1.
 *         In case there is an (internal) error, -1 is returned.
 */
int  awemgr_get_design_count(struct awemgr_ctx *ctx_p);


/**
 * @brief Retrieves information about a specific design
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param index[in] - Index (0 based) which is used to get a specific design information
 * @param info[out] - pointer to a structure to be filled
 *
 * @return error code
 */
enum awemgr_rc  awemgr_get_design_info(struct awemgr_ctx *ctx_p, int index, struct awemgr_design_info *info);


/**
 * @brief Retrieves information about a specific design, given its name.
 *
 * This is the name based counterpart of awemgr_get_design_info(). It saves iterating
 * over all designs when the name is known, e.g. from the AWC index file, but the
 * index is not.
 *
 * The name is resolved the same way as in awemgr_load_design(), so a name accepted
 * there is accepted here as well.
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param design_name[in] - name of the design, as it is stored in the AWC
 * @param info[out] - pointer to a structure to be filled
 *
 * @return error code; `awemgr_RC_ERR_DESIGN_NOTFOUND` in case the design name is not
 *         known in the AWC, which is a convenient way to check for its existence.
 */
enum awemgr_rc  awemgr_get_design_info_by_name(struct awemgr_ctx *ctx_p, const char *design_name, struct awemgr_design_info *info);


/**
 * @brief Retrieves information about a specific control element, given its name or path.
 *
 * A control name or path is the name of a controllable module or variable in the AWE design.
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param control_name[in] - name of the control item
 * @param info[out] - pointer to a structure to be filled
 *
 * @return error code
 */
enum awemgr_rc  awemgr_get_control_info_by_name(struct awemgr_ctx *ctx_p, const char *control_name, struct awemgr_ctl_elem_info *info);

/**
 * @brief Returns the number of event modules in the AWC.
 *
 * @param ctx_p[in] - Handle of an AWC context
 *
 * @return number of events; values is >= 0.
 *         In case there is an (internal) error, -1 is returned.
 */
int  awemgr_get_event_count(struct awemgr_ctx *ctx_p);

/**
 * @brief Retrieves information about a specific event module by its index.
 * This api can be used by applications conjunction with awemgr_get_event_count.
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param index[in] - Index (0 based) which is used to get a specific design information
 * @param mod_p[out] - pointer to a module structure to be filled
 *
 * @return error code
 */
enum awemgr_rc  awemgr_get_event_by_index(struct awemgr_ctx *ctx_p, int index, struct awemgr_module *mod_p);

/**
 * @brief Searches the module with name and returns the number of its user data elements
 * @param ctx_p[in] - Handle of an AWC context
 * @param modulename name of the module.
 * @return number of userdata, <0 = Error
 */
int awemgr_get_module_userdata_count(struct awemgr_ctx *ctx_p, const char* modulename);

/**
 * @brief Searches the control with name and returns the number of its user data elements
 * @param ctx_p[in] - Handle of an AWC context
 * @param controlname name of the module.
 * @return number of userdata, <0 = Error
 */
int awemgr_get_control_userdata_count(struct awemgr_ctx *ctx_p, const char* controlname);

/**
 * @brief Searches the module with name and returns the module user data with index
 * @param ctx_p[in] - Handle of an AWC context
 * @param modulename name of the module.
 * @param index index of the user data.
 * @param data[out] - pointer to a structure to be filled
 * @return error code
 */
enum awemgr_rc awemgr_get_module_userdata_by_index(struct awemgr_ctx *ctx_p, const char* modulename, uint32_t index, awemgr_userdata* data);

/**
 * @brief Searches the control with name and returns the control user data with index
 * @param ctx_p[in] - Handle of an AWC context
 * @param controlname name of the control.
 * @param index index of the user data.
* @param data[out] - pointer to a structure to be filled
 * @return error code
 */
enum awemgr_rc awemgr_get_control_userdata_by_index(struct awemgr_ctx *ctx_p, const char* controlname, uint32_t index, awemgr_userdata* data);

/**
 * @brief Searches the module with name and returns the module user data with matching key
 * @param handle awc handle.
 * @param modulename name of the module.
 * @param key key of the user data.
 * @param data[out] - pointer to a structure to be filled
 * @return error code
 */
enum awemgr_rc awemgr_get_module_userdata_by_key(struct awemgr_ctx *ctx_p, const char* modulename, const char* key, awemgr_userdata* data);

/**
 * @brief Searches the control with name and returns the control user data with matching key
 * @param ctx_p[in] - Handle of an AWC context
 * @param controlname name of the control.
 * @param key key of the user data.
 * @param data[out] - pointer to a structure to be filled
 * @return error code
 */
enum awemgr_rc awemgr_get_control_userdata_by_key(struct awemgr_ctx *ctx_p, const char* controlname, const char* key, awemgr_userdata* data);


/* ****************************************************************************
 * CONTROL
 * ***************************************************************************/

/**
 * Write control data given by an application to the AWE design running under the chosen context.
 * It uses a name lookup inside _AWE-Manager_ to obtain the appropriate AWE module variable handle and size.
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param control_name[in] - name of the control item
 * @param offset[in] - If control item is an array, specify at which position `data` shall be written
 * @param data[in] - pointer to data to be written to the control
 * @param data_sz[in] - size/length of the control data to be written; size is in number of words
 *
 * @return error code
 */
enum awemgr_rc  awemgr_control_write(struct awemgr_ctx *ctx_p, const char *control_name, unsigned int offset, void *data, unsigned int data_sz);

/**
 * Queries an AWE control module/variable and returns its data into the response buffer.
 * It is up to the caller to interpret the data.
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param control_name[in] - name of the control item
 * @param response_buffer[in] - pointer to where the response data from AWE module is written
 * @param response_buffer_size[in] - size of the response_buffer (in number of words)
 * @param response_buffer_filled[out] - will be updated to contain number of words written into `response_buffer`
 * @param type_p[out] - ptr to a variable that will receive the information which data type has been retrieved
 *
 * @return error code
 */
enum awemgr_rc  awemgr_control_read(struct awemgr_ctx *ctx_p, const char *control_name, void *response_buffer, unsigned int response_buffer_size, unsigned int *response_buffer_filled, enum awemgr_vartype *type_p);


/**
 * Similar to awemgr_control_read() this allows to read an AWE variable's data, with the exception
 * that only parts of the variable can be obtained.
 *
 * The parameters `offset` and `nr_words_to_read`, both specified in number of words, determine
 * at which offset position and how many words will be retrieved.
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param offset[in] - name of the control item
 * @param nr_words_to_read[in] - name of the control item
 * @param control_name[in] - name of the control item
 * @param response_buffer[in] - pointer to where the response data from AWE module is written
 * @param response_buffer_size[in] - size of the response_buffer (in number of words)
 * @param response_buffer_filled[out] - will be updated to contain number of words written into `response_buffer`
 * @param type_p[out] - ptr to a variable that will receive the information which data type has been retrieved
 *
 * @return error code
 */
enum awemgr_rc  awemgr_control_read_partial(struct awemgr_ctx *ctx_p,
    const char *control_name,
    unsigned int  offset,
    unsigned int  nr_words_to_read,
    void *response_buffer,
    unsigned int response_buffer_size,
    unsigned int *response_buffer_filled,
    enum awemgr_vartype *type_p
);


/* ****************************************************************************
 * CONTROL - MODULES - module specific modifications/control
 * ***************************************************************************/

/**
 * @brief Returns the number of controllable modules. This also includes event modules!
 *
 * @param ctx_p[in] - Handle of an AWC context
 *
 * @return number of modules for this specific AWE design (context).
 *         In case there is an (internal) error, -1 is returned.
 *
 */
int  awemgr_get_modules_count(struct awemgr_ctx *ctx_p);

/**
 * @brief Retrieves a handle to a controllable AWE module given its name or path.
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param module_name[in] - Name of the module to search
 * @param mod[out] - Pointer to a variable receiving a module handle
 *
 * @return error code
 *
 */
enum awemgr_rc  awemgr_get_module_by_name(struct awemgr_ctx *ctx_p, const char *module_name, struct awemgr_module *mod);

/**
 * @brief Retrieves a handle to a controllable AWE module given its index.
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param index[in] - Index (0 based) which is used to get a specific module
 * @param mod_p[out] - Pointer to a variable receiving a module handle
 *
 * @return error code
 *
 */
enum awemgr_rc  awemgr_get_module_by_index(struct awemgr_ctx *ctx_p, int index, struct awemgr_module *mod_p);

/**
 * @brief Modifies the operating state of a specific AWE controllable module.
 *
 * The operating state of a module can be changed to e.g. set it to BYPASS or to MUTE.
 *
 * The method requires that the module's handle has been obtained before with awemgr_get_module_by_name().
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param mod[in] - handle of an AWE module, see awemgr_get_module_by_name()
 * @param operating_state[in] - Enumeration value for the new operating state
 *
 * @return error code
 *
 */
enum awemgr_rc awemgr_module_set_state(struct awemgr_ctx *ctx_p, struct awemgr_module mod, enum awemgr_module_runtimestate operating_state);


/**
 * @brief Returns the operating state of a specific AWE controllable module.
 *
 * The method requires that the module's handle has been obtained before with awemgr_get_module_by_name().
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param mod[in] - handle of an AWE module, see awemgr_get_module_by_name()
 * @param state_p[out] - pointer to variable to receive the operating state value.
 *
 * @return error code
 */
enum awemgr_rc awemgr_module_get_state(struct awemgr_ctx *ctx_p, struct awemgr_module mod, enum awemgr_module_runtimestate *state_p);


/**
 * @brief Gets the class ID of a specific AWE controllable module.
 *
 * The class ID of an AWE module specifies which kind of module it is. The method requires
 * that the module's handle has been obtained before with awemgr_get_module_by_name().
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param mod[in] - handle of an AWE module, see awemgr_get_module_by_name()
 * @param classId_p[out] - pointer to variable to receive the class ID value. This typically is a value
 *                         like 0xBEEF086E.
 *
 * @return error code
 */
enum awemgr_rc awemgr_module_get_class(struct awemgr_ctx *ctx_p, struct awemgr_module mod, unsigned int *classId_p);



#ifdef POSSIBLY_FUTURE_EXTENSION

/**
 * Fills the handle for a variable inside an AWE design.
 *
 * Basically, this call does nothing else than to re-pack the parameters `handle` and `ìnstanceId`,
 * so that other functions can work with this handle.
 *
 * If the `ìnstanceId` parameter does not fit to `struct awemgr_ctx`, an error is returned.
 *
 * @param ctx - handle of an AWC context
 * @param designIdx[in] - the specifier for the design the variable described by `handle` lives in
 * @param handle[in] - the identifier and handle for a variable just like mentioned in ControlInterface.h
 *                     file generated by Designer.
 * @param var[out] - pointer to a struct to be filled
 *
 * @return error code
 *
 */
enum awemgr_rc  awemgr_get_variable(struct awemgr_ctx *ctx_p, int designIdx, int handle, struct awemgr_variable *var);

/**
 * Fills the handle for a variable inside an AWE design based on a name.
 *
 * It looks up the name in AWC and fills in the data into `struct awemgr_variable`.
 * A control name or path is the name of a controllable module or variable in the AWE design.
 *
 * @param ctx - handle of an AWC context
 * @param control_name[in] - name/path of a variable
 * @param var_p[out] - pointer to a variable info handle
 *
 * @return error code
 *
 */
enum awemgr_rc  awemgr_get_variable_by_name(struct awemgr_ctx *ctx_p, const char *control_name, struct awemgr_variable *var_p);

/**
 * ...
 *
 * @param var - handle of an AWE variable
 * @param offset[in] - ...
 * @param data[in] - ...
 * @param data_sz[in] - ...
 * @param noSetCall[in] - ...
 *
 * @return error code
 *
 */
enum awemgr_rc  awemgr_variable_write(struct awemgr_ctx *ctx_p, struct awemgr_variable var, int offset, void *data, int data_sz, bool noSetCall);

/**
 * ...
 *
 * @param var - handle of an AWE variable
 * @param offset[in] - ...
 * @param response_buffer[in] - ...
 * @param response_buffer_size[in] - ...
 *
 * @return error code
 *
 */
enum awemgr_rc  awemgr_variable_read(struct awemgr_ctx *ctx_p, struct awemgr_variable var, int offset, void *response_buffer, int response_buffer_size);
#endif

/* ****************************************************************************
 * EVENTS
 * ***************************************************************************/
/**
 * @brief Starts the events backend and registers the callback
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param defaultCb[in] - Callback function for receiving event information from AWE system.
 *                         When category specific callback is not registered, then event dispatcher calls this function if not NULL.
 * @param userdata[in] - Pointer to user specific data
 *
 * @return error code
 */
enum awemgr_rc awemgr_events_start(struct awemgr_ctx *ctx_p, event_cb_t defaultCb, void* userdata);

/**
 * @brief Returns the information if the events backend has been started
 *
 * @param ctx_p[in] - Handle of an AWC context
 *
 * @return boolean, true if events backend is up and running
 */
bool awemgr_events_started(struct awemgr_ctx *ctx_p);

/**
 * @brief Registers a callback specific to an event category.
 *
 * Any event with category > 31 will be dispatched via default callback provided in awemgr_events_start.
 * Calling this API again will overwrite the existing handler.
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param category[in] - Event Category between 0 to category 31.
 * @param cb[in] - Callback function for receiving event information from AWE system
 * @param userdata[in] - Pointer to user specific data
 *
 * @return error code
 */
enum awemgr_rc awemgr_events_add_category_listener(struct awemgr_ctx *ctx_p, uint32_t category, event_cb_t cb, void* userdata);

/**
 * @brief Registers a passive inspector callback that is always called after the primary listener.
 *
 * Unlike the default and category callbacks, the inspector does not replace any existing callback —
 * it fires for every event regardless of category, after the primary listener (if any) has been called.
 * Intended for tools such as the shell addon that need to observe all events without interfering
 * with the application's own event handling.
 * Calling this API again will overwrite the existing inspector.
 *
 * @param ctx_p[in]   - Handle of an AWC context
 * @param cb[in]      - Inspector callback function
 * @param userdata[in]- Pointer to user specific data
 *
 * @return error code
 */
enum awemgr_rc awemgr_events_set_listener(struct awemgr_ctx *ctx_p, event_cb_t cb, void* userdata);

/**
 * @brief Deregisters a previously registered inspector callback.
 *
 * Identified by the callback pointer, so each addon can remove only its own registration.
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param cb[in]    - The callback pointer that was passed to awemgr_events_set_listener
 *
 * @return error code
 */
enum awemgr_rc awemgr_events_clear_inspector(struct awemgr_ctx *ctx_p, event_cb_t cb);

/**
 * @brief Deregisters a callback specific to an event category.
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param category[in] - Event Category between 0 to category 31.
 *
 * @return error code
 */
enum awemgr_rc awemgr_events_remove_category_listener(struct awemgr_ctx *ctx_p, uint32_t category);

/**
 * @brief Stops the events backend and deregisters the callback
 *
 * @param ctx_p[in] - Handle of an AWC context
 *
 * @return error code
 */
enum awemgr_rc awemgr_events_stop(struct awemgr_ctx *ctx_p);

/**
 * @brief Waits and Blocks for the next event, and calls the event callback function when an event occurs
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param timeoutMs[in] - 0 = Blocking mode; > 0 to return when no event was indicated within the given time.
 *
 * @return error code
 */
enum awemgr_rc  awemgr_events_process_next(struct awemgr_ctx *ctx_p, uint32_t timeoutMs);

/**
 * @brief Subscribe to a specific event module.
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param module_name[in] - Event module full Path/Name
 *
 * @return error code
 */
enum awemgr_rc  awemgr_enable_event(struct awemgr_ctx *ctx_p, const char* module_name);

/**
 * @brief Unsubscribe from a specific event module.
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param module_name[in] - Event module full Path/Name
 *
 * @return error code
 */
enum awemgr_rc  awemgr_disable_event(struct awemgr_ctx *ctx_p, const char* module_name);

#ifdef EVENT_API_WITH_TYPE // TODO: Enable this when the type information is available in the awc_index file
/**
 * Instructs _AWE-Manager_ and AWE Core to start listening for events originating
 * from AWE modules. Once an event occurs, it will be propagated via a
 * callback function to the client.
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param type[in] - Event type as specified in the event module argument in the designer
 *
 * @return error code
 */
enum awemgr_rc  awemgr_enable_event_type(struct awemgr_ctx *ctx_p, unsigned int type);

/**
 * Unsubscribe from a specific event type.
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param type[in] - Event type as specified in the event module argument in the designer
 *
 * @return error code
 */
enum awemgr_rc  awemgr_disable_event_type(struct awemgr_ctx *ctx_p, unsigned int type);
#endif


/*****************************************************************************
 * UTIL
 ****************************************************************************/
/**
 * Utility function to return a string given a variable type enumeration
 *
 * @param v[in] - variable type to be printed
 * @returns String for the variable type
 */
static inline const char* awemgr_vartype_to_string(enum awemgr_vartype v)
{
    switch (v)
    {
        case AWEMGR_VARTYPE_UNSIGNED_INTEGER: return "unsigned_integer";
        case AWEMGR_VARTYPE_INTEGER: return "integer";
        case AWEMGR_VARTYPE_FLOAT: return "float";
        case AWEMGR_VARTYPE_FRACT32: return "fract32";
        case AWEMGR_VARTYPE_FRACT16: return "fract16";
        case AWEMGR_VARTYPE_BOOL: return "bool";
        case AWEMGR_VARTYPE_ENUM: return "enum";
        default:
            return "UNDEF";
    }
}

/**
 * Utility function to return a string given a user variable type
 *
 * @param type[in] - variable type
 * @returns String for the variable type
 */
static inline const char* awemgr_userdata_type_to_string(awemgr_userdata_type type)
{
    switch(type)
    {
        case AWEMGR_USRDATA_INT: return "INTEGER";
        case AWEMGR_USRDATA_UINT: return "UNSIGNED_INTEGER";
        case AWEMGR_USRDATA_STR: return "STRING";
        case AWEMGR_USRDATA_FLOAT: return "FLOAT";
        default: return "UNDEFINED";
    }
}

#ifdef POSSIBLY_FUTURE_EXTENSION

/* ****************************************************************************
 * AUDIO
 * ***************************************************************************/

/**
 * Returns a handle to the PCM audio device (AWE ALSA handle), given the device's name.
 *
 * NOTE: this might be an optional/alternative way to retrieve a handle to a "PCM device module".
 * Doing it this way is very similar to obtaining a handle to a "controllable module"
 * as done above. As discussed in QC Architecture call 24/05/16, also those "ALSA" devices
 * could be stored and enumerated in AWC. This way they could be queried and enumerated in the same
 * manner as other controls.
 *
 * After the handle is available, it can be used with any "AWE ALSA" function call
 * to handle the exchange of read/write pointer positions.
 *
 * @param ctx_p[in] - Handle of an AWC context
 * @param dev_name[in] - Name of the ALSA device module in the AWE design
 * @param pcm_ctl[out] - pointer to a handle for a PCM audio device (ALSA module); will be updated.
 */
enum awemgr_rc  awemgr_ctx_get_pcm_dev(struct awemgr_ctx *ctx_p, const char *dev_name, awe_alsa_handle_t * pcm_ctl);
#endif

/* ****************************************************************************
 * CONFIG
* ***************************************************************************/
/**
 * @brief Create and register AWE Manager configuration objects.
 *
 * This function creates an AWE Manager configuration object and registers the required
 * configuration parameters (stored with a key, default value, and description).
 * Use this API if you wish to modify the default configurations before initializing the
 * AWE Manager. After creation, you can change configuration values by calling
 * awemgr_config_set, and then initialize the AWE Manager with awemgr_init by passing the
 * configuration object. If you prefer to use the default settings, you may pass NULL to
 * awemgr_init without calling awemgr_config_create.
 *
 * @param cfg_pp [in/out] - Pointer to a pointer that will hold the initialized AWE Manager configuration.
 *
 * @return An error code indicating success or failure.
 */
enum awemgr_rc awemgr_config_create(awe_config** cfg_pp);


/**
 * @brief Destroy and free AWE Manager configuration objects.
 *
 * This function frees all configurations and destroys the AWE Manager configuration object.
 * Call this API only if awemgr_config_create was previously used to create the configuration.
 *
 * @param cfg_pp [in/out] - Pointer to a pointer to the initialized AWE Manager configuration to be destroyed.
 *
 * @return An error code indicating success or failure.
 */
enum awemgr_rc awemgr_config_destroy(awe_config** cfg_pp);


/**
 * @brief Set a configuration value.
 *
 * This function sets the value of a configuration parameter identified by its key.
 * It can be used to modify the default configuration settings before the AWE Manager is initialized.
 *
 * See [Supported Configuration Items][supported-configuration-items] for a list of available configuration keys, their default values, and descriptions.
 *
 * @param cfg_p [in] - Pointer to the initialized AWE Manager configuration object.
 * @param key   [in] - The configuration key to set.
 * @param value [in] - The new value for the configuration key, provided as a string.
 *
 * @return An error code indicating success or failure.
 */
int awemgr_config_set(struct awe_config *cfg_p, const char *key, const char* value);


/**
 * @brief Retrieve a configuration value.
 *
 * This function retrieves the value of a configuration parameter by its key. Optionally,
 * it can also return the description associated with the configuration.
 *
 * @param cfg_p         [in] - Pointer to the initialized AWE Manager configuration object.
 * @param key           [in] - The configuration key to retrieve.
 * @param description   [out] - Pointer to a string that will be set to the configuration's description.
 *
 * @return A pointer to the string representing the value of the specified configuration key.
 */
const char* awemgr_config_get(struct awe_config *cfg_p, const char *key, const char**description);

/* ****************************************************************************
 * ERROR
 * ***************************************************************************/

/**
 * @brief Get the AWECore error code from the last API call on the calling thread.
 *
 * When an AWE Manager API returns `awemgr_RC_AWECORE_ERROR`, call this function from the
 * **same thread** that made the failing call to retrieve the AWECore error code.
 *
 * @note Thread-local: the value is per-thread and is overwritten on every transaction.
 *       Read it immediately after a failing call before making another API call on the same thread.
 *
 * @return AWECore error code (typically an error ID from AWECore's Error.h).
 */
int awemgr_get_awe_error_code(void);

/**
 * @brief Get the AWECore error description string from the last API call on the calling thread.
 *
 * When an AWE Manager API returns `awemgr_RC_AWECORE_ERROR`, call this function from the
 * **same thread** that made the failing call to retrieve a human-readable error description.
 *
 * @note Thread-local: the value is per-thread and is overwritten on every transaction.
 *       Read it immediately after a failing call before making another API call on the same thread.
 *
 * @return Pointer to a null-terminated error description string; never NULL.
 */
const char* awemgr_get_awe_error_string(void);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif // INCLUSION_GUARD_AWE_MANAGER_H
