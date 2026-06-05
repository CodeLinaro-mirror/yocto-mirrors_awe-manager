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

#include "awe_config.h"
#include "awe_config_logging.h"
#include "awosal_string.h"
#include <errno.h>
#include <string.h>

typedef struct {
    char* key;
    char* description;
    char* value;
    aweconfig_cb cb;
    void *usr_data_p;
} awe_config_item;

struct awe_config {
    awe_config_item *items;
    size_t count;
    size_t capacity;
};

#define FAIL_ON_PTR(x)                                          \
    if (!x)                                                     \
    {                                                           \
        AWE_CFG_LOGE("Invalid argument: %s == NULL!", #x);      \
        return AWECFG_RC_FAIL;                                  \
    }

#define FAIL_ON_PTR_RC(x, rc)                                   \
if (!x)                                                         \
{                                                               \
    AWE_CFG_LOGE("Invalid argument: %s == NULL!", #x);          \
    return rc;                                                  \
}

#define CONFIG_INITIAL_CAPACITY (16) // initial capacity to avoid reallocations/fragmantation
// -------------------------------------------------------------------------
// Private helper conversion functions
// -------------------------------------------------------------------------

typedef int (*conversion_func_t)(const char *str, void *out);

static inline int config_get_value(awe_config *config_p, const char *key, void *out, conversion_func_t converter) {
    const char *str = aweconfig_get(config_p, key, NULL);
    if (!str  || str[0] == '\0')
        return AWECFG_RC_NOT_FOUND;

    if (out == NULL)
        return AWECFG_RC_FAIL;
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
        AWE_CFG_LOGE("Memory Allocation Failed for awe_config structure");
        return AWECFG_RC_FAIL;
    }
    awe_config_item* temp = calloc(CONFIG_INITIAL_CAPACITY, sizeof(awe_config_item));
    if (!temp)
    {
        AWE_CFG_LOGE("Memory Allocation Failed for initial config items");
        free(cfg_p);
        return AWECFG_RC_FAIL;
    }
    cfg_p->items = temp;
    cfg_p->capacity = CONFIG_INITIAL_CAPACITY;
    cfg_p->count = 0;
    *config_pp = cfg_p;

    return AWECFG_RC_OK;
}

int aweconfig_add_listener(awe_config *config_p, const char* key, aweconfig_cb cb, void *usr_data_p)
{
    FAIL_ON_PTR(config_p);
    FAIL_ON_PTR(key);

    for (size_t i = 0; i < config_p->count; ++i)
    {
        if (strcmp(config_p->items[i].key, key) == 0)
        {
            config_p->items[i].cb = cb;
            config_p->items[i].usr_data_p = usr_data_p;

            if (cb) {
                cb(key, config_p->items[i].value, config_p->items[i].description, usr_data_p);
            }
            return AWECFG_RC_OK;
        }
    }
    AWE_CFG_LOGE("Could not bind listener: Config '%s' not found", key);
    return AWECFG_RC_NOT_FOUND;
}

int aweconfig_remove_listener(awe_config *config_p, const char* key)
{
    FAIL_ON_PTR(config_p);
    FAIL_ON_PTR(key);

    for (size_t i = 0; i < config_p->count; ++i)
    {
        if (strcmp(config_p->items[i].key, key) == 0)
        {
            config_p->items[i].cb = NULL;
            config_p->items[i].usr_data_p = NULL;
            return AWECFG_RC_OK;
        }
    }
    AWE_CFG_LOGE("Could not remove listener: Config '%s' not found", key);
    return AWECFG_RC_NOT_FOUND;
}

int aweconfig_add(awe_config *config_p, const char *key, const char* description, const char* value)
{
    FAIL_ON_PTR(config_p);
    FAIL_ON_PTR(key);
    FAIL_ON_PTR(value);
    FAIL_ON_PTR(description);

    for (size_t i = 0; i < config_p->count; ++i)
    {
        if (strcmp(config_p->items[i].key, key) == 0)
        {
            AWE_CFG_LOGW("Config '%s' already exists, overwriting (%s -> %s)", key, config_p->items[i].value, value);

            char *new_val = strdup(value);
            char *new_desc = strdup(description);

            if (!new_val || !new_desc)
            {
                AWE_CFG_LOGE("Memory allocation failed during overwrite for config, %s", key);
                free(new_val);
                free(new_desc);
                return AWECFG_RC_FAIL;
            }

            free((void*)config_p->items[i].value);
            config_p->items[i].value = new_val;

            free((void*)config_p->items[i].description);
            config_p->items[i].description = new_desc;

            if(config_p->items[i].cb)
            {
                config_p->items[i].cb(key, value, description, config_p->items[i].usr_data_p);
            }
            return AWECFG_RC_OK;
        }
    }

    if (config_p->count >= config_p->capacity)
    {
        // if the initial capacity is exhausted, then expand capacity by 2, to reduce allocations
        size_t new_cap = config_p->capacity > 0 ? (config_p->capacity + 2) : CONFIG_INITIAL_CAPACITY;

        awe_config_item *temp = realloc(config_p->items, new_cap * sizeof(awe_config_item));
        if (!temp)
        {
            AWE_CFG_LOGE("Could not allocate memory to expand configuration array, %s", key);
            return AWECFG_RC_FAIL;
        }
        config_p->items = temp;
        config_p->capacity = new_cap;
    }

    char *dup_key = strdup(key);
    char *dup_val = strdup(value);
    char *dup_desc = strdup(description);

    if (!dup_key || !dup_val || !dup_desc)
    {
        AWE_CFG_LOGE("Memory allocation failed during strdup for new config, %s", key);
        free(dup_key);
        free(dup_val);
        free(dup_desc);
        return AWECFG_RC_FAIL;
    }

    config_p->items[config_p->count].key = dup_key;
    config_p->items[config_p->count].value = dup_val;
    config_p->items[config_p->count].description = dup_desc;
    config_p->items[config_p->count].cb = NULL;
    config_p->items[config_p->count].usr_data_p = NULL;
    config_p->count++;

    return AWECFG_RC_OK;
}

int aweconfig_add_multiple(awe_config *config_p, aweconfig_init_tuple* tbl_p, size_t nr_entries)
{
    FAIL_ON_PTR(config_p);
    FAIL_ON_PTR(tbl_p);

    int rc = AWECFG_RC_OK;
    for (size_t i = 0; i < nr_entries; i++) {
        aweconfig_init_tuple current_cfg = tbl_p[i];

        rc = aweconfig_add(config_p, current_cfg.key, current_cfg.description, current_cfg.default_value);
        if(rc != AWECFG_RC_OK) {
            AWE_CFG_LOGE("Failed to add configuration: %s", current_cfg.key);
            return rc;
        }

        if (current_cfg.cb != NULL) {
            rc = aweconfig_add_listener(config_p, current_cfg.key, current_cfg.cb, current_cfg.usr_data_p);
            if(rc != AWECFG_RC_OK) {
                AWE_CFG_LOGE("Failed to bind listener for configuration: %s", current_cfg.key);
                return rc;
            }
        }
    }
    return rc;
}

int aweconfig_set(awe_config *config_p, const char *key, const char* value)
{
    FAIL_ON_PTR(config_p);
    FAIL_ON_PTR(key);
    FAIL_ON_PTR(value);

    for (size_t i = 0; i < config_p->count; ++i)
    {
        if (strcmp(config_p->items[i].key, key) == 0)
        {
            char *old_value = config_p->items[i].value;
            config_p->items[i].value = strdup(value);
            if (!config_p->items[i].value) return AWECFG_RC_FAIL;

            free((void*)old_value);
            if(config_p->items[i].cb)
            {
                config_p->items[i].cb(key, value, config_p->items[i].description, config_p->items[i].usr_data_p);
            }
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
        char *inner_ctx = NULL;
        char *key = strtok_r(pair, "=", &inner_ctx);
        char *value = strtok_r(NULL, "=", &inner_ctx);
        if (key != NULL && value != NULL)
        {
            AWE_CFG_LOGI("Key: '%s', Value: '%s'", key, value);
            if (aweconfig_set(config_p, key, value) == AWECFG_RC_FAIL)
            {
                AWE_CFG_LOGE("Ignoring invalid parameter in configuration string: %s=%s", key, value);
            }

        }
        else
        {
            AWE_CFG_LOGE("Malformed key-value pair: %s", pair);
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
        if (strcmp(config_p->items[index].key, key) == 0)
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
