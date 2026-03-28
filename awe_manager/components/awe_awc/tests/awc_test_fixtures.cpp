#include "awc_test_fixtures.hpp"
#include "awemgr_logging.h"
#include "awc_internal.h"

void AWCTestFixture::SetUp() {
	set_loglevel(AWEMGR_LOG_AWC, AWEMGR_LOG_LEVEL_ERROR);
	awc = loadAWC(AWC_DATA_DIR "/simple_awc_file.txt");
	ASSERT_TRUE(awc != NULL) << "Setup Failed";
}

void AWCTestFixture::TearDown() {
	if(awc)
	{
		ASSERT_EQ(awc_uninit(&awc), E_AWC_SUCCESS);
	}
	ASSERT_TRUE(awc == NULL);
}

void* AWCTestFixture::loadAWC(const char* file)
{
	if(awc)
	{
		awc_uninit(&awc);
	}
	return awc_init(file);
}

void AWCTestSchema1Fixture::SetUp() {
	set_loglevel(AWEMGR_LOG_AWC, AWEMGR_LOG_LEVEL_ERROR);
	awc = awc_init(AWC_DATA_DIR "/simple_awc_file_schema1.txt");
	ASSERT_TRUE(awc != NULL) << "Setup Failed";
}

void AWCTestSchema1Fixture::TearDown() {
	ASSERT_EQ(awc_uninit(&awc), E_AWC_SUCCESS);
	ASSERT_TRUE(awc == NULL);
}

void AWCTestInvalidFileFixture::SetUp() {
	set_loglevel(AWEMGR_LOG_AWC, AWEMGR_LOG_LEVEL_ERROR);
	awc = awc_init(AWC_DATA_DIR "/invalid_awc_file.txt");
	ASSERT_TRUE(awc == NULL);
	ASSERT_TRUE(awc_init(NULL) == NULL);
	ASSERT_TRUE(awc_init("data/garbage.txt") == NULL);
}

void AWCTestInvalidFileFixture::TearDown() {
	ASSERT_EQ(awc_uninit(&awc), E_AWC_ERROR);
}


void AWCTestsWithoutFilesFixture::SetUp() {
	set_loglevel(AWEMGR_LOG_AWC, AWEMGR_LOG_LEVEL_ERROR);
	awc = (awe_awc_t*)calloc(1, sizeof(awe_awc_t));
	ASSERT_TRUE(awc != NULL);
}

void AWCTestsWithoutFilesFixture::TearDown() {
	ASSERT_EQ(awc_uninit((void**)&awc), E_AWC_SUCCESS);
	ASSERT_TRUE(awc == NULL);
}

int AWCTestsWithoutFilesFixture::TestParseLine(std::string line)
{
	return parseLine((char*)line.c_str(), (awe_awc_t*)awc);
}
int AWCTestsWithoutFilesFixture::TestParseLineNoAWC(std::string line)
{
	return parseLine((char*)line.c_str(), NULL);
}

void AWCTestEventsFixture::SetUp() {
	set_loglevel(AWEMGR_LOG_AWC, AWEMGR_LOG_LEVEL_ERROR);
	awc = awc_init(AWC_DATA_DIR "/events_awc_file.txt");
	ASSERT_TRUE(awc != NULL);
}

void AWCTestEventsFixture::TearDown() {
	ASSERT_EQ(awc_uninit(&awc), E_AWC_SUCCESS);
}

void AWCTestUserdataFixture::SetUp() {
	set_loglevel(AWEMGR_LOG_AWC, AWEMGR_LOG_LEVEL_ERROR);
	awc = awc_init(AWC_DATA_DIR "/awc_file_schema3.txt");
	ASSERT_TRUE(awc != NULL);
}

void AWCTestUserdataFixture::TearDown() {
	ASSERT_EQ(awc_uninit(&awc), E_AWC_SUCCESS);
}

void AWCMapTest::SetUp() {
	ASSERT_TRUE(awc_map_create(0) == NULL);

	map_m = awc_map_create(257);
	ASSERT_TRUE(map_m != NULL);
}

void AWCMapTest::TearDown() {
    ASSERT_EQ(awc_map_free(&map_m), 0);
	ASSERT_TRUE(map_m == NULL);
    ASSERT_EQ(awc_map_free(&map_m), -1);
    ASSERT_EQ(awc_map_free(NULL), -1);
}