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


#ifndef _AWE_CONFIG_H
#define _AWE_CONFIG_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define AWECFG_RC_OK                    0
#define AWECFG_RC_FAIL                 -1
#define AWECFG_RC_NOT_FOUND            -2
#define AWECFG_RC_CONVERSION_FAIL      -3

typedef struct awe_config awe_config;
typedef void (*aweconfig_cb)(const char* key, const char* value, const char* description, void* context);

/**
 * @brief Initializes the awe_config structure
 * @param config_pp Pointer to awe_config handle.
 * @return int      0 on success, negative value on error.
 */
int aweconfig_create(awe_config **config_pp);

/**
 * @brief Adds a config item to awe_config structure. If the configuration key exists, its
 * value and description is updated.
 * @param config_p      awe_config handle.
 * @param key           name of the config.
 * @param description   helper string/ description of the config
 * @param value         value encoded as string
 * @return int          0 on success, negative value on error.
 */
int aweconfig_add(awe_config *config_p, const char *key, const char* description, const char* value);

/**
 * @brief Sets the value of a configuration if it exists.
 *
 * This function sets the value of a configuration parameter. The value is provided
 * as a string and stored accordingly. If the configuration parameter exists, its
 * value is updated.
 *
 * @param config_p Pointer to the awe_config handle.
 * @param key      Name of the configuration parameter.
 * @param value    Value encoded as a string.
 * @return int     0 on success, negative value on error.
 */
int aweconfig_set(awe_config *config_p, const char *key, const char* value);

/**
 * @brief Retrieves the value and description of a configuration.
 *
 * This function gets the value of a configuration parameter and optionally its description.
 * The returned value is encoded as a string.
 *
 * @param config_p    Pointer to the awe_config handle.
 * @param key         Name of the configuration parameter.
 * @param description (Optional) Pointer to a pointer that will be set to the configuration description.
 * @return const char* The configuration value as a string, or Empty String ("") if not found.
 */
const char* aweconfig_get(awe_config *config_p, const char *key, const char **description);

/**
 * @brief Retrieves a boolean configuration value.
 *
 * This function converts the configuration value to a boolean. It accepts the following
 * case-insensitive string representations:
 * - True: "true", "yes", "on", "1"
 * - False: "false", "no", "off", "0"
 *
 * @param config_p      Pointer to the awe_config handle.
 * @param key           Name of the configuration parameter.
 * @param val           Output pointer where the boolean value will be stored.
 * @return int          0 on success, negative value on error (e.g., not found or conversion failure).
 */
int aweconfig_get_as_bool(awe_config *config_p, const char *key, bool* val);

/**
 * @brief Retrieves an integer configuration value.
 *
 * This function converts the configuration value to an int32_t.
 *
 * @param config_p      Pointer to the awe_config handle.
 * @param key           Name of the configuration parameter.
 * @param val           Output pointer where the integer value will be stored.
 * @return int          0 on success, negative value on error.
 */
int aweconfig_get_as_int(awe_config *config_p, const char *key, int32_t* val);

/**
 * @brief Retrieves an unsigned integer configuration value.
 *
 * This function converts the configuration value to a uint32_t.
 *
 * @param config_p      Pointer to the awe_config handle.
 * @param key           Name of the configuration parameter.
 * @param val           Output pointer where the unsigned integer value will be stored.
 * @return int          0 on success, negative value on error.
 */
int aweconfig_get_as_uint(awe_config *config_p, const char *key, uint32_t* val);

/**
 * @brief Retrieves a floating-point configuration value.
 *
 * This function converts the configuration value to a float.
 *
 * @param config_p      Pointer to the awe_config handle.
 * @param key           Name of the configuration parameter.
 * @param val           Output pointer where the float value will be stored.
 * @return int          0 on success, negative value on error.
 */
int aweconfig_get_as_float(awe_config *config_p, const char *key, float* val);

/**
 * @brief Calls a function foreach config in awe_config.
 * @param config_p      awe_config handle.
 * @param cb            Function to be called.
 * @param user_data_p   pointer to data of calling function.
 * @return int          0 on success, negative value on error.
 */
int aweconfig_foreach_item(awe_config *config_p, aweconfig_cb cb, void *usr_data_p);

/**
 * @brief Updates the internal config object with values from configuration string
 *
 * The string value should be formatted as a sequence of key=value
 * pairs separated by semicolons.
 *
 * For example:
 *   "key1=value1;key2=value2;..."
 *
 * Each parsed key-value pair is applied to update the provided configuration structure.
*
 * @param config_p      awe_config handle.
 * @param cfg_string    AWE configuration string
 * @return int          0 on success, negative value on error.
 */
int aweconfig_from_string(awe_config *config_p, const char *cfg_string);

/**
 * @brief Updates the internal config object with values from ENV variable
 *
 * The environment variable specified with parameter `env_var` will be parsed.
 * It's string value should be formatted so that aweconfig_from_string()
 * can consume it.
 *
 * @param config_p      awe_config handle.
 * @param env_var       name of the config variable to check
 * @return int          0 on success, negative value on error.
 */
int aweconfig_from_envvar(awe_config *config_p, const char *env_var);

/**
 * @brief Destroys all the configs and frees up memory
 * @param config_pp     Pointer to awe_config handle.
 * @return int          0 on success, negative value on error.
 */
int aweconfig_destroy(awe_config **config_pp);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif // _AWE_CONFIG_H