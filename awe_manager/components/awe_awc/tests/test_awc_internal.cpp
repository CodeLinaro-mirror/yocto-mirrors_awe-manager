#include <gtest/gtest.h>
#include "awe_awc.h"
#include "awc_internal.h"
#include "awosal_string.h"
#include "awc_test_fixtures.hpp"

/**
```yaml
- id: utest~AWEMGR.AWC.stringToNumber~1
  covers: dsn~AWEMGR.AWC.CheckConsistency~1
  description: |
    Checks the parser internal functions with incorrect usage.
```
*/
TEST(AWC_INTERNAL_FUNCTIONS, stringToNumber)
{
	const char *str = NULL;
	UINT32 number = 0;
	ASSERT_EQ(stringToNumber(str, &number), E_AWC_ERROR);
	ASSERT_EQ(stringToNumber(str, NULL), E_AWC_ERROR);
	str = "efdf";
	ASSERT_EQ(stringToNumber(str, NULL), E_AWC_ERROR);
	ASSERT_EQ(stringToNumber(str, &number), E_AWC_ERROR);
	str = "45";
	ASSERT_EQ(stringToNumber(str, &number), E_AWC_SUCCESS);
	ASSERT_EQ(number, 45);
}

/**
```yaml
- id: utest~AWEMGR.AWC.parseNumericToken~1
  covers: dsn~AWEMGR.AWC.CheckConsistency~1
  description: |
    Checks the parser internal functions with incorrect usage.
```
*/
TEST(AWC_INTERNAL_FUNCTIONS, parseNumericToken)
{
	char *context = NULL;
	UINT32 number = 0;
	ASSERT_EQ(parseNumericToken(NULL, NULL, NULL), E_AWC_ERROR);
	ASSERT_EQ(parseNumericToken(&number, NULL, NULL), E_AWC_ERROR);
	ASSERT_EQ(parseNumericToken(&number, ",", NULL), E_AWC_ERROR);
	ASSERT_EQ(parseNumericToken(&number, ",", &context), E_AWC_ERROR);
	char line[10] = "abc,45";
	char *token = strtok_r(line, AWC_DELIMITER, &context);
	if (token != NULL)
	{
		ASSERT_EQ(parseNumericToken(&number, ",", &context), E_AWC_SUCCESS);
		ASSERT_EQ(number, 45);
	}
}

/**
```yaml
- id: utest~AWEMGR.AWC.parseStringToken~1
  covers: dsn~AWEMGR.AWC.CheckConsistency~1
  description: |
    Checks the parser internal functions with incorrect usage.
```
*/
TEST(AWC_INTERNAL_FUNCTIONS, parseStringToken)
{
	char *context = NULL;
	char *data;
	ASSERT_EQ(parseStringToken(NULL, NULL, NULL), E_AWC_ERROR);
	ASSERT_EQ(parseStringToken(&data, NULL, NULL), E_AWC_ERROR);
	ASSERT_EQ(parseStringToken(&data, ",", NULL), E_AWC_ERROR);
	ASSERT_EQ(parseStringToken(&data, ",", &context), E_AWC_ERROR);
	char line[10] = "abc,def";
	char *token = strtok_r(line, AWC_DELIMITER, &context);
	if (token != NULL)
	{
		ASSERT_EQ(parseStringToken(&data, ",", &context), E_AWC_SUCCESS);
		ASSERT_STREQ(data, "def");
		free(data);
	}
}

/**
```yaml
- id: utest~AWEMGR.AWC.parseType~1
  covers: dsn~AWEMGR.AWC.CheckConsistency~1
  description: |
    Checks the parser internal functions with incorrect usage.
```
*/
TEST(AWC_INTERNAL_FUNCTIONS, parseType)
{
	char *context = NULL;
	awc_ctl_type_t type;
	ASSERT_EQ(parseType(NULL, NULL), E_AWC_CTL_TYPE_ERROR);
	ASSERT_EQ(parseType(&type, NULL), E_AWC_CTL_TYPE_ERROR);
	ASSERT_EQ(parseType(&type, &context), E_AWC_CTL_TYPE_ERROR);
	
	char line[10] = "abc,int";
	char *token = strtok_r(line, AWC_DELIMITER, &context);
	if (token != NULL)
	{
		ASSERT_EQ(parseType(&type, &context), E_AWC_SUCCESS);
		ASSERT_STREQ(awc_get_typename(type), "int");
	}

	char invalidtypeline[] = "abc,['AWEInstance*', 'AWEInstance *']";
	context = NULL;
	token = strtok_r(invalidtypeline, AWC_DELIMITER, &context);
	if (token != NULL)
	{
		ASSERT_EQ(parseType(&type, &context), E_AWC_CTL_TYPE_ERROR);;
	}
}

/**
```yaml
- id: utest~AWEMGR.AWC.validateModuleParsing~1
  covers: dsn~AWEMGR.AWC.CheckConsistency~1
  description: |
    Checks the parser internal functions with incorrect usage.
```
*/
TEST(AWC_INTERNAL_FUNCTIONS, validateModuleParsing)
{
	awe_awc_t *awc = (awe_awc_t *)calloc(1, sizeof(awe_awc_t));
	ASSERT_TRUE(awc != NULL);
	char *context = NULL;
	ASSERT_EQ(validateModuleParsing(NULL, NULL), E_AWC_ERROR);
	ASSERT_EQ(validateModuleParsing(awc, NULL), E_AWC_ERROR);
	ASSERT_EQ(validateModuleParsing(NULL,  &context), E_AWC_ERROR);
	ASSERT_EQ(validateModuleParsing(awc, &context), E_AWC_ERROR);
	char line[10] = "abc,int";
	char *token = strtok_r(line, AWC_DELIMITER, &context);
	if (token != NULL)
	{
		awc->modules = (awc_module_t *)calloc(1, sizeof(awc_module_t *));
		ASSERT_EQ(validateModuleParsing(awc, &context), E_AWC_ERROR);
		awc->info.modulecount = 1;
		ASSERT_EQ(validateModuleParsing(awc, &context), E_AWC_SUCCESS);
		free(awc->modules);
		awc->modules = NULL;
	}
	awc_uninit((void **)&awc);
}

/**
```yaml
- id: utest~AWEMGR.AWC.validateControlParsing~1
  covers: dsn~AWEMGR.AWC.CheckConsistency~1
  description: |
    Checks the parser internal functions with incorrect usage.
```
*/
TEST(AWC_INTERNAL_FUNCTIONS, validateControlParsing)
{
	awe_awc_t *awc = (awe_awc_t *)calloc(1, sizeof(awe_awc_t));
	ASSERT_TRUE(awc != NULL);
	char *context = NULL;
	ASSERT_EQ(validateControlParsing(NULL, NULL), E_AWC_ERROR);
	ASSERT_EQ(validateControlParsing(awc, NULL), E_AWC_ERROR);
	ASSERT_EQ(validateControlParsing(awc, &context), E_AWC_ERROR);
	char line[10] = "abc,int";
	char *token = strtok_r(line, AWC_DELIMITER, &context);
	if (token != NULL)
	{
		awc->modules = (awc_module_t *)calloc(1, sizeof(awc_module_t *));
		ASSERT_EQ(validateControlParsing(awc, &context), E_AWC_ERROR);
		awc->info.modulecount = 1;
		ASSERT_EQ(validateControlParsing(awc, &context), E_AWC_ERROR);
		awc->controls = (awc_ctl_t *)calloc(1, sizeof(awc_ctl_t *));
		ASSERT_EQ(validateControlParsing(awc, &context), E_AWC_ERROR);
		awc->info.controlcount = 1;
		ASSERT_EQ(validateControlParsing(awc, &context), E_AWC_ERROR);
		awc->moduleindex = 1;
		ASSERT_EQ(validateControlParsing(awc, &context), E_AWC_SUCCESS);
		free(awc->controls);
		free(awc->modules);
	}
	ASSERT_EQ(validateControlModule(NULL), E_AWC_ERROR);
	free(awc);
}

/**
```yaml
- id: utest~AWEMGR.AWC.MapCreateAndFree~1
  covers: dsn~AWEMGR.AWC.CheckConsistency~1
  description: |
    Checks that map can be created (in Fixture Setup) and freed(in Fixture Destroy) .
```
*/
TEST_F(AWCMapTest, MapCreateAndFree)
{
	ASSERT_NE(map_m, nullptr);	 // Ensure the table is created
	ASSERT_EQ(map_m->size, 257); // Check initial size
	ASSERT_EQ(map_m->count, 0);	 // Ensure it's empty
}

/**
```yaml
- id: utest~AWEMGR.AWC.MapInsert~1
  covers: dsn~AWEMGR.AWC.CheckConsistency~1
  description: |
    Checks that elements can be inserted in the map.
```
*/
TEST_F(AWCMapTest, MapInsert)
{
	ASSERT_EQ(awc_map_insert(NULL, NULL, NULL), -1);
	ASSERT_EQ(awc_map_insert(NULL, NULL, (void *)"value1"), -1);
	ASSERT_EQ(awc_map_insert(NULL, "key1", (void *)"value1"), -1);
	ASSERT_EQ(awc_map_insert(map_m, NULL, (void *)"value1"), -1);
	ASSERT_EQ(awc_map_insert(map_m, NULL, NULL), -1);
	ASSERT_EQ(awc_map_insert(NULL, "key1", NULL), -1);

	ASSERT_EQ(awc_map_insert(map_m, "key1", (void *)"value1"), 0);
	ASSERT_EQ(awc_map_insert(map_m, "key2", (void *)"value2"), 0);
	ASSERT_EQ(map_m->count, 2); // Ensure count updates correctly
}

/**
```yaml
- id: utest~AWEMGR.AWC.MapSearch~1
  covers: dsn~AWEMGR.AWC.CheckConsistency~1
  description: |
    Checks that elements can be retreived from the map.
```
*/
TEST_F(AWCMapTest, MapSearch)
{
	EXPECT_EQ(awc_map_insert(map_m, "key1", (void *)"value1"), 0);
	EXPECT_EQ(awc_map_insert(map_m, "key2", (void *)"value2"), 0);

	EXPECT_EQ(awc_map_search(NULL, "key1"), nullptr);
	EXPECT_EQ(awc_map_search(map_m, NULL), nullptr);
	EXPECT_EQ(awc_map_search(NULL, NULL), nullptr);
	EXPECT_EQ(awc_map_search(map_m, "nonexistent"), nullptr); // Ensure nonexistent keys return null

	EXPECT_STREQ((char *)awc_map_search(map_m, "key1"), "value1");
	EXPECT_STREQ((char *)awc_map_search(map_m, "key2"), "value2");
	
}

/**
```yaml
- id: utest~AWEMGR.AWC.MapDelete~1
  covers: dsn~AWEMGR.AWC.CheckConsistency~1
  description: |
    Checks that elements can be deleted from the map.
```
*/
TEST_F(AWCMapTest, MapDelete)
{
	ASSERT_EQ(awc_map_insert(map_m, "key1", (void *)"value1"), 0);
	ASSERT_EQ(awc_map_insert(map_m, "key2", (void *)"value2"), 0);

	EXPECT_EQ(awc_map_delete(NULL, "key1"), -1);
	EXPECT_EQ(awc_map_delete(map_m, NULL), -1);
	EXPECT_EQ(awc_map_delete(NULL, NULL), -1);
	EXPECT_EQ(awc_map_delete(map_m, "nonexistent"), -1); // Ensure nonexistent keys return null

	ASSERT_EQ(awc_map_delete(map_m, "key1"), 0);	   // Successfully deleted
	EXPECT_EQ(awc_map_search(map_m, "key1"), nullptr); // Ensure it's gone
	ASSERT_EQ(map_m->count, 1);						   // Count updated

	ASSERT_EQ(awc_map_delete(map_m, "nonexistent"), -1); // Nonexistent key
}

/**
```yaml
- id: utest~AWEMGR.AWC.MapDuplicateKeys~1
  covers: dsn~AWEMGR.AWC.CheckConsistency~1
  description: |
    Checks that duplicate keys are rejected.
```
*/
TEST_F(AWCMapTest, MapDuplicateKeys)
{
	ASSERT_EQ(awc_map_insert(map_m, "key1", (void *)"value1"), 0);
	ASSERT_EQ(awc_map_insert(map_m, "key1", (void *)"value2"), -1);

	EXPECT_STREQ((char *)awc_map_search(map_m, "key1"), "value1");
}

/**
```yaml
- id: utest~AWEMGR.AWC.MapResize~1
  covers: dsn~AWEMGR.AWC.CheckConsistency~1
  description: |
    Checks that table can resize itself when the number of elements increase map size
```
*/
TEST(AWCMapTest1, MapResize)
{
	AwcMap *map_m = awc_map_create(2); // Small map size
	ASSERT_TRUE(map_m != NULL);

	for (int i = 0; i < 50; ++i)
	{
		char key[10];
		snprintf(key, sizeof(key), "key%d", i);
		ASSERT_EQ(awc_map_insert(map_m, key, (void *)"value"), 0);
	}

	EXPECT_GT(map_m->size, 3);	 // Ensure resizing occurred
	EXPECT_EQ(map_m->count, 50); // Ensure all elements are present

	for (int i = 0; i < 50; ++i)
	{
		char key[10];
		snprintf(key, sizeof(key), "key%d", i);
		EXPECT_STREQ((char *)awc_map_search(map_m, key), "value");
	}
	ASSERT_EQ(awc_map_free(&map_m), 0);
	ASSERT_TRUE(map_m == NULL);
}

/**
```yaml
- id: utest~AWEMGR.AWC.MapHashConsistency~1
  covers: dsn~AWEMGR.AWC.CheckConsistency~1
  description: |
    Checks that hash algorithm produces same hash for same input and different hash for different input.
```
*/
TEST(MurmurHashTest, HashConsistency)
{
	const char *key1 = "test_key";
	const char *key2 = "another_key";
	uint32_t seed = 42;

	uint32_t hash1 = awc_map_hash(key1, strlen(key1), seed);
	uint32_t hash1_repeat = awc_map_hash(key1, strlen(key1), seed);

	// Same input and seed should produce the same hash
	EXPECT_EQ(hash1, hash1_repeat);

	uint32_t hash2 = awc_map_hash(key2, strlen(key2), seed);

	// Different keys should produce different hashes
	EXPECT_NE(hash1, hash2);
}

/**
```yaml
- id: utest~AWEMGR.AWC.MapHashDistribution~1
  covers: dsn~AWEMGR.AWC.CheckConsistency~1
  description: |
	Checks that hash distribution is uniform
*/
TEST(MurmurHashTest, HashDistribution)
{
	const uint32_t num_keys = 200000;	 // Number of keys to insert
	const uint32_t hash_map_size =1031; // Size of the hash table, prime number
	uint32_t seed = 241286;

	// Create a histogram to track how many keys are in each bucket
	uint32_t buckets[hash_map_size] = {0};

	// Generate keys and insert them into the hash table
	for (uint32_t i = 0; i < num_keys; ++i)
	{
		std::string key = "key_" + std::to_string(i);

		uint32_t hash = awc_map_hash(key.c_str(), key.length(), seed);
		uint32_t bucket = hash % hash_map_size;

		buckets[bucket]++;
	}

	// Calculate the expected min and max bucket sizes
	uint32_t min_expected = (num_keys / hash_map_size) / 3;
	uint32_t max_expected = 2 * ((num_keys + hash_map_size - 1) / hash_map_size);

	// Check min and max bucket sizes
	uint32_t min_bucket_size = num_keys;
	uint32_t max_bucket_size = 0;

	for (uint32_t i = 0; i < hash_map_size; ++i)
	{
		if (buckets[i] > 0)
		{
			if (buckets[i] < min_bucket_size)
			{
				min_bucket_size = buckets[i];
			}
			if (buckets[i] > max_bucket_size)
			{
				max_bucket_size = buckets[i];
			}
		}
	}
	std::cout << "Min bucket size: " << min_bucket_size << std::endl;
	std::cout << "Max bucket size: " << max_bucket_size << std::endl;

	EXPECT_GE(min_bucket_size, min_expected); // Min bucket size should be at least the expected min size
	EXPECT_LE(max_bucket_size, max_expected); // Max bucket size should be at most the expected max size
}