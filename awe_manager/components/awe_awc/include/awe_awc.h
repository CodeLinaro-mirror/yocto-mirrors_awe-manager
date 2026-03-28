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


#ifndef _AWE_AWC_H
#define _AWE_AWC_H

#ifdef __cplusplus
extern "C"
{
#endif
    #include "awe_types.h"

    /**
     * @brief Loads the Audio Weaver Container File using the filepath and returns the pointer to the awc handle.
     * @param filename Complete //Path/Filename of the awc file.
     * @return Pointer to awc handle, or NULL = error
     */
    void *awc_init(const char *filename);

    /**
     * @brief Uninitialize the awc and frees the resources.
     * @param pHandle pointer to the awc handle.
     * @return integer error code 0 = Success, <0 = Error
     */
    int awc_uninit(void **pHandle);

    /**
     * @brief Returns the pointer to the awc file info structure.
     * @param handle awc handle.
     * @return Pointer to info structure or NULL = error
     */
    awc_info_t *awc_get_info(const void *handle);

    /**
     * @brief Search a Design by its name, and returns the pointer to the Design Structure.
     * @param handle awc handle.
     * @param name Name of the Design to be searched
     * @return Pointer to the Design Structure, NULL if Design not found
     */
    awc_design_t *awc_get_design(const void *handle, const char *name);

    /**
     * @brief Returns the number of designs in the awc file
     * @param handle awc handle.
     * @return Returns the number of designs in the awc.
     */
    UINT32 awc_design_count(const void* handle);

    /**
     * @brief Returns the pointer to Design structure by its index.
     * @param handle awc handle.
     * @param index Index of the design.
     * @return Pointer to the Design Structure, NULL = Error/Invalid Index.
     */
    awc_design_t* awc_get_design_by_index(const void* handle, UINT32 index);

    /**
     * @brief Search a Module by its name, and returns the pointer to the Module Structure.
     * @param handle awc handle.
     * @param name Name of the module to be searched.
     * @return Pointer to the Module Structure, NULL = Error/Module Not Found.
     */
    awc_module_t *awc_get_module(const void *handle, const char *name);

    /**
     * @brief Returns the number of modules in the awc file
     * @param handle awc handle.
     * @return Returns the number of modules in the awc.
     */
    UINT32 awc_module_count(const void* handle);

    /**
     * @brief Returns the pointer to Module structure by its index.
     * @param handle awc handle.
     * @param index Index of the module.
     * @return Pointer to the Module Structure, NULL = Error/Invalid Index.
     */
    awc_module_t* awc_get_module_by_index(const void* handle, UINT32 index);

    /**
     * @brief Returns the number of controls in the awc file
     * @param handle awc handle.
     * @return Returns the number of controls in the awc file.
     */
    UINT32 awc_control_count(const void* handle);

    /**
     * @brief Returns the pointer to Control structure by its index.
     * @param handle awc handle.
     * @param index Index of the control.
     * @return Pointer to the Control Structure, NULL = Error/Invalid Index.
     */
    awc_ctl_t* awc_get_control_by_index(const void* handle, UINT32 index);

    /**
     * @brief Returns the number of controls in the module
     * @param module pointer to the awc module.
     * @return Returns the number of controls in the module.
     */
    UINT32 awc_module_control_count(const awc_module_t* module);

    /**
     * @brief Returns the pointer to Control structure by its index.
     * @param module pointer to module structure.
     * @param index Index of the control.
     * @return Pointer to the Control Structure, NULL = Error/Invalid Index.
     */
    awc_ctl_t* awc_get_module_control_by_index(const awc_module_t* module, UINT32 index);
    /**
     * @brief Search a Control by its name, and returns the pointer to the Control Structure./
     * @param handle awc handle.
     * @param name Name of the control to be searched.
     * @return Pointer to the Control Structure, NULL = Error/Control Not Found.
     */
    awc_ctl_t *awc_get_control_from_awc(const void *handle, const char *name);

    /**
     * @brief Search the Module object for a Control by its name, and returns the pointer to the Control Structure.
     * @param module Pointer to Module structure.
     * @param name Name of the Control to be searced.
     * @return Pointer to the Control Structure, NULL = Error/Control Not Found.
     */
    awc_ctl_t *awc_get_control_from_module(const awc_module_t *module, const char *name);

    /**
     * @brief Returns the name of the control type.
     * @param type Enum value.
     * @return Pointer to the name string.
     */
    const char *awc_get_typename(const awc_ctl_type_t type);

    /**
     * @brief Calls a function foreach control in a module.
     * @param module Pointer to Module structure.
     * @param Func Function to be called.
     * @param user_data_p pointer to data of calling function.
     * @return integer error code 0 = Success, <0 = Error
     */
    int awc_foreach_control(const awc_module_t *module, control_cb cb, void *usr_data_p);

    /**
     * @brief Calls a function foreach module in awc.
     * @param handle awc handle.
     * @param Func Function to be called.
     * @param user_data_p pointer to data of calling function.
     * @return integer error code 0 = Success, <0 = Error
     */
    int awc_foreach_module(const void *handle, module_cb cb, void *usr_data_p);

    /**
     * @brief Calls a function foreach design in awc.
     * @param handle awc handle.
     * @param Func Function to be called.
     * @param user_data_p pointer to data of calling function.
     * @return integer error code 0 = Success, <0 = Error
     */
    int awc_foreach_design(const void *handle, design_cb cb, void *usr_data_p);

    /**
     * @brief Returns the number of events in the awc file
     * @param handle awc handle.
     * @return Returns the number of event modules in the awc.
     */
    UINT32 awc_event_count(const void* handle);

    /**
     * @brief Returns the pointer to Event structure by its index.
     * @param handle awc handle.
     * @param index Index of the event.
     * @return Pointer to the Event Structure, NULL = Error/Invalid Index.
     */
    awc_event_t* awc_get_event_by_index(const void* handle, UINT32 index);

    /**
     * @brief Calls a function foreach event module in awc.
     * @param handle awc handle.
     * @param Func Function to be called.
     * @param user_data_p pointer to data of calling function.
     * @return integer error code 0 = Success, <0 = Error
     */
    int awc_foreach_event(const void *handle, event_cb cb, void *usr_data_p);

#if 0 // TODO: Enable this when the type information is available in the awc_index file
    /**
     * @brief Calls a function foreach event module with the matching type in awc.
     * @param handle awc handle.
     * @param Func Function to be called.
     * @param user_data_p pointer to data of calling function.
     * @return integer error code 0 = Success, <0 = Error
     */
    int awc_foreach_event_type(const void *handle, event_cb cb, UINT32 type, void *usr_data_p);
#endif

    /**
     * @brief Searches the event Modules with name and returns pointer to awc_event_t structure
     * @param handle awc handle.
     * @param name event module name
     * @return Pointer to the awc_event_t structure
     */
    awc_event_t* awc_get_event(const void *handle, const char* name);

    /**
     * @brief Searches the event Modules with objectid and returns pointer to awc_event_t structure
     * @param handle awc handle.
     * @param objectid event module objectid
     * @return Pointer to the awc_event_t structure
     */
    awc_event_t* awc_get_event_by_objectid(const void *handle, UINT32 objectid);

    /**
     * @brief Searches the module with name and returns the number of its user data elements
     * @param handle awc handle.
     * @param modulename name of the module.
     * @return number of userdata, <0 = Error
     */
    int awc_get_module_userdata_count(const void *handle, const char* modulename);

    /**
     * @brief Searches the control with name and returns the number of its user data elements
     * @param handle awc handle.
     * @param controlname name of the module.
     * @return number of userdata, <0 = Error
     */
    int awc_get_control_userdata_count(const void *handle, const char* controlname);

    /**
     * @brief Searches the module with name and returns the module user data with index
     * @param handle awc handle.
     * @param modulename name of the module.
     * @param index index of the user data.
     * @return Pointer to the awc_dict_element structure or NULL if module or index invalid.
     */
    const awc_dict_element* awc_get_module_userdata_by_index(const void *handle, const char* modulename, uint32_t index);

    /**
     * @brief Searches the control with name and returns the control user data with index
     * @param handle awc handle.
     * @param controlname name of the control.
     * @param index index of the user data.
     * @return Pointer to the awc_dict_element structure or NULL if control or index invalid.
     */
    const awc_dict_element* awc_get_control_userdata_by_index(const void *handle, const char* controlname, uint32_t index);

    /**
     * @brief Searches the module with name and returns the module user data with matching key
     * @param handle awc handle.
     * @param modulename name of the module.
     * @param key key of the user data.
     * @return Pointer to the awc_dict_element structure or NULL if module or key not found.
     */
    const awc_dict_element* awc_get_module_userdata_by_key(const void *handle, const char* modulename, const char* key);

    /**
     * @brief Searches the control with name and returns the control user data with matching key
     * @param handle awc handle.
     * @param controlname name of the control.
     * @param key key of the user data.
     * @return Pointer to the awc_dict_element structure or NULL if control or key not found.
     */
    const awc_dict_element* awc_get_control_userdata_by_key(const void *handle, const char* controlname, const char* key);

    /**
     * @brief Searches the top-level AWC structure for some user data by key.
     * @param handle awc handle.
     * @param key key of the user data.
     * @return Pointer to the awc_dict_element structure or NULL if control or key not found.
     */
    const awc_dict_element* awc_get_top_userdata_by_key(const void *handle, const char* key);

#ifdef __cplusplus
}
#endif

#endif /*_AWE_AWC_H*/