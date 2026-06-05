#include "awc_test_fixtures.hpp"
#include "awe_awc.h"
#include "awemgr_logging.h"
#include "awc_internal.h"

unsigned int designCbCount = 0;
unsigned int moduleCbCount = 0;
unsigned int controlCbCount = 0;
unsigned int eventCbCount = 0;
/**
```yaml
- id: utest~AWEMGR.AWC.TestInfo~2
  covers: dsn~AWEMGR.AWC.VersionInfo~1
  description: |
    Ensures an information structure is correctly parsed.
```
*/
TEST_F(AWCTestFixture, TestInfo)
{
	ASSERT_TRUE(awc != NULL);
	ASSERT_TRUE(awc_get_info(NULL) == NULL);
	auto info = awc_get_info(awc);
	ASSERT_TRUE(info != NULL);

	ASSERT_STREQ(info->version, "0.0.1");
	ASSERT_STREQ(info->date, "18 Mar 2024");
	ASSERT_STREQ(info->description, "Sample AWC File");
	ASSERT_STREQ(info->ctlname_delimiter, "|");
	ASSERT_EQ(info->schema, 0);
	ASSERT_EQ(info->designcount, 3);
	ASSERT_EQ(info->modulecount, 3);
	ASSERT_EQ(info->controlcount, 8);
}

/**
```yaml
- id: utest~AWEMGR.AWC.DesignAccessByIndex~2
  covers: dsn~AWEMGR.AWC.AweDataByIndex~1
  description: |
    Ensures that an AWC design object can be retrieved given an
    index value (rather than a name).
```
*/
TEST_F(AWCTestFixture, DesignAccessByIndex)
{
	ASSERT_TRUE(awc != NULL);
	ASSERT_EQ(awc_design_count(NULL), 0);
	ASSERT_EQ(awc_design_count(awc), 3);

	auto design = awc_get_design_by_index(awc, 0);
	ASSERT_TRUE(design != NULL);
	ASSERT_STREQ(design->name, "Main");
	ASSERT_STREQ(design->file, "sample_design.awb");

	design = awc_get_design_by_index(awc, 1);
	ASSERT_TRUE(design != NULL);
	ASSERT_STREQ(design->name, "Preset1");
	ASSERT_STREQ(design->file, "Preset1.awb");

	design = awc_get_design_by_index(awc, 2);
	ASSERT_TRUE(design != NULL);
	ASSERT_STREQ(design->name, "Preset2");
	ASSERT_STREQ(design->file, "Preset2.awb");
}

/**
```yaml
- id: utest~AWEMGR.AWC.DesignAccessByName~1
  covers:
    - dsn~AWEMGR.AWC.NameToAweData~1
    - dsn~AWEMGR.AWC.AwbDataInfo~1
  description: |
    Ensures that an AWC design object can be retrieved given a
    name (rather than an index value).
```
*/
TEST_F(AWCTestFixture, DesignAccessByName)
{
	ASSERT_TRUE(awc != NULL);
	auto design = awc_get_design(awc, "Main");
	ASSERT_TRUE(design != NULL);

	design = awc_get_design(awc, "Preset1");
	ASSERT_TRUE(design != NULL);

	design = awc_get_design(awc, "Preset2");
	ASSERT_TRUE(design != NULL);
}

/**
```yaml
- id: utest~AWEMGR.AWC.ForEachDesign~2
  covers: dsn~AWEMGR.AWC.AwbDataInfo~1
  description: |
    Checks the looping method by counting the designs.
    It also checks if the generic user data pointer is consumed.
```
*/
TEST_F(AWCTestFixture, ForEachDesign)
{
	int x = 1234;
	ASSERT_TRUE(awc != NULL);
	auto lambda = [](const awc_design_t *design, void *usr_data_p)
	{
		ASSERT_TRUE(design != NULL);
		int *pX = (int *)usr_data_p;
		ASSERT_TRUE(pX != NULL);
		ASSERT_EQ(*pX, 1234);
		designCbCount++;
	};
	ASSERT_EQ(awc_foreach_design(NULL, lambda, &x), E_AWC_ERROR);
	ASSERT_EQ(awc_foreach_design(awc, NULL, &x), E_AWC_ERROR);
	ASSERT_EQ(awc_foreach_design(awc, lambda, &x), E_AWC_SUCCESS);
	ASSERT_EQ(designCbCount, 3);
	designCbCount = 0;
}

/**
```yaml
- id: utest~AWEMGR.AWC.ForEachModuleAndControl~1
  covers: dsn~AWEMGR.AWC.AweDataByIndex~1
  description: |
    Checks the looping method by counting the modules and controls.
```
*/
TEST_F(AWCTestFixture, ForEachModuleAndControl)
{
	int x = 1234;
	ASSERT_TRUE(awc != NULL);
	auto moduleFnc = [](const awc_module_t *module, void *usr_data_p)
	{
		ASSERT_TRUE(module != NULL);
		int *pX = (int *)usr_data_p;
		ASSERT_TRUE(pX != NULL);
		ASSERT_EQ(*pX, 1234);
		auto controlFnc = [](const awc_ctl_t *control, void *usr_data_p)
		{
			ASSERT_TRUE(control != NULL);
			int *pY = (int *)usr_data_p;
			ASSERT_TRUE(pY != NULL);
			ASSERT_EQ(*pY, 5678);
			controlCbCount++;
		};
		int y = 5678;
		ASSERT_EQ(awc_foreach_control(NULL, controlFnc, &y), E_AWC_ERROR);
		ASSERT_EQ(awc_foreach_control(module, NULL, &y), E_AWC_ERROR);
		ASSERT_EQ(awc_foreach_control(module, controlFnc, &y), E_AWC_SUCCESS);
		moduleCbCount++;
	};

	ASSERT_EQ(awc_foreach_module(NULL, moduleFnc, &x), E_AWC_ERROR);
	ASSERT_EQ(awc_foreach_module(awc, NULL, &x), E_AWC_ERROR);
	ASSERT_EQ(awc_foreach_module(awc, moduleFnc, &x), E_AWC_SUCCESS);
	ASSERT_EQ(moduleCbCount, 3);
	ASSERT_EQ(controlCbCount, 8);
	moduleCbCount = 0;
	controlCbCount = 0;
}

/**
```yaml
- id: utest~AWEMGR.AWC.CtlTypeName~2
  covers: dsn~AWEMGR.AWC.OwnVarTypes~1
  description: Checks the name of the awc_ctl_type_t
```
*/
TEST_F(AWCTestFixture, VarTypeString)
{

	ASSERT_STREQ(awc_get_typename(AWC_CTL_BOOL), "bool");
	ASSERT_STREQ(awc_get_typename(AWC_CTL_INT32), "int");
	ASSERT_STREQ(awc_get_typename(AWC_CTL_FRACT32), "fract32");
	ASSERT_STREQ(awc_get_typename(AWC_CTL_UINT32), "uint");
	ASSERT_STREQ(awc_get_typename(AWC_CTL_FLOAT), "float");
	ASSERT_STREQ(awc_get_typename(AWC_CTL_ENUM), "enum");
	ASSERT_STREQ(awc_get_typename((awc_ctl_type_t)0), "invalid");
}

/**
```yaml
- id: utest~AWEMGR.AWC.ModuleAccessByIndex~2
  covers: dsn~AWEMGR.AWC.AweDataByIndex~1
  description: |
    Ensures a module object can be retrieved by index value.
```
*/
TEST_F(AWCTestFixture, ModuleAccessByIndex)
{
	ASSERT_TRUE(awc != NULL);
	ASSERT_EQ(awc_module_count(NULL), 0);
	ASSERT_EQ(awc_module_count(awc), 3);
	ASSERT_TRUE(awc_get_module_by_index(NULL, 0) == NULL);
	ASSERT_TRUE(awc_get_module_by_index(awc, 10000) == NULL);
	auto module = awc_get_module_by_index(awc, 0);
	ASSERT_TRUE(module != NULL);
	ASSERT_STREQ(module->name, "GainControllerClass");
	ASSERT_EQ(module->classid, 2270400520);
	ASSERT_EQ(module->objectid, 0);
	ASSERT_EQ(module->size, 4);
	ASSERT_EQ(awc_module_control_count(module), 4);

	module = awc_get_module_by_index(awc, 1);
	ASSERT_TRUE(module != NULL);
	ASSERT_STREQ(module->name, "SampleModuleOut");
	ASSERT_EQ(module->classid, 2270408712);
	ASSERT_EQ(module->objectid, 0);
	ASSERT_EQ(module->size, 3);
	ASSERT_EQ(awc_module_control_count(module), 3);

	module = awc_get_module_by_index(awc, 2);
	ASSERT_TRUE(module != NULL);
	ASSERT_STREQ(module->name, "VectorModule");
	ASSERT_EQ(module->classid, 2270408713);
	ASSERT_EQ(module->objectid, 0);
	ASSERT_EQ(module->size, 1);
	ASSERT_EQ(awc_module_control_count(module), 1);
}

/**
```yaml
- id: utest~AWEMGR.AWC.ModuleAccessByName~1
  covers: dsn~AWEMGR.AWC.NameToAweData~1
  description: |
    Ensures a module object can be retrieved by a name.
```
*/
TEST_F(AWCTestFixture, ModuleAccessByName)
{
	ASSERT_TRUE(awc != NULL);
	ASSERT_TRUE(awc_get_module(NULL, "GainControllerClass") == NULL);
	ASSERT_TRUE(awc_get_module(awc, "") == NULL);
	auto module = awc_get_module(awc, "GainControllerClass");
	ASSERT_TRUE(module != NULL);

	module = awc_get_module(awc, "SampleModuleOut");
	ASSERT_TRUE(module != NULL);

	module = awc_get_module(awc, "VectorModule");
	ASSERT_TRUE(module != NULL);
}

/**
```yaml
- id: utest~AWEMGR.AWC.ControlAccessByIndex~2
  covers: dsn~AWEMGR.AWC.AweDataByIndex~1
  description: |
    Ensures a control object can be retrieved by index value.
```
*/
TEST_F(AWCTestFixture, ControlAccessByIndex)
{
	ASSERT_TRUE(awc != NULL);
	ASSERT_EQ(awc_control_count(NULL), 0);
	ASSERT_EQ(awc_control_count(awc), 8);
	ASSERT_TRUE(awc_get_control_by_index(NULL, 0) == NULL);
	ASSERT_TRUE(awc_get_control_by_index(awc, 10000) == NULL);
	auto control = awc_get_control_by_index(awc, 0);
	ASSERT_TRUE(control != NULL);
	ASSERT_STREQ(control->fullname, "GainControllerClass|left_channel");
	ASSERT_STREQ(control->varname, "left_channel");
	ASSERT_EQ(control->handle, 2270400520);
	ASSERT_EQ(control->size, 1);
	ASSERT_EQ(control->offset, 0);
	ASSERT_EQ(control->range.def, 0);
	ASSERT_EQ(control->range.min, -50);
	ASSERT_EQ(control->range.max, 5);
	ASSERT_EQ(control->range.step, 0);
	ASSERT_EQ(control->type, AWC_CTL_INT32);
	ASSERT_TRUE(awc_get_module_control_by_index(NULL, 0) == NULL);
	ASSERT_TRUE(awc_get_module_control_by_index(control->module, 10000) == NULL);
	ASSERT_TRUE(awc_get_module_control_by_index(control->module, 0) == control);

	control = awc_get_control_by_index(awc, 1);
	ASSERT_STREQ(control->fullname, "GainControllerClass|right_channel");

	control = awc_get_control_by_index(awc, 2);
	ASSERT_STREQ(control->fullname, "GainControllerClass|mute_all");

	control = awc_get_control_by_index(awc, 3);
	ASSERT_STREQ(control->fullname, "GainControllerClass|EnumControl");
	ASSERT_EQ(control->numenums, 3);
	ASSERT_STREQ(control->enums[0], "low");
	ASSERT_STREQ(control->enums[1], "med");
	ASSERT_STREQ(control->enums[2], "high");

	control = awc_get_control_by_index(awc, 4);
	ASSERT_STREQ(control->fullname, "SampleModuleOut|event_counter");

	control = awc_get_control_by_index(awc, 5);
	ASSERT_STREQ(control->fullname, "SampleModuleOut|rms_left");

	control = awc_get_control_by_index(awc, 6);
	ASSERT_STREQ(control->fullname, "SampleModuleOut|rms_right");

	control = awc_get_control_by_index(awc, 7);
	ASSERT_STREQ(control->fullname, "VectorModule|VectorControl");
	ASSERT_EQ(control->size, 4);
}

/**
```yaml
- id: utest~AWEMGR.AWC.ControlAccessByName~2
  covers: dsn~AWEMGR.AWC.NameToAweData~1
  description: |
    Ensures a control object can be retrieved by a name.
```
*/
TEST_F(AWCTestFixture, ControlAccessByName)
{
	ASSERT_TRUE(awc != NULL);
	ASSERT_TRUE(awc_get_control_from_awc(NULL, "GainControllerClass|left_channel") == NULL);
	ASSERT_TRUE(awc_get_control_from_awc(awc, "") == NULL);
	auto control = awc_get_control_from_awc(awc, "GainControllerClass|left_channel");
	ASSERT_TRUE(control != NULL);

	control = awc_get_control_from_awc(awc, "GainControllerClass|right_channel");
	ASSERT_TRUE(control != NULL);

	control = awc_get_control_from_awc(awc, "GainControllerClass|mute_all");
	ASSERT_TRUE(control != NULL);

	control = awc_get_control_from_awc(awc, "GainControllerClass|EnumControl");
	ASSERT_TRUE(control != NULL);

	control = awc_get_control_from_awc(awc, "SampleModuleOut|event_counter");
	ASSERT_TRUE(control != NULL);

	control = awc_get_control_from_awc(awc, "SampleModuleOut|rms_left");
	ASSERT_TRUE(control != NULL);

	control = awc_get_control_from_awc(awc, "SampleModuleOut|rms_right");
	ASSERT_TRUE(control != NULL);

	control = awc_get_control_from_awc(awc, "VectorModule|VectorControl");
	ASSERT_TRUE(control != NULL);

	ASSERT_TRUE(awc_get_control_from_module(control->module, "VectorControl") == control);
	ASSERT_TRUE(awc_get_control_from_module(NULL, "VectorControl") == NULL);
	ASSERT_TRUE(awc_get_control_from_module(control->module, "falsecontrol") == NULL);

	// Invalid Control
	control = awc_get_control_from_awc(awc, "VectorModule|falsecontrol");
	ASSERT_TRUE(control == NULL);

	ASSERT_TRUE(awc_get_control_from_module(NULL, "VectorControl") == NULL);
}

/**
```yaml
- id: utest~AWEMGR.AWC.InvalidFile~1
  covers: dsn~AWEMGR.AWC.CheckConsistency~1
  description: |
    Checks that incorrect file names are handled.
```
*/
TEST_F(AWCTestInvalidFileFixture, InvalidFile)
{
	ASSERT_TRUE(awc == NULL);
}

/**
```yaml
- id: utest~AWEMGR.AWC.ValidLines~1
  covers: dsn~AWEMGR.AWC.CheckConsistency~1
  description: |
    Checks the parser and feeds it a bunch of valid lines.
```
*/
TEST_F(AWCTestsWithoutFilesFixture, ValidLines)
{
	ASSERT_EQ(TestParseLine("_INFO_,version,0.0.1"), E_AWC_SUCCESS);
	EXPECT_STREQ(awc->info.version, "0.0.1");

	ASSERT_EQ(TestParseLine("_INFO_,nr_ctl,10"), E_AWC_SUCCESS);
	ASSERT_TRUE(awc->controls != NULL);
	ASSERT_EQ(awc->info.controlcount, 10);

	ASSERT_EQ(TestParseLine("_INFO_,nr_mod,10"), E_AWC_SUCCESS);
	ASSERT_TRUE(awc->modules != NULL);
	ASSERT_EQ(awc->info.modulecount, 10);

	ASSERT_EQ(TestParseLine("_AWB_,0,Main,sample_design.awb"), E_AWC_ERROR);
	ASSERT_EQ(TestParseLine("_INFO_,nr_awb,3"), E_AWC_SUCCESS);
	ASSERT_TRUE(awc->designs != NULL);
	ASSERT_EQ(awc->info.designcount, 3);
	// Note: the next line is required before parsing any controls, to setup the delimiter.
	ASSERT_EQ(TestParseLine("_INFO_,ctl_delimiter,|"), E_AWC_SUCCESS);
	// Note: the next line is assummes that "_INFO_,nr_mod,xx" line is already parsed
	ASSERT_EQ(TestParseLine("_MOD_,GainControllerClass,2270400520,4"), E_AWC_SUCCESS);
	// Note: the next line is assummes that "_INFO_,nr_ctl,xx" line is already parsed
	ASSERT_EQ(TestParseLine("_CTL_,GainControllerClass|EnumControl,2270400520,1,3,0,0,2,0,enum"), E_AWC_SUCCESS);
	// Note: next line should result in a failure
	ASSERT_EQ(TestParseLine("_AWB_,0,Main,sample_design.awb"), E_AWC_SUCCESS);
	ASSERT_EQ(TestParseLine("_AWB_,1,Test,Test.awb"), E_AWC_SUCCESS);
	auto pDesign = awc_get_design(awc, "Test");
	ASSERT_TRUE(pDesign != NULL);
	ASSERT_STREQ(pDesign->md5sum, NULL);
	ASSERT_EQ(pDesign->size, 0);
	ASSERT_EQ(TestParseLine("_AWB_,2,Test1,Test1.awb,123,md5sum"), E_AWC_SUCCESS);
	pDesign = awc_get_design(awc, "Test1");
	ASSERT_TRUE(pDesign != NULL);
	ASSERT_STREQ(pDesign->md5sum, "md5sum");
	ASSERT_EQ(pDesign->size, 123);
}

/**
```yaml
- id: utest~AWEMGR.AWC.TestInvalidControlType~2
  covers: dsn~AWEMGR.AWC.CheckConsistency~1
  description: |
    Checks a selection of incorrectly formatted lines when readsing a control object.
```
*/
TEST_F(AWCTestsWithoutFilesFixture, TestInvalidControlType) {
	ASSERT_EQ(parseInfo(NULL, NULL), E_AWC_ERROR);
	ASSERT_EQ(parseDesign(NULL, NULL), E_AWC_ERROR);
	ASSERT_EQ(parseEvent(NULL, NULL), E_AWC_ERROR);
	ASSERT_EQ(TestParseLineNoAWC("_INFO_,nr_mod,1"), E_AWC_ERROR);
	ASSERT_EQ(TestParseLine("_INFO_,nr_mod,1"), E_AWC_SUCCESS);
	ASSERT_TRUE(awc->modules != NULL);
	ASSERT_EQ(TestParseLine("_INFO_,ctl_delimiter,|"), E_AWC_SUCCESS);
	ASSERT_EQ(TestParseLineNoAWC("_MOD_,GainControllerClass,2270400520,4"), E_AWC_ERROR);
	ASSERT_EQ(TestParseLine("_MOD_,GainControllerClass,2270400520,4"), E_AWC_SUCCESS);
	ASSERT_EQ(TestParseLine("_MOD_,SampleModuleOut,2270400521,4"), E_AWC_ERROR);

	ASSERT_EQ(TestParseLine("_CTL_,SampleModuleOut|event_counter,2270408712,1,0,0,0,65000,0,uint"), E_AWC_ERROR);
	ASSERT_EQ(TestParseLine("_CTL_,GainControllerClass|EnumControl,2270400520,1,3,0,0,2,0,enum"), E_AWC_ERROR);
	ASSERT_EQ(TestParseLineNoAWC("_CTL_,GainControllerClass|EnumControl,2270400520,1,3,0,0,2,0,enum"), E_AWC_ERROR);
	ASSERT_EQ(TestParseLineNoAWC("_INFO_,nr_ctl,2"), E_AWC_ERROR);
	ASSERT_EQ(TestParseLine("_INFO_,nr_ctl,2"), E_AWC_SUCCESS);
	ASSERT_EQ(TestParseLine("_CTL_,GainControllerClass|EnumControl,2270400520,1,3,0,0,2,0,enum"), E_AWC_SUCCESS);
	ASSERT_EQ(TestParseLineNoAWC("_CTL_,GainControllerClass|left_channel,2270400520,1,0,0,-50,5,0,int"), E_AWC_ERROR);
	ASSERT_EQ(TestParseLine("_CTL_,GainControllerClass|left_channel,2270400520,1,2,0,0,1,0,invalidtype"), E_AWC_CTL_TYPE_ERROR);
	ASSERT_EQ(TestParseLine("_CTL_,Events_Subsystem/GPDSP1_CPUOverflow.pAWE,123023380,0,0,0,0,0,0,['AWEInstance*', 'AWEInstance *']"), E_AWC_CTL_TYPE_ERROR);
	ASSERT_EQ(TestParseLine("_CTL_,Events_Subsystem/GPDSP1_CPUOverflow.userHandle,123023381,0,0,0,0,0,0,['void *', 'void*']"), E_AWC_CTL_TYPE_ERROR);
	ASSERT_EQ(TestParseLine("_CTL_,QXDM_LPI_QUA_SINK.deferredBuffer,122933274,0,0,0,0,0,0,int *"), E_AWC_CTL_TYPE_ERROR);

	ASSERT_EQ(TestParseLine("_CTL_,GainControllerClassleft_channel,2270400520,1,2,0,0,1,0,int"), E_AWC_ERROR);
	ASSERT_EQ(TestParseLine("_CTL_,GainControllerClass|left_channel,2270400520,1,0,0,-50,5,0,int"), E_AWC_SUCCESS);
	ASSERT_EQ(TestParseLine("_CTL_,GainControllerClass|right_channel,2270400520,1,0,0,-50,5,0,int"), E_AWC_ERROR);
	ASSERT_TRUE(awc->controls != NULL);
	// Note: next line should result in a failure
	ASSERT_EQ(TestParseLine("_INFO_,nr_awb,1"), E_AWC_SUCCESS);
	ASSERT_EQ(TestParseLine("_AWB_,1,Test"), E_AWC_ERROR);
	ASSERT_EQ(TestParseLine("_AWB_,"), E_AWC_ERROR);
	ASSERT_EQ(TestParseLineNoAWC("_AWB_,0,Main,Main.awb"), E_AWC_ERROR);
	ASSERT_EQ(TestParseLine("_AWB_,0,Main,Main.awb"), E_AWC_SUCCESS);
	ASSERT_EQ(TestParseLine("_AWB_,11,Test,Test.awb"), E_AWC_ERROR);

	ASSERT_EQ(TestParseLineNoAWC("_ENUM_,GainControllerClass|EnumControl,3,low,med,high"), E_AWC_ERROR);
	ASSERT_EQ(TestParseLine("_ENUM_,GainControllerClass|EnumControl,"), E_AWC_ERROR);
	ASSERT_EQ(TestParseLine("_ENUM_,GainControllerClass|Enum,3,low"), E_AWC_ERROR);
	ASSERT_EQ(TestParseLine("_ENUM_,GainControllerClass|EnumControl,3,low,med,high"), E_AWC_SUCCESS);

	ASSERT_EQ(TestParseLine("_EVT_,23424,evt1"), E_AWC_ERROR); // nr_evts not set	
	ASSERT_EQ(TestParseLine("_INFO_,nr_evts,1"), E_AWC_SUCCESS);
	ASSERT_EQ(TestParseLine("_EVT_,23424,evt1"), E_AWC_ERROR); // No module found
	// DO not add any more tests below this.
}

/**
```yaml
- id: utest~AWEMGR.AWC.LineEndings~1
  covers: dsn~AWEMGR.AWC.CheckConsistency~1
  description: |
    Verifies the parser works with different line trailings.
```
*/
TEST_F(AWCTestsWithoutFilesFixture, LineEndings)
{
	ASSERT_EQ(TestParseLine("_INFO_,version,0.0.1\t\r\n\f                             "), E_AWC_SUCCESS);
	ASSERT_STREQ(awc->info.version, "0.0.1");

	ASSERT_EQ(TestParseLine("_INFO_,nr_ctl,10\t\t\t\t\n   \n"), E_AWC_SUCCESS);
	ASSERT_TRUE(awc->controls != NULL);
	ASSERT_EQ(awc->info.controlcount, 10);
}

/**
```yaml
- id: utest~AWEMGR.AWC.InValidLines~1
  covers: dsn~AWEMGR.AWC.CheckConsistency~1
  description: |
    Checks the parser with incorrect usage.
```
*/
TEST_F(AWCTestsWithoutFilesFixture, InValidLines)
{
	ASSERT_EQ(parseLine(NULL, awc), E_AWC_ERROR);
	ASSERT_EQ(parseLine(NULL, NULL), E_AWC_ERROR);
	ASSERT_EQ(TestParseLineNoAWC("_INFO_,nr_awb,10"), E_AWC_ERROR);
	ASSERT_EQ(TestParseLine("_INFO_, nr_awb,10"), E_AWC_ERROR); // Space after "_INFO_,"
	ASSERT_EQ(TestParseLine("_INFO_,nr_awb;10"), E_AWC_ERROR);	// ";" instead of ","
	ASSERT_EQ(TestParseLine("_INFO_,nr_awb,10s"), E_AWC_ERROR); // alphabet appended to a number "10s"
}

/**
```yaml
- id: utest~AWEMGR.AWC.NewSchemaUpgrade~1
  covers: dsn~AWEMGR.AWC.Compatibility~1
  description: |
    Loads an AWC with a new schema information. This contains
    information about classId and objectId of a module.
```
*/
TEST_F(AWCTestSchema1Fixture, TestInfo)
{
	ASSERT_TRUE(awc != NULL);

	auto info = awc_get_info(awc);
	ASSERT_EQ(info->schema, 1);
	ASSERT_EQ(info->designcount, 3);
	ASSERT_EQ(info->modulecount, 3);
	ASSERT_EQ(info->controlcount, 8);

	auto module = awc_get_module(awc, "GainControllerClass");
	ASSERT_TRUE(module != NULL);
	ASSERT_EQ(module->classid, 2270400520);
	ASSERT_EQ(module->objectid, 1234);
	ASSERT_EQ(module->size, 4);

	module = awc_get_module(awc, "SampleModuleOut");
	ASSERT_TRUE(module != NULL);
	ASSERT_EQ(module->classid, 2270408712);
	ASSERT_EQ(module->objectid, 1235);
	ASSERT_EQ(module->size, 3);

	module = awc_get_module(awc, "VectorModule");
	ASSERT_TRUE(module != NULL);
	ASSERT_EQ(module->classid, 2270408713);
	ASSERT_EQ(module->objectid, 1236);
	ASSERT_EQ(module->size, 1);
}

/**
```yaml
- id: utest~AWEMGR.AWC.TestEvents~1
  covers: dsn~AWEMGR.AWC.Events~1
  description: |
    Ensures an events are correctly parsed
```
*/
TEST_F(AWCTestEventsFixture, TestEvents)
{
	auto info = awc_get_info(awc);
	ASSERT_EQ(info->eventcount, 2);
	ASSERT_EQ(awc_event_count(awc), 2);

	auto pEvent1 = awc_get_event(awc, "Event1");
	ASSERT_TRUE(pEvent1 != NULL);
	ASSERT_EQ(pEvent1->mod->objectid, 30003);

	auto event1 = awc_get_event_by_index(awc, 0);
	ASSERT_TRUE(event1 != NULL);
	ASSERT_STREQ(event1->mod->name, "Event1");

	auto event2 = awc_get_event_by_index(awc, 1);
	ASSERT_TRUE(event2 != NULL);
	ASSERT_STREQ(event2->mod->name, "Event2");

	ASSERT_TRUE(awc_get_event_by_index(awc, -1) == NULL);
	ASSERT_TRUE(awc_get_event_by_index(awc, 2) == NULL);
	ASSERT_TRUE(awc_get_event_by_index(NULL, 0) == NULL);

	auto pEvent2 = awc_get_event(awc, "Event2");
	ASSERT_TRUE(pEvent2 != NULL);
	ASSERT_EQ(pEvent2->mod->objectid, 31111);

	auto eventCb = [](const awc_event_t *event, void *usr_data_p)
	{
		eventCbCount++;
		ASSERT_TRUE(event != NULL);
		ASSERT_TRUE(usr_data_p != NULL);
		int y = *(int *)usr_data_p;
		ASSERT_EQ(y, 1234);
	};
	int x = 1234;
	awc_foreach_event(awc, eventCb, &x);
	ASSERT_EQ(eventCbCount, 2);
}

/**
```yaml
- id: utest~AWEMGR.AWC.UserDataIndexAccess~1
  covers: dsn~AWEMGR.AWC.UserData~1
  description: |
    Ensures the user data can be accessed by index
```
*/
TEST_F(AWCTestUserdataFixture, UserDataIndexAccess)
{
	ASSERT_EQ(awc_get_control_userdata_count(awc, "VectorModule|VectorControl"), 1);
	auto pData = awc_get_control_userdata_by_index(awc, "VectorModule|VectorControl", 0);
	ASSERT_TRUE(pData != NULL);
	ASSERT_STREQ(pData->key, "key1");
	ASSERT_EQ(pData->type, AWC_USRDATA_STR);
	ASSERT_STREQ(pData->value.str, "testdata");

	ASSERT_EQ(awc_get_control_userdata_count(awc, "SampleModuleOut|rms_left"), 1);
	pData = awc_get_control_userdata_by_index(awc, "SampleModuleOut|rms_left", 0);
	ASSERT_TRUE(pData != NULL);
	ASSERT_STREQ(pData->key, "key2");
	ASSERT_EQ(pData->type, AWC_USRDATA_INT);
	ASSERT_EQ(pData->value.i32, 34324);

	ASSERT_EQ(awc_get_control_userdata_count(awc, "GainControllerClass|left_channel"), 1);
	pData = awc_get_control_userdata_by_index(awc, "GainControllerClass|left_channel", 0);
	ASSERT_TRUE(pData != NULL);
	ASSERT_STREQ(pData->key, "alias");
	ASSERT_EQ(pData->type, AWC_USRDATA_STR);
	ASSERT_STREQ(pData->value.str, "leftGain");

	ASSERT_EQ(awc_get_module_userdata_count(awc, "GainControllerClass"), 1);
	pData = awc_get_module_userdata_by_index(awc, "GainControllerClass", 0);
	ASSERT_TRUE(pData != NULL);
	ASSERT_STREQ(pData->key, "vm_mask");
	ASSERT_EQ(pData->type, AWC_USRDATA_UINT);
	ASSERT_EQ(pData->value.u32, 44332432);

	// Error cases
	ASSERT_EQ(awc_get_control_userdata_count(awc, "GainControllerClass|right_channel"), 0);
	ASSERT_EQ(awc_get_control_userdata_count(NULL, "GainControllerClass|right_channel"), -1);
	ASSERT_EQ(awc_get_control_userdata_count(NULL, NULL), -1);
	pData = awc_get_control_userdata_by_index(NULL, "VectorModule|VectorControl", 0);
	ASSERT_TRUE(pData == NULL);
	pData = awc_get_control_userdata_by_index(NULL, NULL, 0);
	ASSERT_TRUE(pData == NULL);
	pData = awc_get_control_userdata_by_index(NULL, NULL, 34);
	ASSERT_TRUE(pData == NULL);
	pData = awc_get_control_userdata_by_index(awc, "VectorModule|VectorControl", 34);
	ASSERT_TRUE(pData == NULL);

	ASSERT_EQ(awc_get_module_userdata_count(awc, "VectorModule"), 0);
}

/**
```yaml
- id: utest~AWEMGR.AWC.UserDataNamedAccess~1
  covers: dsn~AWEMGR.AWC.UserData~1
  description: |
    Ensures the user data can be accessed by key
```
*/
TEST_F(AWCTestUserdataFixture, UserDataNamedAccess)
{
	auto pData = awc_get_control_userdata_by_key(awc, "VectorModule|VectorControl", "key1");
	ASSERT_TRUE(pData != NULL);
	ASSERT_STREQ(pData->key, "key1");
	ASSERT_EQ(pData->type, AWC_USRDATA_STR);
	ASSERT_STREQ(pData->value.str, "testdata");

	pData = awc_get_control_userdata_by_key(awc, "SampleModuleOut|rms_left", "key2");
	ASSERT_TRUE(pData != NULL);
	ASSERT_STREQ(pData->key, "key2");
	ASSERT_EQ(pData->type, AWC_USRDATA_INT);
	ASSERT_EQ(pData->value.i32, 34324);

	pData = awc_get_control_userdata_by_key(awc, "GainControllerClass|left_channel", "alias");
	ASSERT_TRUE(pData != NULL);
	ASSERT_STREQ(pData->key, "alias");
	ASSERT_EQ(pData->type, AWC_USRDATA_STR);
	ASSERT_STREQ(pData->value.str, "leftGain");

	pData = awc_get_module_userdata_by_key(awc, "GainControllerClass", "vm_mask");
	ASSERT_TRUE(pData != NULL);
	ASSERT_STREQ(pData->key, "vm_mask");
	ASSERT_EQ(pData->type, AWC_USRDATA_UINT);
	ASSERT_EQ(pData->value.u32, 44332432);

	// Error cases
	pData = awc_get_control_userdata_by_key(NULL, NULL, NULL);
	ASSERT_TRUE(pData == NULL);
	pData = awc_get_control_userdata_by_key(awc, NULL, NULL);
	ASSERT_TRUE(pData == NULL);
	pData = awc_get_control_userdata_by_key(awc, "SampleModuleOut|rms_left", NULL);
	ASSERT_TRUE(pData == NULL);
	pData = awc_get_module_userdata_by_key(NULL, NULL, NULL);
	ASSERT_TRUE(pData == NULL);
	pData = awc_get_module_userdata_by_key(awc, NULL, "key1");
	ASSERT_TRUE(pData == NULL);
	pData = awc_get_module_userdata_by_key(NULL, NULL, "key1");
	ASSERT_TRUE(pData == NULL);
	pData = awc_get_module_userdata_by_key(awc, "VectorModule|VectorControl", "invalidkey");
	ASSERT_TRUE(pData == NULL);
}

/**
```yaml
- id: utest~AWEMGR.AWC.ControlNameAndAlias~1
  covers:
    - dsn~AWEMGR.AWC.NameToAweData~1
    - dsn~AWEMGR.AWC.AliasToAweData~1
  description: |
    Ensures that an AWC Control can be searched based on the its name and alias
```
*/
TEST_F(AWCTestAlias, ControlAccessByNameAndAlias)
{
	ASSERT_TRUE(awc != NULL);
	awc_ctl_t *aliasCtl, *nameCtl;
	aliasCtl = awc_get_control_from_awc(awc, "LeftVolume");
	ASSERT_TRUE(aliasCtl != NULL);

	nameCtl = awc_get_control_from_awc(awc, "GainControllerClass|left_channel");
	ASSERT_TRUE(nameCtl != NULL);
	ASSERT_EQ(aliasCtl, nameCtl);

	aliasCtl = awc_get_control_from_awc(awc, "RightVolume");
	ASSERT_TRUE(aliasCtl != NULL);

	nameCtl = awc_get_control_from_awc(awc, "GainControllerClass|right_channel");
	ASSERT_TRUE(nameCtl != NULL);
	ASSERT_EQ(aliasCtl, nameCtl);

	aliasCtl = awc_get_control_from_awc(awc, "MuteAll");
	ASSERT_TRUE(aliasCtl != NULL);

	nameCtl = awc_get_control_from_awc(awc, "GainControllerClass|mute_all");
	ASSERT_TRUE(nameCtl != NULL);
	ASSERT_EQ(aliasCtl, nameCtl);

}

/**
```yaml
- id: utest~AWEMGR.AWC.ModuleNameAndAlias~1
  covers:
    - dsn~AWEMGR.AWC.NameToAweData~1
    - dsn~AWEMGR.AWC.AliasToAweData~1
  description: |
    Ensures that an AWC Control can be searched based on the its name and alias
```
*/
TEST_F(AWCTestAlias, ModuleAccessByNameAndAlias)
{
	ASSERT_TRUE(awc != NULL);
	awc_module_t *aliasMdl, *nameMdl;
	aliasMdl = awc_get_module(awc, "Volume");
	ASSERT_TRUE(aliasMdl != NULL);

	nameMdl = awc_get_module(awc, "GainControllerClass");
	ASSERT_TRUE(nameMdl != NULL);
	ASSERT_EQ(aliasMdl, nameMdl);
}

/**
```yaml
- id: utest~AWEMGR.AWC.DesignCountError~1
  covers: dsn~AWEMGR.AWC.CheckConsistency~1
  description: |
    Checks that the awc_init fails when the nr_awb is not equal to design count(_AWB_ Enteries)
```
*/
TEST_F(AWCTestFixture, DesignCountError)
{
	awc = loadAWC(AWC_DATA_DIR "/awc_designCount_error.txt");
	ASSERT_TRUE(awc == NULL);
}

/**
```yaml
- id: utest~AWEMGR.AWC.ModuleCountError~1
  covers: dsn~AWEMGR.AWC.CheckConsistency~1
  description: |
    Checks that the awc_init fails when the nr_mod is not equal to Module count(_MOD_ Enteries)
```
*/
TEST_F(AWCTestFixture, ModuleCountError)
{
	awc = loadAWC(AWC_DATA_DIR "/awc_moduleCount_error.txt");
	ASSERT_TRUE(awc == NULL);
}

/**
```yaml
- id: utest~AWEMGR.AWC.ControlCountError~2
  covers: dsn~AWEMGR.AWC.CheckConsistency~1
  description: |
    Checks that the awc_init succeeds when the nr_ctl is not equal to Control count(_CTL_ Enteries)
```
*/
TEST_F(AWCTestFixture, ControlCountError)
{
	awc = loadAWC(AWC_DATA_DIR "/awc_controlCount_error.txt");
	// load awc should still succeed. 
	ASSERT_TRUE(awc != NULL);
}

/**
```yaml
- id: utest~AWEMGR.AWC.TopDataAccess~1
  covers: dsn~AWEMGR.AWC.TopData~1
  description: |
    Ensures the Top data can be accessed by key
```
*/
TEST_F(AWCTestFixture, TopDataAccess)
{
	awc = loadAWC(AWC_DATA_DIR "/awc_file_schema3.txt");
	ASSERT_TRUE(awc != NULL);

	auto pData = awc_get_top_userdata_by_key(NULL, NULL);
	ASSERT_TRUE(pData == NULL);

	pData = awc_get_top_userdata_by_key(NULL, "name");
	ASSERT_TRUE(pData == NULL);

	pData = awc_get_top_userdata_by_key(awc, NULL);
	ASSERT_TRUE(pData == NULL);

	pData = awc_get_top_userdata_by_key(awc, "unknown");
	ASSERT_TRUE(pData == NULL);

	pData = awc_get_top_userdata_by_key(awc, "name");
	ASSERT_TRUE(pData != NULL);
	ASSERT_EQ(pData->type, AWC_USRDATA_STR);
	ASSERT_STREQ(pData->value.str, "schema");

	pData = awc_get_top_userdata_by_key(awc, "age");
	ASSERT_TRUE(pData != NULL);
	ASSERT_EQ(pData->type, AWC_USRDATA_UINT);
	ASSERT_EQ(pData->value.u32, 3);
}