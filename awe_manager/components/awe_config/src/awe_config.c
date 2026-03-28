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

#include "awe_config.h"
#include "awe_config_logging.h"
#include "awosal_string.h"
#include <errno.h>

typedef struct {
    char* key;
    char* description;
    char* value;
} awe_config_item;

typedef struct awe_config{
    awe_config_item *items;
    size_t count;
}awe_config;

#define FAIL_ON_PTR(x)                                      \
    if (!x)                                                 \
    {                                                       \
        AWE_CFG_LOGE("Invalid argument: %s == NULL!", #x); \
        return AWECFG_RC_FAIL;                       \
    }

#define FAIL_ON_PTR_RC(x, rc)                                      \
if (!x)                                                 \
{                                                       \
    AWE_CFG_LOGE("Invalid argument: %s == NULL!", #x); \
    return rc;                       \
}

// -------------------------------------------------------------------------
// Private helper conversion functions
// -------------------------------------------------------------------------

typedef int (*conversion_func_t)(const char *str, void *out);

// A generic helper that uses a conversion function
static inline int config_get_value(awe_config *config_p, const char *key, void *out, conversion_func_t converter) {
    const char *str = aweconfig_get(config_p, key, NULL);
    if (!str  || str[0] == '\0')
        return AWECFG_RC_NOT_FOUND;
    return converter(str, out);
}

static inline int conversion_int(const char *str, void *out) {
    char *endptr = NULL;
    errno = 0;
    long result = strtol(str, &endptr, 10);
    if (errno != 0 || endptr == str)
        return AWECFG_RC_CONVERSION_FAIL;
    *((int32_t *)out) = (int32_t)result;
    return AWECFG_RC_OK;
}

static inline int conversion_uint(const char *str, void *out) {
    char *endptr = NULL;
    errno = 0;
    if (*str == '-') {
        return AWECFG_RC_CONVERSION_FAIL;
    }
    unsigned long result = strtoul(str, &endptr, 10);
    if (errno != 0 || endptr == str)
        return AWECFG_RC_CONVERSION_FAIL;
    *((uint32_t *)out) = (uint32_t)result;
    return AWECFG_RC_OK;
}

static inline int conversion_float(const char *str, void *out) {
    char *endptr = NULL;
    errno = 0;
    float result = strtof(str, &endptr);
    if (errno != 0 || endptr == str)
        return AWECFG_RC_CONVERSION_FAIL;
    *((float *)out) = result;
    return AWECFG_RC_OK;
}

// Convert to bool. Accepts "true", "on", "1" for true,
// and "false", "off", "0" for false
static inline int conversion_bool(const char *str, void *out) {
    if (strcmp(str, "true") == 0 ||
        strcmp(str, "on") == 0 ||
        strcmp(str, "1") == 0) {
        *((bool *)out) = true;
        return AWECFG_RC_OK;
    }
    if (strcmp(str, "false") == 0 ||
        strcmp(str, "off") == 0 ||
        strcmp(str, "0") == 0) {
        *((bool *)out) = false;
        return AWECFG_RC_OK;
    }
    return AWECFG_RC_CONVERSION_FAIL;
}

// -------------------------------------------------------------------------
// Public methods
// -------------------------------------------------------------------------

int aweconfig_create(awe_config **config_pp)
{
    FAIL_ON_PTR(config_pp);

    awe_config* cfg_p = calloc(1, sizeof(awe_config));
    if(cfg_p == NULL)
    {
        AWE_CFG_LOGE("Memory Allocation Failed for awe_config");
        return AWECFG_RC_FAIL;
    }
    cfg_p->items = NULL;
    cfg_p->count = 0;
    *config_pp = cfg_p;

    return AWECFG_RC_OK;
}

int aweconfig_add(awe_config *config_p, const char *key, const char* description, const char* value)
{
    FAIL_ON_PTR(config_p);
    FAIL_ON_PTR(key);
    FAIL_ON_PTR(value);
    FAIL_ON_PTR(description);

    for (size_t i = 0; i < config_p->count; ++i)
    {
        if ((config_p->items[i].key != NULL) && strcmp(config_p->items[i].key, key) == 0)
        {
            AWE_CFG_LOGW("Config '%s' already exists, value and description will be overwritten.", key);

            free((void*)config_p->items[i].value);
            config_p->items[i].value = strdup(value);

            free((void*)config_p->items[i].description);
            config_p->items[i].description = strdup(description);

            return AWECFG_RC_OK;
        }
    }

    awe_config_item *temp = realloc(config_p->items, (config_p->count + 1) * sizeof(awe_config_item));
    if (!temp)
    {
        AWE_CFG_LOGE("Could not allocate memory for the configuration, %s", key);
        return AWECFG_RC_FAIL;
    }
    config_p->items = temp;
    config_p->items[config_p->count].key =strdup(key);
    config_p->items[config_p->count].value = strdup(value);
    config_p->items[config_p->count].description = strdup(description);
    config_p->count++;

    return AWECFG_RC_OK;
}

int aweconfig_set(awe_config *config_p, const char *key, const char* value)
{
    FAIL_ON_PTR(config_p);
    FAIL_ON_PTR(key);
    FAIL_ON_PTR(value);

    for (size_t i = 0; i < config_p->count; ++i)
    {
        if (config_p->items[i].key && strcmp(config_p->items[i].key, key) == 0)
        {
            free((void*)config_p->items[i].value);
            config_p->items[i].value = strdup(value);
            return AWECFG_RC_OK;
        }
    }
    AWE_CFG_LOGE("Could not find configuration, %s", key);

    return AWECFG_RC_FAIL;
}

// Private function to free one configuration item
static inline void aweconfig_item_free(awe_config_item* item_p)
{
    if(item_p != NULL)
    {
        free((void*)item_p->key);
        free((void*)item_p->value);
        free((void*)item_p->description);
    }
}

int aweconfig_from_string(awe_config *config_p, const char *cfg_string)
{
    char *copy = strdup(cfg_string);
    if (copy == NULL)
    {
        AWE_CFG_LOGE("Unable to duplicate config string");
        return AWECFG_RC_FAIL;
    }

    char *context = NULL;
    char *pair = strtok_r(copy, ";", &context);
    while (pair != NULL)
    {
        // Split each pair by '='
        char *equal_sign = strchr(pair, '=');
        if (equal_sign != NULL)
        {
            *equal_sign = '\0'; // Null-terminate the key
            char *key = pair;
            char *value = equal_sign + 1;
            AWE_CFG_LOGI("Key: '%s', Value: '%s'\n", key, value);
            (void)aweconfig_set(config_p, key, value);
        }
        else
        {
            AWE_CFG_LOGE("Malformed pair: %s\n", pair);
            free(copy);
            return AWECFG_RC_CONVERSION_FAIL;
        }
        pair = strtok_r(NULL, ";", &context);
    }
    free(copy);
    return AWECFG_RC_OK;
}

int aweconfig_from_envvar(awe_config *config_p, const char *env_var)
{
    FAIL_ON_PTR(config_p);
    FAIL_ON_PTR(env_var);

    const char *env_str = getenv(env_var);
    if (env_str == NULL)
    {
        AWE_CFG_LOGD("Environment variable %s not defined. No config from environment applied.", env_var);
        return AWECFG_RC_OK;
    }
    AWE_CFG_LOGI("Environment Var %s : %s", env_var, env_str);
    return aweconfig_from_string(config_p, env_str);
}

int aweconfig_destroy(awe_config** config_pp)
{
    FAIL_ON_PTR(config_pp);
    FAIL_ON_PTR(*config_pp);

    awe_config *config_p = *config_pp;
    for (size_t i = 0; i < config_p->count; ++i)
    {
        aweconfig_item_free(&config_p->items[i]);
    }

    free((*config_pp)->items);
    free((*config_pp));
    *config_pp = NULL;

    return AWECFG_RC_OK;
}

const char * aweconfig_get(awe_config *config_p, const char *key, const char**description)
{
    FAIL_ON_PTR_RC(config_p, "");
    FAIL_ON_PTR_RC(key, "");

    for (size_t index = 0; index < config_p->count; ++index)
    {
        if (config_p->items[index].key && strcmp(config_p->items[index].key, key) == 0)
        {
            // optionally return description
            if(description != NULL)
            {
                *description = config_p->items[index].description;
            }
            return config_p->items[index].value;
        }
    }
    AWE_CFG_LOGE("Could not find configuration %s", key);

    return "";  // return empty string
}

int aweconfig_get_as_int(awe_config *config_p, const char *key, int32_t* val) {
    return config_get_value(config_p, key, val, conversion_int);
}

int aweconfig_get_as_uint(awe_config *config_p, const char *key, uint32_t* val) {
    return config_get_value(config_p, key, val, conversion_uint);
}

int aweconfig_get_as_float(awe_config *config_p, const char *key, float* val) {
    return config_get_value(config_p, key, val, conversion_float);
}

int aweconfig_get_as_bool(awe_config *config_p, const char *key, bool* val) {
    return config_get_value(config_p, key, val, conversion_bool);
}

int aweconfig_foreach_item(awe_config *config_p, aweconfig_cb cb, void *usr_data_p)
{
    if ((config_p != NULL) && (cb != NULL))
    {
        for (size_t index = 0; index < config_p->count; ++index)
        {
            awe_config_item item_p = config_p->items[index];
            cb(item_p.key, item_p.value, item_p.description, usr_data_p);
        }
        return AWECFG_RC_OK;
    }
    return AWECFG_RC_FAIL;
}