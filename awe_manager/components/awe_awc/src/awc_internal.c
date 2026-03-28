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


#include "awc_internal.h"
#include "awosal_string.h"
#include "awc_logging.h"
#include "awe_cmd.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <stddef.h>
#include <math.h>    // for fabs

inline int stringToU32(const char *str, UINT32 *number)
{
    int ret = E_AWC_ERROR;
    if ((str != NULL) && (number != NULL))
    {
        char *endptr;
        errno = 0;
        *number = strtoul(str, &endptr, 10);
        if (errno == 0)
        {
            if (*endptr == '\0')
            {
                ret = E_AWC_SUCCESS;
            }
            else
            {
                AWC_LOGE("Trailing characters found in the string after parsing %s", str);
            }
        }
    }
    return ret;
}

inline int parseNumericTokenU32(UINT32 *number, const char *delimiter, char** context)
{
    int ret = E_AWC_ERROR;
    if ((number != NULL) && (delimiter != NULL) && (context != NULL) && (*context != NULL))
    {
        char *token = strtok_r(NULL, delimiter, context);
        if (token != NULL)
        {
            *(token - 1) = delimiter[0]; // restore token in original string
            ret = stringToU32(token, number);
        }
    }
    return ret;
}

int parseStringToken(char** str, const char *delimiter, char** context)
{
    int ret = E_AWC_ERROR;
    if ((str != NULL) && (delimiter != NULL) && (context != NULL) && (*context != NULL))
    {
        char* token = strtok_r(NULL, delimiter, context);
        if (token != NULL)
        {
            *(token - 1) = delimiter[0]; // restore token in original string
            *str = strdup(token);
            ret = E_AWC_SUCCESS;
        }
    }
    return ret;
}

int parseType(awc_ctl_type_t *type, char** context)
{
    int ret = E_AWC_CTL_TYPE_ERROR;
    if((type != NULL) && (context != NULL) && (*context != NULL))
    {
        char* token = strtok_r(NULL, AWC_DELIMITER, context);
        if (token != NULL)
        {
            *(token - 1) = AWC_DELIMITER[0]; // restore token in original string
            for (int i = 1; i < (int)AWC_CTL_TYPE_MAX; i++) {
                if (tokenCompare(token, awc_get_typename((awc_ctl_type_t)i)) == E_AWC_SUCCESS)
                {
                    ret = E_AWC_SUCCESS;
                    *type = (awc_ctl_type_t)i;
                    break;
                }
            }
        }
    }
    return ret;
}

inline int validateModuleParsing(awe_awc_t *awc, char** context)
{
    if ((awc != NULL) && (context != NULL) && (*context != NULL))
    {
        if ((awc->modules != NULL) && (awc->moduleindex < awc->info.modulecount))
        {
            return E_AWC_SUCCESS;
        }
    }
    if (awc != NULL) AWC_LOGD("Module parsing validation failed, module count: %u, moduleindex %u", awc->info.modulecount, awc->moduleindex);
    return E_AWC_ERROR;
}


inline int parseModule(awe_awc_t *awc, char** context)
{

    int ret = E_AWC_ERROR;
    if(validateModuleParsing(awc, context) == E_AWC_ERROR)
    {
        return E_AWC_ERROR;
    }
    AWC_LOGD("Parse Module");

    awc_module_t* module = &awc->modules[awc->moduleindex];

    // 1: Parse Module Name
    if(parseStringToken(&module->name, AWC_DELIMITER, context) == E_AWC_SUCCESS)
    {
        // 2: Parse Class ID
        if (parseNumericTokenU32(&module->classid, AWC_DELIMITER, context) == E_AWC_SUCCESS)
        {
             // 3: Parse Object ID for Schema Version >= 1
            if(awc->info.schema >= 1)
            {

                if (parseNumericTokenU32(&module->objectid, AWC_DELIMITER, context) != E_AWC_SUCCESS)
                {
                    AWC_LOGE("Error parsing module Object ID %s", module->name);
                    return ret;
                }
            }
            // 4: Parse size (number of controls)
            if (parseNumericTokenU32(&module->size, AWC_DELIMITER, context) == E_AWC_SUCCESS)
            {
                AWC_LOGD("Parsed Module %s", module->name);
                awc->moduleindex++;
                module->controls = calloc(module->size, sizeof(awc_ctl_t*));
                awc_map_insert(awc->moduleMap, module->name, (void*)module);
                ret = E_AWC_SUCCESS;
            }
            // 5: Optional Alias for Schema Version >= 3
            if(awc->info.schema >= 3)
            {
                parseStringToken(&module->alias, AWC_DELIMITER, context);
                awc_map_insert(awc->moduleMap, module->alias, module); // If alias is Null, the api fails, so no need to null check.
            }
        }
    }
    return ret;
}

inline int validateControlParsing(awe_awc_t *awc, char** context)
{
    if (awc && context && *context && awc->modules && awc->moduleindex && awc->controls && awc->controlindex < awc->info.controlcount)
    {
        return E_AWC_SUCCESS;
    }
    if(awc != NULL) AWC_LOGD("Control parsing validation failed, control count: %u, controlindex %u moduleindex %u", awc->info.controlcount, awc->controlindex, awc->moduleindex);
    return E_AWC_ERROR;
}

inline int validateControlModule(awc_module_t *pMod)
{
    if (pMod && pMod->size && pMod->controls && pMod->controlIndex < pMod->size)
    {
        return E_AWC_SUCCESS;
    }
    return E_AWC_ERROR;
}

inline int splitControlName(awc_ctl_t *pCtl, const char* delim)
{
    if (pCtl)
    {
        char* nameSplitCtxt = NULL;
        pCtl->varname = strtok_r(pCtl->fullname, delim, &nameSplitCtxt);
        if (pCtl->varname != NULL)
        {
            pCtl->varname = strtok_r(NULL, delim, &nameSplitCtxt);
            if (pCtl->varname != NULL)
            {
                *(pCtl->varname - 1) = delim[0]; // restore token in original string
                return E_AWC_SUCCESS;
            }
        }
    }
    return E_AWC_ERROR;
}

static inline int parse_uint32_as_double(const char *token, double *out)
{
    char *endptr = NULL;
    errno = 0;
    unsigned long val = strtoul(token, &endptr, 10);

    if (errno != 0 || *endptr != '\0' || val > UINT32_MAX)
        return E_AWC_ERROR;

    *out = (double)val;
    return E_AWC_SUCCESS;
}

static inline int parse_int32_as_double(const char *token, double *out)
{
    char *endptr = NULL;
    errno = 0;
    long val = strtol(token, &endptr, 10);

    if (errno != 0 || *endptr != '\0' || val < INT32_MIN || val > INT32_MAX)
        return E_AWC_ERROR;

    *out = (double)(int32_t)val;
    return E_AWC_SUCCESS;
}

static inline int parse_float_as_double(const char *token, double *out)
{
    char *endptr = NULL;
    errno = 0;
    float val = strtof(token, &endptr);

    if (errno != 0 || *endptr != '\0')
        return E_AWC_ERROR;

    *out = (double)val;
    return E_AWC_SUCCESS;
}

static inline int fill_range(double *def, double *min, double *max, double *step,
                      char *tokens[4], int (*parser)(const char*, double*))
{
    double values[4];
    for (int i = 0; i < 4; i++)
    {
        if (parser(tokens[i], &values[i]) != 0)
        {
            AWC_LOGE("Invalid token: '%s'", tokens[i]);
            return E_AWC_ERROR;
        }
    }

    *def  = values[0];
    *min  = values[1];
    *max  = values[2];
    *step = values[3];
    return E_AWC_SUCCESS;
}

static inline int parseRangeTokens(awc_ctl_t *ctl, char *range_tokens[4])
{
    int res = E_AWC_SUCCESS;
    switch (ctl->type)
    {
        case AWC_CTL_UINT32:
            res = fill_range(&ctl->range.def, &ctl->range.min,
                             &ctl->range.max, &ctl->range.step,
                             range_tokens, parse_uint32_as_double);
            ctl->checkRange = !(ctl->range.max == ctl->range.min);
            break;

        case AWC_CTL_BOOL:
        case AWC_CTL_INT32:
            res = fill_range(&ctl->range.def, &ctl->range.min,
                             &ctl->range.max, &ctl->range.step,
                             range_tokens, parse_int32_as_double);
            ctl->checkRange = !(ctl->range.max == ctl->range.min);
            break;

        case AWC_CTL_FLOAT:
        case AWC_CTL_FRACT32:
        case AWC_CTL_FRACT16:
            res = fill_range(&ctl->range.def, &ctl->range.min,
                             &ctl->range.max, &ctl->range.step,
                             range_tokens, parse_float_as_double);
            ctl->checkRange = !(fabs(ctl->range.max - ctl->range.min) < 1e-12);
            break;

        case AWC_CTL_ENUM:
            ctl->checkRange = false;
            break;

        default:
            AWC_LOGE("Unsupported control type: %d", ctl->type);
            res = E_AWC_ERROR;
            break;
    }

    return res;
}

inline int parseControl(awe_awc_t *awc, char** context)
{
    int ret = validateControlParsing(awc, context);
    if( ret == E_AWC_ERROR)
    {
        return ret;
    }
    AWC_LOGD("Parse Control");
    awc_module_t* pMod = &awc->modules[awc->moduleindex - 1];
    ret = validateControlModule(pMod);
    if(ret == E_AWC_ERROR)
    {
        return ret;
    }
    awc_ctl_t* ctl = &awc->controls[awc->controlindex];
    ret = parseStringToken(&ctl->fullname, AWC_DELIMITER, context);
    if (ret == E_AWC_ERROR)
    {
        return ret;
    }
    UINT32* controlNumericParams[] = 
    {   
        &ctl->handle, 
        &ctl->size, 
        &ctl->offset 
    };
    for(int i = 0; i < sizeof(controlNumericParams)/sizeof(controlNumericParams[0]); i++)
    {
        ret = parseNumericTokenU32(controlNumericParams[i], AWC_DELIMITER, context);
        if(ret != E_AWC_SUCCESS)
        {
            controlDestructer(ctl);
            return ret;
        }
    }

    // Parse the range information: default, min, max and step
    // Values are stored as strings in temporary array range_tokens
    // The string values are later converted to double once we know the type of control
    char* range_tokens[4] = {0};
    for (int i = 0; i < 4; i++) {
        if (parseStringToken(&range_tokens[i], AWC_DELIMITER, context) != E_AWC_SUCCESS) {
            for (int j = 0; j < i; j++) free(range_tokens[j]);
            controlDestructer(ctl);
            return E_AWC_ERROR;
        }
    }

    ret = parseType(&ctl->type, context);
    if (ret != E_AWC_SUCCESS) {
        for (int i = 0; i < 4; i++) free(range_tokens[i]);
        controlDestructer(ctl);
        return ret;
    }

    // Now Convert ranges based on type
    ret = parseRangeTokens(ctl, range_tokens);
    if (ret != E_AWC_SUCCESS) {
        for (int i = 0; i < 4; i++) free(range_tokens[i]);
        controlDestructer(ctl);
        return ret;
    }

    // free temporary range strings
    for (int i = 0; i < 4; i++) free(range_tokens[i]);

    // --- Parse optional alias ---
    parseStringToken(&ctl->alias, AWC_DELIMITER, context);

    ret = splitControlName(ctl, awc->info.ctlname_delimiter);
    if (ret != E_AWC_SUCCESS) {
        controlDestructer(ctl);
        return ret;
    }

    // --- Insert control into module and map ---
    ctl->module = pMod;
    awc->controlindex++;
    pMod->controls[pMod->controlIndex] = ctl;
    pMod->controlIndex++;
    awc_map_insert(awc->controlMap, ctl->alias, (void*)ctl); // If alias is Null, the api fails, so no need to null check.
    awc_map_insert(awc->controlMap, ctl->fullname, (void*)ctl);
    pMod->coreid = AWE_COREID_FROM_HANDLE(ctl->handle);
    return E_AWC_SUCCESS;
}

inline void onParseDummy(awe_awc_t* awc)
{
}

inline void onParseControlCount(awe_awc_t* awc)
{
    if (!awc->controls)
    {
        awc->controls = (awc_ctl_t*)calloc(awc->info.controlcount, sizeof(awc_ctl_t));
        awc->controlMap = awc_map_create(awc->info.controlcount);
    }
}

inline void onParseDesignCount(awe_awc_t* awc)
{
    if (!awc->designs)
    {
        awc->designs = (awc_design_t*)calloc(awc->info.designcount, sizeof(awc_design_t));
    }
}

inline void onParseModuleCount(awe_awc_t* awc)
{
    if (!awc->modules)
    {
        awc->modules = (awc_module_t*)calloc(awc->info.modulecount, sizeof(awc_module_t));
        awc->moduleMap = awc_map_create(awc->info.modulecount);
    }
}

void onParseEventsCount(awe_awc_t *awc)
{
    if (!awc->events)
    {
        awc->events = (awc_event_t*)calloc(awc->info.eventcount, sizeof(awc_event_t));
    }
}

int parseInfoStringToken(void** item, const char* delim, char** context)
{
    return parseStringToken((char**)item, delim, context);
}

int parseInfoNumericToken(void** item, const char* delim, char** context)
{
    return parseNumericTokenU32((UINT32*)item, delim, context);
}


inline int parseInfo(awe_awc_t* awc, char** context)
{
    int ret = E_AWC_ERROR;
    if (awc && context && *context)
    {
        AWC_LOGD("Parse Info");
        char* token = NULL;
        token = strtok_r(NULL, AWC_DELIMITER, context);
        if (token != NULL)
        {
            typedef int(*parseFn_t)(void**, const char*, char**);
            typedef void(*postParseFn_t)(awe_awc_t*);
            typedef struct infoParserTable
            {
                const char* name;
                unsigned memberoffset;
                parseFn_t parser;
                postParseFn_t postParser;
            }infoParserTable_t;
            infoParserTable_t parserTable[] =
            {
                {"version",offsetof(awc_info_t, version), parseInfoStringToken, onParseDummy},
                {"date",offsetof(awc_info_t, date), parseInfoStringToken, onParseDummy},
                {"description",offsetof(awc_info_t, description), parseInfoStringToken, onParseDummy},
                {"ctl_delimiter",offsetof(awc_info_t, ctlname_delimiter), parseInfoStringToken, onParseDummy},
                {"nr_ctl",offsetof(awc_info_t, controlcount), parseInfoNumericToken, onParseControlCount},
                {"nr_mod",offsetof(awc_info_t, modulecount), parseInfoNumericToken, onParseModuleCount},
                {"nr_awb",offsetof(awc_info_t, designcount), parseInfoNumericToken, onParseDesignCount},
                {"nr_evts",offsetof(awc_info_t, eventcount), parseInfoNumericToken, onParseEventsCount},
                {"schema_version",offsetof(awc_info_t, schema), parseInfoNumericToken, onParseDummy}
            };

            *(token - 1) = AWC_DELIMITER[0]; // restore token in original string

            for (int i = 0; i < sizeof(parserTable)/sizeof(infoParserTable_t); i++) {
                if (tokenCompare(token, parserTable[i].name) == E_AWC_SUCCESS)
                {
                    char* elem = (char*)&awc->info;
                    ret = parserTable[i].parser((void**)&(elem[parserTable[i].memberoffset]), AWC_DELIMITER, context);
                    parserTable[i].postParser(awc);
                    break;
                }
            }
            if (ret != E_AWC_SUCCESS)
            {
                AWC_LOGE("Unknown Info Token: %s", token);
            }
        }
    }
    return ret;
}

inline int parseDesign(awe_awc_t* awc, char** context)
{
    int ret = E_AWC_ERROR;
    if (awc && context && *context && awc->designs && awc->designindex < awc->info.designcount)
    {
        AWC_LOGD("Parse Design %p %d %d", awc->designs , awc->designindex , awc->info.designcount);
        awc_design_t* design = &awc->designs[awc->designindex];
        if (parseNumericTokenU32(&design->coreid_objectid, AWC_DELIMITER, context) == E_AWC_SUCCESS)
        {
            if (parseStringToken(&design->name, AWC_DELIMITER, context) == E_AWC_SUCCESS)
            {
                if(parseStringToken(&design->file, AWC_DELIMITER, context) == E_AWC_SUCCESS)
                {
                    // Parse optional size and md5sum
                    parseNumericTokenU32(&design->size, AWC_DELIMITER, context);
                    parseStringToken(&design->md5sum, AWC_DELIMITER, context);
                    awc->designindex++;
                    ret = E_AWC_SUCCESS;
                }
                else
                {
                    designDestructor(design);
                    AWC_LOGE("Error during Parsing design file");
                }
            }
            else
            {
                designDestructor(design);
                AWC_LOGE("Error during Parsing design name");
            }
        }
        else
        {
            AWC_LOGE("Error during Parsing design Index");
        }
    }
    return ret;
}

inline int parseEnum(awe_awc_t* awc, char** context)
{
    int ret = E_AWC_ERROR;
    if (awc && context && *context)
    {
        AWC_LOGD("Parse Control Enum");
        char* token = NULL;
        token = strtok_r(NULL, AWC_DELIMITER, context);
        if (token != NULL)
        {
            *(token - 1) = AWC_DELIMITER[0]; // restore token in original string
            awc_ctl_t* pCtl = awc_get_control_from_awc(awc, token);
            if(pCtl && pCtl->type == AWC_CTL_ENUM)
            {
                if (parseNumericTokenU32(&pCtl->numenums, AWC_DELIMITER, context) == E_AWC_SUCCESS)
                {
                    pCtl->enums = calloc(pCtl->numenums, sizeof(char*));
                    if (pCtl->enums)
                    {
                        unsigned i = 0;
                        while(i < pCtl->numenums && parseStringToken(&pCtl->enums[i], AWC_DELIMITER, context) == E_AWC_SUCCESS)
                        {
                            i++;
                        }
                        ret = E_AWC_SUCCESS;
                    }
                }
            }
            else
            {
                AWC_LOGE("Control not found %s", token);
            }
        }
    }
    return ret;
}

int parseEvent(awe_awc_t *awc, char** context)
{
    int ret = E_AWC_ERROR;
    if (awc && context && *context && awc->events && awc->eventindex < awc->info.eventcount)
    {
        AWC_LOGD("Parse Event");
        UINT32 id = 0;
        if (parseNumericTokenU32(&id, AWC_DELIMITER, context) == E_AWC_SUCCESS)
        {
            for (UINT32 idx = 0; idx < awc->info.modulecount; idx++)
            {
                awc_module_t* pMdl = &awc->modules[idx];
                if(pMdl->objectid == id)
                {
                    awc_event_t* evt = &awc->events[awc->eventindex];
                    awc->eventindex++;
                    evt->mod = pMdl;
                    return E_AWC_SUCCESS;
                }
            }
            AWC_LOGE("Could not find Module");
        }
    }
    return ret;
}

static inline int parseUserDataValue(char** context, awc_dict_element* elem)
{
    int ret = E_AWC_ERROR;
    char* token = strtok_r(NULL, AWC_DELIMITER, context);
    if (token != NULL)
    {
        *(token - 1) = AWC_DELIMITER[0]; // restore token in original string
        errno = 0;
        char *endptr = NULL;
        switch(elem->type)
        {
            case AWC_USRDATA_UINT:
                elem->value.u32 = (uint32_t)strtoul(token, &endptr, 10);
                if (errno == 0 && *endptr == '\0')
                {
                    ret = E_AWC_SUCCESS;
                }
                break;
            case AWC_USRDATA_INT:
                elem->value.i32 = (int32_t)strtol(token, &endptr, 10);
                if (errno == 0 && *endptr == '\0')
                {
                    ret = E_AWC_SUCCESS;
                }
                break;
            case AWC_USRDATA_FLOAT:
                elem->value.f32 = strtof(token, &endptr);
                if (errno == 0 && *endptr == '\0')
                {
                    ret = E_AWC_SUCCESS;
                }
                break;
            case AWC_USRDATA_STR:
                elem->value.str = strdup(token);
                if (elem->value.str != NULL)
                {
                    ret = E_AWC_SUCCESS;
                }
                break;
            default:
                AWC_LOGE("Unsupported element type: %d", (int)elem->type);
                break;
        }
    }
    return ret;
}

static inline int parseUserDataType(char** context, awc_dict_element* elem)
{
    int ret = E_AWC_ERROR;
    char* token = strtok_r(NULL, AWC_DELIMITER, context);
    if (token != NULL)
    {
        *(token - 1) = AWC_DELIMITER[0]; // restore token in original string
        ret = E_AWC_SUCCESS;
        if(strcmp("uint", token) == 0)
        {
            elem->type = AWC_USRDATA_UINT;
        }
        else if(strcmp("int", token) == 0)
        {
            elem->type = AWC_USRDATA_INT;
        }
        else if(strcmp("float", token) == 0)
        {
            elem->type = AWC_USRDATA_FLOAT;
        }
        else if(strcmp("str", token) == 0)
        {
            elem->type = AWC_USRDATA_STR;
        }
        else
        {
            AWC_LOGE("Unsupported user data type %s", token);
            ret = E_AWC_ERROR;
        }
    }
    return ret;
}

static inline int parseDictionaryElement(awe_awc_t *awc, char** context, awc_dict_element* elem)
{
    int ret = parseStringToken(&elem->key, AWC_DELIMITER, context);
    if(E_AWC_SUCCESS == ret)
    {
        ret = parseUserDataType(context, elem);
        if(E_AWC_SUCCESS == ret)
        {
            ret = parseUserDataValue(context, elem);
        }
    }
    if(ret != E_AWC_SUCCESS)
    {
        freeDictionaryItem(elem);
    }
    return ret;
}

int parseControlUserData(awe_awc_t *awc, char** context)
{
    int ret = E_AWC_ERROR;
    if (awc && context && *context)
    {
        AWC_LOGD("Parse Control User Data");
        char* token = strtok_r(NULL, AWC_DELIMITER, context);
        if (token != NULL)
        {
            *(token - 1) = AWC_DELIMITER[0]; // restore token in original string
            awc_ctl_t* ctl_p = getControlByName(awc, token);
            if(ctl_p != NULL)
            {
                awc_dict_element elem = {0};
                if(parseDictionaryElement(awc, context, &elem) == E_AWC_SUCCESS)
                {
                    return addToDictionary(&ctl_p->userdata, &elem);
                }
            }
            else
            {
                AWC_LOGE("Could not find Control");
            }
        }
    }
    return ret;
}

int parseModuleUserData(awe_awc_t *awc, char** context)
{
    int ret = E_AWC_ERROR;
    if (awc && context && *context)
    {
        AWC_LOGD("Parse Module User Data");
        char* token = strtok_r(NULL, AWC_DELIMITER, context);
        if (token != NULL)
        {
            *(token - 1) = AWC_DELIMITER[0]; // restore token in original string
            awc_module_t* mod_p = getModuleByName(awc, token);
            if(mod_p != NULL)
            {
                awc_dict_element elem = {0};
                if(parseDictionaryElement(awc, context, &elem) == E_AWC_SUCCESS)
                {
                    if( strcmp(elem.key, "tunnelAddress") == 0 && elem.type == AWC_USRDATA_INT)
                    {
                        mod_p->tunnelAddress = elem.value.i32;
                        // Only overwrite the coreId if not set, potentially by instanceId or module variable.
                        if(mod_p->coreid == 0)
                        {
                            mod_p->coreid = AWE_COREID_FROM_TUNNELADDR(mod_p->tunnelAddress);
                        }
                        return E_AWC_SUCCESS;
                    }
                    else if( strcmp(elem.key, "instanceId") == 0 && elem.type == AWC_USRDATA_INT)
                    {
                        mod_p->coreid = elem.value.i32;
                        return E_AWC_SUCCESS;
                    }
                    else
                    {
                        return addToDictionary(&mod_p->userdata, &elem);
                    }
                }
            }
            else
            {
                AWC_LOGE("Could not find Module");
            }
        }
    }
    return ret;
}

int parseTopUserData(awe_awc_t *awc, char** context)
{
    int ret = E_AWC_ERROR;
    if (awc && context && *context)
    {
        AWC_LOGD("Parse TopLevel User Data");
        char* token = strtok_r(NULL, AWC_DELIMITER, context);
        if (token != NULL)
        {
            *(token - 1) = AWC_DELIMITER[0]; // restore token in original string
            awc_dict_element elem = {0};
            if(parseDictionaryElement(awc, context, &elem) == E_AWC_SUCCESS)
            {
                return addToDictionary(&awc->userdata, &elem);
            }
        }
    }
    return ret;
}


inline int tokenCompare(const char *token, const char *type)
{
    int ret = E_AWC_ERROR;
    if (token != NULL && strcmp(token, type) == 0)
    {
        ret = E_AWC_SUCCESS;
    }
    return ret;
}

awc_dict_element* searchInDictionary(const awc_dictionary* dict, const char* name)
{
    if(dict != NULL)
    {
        if(dict->data && dict->size)
        {
            for(uint32_t i = 0; i < dict->size; i++)
            {
                if(tokenCompare(name, dict->data[i].key) == E_AWC_SUCCESS)
                {
                    return &dict->data[i];
                }
            }
        }
    }
    return NULL;
}

inline int addToDictionary(awc_dictionary* dict, awc_dict_element* elem)
{
    if((dict != NULL) && (elem != NULL))
    {
        if(searchInDictionary(dict, elem->key) == NULL)
        {
            awc_dict_element* new_p = realloc(dict->data, (dict->size+1)*sizeof(awc_dict_element));
            if(new_p != NULL)
            {
                dict->data = new_p;
                dict->data[dict->size].key =elem->key;
                dict->data[dict->size].type = elem->type;
                dict->data[dict->size].value = elem->value;
                dict->size++;
                return E_AWC_SUCCESS;
            }
        }
    }
    freeDictionaryItem(elem); // If we reach here, add to dictionary is failed. Free
    return E_AWC_ERROR;
}

awc_module_t* getModuleByName(const awe_awc_t *awc, const char *name)
{
    if ((awc != NULL) && (name != NULL))
    {
        // Map based search, works with alias and name
        return (awc_module_t*)awc_map_search(awc->moduleMap, name);
    }
    return NULL;
}

awc_ctl_t* getControlByName(const awe_awc_t *awc, const char *name)
{
    if ((awc != NULL) && (name != NULL))
    {
        // Map based search, works with alias and name
        return (awc_ctl_t*)awc_map_search(awc->controlMap, name);
    }
    return NULL;
}

void freeUserData(awc_dictionary* dict)
{
    if ((dict != NULL) && (dict->size > 0))
    {
        AWC_LOGD("Freeing userdata...");
        if(dict->data != NULL)
        {
            for(uint32_t i = 0; i < dict->size; i++)
            {
                awc_dict_element* elem = &dict->data[i];
                freeDictionaryItem(elem);
            }
            free(dict->data);
            dict->data = NULL;
            dict->size = 0;
        }
    }
}

void freeDictionaryItem(awc_dict_element* elem)
{
    if(elem != NULL)
    {
        free(elem->key);
        elem->key = NULL;
        if(elem->type == AWC_USRDATA_STR)
        {
            free(elem->value.str);
            elem->value.str = NULL;
        }
    }
}

void freeEvents(awe_awc_t *awc)
{
    if (awc != NULL)
    {
        AWC_LOGD("Destructing Events...");
        free(awc->events);
        awc->events = NULL;
    }
}

void freeInfo(awe_awc_t *awc)
{
    if (awc != NULL)
    {
        AWC_LOGD("Destructing Info...");
        free(awc->info.version);
        awc->info.version = NULL;

        free(awc->info.date);
        awc->info.date = NULL;

        free(awc->info.description);
        awc->info.description = NULL;

        free(awc->info.ctlname_delimiter);
        awc->info.ctlname_delimiter = NULL;
    }
}

void freeModules(awe_awc_t *awc)
{
    if (awc != NULL)
    {
        AWC_LOGD("Destructing Designs...");
        for (UINT32 idx = 0; idx < awc->info.modulecount; idx++)
        {
            awc_module_t* pMdl = &awc->modules[idx];
            moduleDestructor(pMdl);
        }
        free(awc->modules);
        awc->modules = NULL;
    }
}

void freeDesigns(awe_awc_t *awc)
{
    if (awc != NULL)
    {
        AWC_LOGD("Destructing Designs...");
        for (UINT32 idx = 0; idx < awc->info.designcount; idx++)
        {
            awc_design_t* pDsn = &awc->designs[idx];
            designDestructor(pDsn);
        }
        free(awc->designs);
        awc->designs = NULL;
    }
}

void freeControls(awe_awc_t *awc)
{
    if (awc != NULL)
    {
        AWC_LOGD("Destructing Controls...");
        for (UINT32 idx = 0; idx < awc->info.controlcount; idx++)
        {
            awc_ctl_t* pCtl = &awc->controls[idx];
            controlDestructer(pCtl);
        }
        free(awc->controls);
        awc->controls = NULL;
    }
}

void awcDestructor(awe_awc_t *awc)
{
    if (awc != NULL)
    {
        AWC_LOGD("Destructing AWC...");
        freeEvents(awc);
        freeInfo(awc);
        freeDesigns(awc);
        freeModules(awc);
        freeControls(awc);
        freeUserData(&awc->userdata);
        awc_map_free(&awc->moduleMap);
        awc_map_free(&awc->controlMap);
    }
}

void controlDestructer(awc_ctl_t* pControl)
{
    if (pControl != NULL)
    {
        AWC_LOGD("Destructing Control: %s", pControl->fullname);
        controlEnumDestructer(pControl);

        free(pControl->fullname);
        pControl->fullname = NULL;

        free(pControl->alias);
        pControl->alias = NULL;

        freeUserData(&pControl->userdata);
    }
}

void controlEnumDestructer(awc_ctl_t* pControl)
{
    if ((pControl != NULL) && (pControl->type == AWC_CTL_ENUM) && (pControl->enums != NULL))
    {
        for (unsigned int i = 0; i < pControl->numenums; i++)
        {
            free(pControl->enums[i]);
            pControl->enums[i] = NULL;
        }
        free(pControl->enums);
        pControl->enums = NULL;
    }
}

void moduleDestructor(awc_module_t * pModule)
{
    if (pModule != NULL)
    {
        AWC_LOGD("Destructing Module: %s", pModule->name);
        free(pModule->controls);
        pModule->controls = NULL;

        free(pModule->name);
        pModule->name = NULL;

        free(pModule->alias);
        pModule->alias = NULL;

        freeUserData(&pModule->userdata);
    }
}

void designDestructor(awc_design_t *pDesign)
{
    if (pDesign != NULL)
    {
        AWC_LOGD("Destructing Design: %s", pDesign->name);
        free(pDesign->file);
        pDesign->file = NULL;

        free(pDesign->name);
        pDesign->name = NULL;

        free(pDesign->md5sum);
        pDesign->md5sum = NULL;
    }
}

//Remove trailing spaces, line endings, tabs
void rtrim(char *str) {
    size_t len = strlen(str);
    if (len == 0) return; // If the string is empty, just return

    // Start from the end of the string
    size_t i = len - 1;

    // Iterate backwards until a non-whitespace character is found
    while (i >= 0 && (isspace((unsigned char)str[i]))) {
        i--;
    }

    // Null-terminate the string at the new position
    str[i + 1] = '\0';
}

int parseLine(char *line, awe_awc_t *awc)
{
    int ret = E_AWC_SUCCESS;
    const char delimiter[2] = AWC_DELIMITER;
    char *token;
    char* context = NULL;
    char** pContext = &context;
    if (!awc || !line)
    {
        return E_AWC_ERROR;
    }
    rtrim(line);
    AWC_LOGD("Parsing Line %s", line);
    token = strtok_r(line, delimiter, pContext);
    if(!token)
    {
        //TODO: What to do if some unknown text is int the file.
        //Ignore the line for now.
        return ret;
    }
    typedef int(*lineparseFn_t)(awe_awc_t*, char**);
    typedef struct lineParserTable
    {
        const char* name;
        lineparseFn_t parser;
    }lineParserTable_t;
    lineParserTable_t parserTable[] =
    {
        {"_MOD_",parseModule},
        {"_CTL_",parseControl},
        {"_AWB_",parseDesign},
        {"_INFO_",parseInfo},
        {"_ENUM_",parseEnum},
        {"_EVT_",parseEvent},
        {"_CTLDATA_",parseControlUserData},
        {"_MODDATA_",parseModuleUserData},
        {"_TOPDATA_",parseTopUserData},
    };
    for (int i = 0; i < sizeof(parserTable)/sizeof(lineParserTable_t); i++) {
        if (tokenCompare(token, parserTable[i].name) == E_AWC_SUCCESS)
        {
            ret = parserTable[i].parser(awc, pContext);
            break;
        }
    }
    return ret;
}
