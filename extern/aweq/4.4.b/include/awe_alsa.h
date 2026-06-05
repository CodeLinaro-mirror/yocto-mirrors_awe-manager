/* MIT License
**
** Copyright (c) 2024 DSP Concepts, Inc.
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

/****************************************************************************
 *
 *      Audio Weaver HLOS Interface
 *      ---------------------------
 *
 ****************************************************************************
 *  awe_alsa.h
 ****************************************************************************
 *
 *  Description:    Functions for interfacing the high-level OS (Linux,
 *                      Android, or QNX) to an Audio Weaver instance on a
 *                      DSP or Arm core.
 *
 *                      It provides APIs for control and real-time audio
 *                      exchange with ALSA devices.
 *
 *
 *    Copyright:    (c) 2024 DSP Concepts Inc., All rights reserved
 *                  3235 Kifer Road
 *                  Santa Clara, CA 95054
 *
 ***************************************************************************/


#ifndef AWE_ALSA_H
#define AWE_ALSA_H

//#include <stddef.h>
//#include <stdint.h>

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

  // Maximum number of unique ALSA devices that can be supported.
  // This is used to allocate memory for the device pointer array.

#define AWE_ALSA_MAX_DEVICE_COUNT 256

  // Maximum name of an ALSA device as specified in the Audio Weaver
  // modules.
#define AWE_ALSA_DEVICE_NAME_LENGTH 32

  // Indicates whether this is an capture or playback device as seen
  // by ALSA.

  // Capture devices
#define AWE_ALSA_INPUT_DEVICE 1

  // Playback device
#define AWE_ALSA_OUTPUT_DEVICE 2

  // device samplerate
#define  AWE_MAX_VALUE_SAMPLERATE 384000
#define  AWE_MIN_VALUE_SAMPLERATE 8000

  // Clocking modes for devices
#define AWE_CLOCKING_MODE_SYNCHRONOUS_MASTER 0
#define AWE_CLOCKING_MODE_SYNCHRONOUS_ALIGNED 1
#define AWE_CLOCKING_MODE_SYNCHRONOUS_UNALIGNED 2
#define AWE_CLOCKING_MODE_ASYNCHRONOUS 3

  // Error conditions
#define AWE_ALSA_SUCCESS (0)
#define AWE_ALSA_ERROR_ALREADY_INITIALIZED (-1)
#define AWE_ALSA_ERROR_NOT_INITIALIZED (-2)
#define AWE_ALSA_ERROR_INVALID_TABLE_INDEX (-3)
#define AWE_ALSA_ERROR_DEVICE_NOT_FOUND (-4)
#define AWE_ALSA_ERROR_OUT_OF_RANGE (-5)
#define AWE_ALSA_ERROR_UNSUPPORTED_NUMCHANNELS (-6)
#define AWE_ALSA_ERROR_UNSUPPORTED_SAMPLERATE (-7)
#define AWE_ALSA_ERROR_UNSUPPORTED_FORMAT (-8)
#define AWE_ALSA_ERROR_DEVICE_IN_WRONG_STATE (-9)
#define AWE_ALSA_ERROR_CORRUPTED (-10)
#define AWE_ALSA_ERROR_LOADAWB_TIMEOUT (-11)
#define AWE_ALSA_ERROR_INVAILD_CHECKSUM (-12)
#define AWE_ALSA_ERROR_MESSAGE_TOO_LONG (-13)


#define AWE_ALSA_ERROR_MESSAGE_ERROR_LEN                (-35)
#define AWE_ALSA_ERROR_REGISTER_ERROR                   (-36)
#define AWE_ALSA_ERROR_RECEIVE_ERROR                    (-37)
#define AWE_ALSA_ERROR_SEND_ERROR                       (-38)
#define AWE_ALSA_ERROR_NOSUCHFILE                       (-44)
#define AWE_ALSA_ERROR_IOERROR                          (-45)
#define AWE_ALSA_ERROR_OUT_OF_SPACE                     (-54)
#define AWE_ALSA_ERROR_ARGUMENT_ERROR                   (-67)
#define AWE_ALSA_ERROR_CRC_ERROR                        (-76)
#define AWE_ALSA_ERROR_MODULE_NOT_INITIALIZED           (-95)

#define AWE_ALSA_MAGIC_INIT_VALUE 0xABCD0123

#define AWE_MAX_NUM_SAMPLERATES        (18)

#define AWE_ASRC_DEFAULT_DELAY_US          (150)
#define AWE_ASRC_MIN_SAMPLE_RATE           (8000)

#ifdef LINUX
#define AWE_ALSA_INVALIDATE(addr, size)     awe_audiolite_buffer_cache_inval(addr, size)
#define AWE_ALSA_FLUSH(addr, size)          awe_audiolite_buffer_cache_flush(addr, size)
#elif defined(HEXAGON)
#define AWE_ALSA_INVALIDATE(addr, size)     qurt_mem_cache_clean((qurt_addr_t)addr, size, QURT_MEM_CACHE_INVALIDATE, QURT_MEM_DCACHE)
#define AWE_ALSA_FLUSH(addr, size)          qurt_mem_cache_clean((qurt_addr_t)addr, size, QURT_MEM_CACHE_FLUSH, QURT_MEM_DCACHE)
#else
#define AWE_ALSA_INVALIDATE(addr, size)
#define AWE_ALSA_FLUSH(addr, size)
#endif

#ifndef AWE_SH_SET
#define AWE_SH_SET(dst, src)             {(dst) = (src); AWE_ALSA_FLUSH(&(dst), sizeof(dst));}
#endif

#ifndef AWE_SH_GET
#define AWE_SH_GET(val)                  ({AWE_ALSA_INVALIDATE(&val, sizeof(val)); val;})
#endif

#ifndef AWE_SH_ALSAIO_GET
#define AWE_SH_ALSAIO_GET(dst, val)      {AWE_ALSA_INVALIDATE(&val, sizeof(val)); (dst) = (val);}
#endif

#ifndef AWE_SH_MEMSET
#define AWE_SH_MEMSET(dst, src, size)    {memset((dst), (src), (size)); AWE_ALSA_FLUSH((dst), (size));}
#endif

#ifndef AWE_SH_MEMCPY
#define AWE_SH_MEMCPY(dst, src, size)    {memcpy((dst), (src), (size)); AWE_ALSA_FLUSH((dst), (size));}
#endif

#define DEVICE_HOSTLESS_PREFIX "Hostless"
enum device_mode_type {
  DEFAULT_MODE = 0,
  HOSTLESS_MODE,
};

  /** Audio sample format in the circular transfer buffer.
   * The first letter specifiers whether the sample is signed or unsigned.
   * The letter 'S' means signed. The letter 'U' means unsigned.
   * The following number is the amount of bits that the sample occupies in memory.
   * Following the underscore, specifiers whether the sample is big endian or little endian.
   * The letters 'LE' mean little endian.
   * The letters 'BE' mean big endian.
   */

  typedef enum {

    /* Note: This section must stay in the same
     * order for binary compatibility with older
     * versions of TinyALSA. */

    AWE_ALSA_FORMAT_INVALID = -1,
    /** Signed 16-bit, little endian */
    AWE_ALSA_FORMAT_S16_LE = 0,
    /** Signed, 32-bit, little endian */
    AWE_ALSA_FORMAT_S32_LE,
    /** Signed, 8-bit */
    AWE_ALSA_FORMAT_S8,
    /** Signed, 24-bit (32-bit in memory), little endian */
    AWE_ALSA_FORMAT_S24_LE,
    /** Signed, 24-bit, little endian */
    AWE_ALSA_FORMAT_S24_3LE,

    /** Max of the enumeration list, not an actual format. */
    AWE_ALSA_FORMAT_MAX
  } awe_alsa_format_t;


  typedef enum {

    /* The DSP initialization has been completed.
    ** We also return here after the HLOS closes the
    ** device. */
    AWE_ALSA_STATE_DSP_INIT = 1,

    /* Next the HLOS does the initialization of the shared
    ** memory structures.  Then it enters this state.  In
    ** this state, the DSP does final initialization (translates
    ** to virtual addresses). */
    AWE_ALSA_STATE_HLOS_INIT,

    /* After HLOS initialization has been completed.
    ** Waiting for the HLOS to start the device. */
    AWE_ALSA_STATE_STOPPED,

    /* Device has been started by the HLOS and is
    ** streaming data. */
    AWE_ALSA_STATE_STARTED

  } awe_alsa_state_t;

  /* THe voice APP get the AWE core device on which platform, dsp or hlos */
  typedef enum {
    /* The device module run on adsp */
    AWE_ALSA_INSTANCE_ADSP = 0,

    /* The device module run on gpdsp0 */
    AWE_ALSA_INSTANCE_GPDSP0,

    /* The device module run on gpdsp1 */
    AWE_ALSA_INSTANCE_GPDSP1,

    /* The device module run on HLOS */
    AWE_ALSA_INSTANCE_HLOS,

    AWE_ALSA_INSTANCE_PLAT_CNT
  } awe_alsa_instance_plat_t;

  /* In linux, the libawe_alsa.so used by voice APP and AWE core */
  typedef enum {
    /* Voice playback app */
    AWE_ALSA_VOICE_APP = 0,

    /* AWE instance APP on arm */
    AWE_ALSA_INSTANCE_APP
  } awe_alsa_awecore_flag_t;

  typedef enum {
    /* AWE core stop message */
    AWE_ALSA_CORE_STOP = 0,

    /* End of stream */
    AWE_ALSA_EOS,

    /* Detect Underrun message, During playback, if the circular buffer is empty,
     * it will lead to the sound interruption.*/
    AWE_ALSA_UNDER_RUN,

    /* Detect Overrun message, During recording, if the circular buffer is full,
     * it will lead to the sound interruption of the recordings.*/
    AWE_ALSA_OVER_RUN,

    /* Detect BUFFER_UNDERFLOW message, During playback, if the spare space of the
     * circular buffer exceeds the callback_threshold, AWE Core will send message
     * to Voice APP to inform the Voice APP to write data in the circular buffer. */
    AWE_ALSA_BUFFER_UNDERFLOW,

    /* Detect BUFFER_OVERFLOW message, During recording, if the valid frames of
     * the circular buffer exceeds the callback_threshold, the AWE Core will send
     * message to Voice APP to inform the Voice APP to read data from the circular
     * buffer.*/
    AWE_ALSA_BUFFER_OVERFLOW,

    AWE_ALSA_MSG_CNT,
  } awe_alsa_message_e;

  typedef enum
  {
    /* Idle (Stream not started) */
    AWE_ALSA_BOS_IDLE = 0,

    /* Beginning of Stream.  (First block in the stream) */
    AWE_ALSA_BOS_BEGINNING,

    /* Middle of Stream */
    AWE_ALSA_BOS_MIDDLE,

    /* End of Stream (Last block in the stream) */
    AWE_ALSA_BOS_END,
  }
  awe_alsa_bos_state_e;

  #define AWE_CORE_STOP_MSK          (1 << AWE_ALSA_CORE_STOP)
  #define AWE_CORE_EOS_MSK           (1 << AWE_ALSA_EOS)
  #define AWE_UNDERRUN_MSK           (1 << AWE_ALSA_UNDER_RUN)
  #define AWE_OVERRUN_MSK            (1 << AWE_ALSA_OVER_RUN)
  #define AWE_BUFFER_UNDERFLOW_MSK   (1 << AWE_ALSA_BUFFER_UNDERFLOW)
  #define AWE_BUFFER_OVERFLOW_MSK    (1 << AWE_ALSA_BUFFER_OVERFLOW)

  #define AWE_TS_DEBUG_CNT           (16)

  /* Data type which represents device handles.  These is really an index
  ** into the global control table. */
  typedef uint32_t awe_alsa_handle_t;

  /* Position buffer which tracks input and output flows from the circular
     buffer.  This will be allocated by HLOS and mapped to shared memory. */

typedef struct _awe_alsa_pos_t {
  /* write_index is an offset from the start of the circular transfer buffer.
     It specifies where new data should be written to. */
  int32_t write_index;

  /* read_index is an offset from the start of the circular transfer buffer.
     It specifies where data should be read from. */
  int32_t read_index;

  /* 64-bit time stamps for the received data */
  // PB.  Is this the time stamp of the input data or output data???
  uint32_t wall_clock_us_lsw;
  uint32_t wall_clock_us_msw;
} awe_alsa_pos_t;

  /* Structure to describe a region of shared mapped memory. */
  typedef struct _awe_alsa_memory_map_t {
    uint32_t ipa_lsw;         // Physical address (low 32-bits)
    uint32_t ipa_msw;         // Physical address (high 32-bits)
    uint64_t mem_map_handle;  // Handle to the allocated region (optional)
    uint32_t size_bytes;      // Size of the buffer in bytes
    //if Audio Weaver Instance on ARM, use dmabufheap_import to get share memory
    int32_t sh_fd_hlos;
    int32_t sh_fd_awe;
    int32_t pid_hlos;
  } awe_alsa_memory_map_t;

  /* This structure represents the shared memory communication structure which links
     the Audio Weaver run-time with the higher-level OS.
     It implements a circular buffer with separate read and write pointers. */

  typedef struct _awe_alsa_control_t {
    /* 1: device on arm  0:device on dsp */
    int32_t running_on_arm;

    /* Current status of the device. */
    int32_t device_state;

    /* Indicates direction of the transfer */
    int32_t direction;

    /* ID of the device.  Used by pcm_open */
    int32_t device_id;

    /* Bit mask which defines which virtual machines the audio device is
    ** visible to. */
    uint32_t vm_mask;

    /* "internal" variables reflect the pin properties in Audio Weaver.
       "external" variables are those configured by the HLOS. */

    /* The number of channels in a frame */
    uint32_t internal_num_channels;

    /* blockSize on the Audio Weaver side.
       For ALSASource modules, this is how many samples are read from the
       buffer each time Audio Weaver pumps.
       For ALSASink modules, this is how many samples are written to the
       buffer each time Audio Weaver pumps.
    */
    uint32_t internal_block_size;

    /* Sample rate on the Audio Weaver side. */
    uint32_t internal_sample_rate[AWE_MAX_NUM_SAMPLERATES];

    /* The number of channels as set by the HLOS.  This is also the number
       of channels in the transfer buffer. */
    uint32_t external_num_channels;

    /* This is the sample rate as set by the HLOS and corresponds to the
       sample rate of the data in the transfer buffer. */
    uint32_t external_sample_rate;

    /* Boolean which indicates whether the device contains an asynchronous
       sample rate converter.  CURRENTLY NOT USED. */
    uint32_t is_asrc;

    /* Set by Audio Weaver.  isStreaming=0 during the prefill stage of the
       buffer.  For an ALSASource device, if the number of samples in the
       buffer is < startThreshold, then isStreaming=0.
       isStreaming is set to 1 once prefill level exceeds startThreshold.
    */
    uint32_t is_streaming;

    /* Number of overrun or underrun samples.
       For ALSASource devices, this is the number of blocks that could not
       be pulled from the buffer by Audio Weaver (HLOS not writing fast enough).
       For ALSASink devices, this is the number of blocks that could not
       be written into the buffer by Audio Weaver (HLOS not reading fast enough).
    */
    uint32_t xrun_count;

    /* A TBD error status to be written by Audio Weaver and reported back to
       the HLOS. */
    int32_t err_status;

    /* This is the size of the transfer buffer as set by the HLOS.
    ** This is the size in sample frames. */
    uint32_t buffer_size;

    /* Prefill level that has to be reached before streaming starts.
    ** In units of sample frames. */
    uint32_t start_threshold;

    /* Now sure what this is */
    uint32_t stop_threshold;

    /* Threshold at which the callback function is called.
    ** In units of sample frames. */
    uint32_t callback_threshold;

    /* If drain_flag set, the stream is end and wait the AWE core send the
     * AWE_ALSA_EOF message */
    uint32_t drain_flag;

    /* Enumerated format of the data in the circular buffer. */
    int32_t external_format;

    /* Enumerated format of the output format of Audio Weaver. */
    int32_t internal_format;

    /* This holds the name of the device as set in Audio Weaver. */
    char device_name[AWE_ALSA_DEVICE_NAME_LENGTH];

    /* These structures are used to pass the memory mapped spaces to the
    ** DSP. */

    awe_alsa_memory_map_t transfer_buf_map;
    awe_alsa_memory_map_t pos_buf_map;

    /* The AWECore instance send IPCC interrupt to HLOS, the HLOS interrupt server thread
     * receive the interrpt and the callbackFunc_hlos, callbackFunc_hlos will Wake up
     * the playback/record thread, playback/record thread will call awe_alsa_circular_read or
     * awe_alsa_circular_write to read/write data.
     * alsaio_send_idx is the AWECore instance send IPCC interrupt index.
     * callback_recv_idx is the HLOS interrupt server thread interrupt index.
     * If alsaio_send_idx == callback_recv_idx, this device not have interrupt must process in
     *     HLOS interrupt server thread.
     *  */
    uint32_t alsaio_send_idx[AWE_ALSA_MSG_CNT];
    uint32_t callback_recv_idx[AWE_ALSA_MSG_CNT];

    /* The total number of data frames that have been generated/consumed,
     * This variable does not count the frames filled with zero; it only
     * counts the valid frames. */
    uint64_t handled_frames;
    /* the most recent audio timestamp */
    int64_t latestAudioTimestamp_us;
    /* The QCM 19.2 MHz time, the dsp set playbackTimestamp's time,
     * the system clock when this timestamp was recorded */
    int64_t referenceSystemTime_us;
    /* an XOR of the latestAudioTimestamp_us and referenceSystemTime_us */
    uint64_t timeStampCheckword;
    /* The start system clock */
    int64_t startTime_us;
    /* Information about the TDM device connected to the ALSA module */
    int32_t tdm_valid;
    int32_t tdm_type;
    int32_t tdm_idx;
    int32_t tdm_direction;

    /* Beginning of Stream */
    uint32_t BOS;
    uint32_t fill_BOS;

    /* Pointers to the vaddr of the transfer buffer and position
       structure as seen by the dsp. */
    void *transfer_buf_dsp;
#if (__SIZEOF_POINTER__ == 4)
    uint32_t fill_transfer_buf_dsp;
#endif

    /* Points to the position buffer for the associated circular buffer.
     ** As seen by the dsp. */
    awe_alsa_pos_t *pos_buf_dsp;
#if (__SIZEOF_POINTER__ == 4)
    uint32_t fill_pos_buf_dsp;
#endif

    /* Pointer to the callback function registered by the dsp. */
    int32_t(*callbackFunc_dsp)(void *);
#if (__SIZEOF_POINTER__ == 4)
    uint32_t reserved_callbackFunc_dsp;
#endif

    /* Pointer to the callback function patload registered by the dsp. */
    void* event_payload_dsp;
#if (__SIZEOF_POINTER__ == 4)
    uint32_t fill_event_payload_dsp;
#endif

    uint32_t pos_buf_handle;
    uint32_t transfer_buf_handle;

    /* Pointers to the vaddr of the transfer buffer and position
       structure as seen by the HLOS. */
    void *transfer_buf_hlos;
#if (__SIZEOF_POINTER__ == 4)
    uint32_t fill_transfer_buf_hlos;
#endif

    /* Points to the position buffer for the associated circular buffer.
     ** As seen by the HLOS. */
    awe_alsa_pos_t *pos_buf_hlos;
#if (__SIZEOF_POINTER__ == 4)
    uint32_t fill_pos_buf_hlos;
#endif

    /* Pointer to the callback function registered by the HLOS. */
    int32_t(*callbackFunc_hlos)(void *, uint32_t);
#if (__SIZEOF_POINTER__ == 4)
    uint32_t fill_callbackFunc_hlos;
#endif

    /* Pointer to the callback function patload registered by the HLOS. */
    void* event_payload_hlos;
#if (__SIZEOF_POINTER__ == 4)
    uint32_t fill_event_payload_hlos;
#endif
  } awe_alsa_control_t;

  /* This is the global device table which will be in shared global
** memory. */

  typedef struct _awe_alsa_device_table_t
  {
      uint32_t magic;
      uint32_t count;
      uint32_t reserved[2];
      awe_alsa_control_t C[AWE_ALSA_MAX_DEVICE_COUNT];
  } awe_alsa_device_table_t;

  // Functions which execute only the HLOS will have _hlos_ in the name.
  // Functions which execute only the DSP will have _dsp_ in the name.

  /* Returns a Boolean which indicates whether the global device table was
     initialized by Audio Weaver. */
  int32_t awe_alsa_is_device_table_initialized(void);

  /* Returns the number of audio devices that have been defined in the Audio
     Weaver design. */
    int32_t awe_alsa_get_device_count(uint32_t *COUNT);

    /* Resets the device tables.  Clears out all entries, sets the magic number,
     * and sets the device count to zero. */
    int32_t awe_alsa_reset_device_table(void);

/* Returns a pointer to the control structure for a specified device handle.
  ** It essentially converts the handle to the location in shared memory. */
  int32_t awe_alsa_internal_get_control(awe_alsa_handle_t H, awe_alsa_control_t **C);

  /* Adds a control structure C to the global table to audio devices.  This
  ** should only be called by the DSP and never by the HLOS. You should initialize
  ** the structure C and then call this function.  The function adds C to the
  ** next available location in the device table, increments the device count,
  ** and returns a handle to the just added device. */
  int32_t awe_alsa_add_device(awe_alsa_control_t* C, awe_alsa_handle_t* H);

  /* Remove all the devices running on arm */
  int32_t awe_alsa_remove_arm(void);

  /* Returns a pointer to the circular transfer structure used by the specified
     device. */
  int32_t awe_alsa_get_buffer_pointer(awe_alsa_handle_t H, void **ptr);

  /* Returns the handles for the Nth device as specified
     by the table INDEX. */
  int32_t awe_alsa_get_device_by_index(uint32_t INDEX, awe_alsa_handle_t *H);

    /* Returns a handle to the audio device with the matching device ID. */
  int32_t awe_alsa_get_device_by_id(int32_t ID, awe_alsa_handle_t *H);

  /* Returns a pointer to the control structure for the named device. */
  int32_t awe_alsa_get_device_by_name(char *NAME, awe_alsa_handle_t *H);

  /* Returns the a pointer to the name of the audio device as set in Audio Weaver. */
  int32_t awe_alsa_get_device_name(awe_alsa_handle_t H, char *NAME);

  /* Returns the direction of the device.
     Capture devices return AWE_ALSA_INPUT_DEVICE.
     Playback devices return AWE_ALSA_OUTPUT_DEVICE.
  */
  int32_t awe_alsa_get_device_direction(awe_alsa_handle_t H, int32_t *direction);

    /* Returns the internal device ID (as set by Audio Weaver). */
  int32_t awe_alsa_get_device_id(awe_alsa_handle_t H, int32_t *id);

    /* Returns the vm_mask (as set by Audio Weaver). */
  int32_t awe_alsa_get_vm_mask(awe_alsa_handle_t H, uint32_t *vm_mask);

  /* Returns the transfer buffer size as set by the HLOS. */
  int32_t awe_alsa_get_transfer_buffer_size(awe_alsa_handle_t H, uint32_t *sz);

  /* The calling app allocations the transfer buffer in shared memory and then passes
  ** it to awe_alsa with this call.  This will will update the internal pointers and
  ** computer buffer_size based on the size of the mapped memory buffer. */
  int32_t awe_alsa_map_transfer_buf(awe_alsa_handle_t H, awe_alsa_memory_map_t *map);

  int32_t awe_alsa_unmap_transfer_buf(awe_alsa_handle_t H);

  int32_t awe_alsa_map_pos_buf(awe_alsa_handle_t H, awe_alsa_memory_map_t *map);

  int32_t awe_alsa_unmap_pos_buf(awe_alsa_handle_t H);

  /* Allows the HLOS to set the data format used in the circular transfer
     buffer. The same data format used by the ALSA playback and capture devices.
     Data is always converted to fract32 for use in Audio Weaver. */
  int32_t awe_alsa_set_transfer_buffer_format(awe_alsa_handle_t H, awe_alsa_format_t format);

  /* Returns the data format set by the HLOS. */
  int32_t awe_alsa_get_transfer_buffer_format(awe_alsa_handle_t H, awe_alsa_format_t *format);

  /* Allows the HLOS to set the start threshold which act as a prefill threshold for
     playback devices. */
  int32_t awe_alsa_set_start_threshold(awe_alsa_handle_t H, uint32_t start_threshold);
  int32_t awe_alsa_get_start_threshold(awe_alsa_handle_t H, uint32_t *start_threshold);
  int32_t awe_alsa_get_sample_size_format(awe_alsa_format_t external_format, uint32_t *sizeBytes);
  int32_t awe_alsa_get_sample_size(awe_alsa_handle_t H, uint32_t *sizeBytes);

  /* stop_threshold is not currently used. */
  int32_t awe_alsa_set_stop_threshold(awe_alsa_handle_t H, uint32_t stop_threshold);
  int32_t awe_alsa_get_stop_threshold(awe_alsa_handle_t H, uint32_t *stop_threshold);

  /* stop_threshold is not currently used. */
  int32_t awe_alsa_set_callback_threshold(awe_alsa_handle_t H, uint32_t callback_threshold);
  int32_t awe_alsa_get_callback_threshold(awe_alsa_handle_t H, uint32_t *callback_threshold);

  /* Called by the HLOS to set the external device properties. */
  int32_t awe_alsa_set_external_properties(awe_alsa_handle_t H, uint32_t numChannels, uint32_t sample_rate, awe_alsa_format_t format);

  /* Allows you to query the external device properties. */
  int32_t awe_alsa_get_external_properties(awe_alsa_handle_t H, uint32_t *numChannels, uint32_t *blockSize, uint32_t *sample_rate, awe_alsa_format_t *format);

  /* Returns the properties of the audio device as set by Audio Weaver. */
  int32_t awe_alsa_get_internal_properties(awe_alsa_handle_t H, uint32_t *numChannels, uint32_t *blockSize, uint32_t *sample_rate, awe_alsa_format_t *format);

  /* Returns the xrun counter which counts sample frames.
     For ALSA playback devices, this counts the number of underruns (HLOS not writing
     data fast enough).
     For ALSA capture devices, this counts the number of overruns (HLOS not reading
     data fast enough).
     The count equals the number of Audio Weaver blocks which could not be completely
     written or read.
  */

  int32_t awe_alsa_get_xrun_count(awe_alsa_handle_t H, uint32_t *count);

  /* Sets the xrun counter to 0. */
  int32_t awe_alsa_clear_xrun_count(awe_alsa_handle_t H);

  /* The audio device must be initialized and then started by this API.
   * Audio Weaver ignores a device if it is not started.
   * event: bit 0:AWE_ALSA_CORE_STOP
   *         bit 1:AWE_ALSA_EOF
   *         bit 2:AWE_ALSA_UNDER_RUN
   *         bit 3:AWE_ALSA_OVER_RUN
   *         bit 4:AWE_ALSA_BUFFER_UNDERFLOW
   *         bit 5:AWE_ALSA_BUFFER_OVERFLOW
   */
  int32_t awe_alsa_start_device(awe_alsa_handle_t H,
      int32_t (*callbackFunc)(void *, uint32_t event), void *payload);
  int32_t awe_alsa_stop_device(awe_alsa_handle_t H);

  /* Closes a specified audio device.
     Only STOPPED devices may be closed.
     The class frees the shared memory associated with the device.
     The device still exists in the global device table and may be reopened again
     by the HLOS. */

  int32_t awe_alsa_close_device(awe_alsa_handle_t H);

  /* Returns the state of the device using an enum:
     AWE_ALSA_STATE_DSP_INIT - the device was initialized by Audio Weaver only.  It hasn't
        been initialized by the HLOS.
     AWE_ALSA_STATE_STOPPED - after the device is initialized by the HLOS (setting external
        device properties and allocating the shared buffer, it enters this state.  It is
    ready to be started.
     AWE_ALSA_STATE_STARTED - the device has been started and is processing samples in real-time.

     When a STARTED device is stopped, it goes to the STOPPED state.
     When a STOPPED device is closed, it goes to the DSP_INIT state.
   */

  int32_t awe_alsa_get_device_state(awe_alsa_handle_t H, awe_alsa_state_t *state);

    /* Function used by the HLOS indicating that is has completed its
     * initialization phase. */
  int32_t awe_alsa_hlos_init_done(awe_alsa_handle_t H);

  /* Only stopped audio devices can be reset.
     When reset, the read and write pointers are set back to 0.
     The transfer buffer contents are also set to zero.
  */
  int32_t awe_alsa_reset_device(awe_alsa_handle_t H);


  /* Indicates that the device's prefill level has been reached and
     audio is being read out (playback devices). */
  int32_t awe_alsa_is_device_streaming(awe_alsa_handle_t H, uint32_t *is_streaming);

  /* Returns the number of samplesa available in the circular transfer buffer. */
  int32_t awe_alsa_get_num_samples(awe_alsa_handle_t H, uint32_t *num_frames);

  /* Slightly faster version which accesses the control structure directly. */
  int32_t awe_alsa_get_num_samplesC(awe_alsa_control_t* C, uint32_t* num_frames);

  /* Returns the number of samples that can be written into the circular transfer
     buffer.  This is based on how many samples have been filled and also the
     buffer_size. */
  int32_t awe_alsa_get_free_space(awe_alsa_handle_t H, uint32_t *free_space);

  /* Writes data into the circular transfer buffer.
     The source data is always fract32 and the function converts to the
     internal format used by the circular buffer. */
  int32_t awe_alsa_circular_write(awe_alsa_handle_t H, char *pSrc, int32_t frames_to_copy, awe_alsa_format_t src_format);

  /* Reads data out of the circular buffer.  The internal data format is converted
     to fract32. */
 int32_t awe_alsa_circular_read(awe_alsa_handle_t H, char *pOut, uint32_t frames_to_copy, awe_alsa_format_t dst_format);

  /* Returns the location at which data should be read from the circular buffer.
  ** The function returns an index which corresponds to the number of samples frames
  ** from the start of the buffer at which the data to read exists. */
  int32_t awe_alsa_get_read_pointer(awe_alsa_handle_t H, uint32_t *offset);

  /* Hlos set the read pointer when use mmap, the hlos read from dma buf direct */
  int32_t awe_alsa_set_read_pointer(awe_alsa_handle_t H, uint32_t offset);

  /* Returns the location at which data should be written into the circular buffer.
  ** The function returns an index which corresponds to the number of samples frames
  ** from the start of the buffer at which new data should be written. */
  int32_t awe_alsa_get_write_pointer(awe_alsa_handle_t H, uint32_t *offset);

  /* Hlos set the write pointer when use mmap, the hlos read to dma buf direct */
  int32_t awe_alsa_set_write_pointer(awe_alsa_handle_t H, uint32_t offset);

  /* ----------------------------------------------------------------------
  ** Internally used functions.  These need to be moved out eventually.
  ** -------------------------------------------------------------------- */

  /* Helper functions for copying and converting data between the various formats. */

  void awe_alsa_copy_format_to_format(char *src, awe_alsa_format_t src_format, uint32_t src_ch,
                      char *dst, awe_alsa_format_t dst_format, uint32_t dst_ch,
                      uint32_t num_frames);

  /* Use this function to share the fixed address of the global device table
  ** with the awe_alsa library. address is the virtual address where the
  ** device table is mapped.  Before calling this function, the .magic
  ** field of the device table should be initialized. */
  int32_t awe_alsa_link_device_table(void *address);

  // Returns a vaddr to the start of the shared memory
  // control table.  This is used on the HLOS.
  int32_t awe_alsa_hlos_get_device_table(void **ptr);

  // Returns a vaddr to the start of the shared memory
  // control table.  This is used on the DSP.
  int32_t awe_alsa_dsp_get_device_table(void **ptr);

  uint32_t awe_alsa_offset_to_handle(uint32_t offset);
  int32_t awe_alsa_handle_to_offset(uint32_t handle);

  int32_t awe_alsa_map_to_hlos_vaddr(awe_alsa_memory_map_t *map, void **ptr,
      int cache_flag, uint32_t *handle);

#ifdef LINUX
  int32_t awe_alsa_load_awb(const char *filename);
  int32_t awe_alsa_remote_set_value_mask(uint32_t handle, const void *value, uint32_t arrayOffset, uint32_t length, uint32_t mask);
  int32_t awe_alsa_init(void);
  int32_t awe_alsa_uninit(void);

  //In linux, the libawe_alsa.so used by voice APP and AWE core
  //flag 1:The awe alsa run with AWECore
  //flag 0:The awe alsa run with voice APP
  int32_t awe_alsa_set_core_flag(awe_alsa_awecore_flag_t flag);

  /* THe voice APP get the AWE core on which platform, dsp or hlos */
  int32_t awe_alsa_get_instance_plat(awe_alsa_handle_t H, awe_alsa_instance_plat_t *plat);

  /* Call this function before load awb file */
  int32_t awe_alsa_load_awb_start(int32_t client_fd);

  /* Call this function after load awb file */
  int32_t awe_alsa_load_awb_end(int32_t client_fd);
#endif

  //AWE Core instance send notify to voice APP
  void awe_send_awe_alsa_notify(void);

  /* Set the drain flag, means the stream end */
  int32_t awe_alsa_set_drain(awe_alsa_handle_t H);

  /* Get the drain flag */
  int32_t awe_alsa_get_drain(awe_alsa_handle_t H, uint32_t *drain_flag);

  /* Get the handled_frames */
  int32_t awe_alsa_get_handled_frames(awe_alsa_handle_t H, uint64_t *handled_frames);

  /**
   * @brief This function Get the time_stamp of the running device
   * @param[in] H                      The device handle
   * @param[out] time_stamp            Timestamp from the start of the run to now
   * @return                           0:Success,otherwise fail
   */
  int32_t awe_alsa_get_timestamp(awe_alsa_handle_t H, int64_t *time_stamp);

  /**
   * @brief This function Get the TDM device
   * @param[in] H                      The device handle
   * @param[out] p_tdm_type            The TDM device type
   * @param[out] p_tdm_idx             The TDM device index
   * @param[out] p_tdm_direction       The TDM device direction
   * @return                           0:Success,otherwise fail
   */
  int32_t awe_alsa_get_tdm_info(awe_alsa_handle_t H, int32_t *p_tdm_type,
      int32_t *p_tdm_idx, int32_t *p_tdm_direction);

  /**
   * @brief This function Get the time elapsed since the ADSP
   * @return                           Returns the time elapsed since the ADSP
   *                                   was started, in microseconds (us).
   */
  int64_t awe_alsa_get_sync_ts(void);

  /**
   * @brief This function Get the delay(us) of TDM data in PingPong buf current
   * @param[in] lpaif_type			   The TDM lpaif_type
   * @param[in] lpaif_idx			   The TDM lpaif_idx
   * @param[in] dir 				   The TDM direction
   * @return						   Duration of the data in the current TDM
   */
  int32_t awe_get_tdm_delay_us(int32_t lpaif_type, int32_t lpaif_idx, int32_t dir);

  /* Get the device mode */
  int32_t awe_alsa_get_device_mode(awe_alsa_handle_t H);

#if defined(HEXAGON) && !defined(ENABLE_TESTING)
extern void awe_module_set_tdm_source_data_part_a(uint32_t lpaif_type, uint32_t lpaif_idx, uint32_t bit_width,
                                                                    uint32_t num_channels, uint32_t block_size, uint32_t slot_mask,
                                                                    uint32_t lane_mask);
extern void awe_module_set_tdm_source_data_part_b(uint32_t lpaif_type, uint32_t lpaif_idx, uint32_t slot_width,
                                                                    uint32_t sync_src, uint32_t sample_rate, uint32_t sync_mode,
                                                                    uint32_t invert_sync, uint32_t data_delay, uint32_t slot_count);
extern void awe_module_set_tdm_sink_data_part_a(uint32_t lpaif_type, uint32_t lpaif_idx, uint32_t bit_width,
                                                                 uint32_t num_channels, uint32_t block_size, uint32_t slot_mask,
                                                                 uint32_t lane_mask);
extern void awe_module_set_tdm_sink_data_part_b(uint32_t lpaif_type, uint32_t lpaif_idx, uint32_t slot_width,
                                                                 uint32_t sync_src, uint32_t sample_rate, uint32_t sync_mode,
                                                                 uint32_t invert_sync, uint32_t data_delay, uint32_t slot_count);
extern int awe_qurt_mem_cache_clean(void *data_addr, int buf_size, bool is_write);
extern void awe_update_dma_net_avai_addr(uint32_t  idx, uint32_t  type, uint32_t dir, uint32_t *next_avail_addr);

extern int awe_set_lpass_ddr_bw(uint64_t bw_m);
extern int awe_set_core_mips(uint32_t mips);
extern uint32_t awe_get_clock_rate(void);
extern uint32_t awe_get_core_ddr_bw(void);
extern void awe_log_to_shmem(const char *file, const char *functionname, uint32_t linenumber, const char *format, ...);

#include "hexagon_types.h"

#define MAX_L2FETCH_SIZE   (128 * 128)

#define L2fetch(addr, param) \
    do { \
        __asm__ __volatile__ ("l2fetch(%0,%1)" : : "r"(addr), "r"(param)); \
    } while (0)

#define  CreateL2pfParam(stride, w, h, dir)   (unsigned long long)HEXAGON_V64_CREATE_H((dir), (stride), (w), (h))
#define  CreateL2pfLine(size)                 CreateL2pfParam(128, 128, (((size + 127) / 128) * 128), 0)

#if defined(AWE_LEMANSAU)
extern int32_t awe_memory_map(uint64_t phy_address, uint32_t lens, uint32_t *mem_map_handle, uint32_t is_cached);
extern int32_t awe_memory_unmap(uint32_t mem_map_handle);
extern int32_t awe_get_virtual_addr_from_shm_handle(uint32_t mem_map_handle, uint64_t phy_address, uint32_t lens, void **virt_addr_ptr);
#endif

#endif
#if defined(__cplusplus)
}  /* extern "C" */
#endif

#endif
