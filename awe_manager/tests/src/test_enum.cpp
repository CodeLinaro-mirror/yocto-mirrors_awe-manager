#include "test_fixtures.h"


/* ****************************************************************************
 * TEST CASES
 * ***************************************************************************/


/**
```yaml
- id: itest~AWEMGR.Enum.Fail~1
  covers: req~AWEMGR.ControlEnumeration~1
  description: Checks that incorrect control enumerate API calls return error.
```
*/
TEST_F(AweMgrTestFixture, ControlEnumFail) {
	ASSERT_EQ(awemgr_get_controls_count(m_ctx), awemgr_RC_ERR);

	struct awemgr_ctl_elem_info info;
	EXPECT_EQ(awemgr_get_control_info(m_ctx, 0, &info), awemgr_RC_ERR);

	EXPECT_EQ(awemgr_get_control_info_by_name(m_ctx, "name", &info), awemgr_RC_ERR);

	load_awc(TEST_DATA_DIR "/designs/minimal/target_files/awc_index.txt");
	load_main_design();
	EXPECT_EQ(awemgr_get_control_info(m_ctx, 0, NULL), awemgr_RC_ERR);
	EXPECT_EQ(awemgr_get_control_info_by_name(m_ctx, "name", NULL), awemgr_RC_ERR);
	EXPECT_EQ(awemgr_get_control_info(m_ctx, 32423, &info), awemgr_RC_ERR);
	EXPECT_EQ(awemgr_get_control_info_by_name(m_ctx, "name", &info), awemgr_RC_ERR);
}

/**
```yaml
- id: itest~AWEMGR.Enum.DesignEnumFail~1
  covers: req~AWEMGR.DesignEnumeration~1
  description: Checks that incorrect design enumerate API calls return error.
```
*/
TEST_F(AweMgrTestFixture, DesignEnumFail) {
	ASSERT_EQ(awemgr_get_design_count(m_ctx), awemgr_RC_ERR);

	struct awemgr_design_info info;
	EXPECT_EQ(awemgr_get_design_info(m_ctx, 0, &info), awemgr_RC_ERR);

	load_awc(TEST_DATA_DIR "/designs/minimal/target_files/awc_index.txt");
	load_main_design();
	EXPECT_EQ(awemgr_get_design_info(m_ctx, 0, NULL), awemgr_RC_ERR);
	EXPECT_EQ(awemgr_get_design_info(m_ctx, 32432, &info), awemgr_RC_ERR);
}

static struct awemgr_ctl_elem_info expected_infos_for_enum[] = {
	{0, (char*)"SourceFloat_10.profileTime", AWEMGR_VARTYPE_INTEGER, 1,    0, 0, 0, 0},
	{0, (char*)"SourceFloat_10.value", AWEMGR_VARTYPE_FLOAT, 10,    0, 0, 0, 0},
	{0, (char*)"SinkFloat_10.profileTime", AWEMGR_VARTYPE_INTEGER, 1,    0, 0, 0, 0},
	{0, (char*)"SinkFloat_10.enable", AWEMGR_VARTYPE_INTEGER, 1,    0, 0, 0, 0},
	{0, (char*)"SinkFloat_10.value", AWEMGR_VARTYPE_FLOAT, 10,    0, 0, 0, 0},
	{0, (char*)"SinkFloat_10.yRange", AWEMGR_VARTYPE_FLOAT, 2,    0, 0, 0, 0},
	{0, (char*)"SourceFloat_1.profileTime", AWEMGR_VARTYPE_INTEGER, 1,    0, 0, 0, 0},
	{0, (char*)"SourceFloat_1.value", AWEMGR_VARTYPE_FLOAT, 1,    0, 0, 0, 0},
	{0, (char*)"SinkFloat_1.profileTime", AWEMGR_VARTYPE_INTEGER, 1,    0, 0, 0, 0},
	{0, (char*)"SinkFloat_1.enable", AWEMGR_VARTYPE_INTEGER, 1,    0, 0, 0, 0},
	{0, (char*)"SinkFloat_1.value", AWEMGR_VARTYPE_FLOAT, 1,    0, 0, 0, 0},
	{0, (char*)"SinkFloat_1.yRange", AWEMGR_VARTYPE_FLOAT, 2,    0, 0, 0, 0},
	{0, (char*)"SourceInt_1.profileTime", AWEMGR_VARTYPE_INTEGER, 1,    0, 0, 0, 0},
	{0, (char*)"SourceInt_1.value", AWEMGR_VARTYPE_INTEGER, 1,    0, 0, 0, 0},
	{0, (char*)"SinkInt_1.profileTime", AWEMGR_VARTYPE_INTEGER, 1,    0, 0, 0, 0},
	{0, (char*)"SinkInt_1.value", AWEMGR_VARTYPE_INTEGER, 1,    0, 0, 0, 0},
	{0, (char*)"SourceInt_10.profileTime", AWEMGR_VARTYPE_INTEGER, 1,    0, 0, 0, 0},
	{0, (char*)"SourceInt_10.value", AWEMGR_VARTYPE_INTEGER, 10,    0, 0, 0, 0},
	{0, (char*)"SinkInt_10.profileTime", AWEMGR_VARTYPE_INTEGER, 1,    0, 0, 0, 0},
	{0, (char*)"SinkInt_10.value", AWEMGR_VARTYPE_INTEGER, 10,    0, 0, 0, 0},
	{0, (char*)"SourceInt_2000.profileTime", AWEMGR_VARTYPE_INTEGER, 1,    0, 0, 0, 0},
	{0, (char*)"SourceInt_2000.value", AWEMGR_VARTYPE_INTEGER, 2000,    0, 0, 0, 0},
	{0, (char*)"SinkInt_2000.profileTime", AWEMGR_VARTYPE_INTEGER, 1,    0, 0, 0, 0},
	{0, (char*)"SinkInt_2000.value", AWEMGR_VARTYPE_INTEGER, 2000,    0, 0, 0, 0},
	{0, (char*)"MixerFract.profileTime", AWEMGR_VARTYPE_INTEGER, 1,    0, 0, 0, 0},
	{0, (char*)"MixerFract.maxNonZero", AWEMGR_VARTYPE_INTEGER, 1,    0, 0, 0, 0},
	{0, (char*)"MixerFract.postShift", AWEMGR_VARTYPE_INTEGER, 1,    0, 0, 0, 0},
	{0, (char*)"MixerFract.gainScale", AWEMGR_VARTYPE_FLOAT, 1,    0, 0, 0, 0},
	{0, (char*)"MixerFract.numIn", AWEMGR_VARTYPE_INTEGER, 1,    0, 0, 0, 0},
	{0, (char*)"MixerFract.gain", AWEMGR_VARTYPE_FLOAT, 512,    0, 0, 0, 0},
	{0, (char*)"MixerFract.nonZeroGainFract32", AWEMGR_VARTYPE_FRACT32, 512,    0, 0, 0, 0},
	{0, (char*)"SinkFract_MixOut.profileTime", AWEMGR_VARTYPE_INTEGER, 1,    0, 0, 0, 0},
	{0, (char*)"SinkFract_MixOut.enable", AWEMGR_VARTYPE_INTEGER, 1,    0, 0, 0, 0},
	{0, (char*)"SinkFract_MixOut.value", AWEMGR_VARTYPE_FRACT32, 32,    0, 0, 0, 0},
	{0, (char*)"SinkFract_MixOut.yRange", AWEMGR_VARTYPE_FLOAT, 2,    0, 0, 0, 0},
	{0, NULL, AWEMGR_VARTYPE_UNDEF, 0, 0, 0, 0, 0},
};

/**
```yaml
- id: itest~AWEMGR.EnumControls~1
  covers: req~AWEMGR.ControlEnumeration~1
  description: |
    Checks that information on all control items are provided and in line with the
    information in the AWC file.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWC, EnumerateControls) {

	struct awemgr_ctx *awectx_p = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(awectx_p != NULL);

	int nr_ctls = awemgr_get_controls_count(awectx_p);
	ASSERT_EQ(nr_ctls, 35);

	for (int x=0; x<nr_ctls; x++)
	{
		ASSERT_TRUE(expected_infos_for_enum[x].id.name != 0) << "Max number of checks reached. Adapt test case expectations" << x;

		struct awemgr_ctl_elem_info info;
		EXPECT_EQ(awemgr_get_control_info(awectx_p, x, &info), awemgr_RC_OK);

		EXPECT_STREQ(info.id.name, expected_infos_for_enum[x].id.name) << "name differs at pos " << x;
		EXPECT_EQ(info.id.instanceId, expected_infos_for_enum[x].id.instanceId) << "instanceId differ at pos " << x << "; name: " << info.id.name;
		EXPECT_EQ(info.id.nr_items, expected_infos_for_enum[x].id.nr_items) << "nr_items differ at pos " << x << "; name: " << info.id.name;
		EXPECT_EQ(info.id.type, expected_infos_for_enum[x].id.type) << "type differs at pos " << x << "; name: " << info.id.name;
	}

}


/**
```yaml
- id: itest~AWEMGR.ControlInfoByName~1
  covers: req~AWEMGR.ControlEnumeration~1
  description: |
    Checks if control info can also be retrieved by name.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWC, ControlInfoByName) {

	struct awemgr_ctx *awectx_p = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(awectx_p != NULL);

	for (int x=0; x<24; x++)
	{
		struct awemgr_ctl_elem_info info;
		EXPECT_EQ(awemgr_get_control_info_by_name(awectx_p, expected_infos_for_enum[x].id.name, &info), awemgr_RC_OK);

		EXPECT_EQ(info.id.instanceId, expected_infos_for_enum[x].id.instanceId) << "instanceId differ at pos " << x;
		EXPECT_EQ(info.id.nr_items, expected_infos_for_enum[x].id.nr_items) << "nr_items differ at pos " << x;
		EXPECT_EQ(info.id.type, expected_infos_for_enum[x].id.type) << "type differs at pos " << x;
		EXPECT_STREQ(info.id.name, expected_infos_for_enum[x].id.name) << "name differs at pos " << x;
	}
}


/**
```yaml
- id: itest~AWEMGR.EnumDesigns~2
  covers: req~AWEMGR.DesignEnumeration~1
  description: |
    Checks if all information about designs inside an AWC can be retireved.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWC, EnumerateDesigns) {

	struct awemgr_ctx *awectx_p = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(awectx_p != NULL);

	ASSERT_EQ(awemgr_get_design_count(NULL), -1); //The API must return -1 if the awemgr_ctx is NULL

	int nr_designs = awemgr_get_design_count(awectx_p);
	ASSERT_EQ(nr_designs, 4);

	struct awemgr_design_info expected_infos[] = {
		{(char*)"Main"},
		{(char*)"Original"},
		{(char*)"Modified"},
		// note: setget-AWC contains one invalid AWB entry! but this does not matter here
		{(char*)"PresetWithAProblem"},
	};
	for (int x=0; x<nr_designs; x++)
	{
		struct awemgr_design_info info;
		EXPECT_EQ(awemgr_get_design_info(awectx_p, x, &info), awemgr_RC_OK);

		EXPECT_STREQ(info.name, expected_infos[x].name) << "design name is not equal at position " << x;
		EXPECT_EQ(info.coreid_objectid, 0) << "design coreid_objectid is not " << 0;
	}
}

/**
```yaml
- id: itest~AWEMGR.GetDesignByName~1
  covers: req~AWEMGR.DesignLookupByName~1
  description: |
    Checks that a design can be looked up by its name and that the information
    returned is the same as the one returned for the design's index. An unknown
    name is reported with awemgr_RC_ERR_DESIGN_NOTFOUND, so the call can be used
    to check whether a design exists.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWC, GetDesignInfoByName) {

	struct awemgr_ctx *awectx_p = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(awectx_p != NULL);

	int nr_designs = awemgr_get_design_count(awectx_p);
	ASSERT_GT(nr_designs, 0);

	// every design found by index must be found by its name as well, with equal content
	for (int x=0; x<nr_designs; x++)
	{
		// fill both structures with a non-zero pattern first, so that a field left
		// untouched by the API shows up as a difference instead of accidentally matching
		struct awemgr_design_info by_index;
		memset(&by_index, 0xA5, sizeof(by_index));
		ASSERT_EQ(awemgr_get_design_info(awectx_p, x, &by_index), awemgr_RC_OK);

		struct awemgr_design_info by_name;
		memset(&by_name, 0x5A, sizeof(by_name));
		EXPECT_EQ(awemgr_get_design_info_by_name(awectx_p, by_index.name, &by_name), awemgr_RC_OK)
			<< "design not found by name: " << by_index.name;

		EXPECT_STREQ(by_name.name, by_index.name);
		EXPECT_EQ(by_name.coreid_objectid, by_index.coreid_objectid);
		EXPECT_EQ(by_name.size, by_index.size);
		EXPECT_EQ(by_name.plugin_count, by_index.plugin_count);

		// the whole plugin array must be defined, also for a design without plugins,
		// and must be identical no matter which of the two getters filled it.
		// note: compared over the full array length, not with STREQ, because the
		// poison pattern above leaves no string termination if the API does not fill it
		for (unsigned int p = 0; p < AWEMGR_MAX_PLUGINS_PER_DESIGN; p++)
		{
			EXPECT_EQ(memcmp(by_name.plugins[p].name, by_index.plugins[p].name, AWEMGR_MAX_PLUGIN_NAME_LEN), 0)
				<< "plugin name differs at index " << p << " for design " << by_index.name;
			EXPECT_EQ(by_name.plugins[p].core, by_index.plugins[p].core)
				<< "plugin core differs at index " << p << " for design " << by_index.name;
		}
	}

	// an unknown name is reported as such, not as a generic error
	struct awemgr_design_info info;
	EXPECT_EQ(awemgr_get_design_info_by_name(awectx_p, "NoSuchDesign", &info), awemgr_RC_ERR_DESIGN_NOTFOUND);

	// parameter checks, consistent with the index based getter
	EXPECT_EQ(awemgr_get_design_info_by_name(NULL, "Main", &info), awemgr_RC_ERR);
	EXPECT_EQ(awemgr_get_design_info_by_name(awectx_p, NULL, &info), awemgr_RC_ERR);
	EXPECT_EQ(awemgr_get_design_info_by_name(awectx_p, "Main", NULL), awemgr_RC_ERR);
}

/**
```yaml
- id: itest~AWEMGR.PluginInfo~1
  covers: req~AWEMGR.DesignEnumeration~1
  description: |
    Checks if plugin information about designs inside an AWC can be retireved.
```
*/
TEST_F(AweMgrTestFixture, EnumerateDesigns) {
    load_awc(TEST_DATA_DIR "/designs/subcanvas/target_files/awc_index.txt");
    m_ctx = awemgr_get_awc_context(m_mgr_p, 0);
    ASSERT_TRUE(m_ctx != NULL);

    auto count = awemgr_get_design_count(m_ctx);

    bool testMain = false, testM0 = false, testM1 = false, testM2 = false, testM3 = false;

    for (int x = 0; x < count; x++) {
        struct awemgr_design_info info;
        EXPECT_EQ(awemgr_get_design_info(m_ctx, x, &info), awemgr_RC_OK);

        std::string designName = info.name;

        if (designName == "Main") {
            ASSERT_EQ(info.plugin_count, 5);
            EXPECT_STREQ(info.plugins[0].name, "lib1.so"); EXPECT_EQ(info.plugins[0].core, 0);
            EXPECT_STREQ(info.plugins[1].name, "lib1.so"); EXPECT_EQ(info.plugins[1].core, 1);
            EXPECT_STREQ(info.plugins[2].name, "lib2.so"); EXPECT_EQ(info.plugins[2].core, 1);
            EXPECT_STREQ(info.plugins[3].name, "lib3.so"); EXPECT_EQ(info.plugins[3].core, 1);
            EXPECT_STREQ(info.plugins[4].name, "lib2.so"); EXPECT_EQ(info.plugins[4].core, 2);
	        testMain = true;
        }
        else if (designName == "Multiplier0") {
            ASSERT_EQ(info.plugin_count, 1);
            EXPECT_STREQ(info.plugins[0].name, "lib1.so"); EXPECT_EQ(info.plugins[0].core, 0);
			testM0 = true;
        }
        else if (designName == "Multiplier1") {
            ASSERT_EQ(info.plugin_count, 2);
            EXPECT_STREQ(info.plugins[0].name, "lib1.so"); EXPECT_EQ(info.plugins[0].core, 1);
            EXPECT_STREQ(info.plugins[1].name, "lib2.so"); EXPECT_EQ(info.plugins[1].core, 1);
            testM1 = true;
        }
        else if (designName == "Multiplier2") {
            ASSERT_EQ(info.plugin_count, 1);
            EXPECT_STREQ(info.plugins[0].name, "lib2.so"); EXPECT_EQ(info.plugins[0].core, 2);
            testM2 = true;
        }
        else if (designName == "Multiplier3") {
            EXPECT_EQ(info.plugin_count, 0);
		    testM3 = true;
        }
    }

    // Ensure all required designs were hit
    EXPECT_TRUE(testMain & testM0 & testM1 & testM2 & testM3);
}
