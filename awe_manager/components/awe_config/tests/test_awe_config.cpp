#include "awe_config.h"
#include <gtest/gtest.h>

class AweConfigTestFixture: public testing::Test
{
	public:
    void SetUp()
    {
        ASSERT_EQ(aweconfig_create(NULL), AWECFG_RC_FAIL);
        ASSERT_EQ(aweconfig_create(&config_m), AWECFG_RC_OK);
        ASSERT_TRUE(config_m != NULL);

        // Add keys of different types
        ASSERT_EQ(aweconfig_add(config_m, "bool_true", "Bool True", "true"), AWECFG_RC_OK);
        ASSERT_EQ(aweconfig_add(config_m, "bool_false", "Bool False", "false"), AWECFG_RC_OK);
        ASSERT_EQ(aweconfig_add(config_m, "bool_on", "Bool On", "on"), AWECFG_RC_OK);
        ASSERT_EQ(aweconfig_add(config_m, "bool_off", "Bool Off", "off"), AWECFG_RC_OK);
        ASSERT_EQ(aweconfig_add(config_m, "bool_0", "Bool On", "on"), AWECFG_RC_OK);
        ASSERT_EQ(aweconfig_add(config_m, "bool_1", "Bool Off", "off"), AWECFG_RC_OK);
        ASSERT_EQ(aweconfig_add(config_m, "int_val", "Integer", "42"), AWECFG_RC_OK);
        ASSERT_EQ(aweconfig_add(config_m, "uint_val", "Unsigned", "100"), AWECFG_RC_OK);
        ASSERT_EQ(aweconfig_add(config_m, "float_val", "Float", "3.14"), AWECFG_RC_OK);

        // Add keys with invalid conversion values.
        ASSERT_EQ(aweconfig_add(config_m, "invalid_bool", "Invalid Bool", "notabool"), AWECFG_RC_OK);
        ASSERT_EQ(aweconfig_add(config_m, "invalid_int", "Invalid Int", "abc"), AWECFG_RC_OK);
        ASSERT_EQ(aweconfig_add(config_m, "invalid_uint", "Invalid UInt", "-50"), AWECFG_RC_OK);
        ASSERT_EQ(aweconfig_add(config_m, "invalid_float", "Invalid Float", "pi"), AWECFG_RC_OK);
    };
	void TearDown()
    {
        ASSERT_EQ(aweconfig_destroy(NULL), AWECFG_RC_FAIL);
        ASSERT_EQ(aweconfig_destroy(&config_m), AWECFG_RC_OK);
        ASSERT_TRUE(config_m == NULL);
    };

    awe_config* config_m = NULL;
};


/**
```yaml
- id: utest~AWEMGR.AWECONFIG.add_set_get~1
  covers: dsn~AWEMGR.AWECONFIG.Add~1
  description: |
    Checks that the configs can be added and set / get can be performed on the config.
```
*/
TEST_F(AweConfigTestFixture, ADD_SET_GET) {
    ASSERT_EQ(aweconfig_add(config_m, "key", "one random config without description", "value"), AWECFG_RC_OK);

    const char* description;
    ASSERT_STREQ(aweconfig_get(config_m, "key", &description), "value");
    ASSERT_STREQ(description, "one random config without description");

    ASSERT_EQ(aweconfig_set(config_m, "key", "value1"), AWECFG_RC_OK);
    ASSERT_STREQ(aweconfig_get(config_m, "key", &description), "value1");

    // Add the same config again
    ASSERT_EQ(aweconfig_add(config_m, "key", "another description", "newvalue"), AWECFG_RC_OK);
    ASSERT_STREQ(aweconfig_get(config_m, "key", &description), "newvalue");
    ASSERT_STREQ(description, "another description");
}

/**
```yaml
- id: utest~AWEMGR.AWECONFIG.add_string_set~1
  covers:
    - dsn~AWEMGR.AWECONFIG.BatchAdd~1
    - dsn~AWEMGR.AWECONFIG.Get~1
  description: |
    Checks that the configs can be added and set via a config string, and then can be retrieved.
```
*/
TEST_F(AweConfigTestFixture, ADD_STRING_SET) {
    ASSERT_EQ(aweconfig_add(config_m, "key", "desc", "none"), AWECFG_RC_OK);
    ASSERT_EQ(aweconfig_add(config_m, "key2", "desc2", "none"), AWECFG_RC_OK);

    ASSERT_EQ(aweconfig_from_string(config_m, "key=value;key2=value2;"), AWECFG_RC_OK);

    const char* description;
    ASSERT_STREQ(aweconfig_get(config_m, "key", &description), "value");
    ASSERT_STREQ(aweconfig_get(config_m, "key2", &description), "value2");
}

/**
```yaml
- id: utest~AWEMGR.AWECONFIG.add_string_get~1
  covers:
    - dsn~AWEMGR.AWECONFIG.EnvAdd~1
    - dsn~AWEMGR.AWECONFIG.Get~1
  description: |
    Checks that the configs can be added and set via a config string, and then can be retrieved.
```
*/
TEST_F(AweConfigTestFixture, ADD_ENV_SET) {
    ASSERT_EQ(aweconfig_add(config_m, "key", "desc", "none"), AWECFG_RC_OK);
    ASSERT_EQ(aweconfig_add(config_m, "key2", "desc2", "none"), AWECFG_RC_OK);

    ASSERT_EQ(setenv("MYTESTVAR123", "key=value;key2=value2;", 1), 0);

    ASSERT_EQ(aweconfig_from_envvar(config_m, "MYTESTVAR123"), AWECFG_RC_OK);

    const char* description;
    ASSERT_STREQ(aweconfig_get(config_m, "key", &description), "value");
    ASSERT_STREQ(aweconfig_get(config_m, "key2", &description), "value2");
}

/**
```yaml
- id: utest~AWEMGR.AWECONFIG.bool_test~1
  covers: dsn~AWEMGR.AWECONFIG.Get~1
  description: |
    Checks that the bool values are correctly handled.
```
*/
TEST_F(AweConfigTestFixture, bool_test) {
    bool val = false;
    EXPECT_EQ(aweconfig_get_as_bool(config_m, "bool_true", &val), AWECFG_RC_OK);
    EXPECT_TRUE(val);
    val = true;
    EXPECT_EQ(aweconfig_get_as_bool(config_m, "bool_false", &val), AWECFG_RC_OK);
    EXPECT_FALSE(val);

    val = false;
    EXPECT_EQ(aweconfig_get_as_bool(config_m, "bool_on", &val), AWECFG_RC_OK);
    EXPECT_TRUE(val);
    val = true;
    EXPECT_EQ(aweconfig_get_as_bool(config_m, "bool_off", &val), AWECFG_RC_OK);
    EXPECT_FALSE(val);

    val = false;
    EXPECT_EQ(aweconfig_get_as_bool(config_m, "bool_0", &val), AWECFG_RC_OK);
    EXPECT_TRUE(val);
    val = true;
    EXPECT_EQ(aweconfig_get_as_bool(config_m, "bool_1", &val), AWECFG_RC_OK);
    EXPECT_FALSE(val);
}

/**
```yaml
- id: utest~AWEMGR.AWECONFIG.int_test~1
  covers: dsn~AWEMGR.AWECONFIG.Get~1
  description: |
    Checks that the int values are correctly handled.
```
*/
TEST_F(AweConfigTestFixture, int_test) {
    int32_t value = 0;
    EXPECT_EQ(aweconfig_get_as_int(config_m, "int_val", &value), AWECFG_RC_OK);
    EXPECT_EQ(value, 42);
}

/**
```yaml
- id: utest~AWEMGR.AWECONFIG.uint_test~1
  covers: dsn~AWEMGR.AWECONFIG.Get~1
  description: |
    Checks that the unsigned int values are correctly handled.
```
*/
TEST_F(AweConfigTestFixture, uint_test) {
    uint32_t value = 0;
    EXPECT_EQ(aweconfig_get_as_uint(config_m, "uint_val", &value), AWECFG_RC_OK);
    EXPECT_EQ(value, 100u);
}

/**
```yaml
- id: utest~AWEMGR.AWECONFIG.float_test~1
  covers: dsn~AWEMGR.AWECONFIG.Get~1
  description: |
    Checks that the float values are correctly handled.
```
*/
TEST_F(AweConfigTestFixture, float_test) {
    float value = 0.f;
    EXPECT_EQ(aweconfig_get_as_float(config_m, "float_val", &value), AWECFG_RC_OK);
    EXPECT_EQ(value, 3.14f);
}

/**
```yaml
- id: utest~AWEMGR.AWECONFIG.ErrorCases~1
  covers: dsn~AWEMGR.AWECONFIG.Errorhandling~1
  description: |
    Checks that the component correctly handles the error cases
```
*/
TEST_F(AweConfigTestFixture, ErrorCases) {
    bool bvalue = false;
    EXPECT_EQ(aweconfig_get_as_bool(config_m, "invalid_bool", &bvalue), AWECFG_RC_CONVERSION_FAIL);
    EXPECT_EQ(aweconfig_get_as_bool(config_m, "nonexistent_bool", &bvalue), AWECFG_RC_NOT_FOUND);

    int32_t ivalue = 0;
    EXPECT_EQ(aweconfig_get_as_int(config_m, "invalid_int", &ivalue), AWECFG_RC_CONVERSION_FAIL);
    EXPECT_EQ(aweconfig_get_as_int(config_m, "nonexistent_int", &ivalue), AWECFG_RC_NOT_FOUND);

    uint32_t uvalue = 0;
    EXPECT_EQ(aweconfig_get_as_uint(config_m, "invalid_uint", &uvalue), AWECFG_RC_CONVERSION_FAIL);
    EXPECT_EQ(aweconfig_get_as_uint(config_m, "nonexistent_uint", &uvalue), AWECFG_RC_NOT_FOUND);

    float fvalue = 0.f;
    EXPECT_EQ(aweconfig_get_as_float(config_m, "invalid_float", &fvalue), AWECFG_RC_CONVERSION_FAIL);
    EXPECT_EQ(aweconfig_get_as_float(config_m, "nonexistent_float", &fvalue), AWECFG_RC_NOT_FOUND);

    const char* description;
    ASSERT_EQ(aweconfig_add(NULL, "config", "description", "value"), AWECFG_RC_FAIL);
    ASSERT_EQ(aweconfig_set(NULL, "config", "value"), AWECFG_RC_FAIL);
    ASSERT_STREQ("", aweconfig_get(NULL, "config", &description));

    ASSERT_EQ(aweconfig_add(config_m, NULL, "description", "value"), AWECFG_RC_FAIL);
    ASSERT_EQ(aweconfig_set(config_m, NULL, "value"), AWECFG_RC_FAIL);
    ASSERT_STREQ("", aweconfig_get(config_m, NULL, &description));

    ASSERT_EQ(aweconfig_add(config_m, "config", "description", "value"), AWECFG_RC_OK);
    ASSERT_EQ(aweconfig_set(config_m, "fakeconfig", "value"), AWECFG_RC_FAIL);
    ASSERT_STREQ("", aweconfig_get(config_m, "fakeconfig", &description));

    ASSERT_EQ(aweconfig_from_string(config_m, "blablabla"), AWECFG_RC_CONVERSION_FAIL);
}