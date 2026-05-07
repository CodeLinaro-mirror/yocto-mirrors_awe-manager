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

#ifndef _AWE_PACKET_CLIENT_H
#define _AWE_PACKET_CLIENT_H

#include <syslog.h>

#define MAX_COMMAND_BUFFER_LEN (4096)
#define MIN_BUFFER_SIZE        (512)
#define MAX_BUFFER_SIZE        (20480)
#define DUMMY_LEN              (16)

#define SOCKET_PATH "/tmp/awe_pkt_socket"
#define TAG "AWE_PACKET_CLIENT"

#define AWEPKT_ERROR  0
#define AWEPKT_WARN   1
#define AWEPKT_INFO   2
#define AWEPKT_DEBUG  3

#define AWEPKT_LOGE(format,...) if(c_pkt_level>AWEPKT_ERROR) syslog(LOG_ERR|LOG_USER, "[AWE][%s][%d]" format, __FILE__, __LINE__, ##__VA_ARGS__)
#define AWEPKT_LOGW(format,...) if(c_pkt_level>AWEPKT_WARN) syslog(LOG_WARNING|LOG_USER, "[AWE][%s][%d]" format, __FILE__, __LINE__, ##__VA_ARGS__)
#define AWEPKT_LOGI(format,...) if(c_pkt_level>AWEPKT_INFO) syslog(LOG_INFO|LOG_USER, "[AWE][%s][%d]" format, __FILE__, __LINE__, ##__VA_ARGS__)
#define AWEPKT_LOGD(format,...) if(c_pkt_level>AWEPKT_DEBUG) syslog(LOG_DEBUG|LOG_USER, "[AWE][%s][%d]" format, __FILE__, __LINE__, ##__VA_ARGS__)

#define TRANS_TYPE_OFFSET  0
#define TRANS_COMMAND_OFFSET  4
#define TRANS_DATA_OFFSET  8

#define TRANS_APP_ID_LEN  12
#define TRANS_MODE_LEN  12
#define TRANS_ACK_LEN  8
#define TRANS_ADDITIONAL_LEN 8

#define GET_UINT32(ptr)  ((uint32_t)(((uint8_t*)(ptr))[0]) | \
                          ((uint32_t)(((uint8_t*)(ptr))[1]) << 8) | \
                          ((uint32_t)(((uint8_t*)(ptr))[2]) << 16) | \
                          ((uint32_t)(((uint8_t*)(ptr))[3]) << 24))

#define SET_UINT32(ptr, value) do { \
    ((uint8_t*)(ptr))[0] = (uint8_t)((value) & 0xFF); \
    ((uint8_t*)(ptr))[1] = (uint8_t)(((value) >> 8) & 0xFF); \
    ((uint8_t*)(ptr))[2] = (uint8_t)(((value) >> 16) & 0xFF); \
    ((uint8_t*)(ptr))[3] = (uint8_t)(((value) >> 24) & 0xFF); \
} while(0)

#define SET_TRANS_HEAD(ptr, ttype, command) do {\
        SET_UINT32((uint8_t *)ptr+TRANS_TYPE_OFFSET, ttype); \
        SET_UINT32(((uint8_t *)ptr)+TRANS_COMMAND_OFFSET, command); \
}while(0)


/* error define */
#define    PKT_SUCCESS              ( 0)
#define    PKT_E_CONNECT            (-1)
#define    PKT_E_SCOKET             (-2)
#define    PKT_E_BIND               (-3)
#define    PKT_E_LISTEN             (-4)
#define    PKT_E_SEND_DATA          (-5)
#define    PKT_E_RECV_DATA          (-6)
#define    PKT_E_SCOKET_EXECEPTION  (-7)
#define    PKT_E_PARAMS             (-8)
#define    PKT_E_SIZE_OVERFLOW      (-9)
#define    PKT_E_SOCKET_DISCONNECT  (-10)
#define    PKT_E_DATA_MISMATCH      (-11)

/* define transfer data*/
typedef enum {

    CMD_REQ_APPID = 0x30030001,
    CMD_ACK_APPID = 0x30030002,

    CMD_REQ_TRANS_MODE = 0x30030003,
    CMD_ACK_TRANS_MODE = 0x30030004,

    CMD_REQ_RAW_DATA = 0x30030005,
    CMD_ACK_RAW_DATA = 0x30030006,

    CMD_REQ_DWCS_DATA = 0x30030007,
    CMD_ACK_DWCS_DATA = 0x30030008,

    CMD_REQ_IPCC =  0x30030009,
    CDM_ACK_IPCC =  0x30030010,

    CMD_REQ_TEST_DATA = 0x30030011,
    CMD_ACK_TEST_DATA = 0x30030012,

    
} transfer_command;

/* define transfer type */
typedef enum {

  TRANS_TYPE_COMMAND = 0x80000001,
  TRANS_TYPE_DATA = 0x80000002,
  
} transfer_type;

typedef enum {
    
    SHARED_MODE = 0x00, 
    EXCLUSIVE_MODE = 0x01,
    
} transfer_mode;

#ifdef __cplusplus
extern "C" {
#endif

void awe_packet_set_client_log_level(int32_t level);
void awe_packet_deregister_client(int32_t client_fd);
int32_t awe_packet_register_client(int32_t *p_client_fd, uint32_t *p_app_id);
void awe_packet_set_transfer_mode(int32_t client_fd, uint32_t mode);
void awe_packet_clear_transfer_mode(int32_t client_fd);
void awe_packet_trigger_ipcc(int32_t client_fd);

/* for user data */
int32_t awe_packet_client_write(int32_t client_fd, const uint8_t * write_buf, uint32_t write_size_bytes);
int32_t awe_packet_client_read(int32_t client_fd, uint8_t * read_buf, uint32_t read_size_bytes, uint32_t * p_out_len);

/* for raw data */
int32_t awe_packet_send_data(int32_t client_fd, uint8_t * data_buffer, uint32_t data_len);
int32_t awe_packet_receive_data(int32_t client_fd, uint8_t * data_buffer, 
                                    uint32_t max_buffer_size , uint32_t * p_out_len);

int32_t awe_packet_ping_socket(int32_t client_fd);

#ifdef __cplusplus
}
#endif


#endif
