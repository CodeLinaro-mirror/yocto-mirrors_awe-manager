#include "test_fixtures.h"
#include "awemgr_logging.h"
#include "awe_awc.h"
#include "awe_comm.h"
#include "Errors.h"

static const char *awc_file = TEST_DATA_DIR "/designs/subcanvas/target_files/awc_index.txt";

/* ****************************************************************************
 * TEST CASES
 * ***************************************************************************/

/**
```yaml
- id: itest~AWEMGR.AweCoreInfo.Memory~3
  covers: req~AWEMGR.AWECoreInformationQuery~2
  description: Checks that memory info can be retrieved. Loads a Subcanvas design
    to ensure heap values are different on different cores.
```
*/
TEST_F(AweMgrTestFixture, AweCoreMemory) {

	load_awc(awc_file);
	m_ctx = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_EQ(awemgr_load_design(m_ctx, "Main"), awemgr_RC_OK);
	delay_ms(10);


	awemgr_heapinfo info_buffer;
	ASSERT_EQ(awemgr_get_heap_info(m_mgr_p, 0, 0, &info_buffer), awemgr_RC_OK);

	// note: checks here depend on LinuxApp version/variant used !!!
	ASSERT_EQ(info_buffer.nr_heaps, 4);
	ASSERT_EQ(info_buffer.fast_a.nr_free, 4996046);
	ASSERT_EQ(info_buffer.fast_b.nr_free, 4998994);
	ASSERT_EQ(info_buffer.slow.nr_free, 4998631);
	ASSERT_EQ(info_buffer.shared.nr_free, 19998849);

	// test the helper functions for coverage.
	ASSERT_EQ(awemgr_heapinfo_allocated(info_buffer.fast_a), 3954);
	ASSERT_GT(awemgr_heapinfo_allocated_percent(info_buffer.fast_a), 0.0f);

	ASSERT_EQ(awemgr_get_heap_info(m_mgr_p, 0, 1, &info_buffer), awemgr_RC_OK);
	ASSERT_EQ(info_buffer.nr_heaps, 4);
	ASSERT_EQ(info_buffer.fast_a.nr_free, 4997304);
	ASSERT_EQ(info_buffer.fast_b.nr_free, 4998994);
	ASSERT_EQ(info_buffer.slow.nr_free, 4998788);
	ASSERT_EQ(info_buffer.shared.nr_free, 19998849);

}


/**
```yaml
- id: itest~AWEMGR.AweCoreInfo.Cpu~1
  covers: req~AWEMGR.AWECoreInformationQuery~2
  description: Checks that CPU load can be retrieved.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWCLoaded, AweCoreCPU) {
	awemgr_cpuinfo info_buffer;

	// note: checks here depend on LinuxApp version/variant used !!!
	ASSERT_EQ(awemgr_get_cpu_info(m_mgr_p, 0, 0, &info_buffer), awemgr_RC_OK);
	ASSERT_NE(info_buffer.AverageCycles, 0);
	ASSERT_NE(info_buffer.TimePerProcess, 0);

	ASSERT_TRUE(awemgr_getCpuPercentage(&info_buffer) > 0.0);
}


/**
```yaml
- id: itest~AWEMGR.AweCoreInfo.Modules~1
  covers: req~AWEMGR.AWECoreInformationQuery~2
  description: Checks that a module list can be retrieved.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWCLoaded, AweCoreModuleList) {
	awemgr_modulelist_info info_buffer;

	// note: checks here depend on LinuxApp version/variant used !!!
	ASSERT_EQ(awemgr_get_modulelist_info(m_mgr_p, 0, 0, &info_buffer), awemgr_RC_OK);
	ASSERT_TRUE(info_buffer.nr_classes > 500 && info_buffer.nr_classes < 600);
}

/**
```yaml
- id: itest~AWEMGR.AweCoreInfo.LayoutProfiling~1
  covers: req~AWEMGR.AWECoreInformationQuery~2
  description: Checks that profiling info of layouts can be retrieved.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWCLoaded, AweCoreLayoutProfiling) {
	awemgr_layoutinfo info_buffer;

	ASSERT_EQ(awemgr_get_layout_info(m_mgr_p, 0, 0, &info_buffer), awemgr_RC_OK);
	ASSERT_EQ(info_buffer.nr_layouts, 1);
	ASSERT_EQ(info_buffer.profilingValuesPerLayout, 6);
}

/**
```yaml
- id: itest~AWEMGR.AweCoreInfo.AweCoreLayoutProfiling_Fails~1
  covers: req~AWEMGR.AWECoreInformationQuery~2
  description: Checks incorrect parameter handling of method.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWCLoaded, AweCoreLayoutProfiling_Fails) {
	awemgr_layoutinfo info_buffer;

	ASSERT_EQ(awemgr_get_layout_info(NULL, 0, 0, &info_buffer), awemgr_RC_ERR);
	ASSERT_EQ(awemgr_get_layout_info(m_mgr_p, 0, 0, NULL), awemgr_RC_ERR);
}


/**
```yaml
- id: itest~AWEMGR.AweCoreInfo.Fails~1
  covers: req~AWEMGR.AWECoreInformationQuery~2
  description: Checks capture of incorrect command usages.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWCLoaded, AweCoreInfoFails) {
	awemgr_heapinfo heaps;
	awemgr_modulelist_info modulelist;
	awemgr_cpuinfo cpuinfo;

	ASSERT_EQ(awemgr_get_heap_info(NULL, 0, 0, &heaps), awemgr_RC_ERR);
	ASSERT_EQ(awemgr_get_heap_info(m_mgr_p, 0, 0, NULL), awemgr_RC_ERR);

	ASSERT_EQ(awemgr_get_modulelist_info(NULL, 0, 0, &modulelist), awemgr_RC_ERR);
	ASSERT_EQ(awemgr_get_modulelist_info(m_mgr_p, 0, 0, NULL), awemgr_RC_ERR);

	ASSERT_EQ(awemgr_get_cpu_info(NULL, 0, 0, &cpuinfo), awemgr_RC_ERR);
	ASSERT_EQ(awemgr_get_cpu_info(m_mgr_p, 0, 0, NULL), awemgr_RC_ERR);
}

/**
```yaml
- id: itest~AWEMGR.AweCoreInfo.EndpointOutOfRange~1
  covers: req~AWEMGR.ApiArgumentValidation~1
  description: Checks that an endpoint index outside the supported range is
    rejected by the info queries. An index of -1 - as seen when no AWC is
    loaded - previously corrupted the message header and crashed.
```
*/
TEST_F(AweMgrTestFixture, AweCoreInfoEndpointOutOfRange) {
	awemgr_heapinfo heaps;
	awemgr_modulelist_info modulelist;
	awemgr_cpuinfo cpuinfo;
	awemgr_layoutinfo layoutinfo;

	const int bad_endpoints[] = { -1, MAX_AWE_ENDPOINTS, MAX_AWE_ENDPOINTS + 1, 9999 };

	for (int endpoint : bad_endpoints)
	{
		EXPECT_EQ(awemgr_get_cpu_info(m_mgr_p, endpoint, 0, &cpuinfo), awemgr_RC_ERR) << "endpoint " << endpoint;
		EXPECT_EQ(awemgr_get_layout_info(m_mgr_p, endpoint, 0, &layoutinfo), awemgr_RC_ERR) << "endpoint " << endpoint;
		EXPECT_EQ(awemgr_get_heap_info(m_mgr_p, endpoint, 0, &heaps), awemgr_RC_ERR) << "endpoint " << endpoint;
		EXPECT_EQ(awemgr_get_modulelist_info(m_mgr_p, endpoint, 0, &modulelist), awemgr_RC_ERR) << "endpoint " << endpoint;
	}
}

/**
```yaml
- id: itest~AWEMGR.AweCoreInfo.CoreOutOfRange~1
  covers: req~AWEMGR.ApiArgumentValidation~1
  description: Checks that a core index outside the supported range is rejected
    by the info queries; the core index shares the message header byte with the
    endpoint index and corrupts it the same way.
```
*/
TEST_F(AweMgrTestFixture, AweCoreInfoCoreOutOfRange) {
	awemgr_heapinfo heaps;
	awemgr_modulelist_info modulelist;
	awemgr_cpuinfo cpuinfo;
	awemgr_layoutinfo layoutinfo;

	const int bad_cores[] = { -1, MAX_AWE_CORES, MAX_AWE_CORES + 1, 9999 };

	for (int core : bad_cores)
	{
		EXPECT_EQ(awemgr_get_cpu_info(m_mgr_p, 0, core, &cpuinfo), awemgr_RC_ERR) << "core " << core;
		EXPECT_EQ(awemgr_get_layout_info(m_mgr_p, 0, core, &layoutinfo), awemgr_RC_ERR) << "core " << core;
		EXPECT_EQ(awemgr_get_heap_info(m_mgr_p, 0, core, &heaps), awemgr_RC_ERR) << "core " << core;
		EXPECT_EQ(awemgr_get_modulelist_info(m_mgr_p, 0, core, &modulelist), awemgr_RC_ERR) << "core " << core;
	}
}

/**
```yaml
- id: itest~AWEMGR.Awc.EndpointOutOfRange~1
  covers: req~AWEMGR.ApiArgumentValidation~1
  description: Checks that AWC load/unload reject an endpoint index outside the
    supported range.
```
*/
TEST_F(AweMgrTestFixture, AwcEndpointOutOfRange) {
	EXPECT_EQ(awemgr_load_awc(m_mgr_p, awc_file, -1), awemgr_RC_ERR);
	EXPECT_EQ(awemgr_load_awc(m_mgr_p, awc_file, MAX_AWE_ENDPOINTS), awemgr_RC_ERR);
	EXPECT_EQ(awemgr_unload_awc(m_mgr_p, -1), awemgr_RC_ERR);
	EXPECT_EQ(awemgr_unload_awc(m_mgr_p, MAX_AWE_ENDPOINTS), awemgr_RC_ERR);
	EXPECT_TRUE(awemgr_get_awc_context(m_mgr_p, -1) == NULL);
	EXPECT_TRUE(awemgr_get_awc_context(m_mgr_p, MAX_AWE_ENDPOINTS) == NULL);
}

