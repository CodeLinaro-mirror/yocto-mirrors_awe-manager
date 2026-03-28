#pragma once
#include <gtest/gtest.h>
#include "awc_internal.h"

class AWCTestFixture : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
    void* loadAWC(const char* file);
	void* awc = NULL;
};


class AWCTestSchema1Fixture : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
	void* awc = NULL;
};

class AWCTestInvalidFileFixture : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
	void* awc = NULL;
};

class AWCTestsWithoutFilesFixture : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
	int TestParseLine(std::string line);
	int TestParseLineNoAWC(std::string line);
	awe_awc_t* awc = NULL;
};

class AWCTestEventsFixture : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
	void* awc = NULL;
};

class AWCTestUserdataFixture : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
	void* awc = NULL;
};

class AWCTestAlias : public AWCTestUserdataFixture {
};

class AWCMapTest : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
    AwcMap *map_m = NULL;
};