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


#include "awe_awc.h"
#include <stdio.h>

#include "awemgr_logging.h"


void* handle;
void printRange(const awc_ctl_range_t *range)
{
	if (range != NULL)
	{
		printf("(default: %f, min: %f, max: %f, step: %f)\n", range->def, range->min, range->max, range->step);
	}
}

void printEnums(const awc_ctl_t *ctl)
{
	if ((ctl != NULL) && (ctl->numenums > 0) && (ctl->enums != NULL) && (ctl->type == AWC_CTL_ENUM))
	{
		printf(", Enums: ");
		for (unsigned int i = 0; i < ctl->numenums; i++)
		{
			if (ctl->enums[i])
				printf("%s, ", ctl->enums[i]);
		}
	}
}

void printControl(const awc_ctl_t* pCTL, void *usr_data_p)
{
	if (pCTL != NULL)
	{
		printf("\tControl: %s, handle: %u, size: %u, offset: %u, type: %s", pCTL->fullname, pCTL->handle, pCTL->size, pCTL->offset, awc_get_typename(pCTL->type));
		printEnums(pCTL);
		printRange(&pCTL->range);
	}
}

void printModule(const awc_module_t* pMod, void *usr_data_p)
{
	if (pMod != NULL)
	{
		printf("\nModule: %s, classId: %u, objectId: %u, size: %u\n", pMod->name, pMod->classid, pMod->objectid, pMod->size);
		awc_foreach_control(pMod, printControl, NULL);
	}
}

void printDesign(const awc_design_t * pDesign, void *usr_data_p)
{
	if (pDesign != NULL)
	{
		printf("\nDesign: %s, file: %s, size: %u, md5sum: %s \n", pDesign->name, pDesign->file, pDesign->size, pDesign->md5sum);
	}
}

int main(int argc, char* argv[])
{
	const char* awcfile = AWC_DATA_DIR "/awc_file_schema3.txt";
	printf("AWC File: %s", awcfile);
	if (argc == 2) {
		awcfile = argv[1];
	}
	else
	{
		printf("Usage: %s <awcfilename>\n", argv[0]);
		printf("Trying to parse %s\n", awcfile);
	}
	set_loglevel(AWEMGR_LOG_AWC, AWEMGR_LOG_LEVEL_INFO);
	handle = awc_init(awcfile);

	if (handle != NULL)
	{

		printf("\nAWC File Parsed Successfully\n");

		printf("\nDesign : Automatic Discovery ...\n");

		awc_foreach_design(handle, printDesign, NULL);

		printf("\nModules : Automatic Discovery ...\n");

		awc_foreach_module(handle, printModule, NULL);

		printf("\nManual Module Search ...\n");

		awc_module_t *mod = awc_get_module(handle, "GainControllerClass");
		mod = awc_get_module(handle, "GainControllerClass");
		printModule(mod, NULL);
		mod = awc_get_module(handle, "SampleModuleOut");
		printModule(mod, NULL);

		printf("\nControl Search ...\n");
		awc_ctl_t *ctl = awc_get_control_from_awc(handle, "GainControllerClass/left_channel");
		printControl(ctl, NULL);

		printf("\nIndex based Enumeration ...\n");
		UINT32 designcount = awc_design_count(handle);
		UINT32 modulecount = awc_module_count(handle);
		UINT32 controlcount = awc_control_count(handle);

		printf("\nDesigns ...\n");
		for (UINT32 idx = 0; idx < designcount; idx++)
		{
			printDesign(awc_get_design_by_index(handle, idx), NULL);
		}

		printf("\nModules ...\n");
		for (UINT32 idx = 0; idx < modulecount; idx++)
		{
			printModule(awc_get_module_by_index(handle, idx), NULL);
		}

		printf("\nControls ...\n");
		for (UINT32 idx = 0; idx < controlcount; idx++)
		{
			printControl(awc_get_control_by_index(handle, idx), NULL);
		}

		awc_uninit(&handle);
	}
	else
	{
		printf("\nAWC File Parsing Failed: %s\n", awcfile);
		return -1;
	}
	return 0;
}