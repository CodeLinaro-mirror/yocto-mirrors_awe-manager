#include "test_fixtures.h"
#include "awemgr_logging.h"
#include "awe_awc.h"
#include "awe_ctrl.h"
#include "Errors.h"

static const char* awc_file = TEST_DATA_DIR "/designs/set_get/target_files/awc_index.txt";

/* ****************************************************************************
 * TEST CASES
 * ***************************************************************************/

/**
```yaml
- id: itest~AWEMGR.AweCoreInfo.Memory~2
  covers: req~AWEMGR.AWECoreInformationQuery~1
  description: Checks that memory info can be retrieved
```
*/
TEST_F(AweMgrTestFixtureSetGetAWC, AweCoreMemory) {
	awemgr_heapinfo info_buffer;
	ASSERT_EQ(awemgr_get_heap_info(m_mgr_p, 0, &info_buffer), awemgr_RC_OK);

	// note: checks here depend on LinuxApp version/variant used !!!
	ASSERT_EQ(info_buffer.nr_heaps, 4);
	ASSERT_EQ(info_buffer.fast_a.nr_free, 4999999);
	ASSERT_EQ(info_buffer.fast_b.nr_free, 4999999);
	ASSERT_EQ(info_buffer.slow.nr_free, 4999999);
	ASSERT_EQ(info_buffer.shared.nr_free, 19999932);

	// test the helper functions for coverage.
	ASSERT_EQ(awemgr_heapinfo_allocated(info_buffer.fast_a), 1);
	ASSERT_GT(awemgr_heapinfo_allocated_percent(info_buffer.fast_a), 0.0f);
}


/**
```yaml
- id: itest~AWEMGR.AweCoreInfo.Cpu~1
  covers: req~AWEMGR.AWECoreInformationQuery~1
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
  covers: req~AWEMGR.AWECoreInformationQuery~1
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
- id: itest~AWEMGR.AweCoreInfo.Fails~1
  covers: req~AWEMGR.AWECoreInformationQuery~1
  description: Checks capture of incorrect command usages.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWCLoaded, AweCoreInfoFails) {
	awemgr_heapinfo heaps;
	awemgr_modulelist_info modulelist;
	awemgr_cpuinfo cpuinfo;

	ASSERT_EQ(awemgr_get_heap_info(NULL, 0, &heaps), awemgr_RC_ERR);
	ASSERT_EQ(awemgr_get_heap_info(m_mgr_p, 0, NULL), awemgr_RC_ERR);

	ASSERT_EQ(awemgr_get_modulelist_info(NULL, 0, 0, &modulelist), awemgr_RC_ERR);
	ASSERT_EQ(awemgr_get_modulelist_info(m_mgr_p, 0, 0, NULL), awemgr_RC_ERR);

	ASSERT_EQ(awemgr_get_cpu_info(NULL, 0, 0, &cpuinfo), awemgr_RC_ERR);
	ASSERT_EQ(awemgr_get_cpu_info(m_mgr_p, 0, 0, NULL), awemgr_RC_ERR);
}
