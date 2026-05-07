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


#ifndef _AWC_INTERNAL_H
#define _AWC_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif
#include "awe_awc.h"
#include "awc_map.h"
#define MAX_MODULE_NAME_SIZE (256)
#define MAX_CTRL_NAME_SIZE (MAX_MODULE_NAME_SIZE/2)
#define MAX_CTL_FULLNAME_SIZE (MAX_MODULE_NAME_SIZE + MAX_CTRL_NAME_SIZE + 2)
#define MAX_ALIAS_SIZE (MAX_CTRL_NAME_SIZE)
#define MAX_LINE_SIZE (MAX_CTL_FULLNAME_SIZE + MAX_ALIAS_SIZE + 50)
#define AWC_DELIMITER ","
typedef struct
{
    awc_info_t info;
    awc_design_t* designs;
    awc_module_t* modules;
    awc_ctl_t* controls;
    awc_event_t* events;
    AwcMap* controlMap;
    AwcMap* moduleMap;
    awc_dictionary userdata;
    UINT32 designindex;  // Private: Only used during parsing
    UINT32 moduleindex;  // Private: Only used during parsing
    UINT32 controlindex; // Private: Only used during parsing
    UINT32 eventindex;  // Private: Only used during parsing
} awe_awc_t;

int stringToU32(const char *str, UINT32 *number);
int parseLine(char *line, awe_awc_t *awc);
int parseNumericTokenU32(UINT32 *number, const char *delimiter, char** context);
int parseStringToken(char** str, const char *delimiter, char** context);
int parseInfoStringToken(void** item, const char* delim, char** context);
int parseInfoNumericToken(void** item, const char* delim, char** context);
int onParseDummy(awe_awc_t *awc);
int onParseControlCount(awe_awc_t *awc);
int onParseDesignCount(awe_awc_t *awc);
int onParseModuleCount(awe_awc_t *awc);
int onParseEventsCount(awe_awc_t *awc);
int validateControlModule(awc_module_t *pMod);
int validateControlParsing(awe_awc_t *awc, char** context);
int validateModuleParsing(awe_awc_t *awc, char** context);
int splitControlName(awc_ctl_t *pCtl, const char* delim);
int parseType(awc_ctl_type_t *type, char** context);
int parseModule(awe_awc_t *awc, char** context);
int parseControl(awe_awc_t* awc, char** context);
int parseInfo(awe_awc_t* awc, char** context);
int parseDesign(awe_awc_t* awc, char** context);
int parseEnum(awe_awc_t* awc, char** context);
int parseEvent(awe_awc_t *awc, char** context);
int parseControlUserData(awe_awc_t *awc, char** context);
int parseModuleUserData(awe_awc_t *awc, char** context);
int parseTopUserData(awe_awc_t *awc, char** context);
int tokenCompare(const char *token, const char *type);
awc_dict_element* searchInDictionary(const awc_dictionary*, const char* name);
int addToDictionary(awc_dictionary*, awc_dict_element* elem);
awc_module_t* getModuleByName(const awe_awc_t *awc, const char *name);
awc_ctl_t* getControlByName(const awe_awc_t *awc, const char *name);
void freeUserData(awc_dictionary* dict);
void freeDictionaryItem(awc_dict_element* elem);
void freeEvents(awe_awc_t *awc);
void freeInfo(awe_awc_t *awc);
void freeModules(awe_awc_t *awc);
void freeDesigns(awe_awc_t *awc);
void freeControls(awe_awc_t *awc);
void awcDestructor(awe_awc_t *awc);
void controlDestructer(awc_ctl_t *control);
void controlEnumDestructer(awc_ctl_t* control);
void moduleDestructor(awc_module_t *module);
void designDestructor(awc_design_t *design);
void ftrim(char *str);
void rtrim(char *str);
int validateAwc(awe_awc_t *awc);

#ifdef __cplusplus
}
#endif
#endif /*_AWC_INTERNAL_H*/