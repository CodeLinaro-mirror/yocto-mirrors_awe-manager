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


#ifndef _AWC_TYPES_H
#define _AWC_TYPES_H

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>

#ifndef UINT32
    typedef unsigned int UINT32;
#endif
#ifndef INT32
    typedef int INT32;
#endif
#ifndef INT64
    typedef long long INT64;
#endif



/*TODO: Need to add more code documentation*/

    #define E_AWC_SUCCESS 0
    #define E_AWC_ERROR -1
    #define E_AWC_CTL_TYPE_ERROR -2

    typedef enum
    {
        AWC_CTL_UNKNOWN = 0,
        AWC_CTL_BOOL,   // Boolean type
        AWC_CTL_INT32,  // Integer type
        AWC_CTL_FRACT32,// fract type
        AWC_CTL_UINT32, // Integer type
        AWC_CTL_ENUM,   // Enumeration type
        AWC_CTL_FLOAT,  // Integer type
        AWC_CTL_TYPE_MAX,  // !!!ADD New Types before this, Not a valid type
    } awc_ctl_type_t;

    typedef struct
    {
        UINT32 def;
        UINT32 min;
        UINT32 max;
        UINT32 step;
    } awc_ctl_range_t;

    typedef enum
    {
        AWC_USRDATA_INT = 0,
        AWC_USRDATA_UINT,
        AWC_USRDATA_STR,
        AWC_USRDATA_FLOAT,
        AWC_USRDATA_TYPE_MAX,
    } awc_usrdata_type;

    typedef union
    {
        uint32_t u32;
        int32_t i32;
        float f32;
        char* str;  // Note: On 64 big machine str is the Largest member (8 bytes)
    } awc_usrdata_value;

    typedef struct
    {
        char* key;
        awc_usrdata_type type;
        awc_usrdata_value value;
    } awc_dict_element;

    typedef struct
    {
        awc_dict_element* data;
        uint32_t size;
    } awc_dictionary;

    struct _awc_module;
    typedef struct _awc_module awc_module_t;
    typedef struct 
    {
        char *fullname; // This is the full name including ModuleName, delimiter (e.g /, |, :: etc) and the variable name
        char *varname; // This is the pointer to the variable part of the fullname
        char *alias; // This is the userfriendly name for the control - Optional 
        char** enums;
        awc_module_t* module;
        awc_dictionary userdata;
        UINT32 handle;
        UINT32 size;
        UINT32 offset;
        awc_ctl_type_t type;
        awc_ctl_range_t range;
        UINT32 numenums;
    } awc_ctl_t;

    typedef struct _awc_module
    {
        char *name;
        char *alias;
        awc_ctl_t **controls;
        awc_dictionary userdata;
        UINT32 classid;
        UINT32 objectid;        //Introduced in Schema 1
        UINT32 coreid;          //Obtained from the module variables or from the tunnelAddress if exists
        UINT32 size;
        UINT32 controlIndex;
        INT32 tunnelAddress;
    } awc_module_t;

    typedef struct _awc_event
    {
        awc_module_t *mod;
        UINT32 type;
    } awc_event_t;

    typedef struct
    {
        UINT32 coreid_objectid;
        char *name;
        char *file;
        UINT32 size;
        char *md5sum;
    } awc_design_t;

    typedef struct
    {
        char *version;
        char *date;
        char *description;
        char *ctlname_delimiter; // This is the string to be used as a delimiter between Module name and Variable name (e.g. "/", "::", "|" etc)
        UINT32 designcount;
        UINT32 modulecount;
        UINT32 controlcount;
        UINT32 eventcount;
        UINT32 schema; // Used to identify the awc schema used to parse the file.
    } awc_info_t;

    typedef void (*awc_log_cb)(const char* message);
    typedef void (*control_cb)(const awc_ctl_t* ctl, void* usr_data_p);
    typedef void (*module_cb)(const awc_module_t* module, void* usr_data_p);
    typedef void (*design_cb)(const awc_design_t* design, void* usr_data_p);
    typedef void (*event_cb)(const awc_event_t* event, void* usr_data_p);
#ifdef __cplusplus
}
#endif

#endif /*_AWC_TYPES_H*/