#include <gtest/gtest.h>
#include "awe_config.h"

#include <stdlib.h>  // for setenv and _putenv_s

#include <string>

// --- Helper structures for testing callbacks ---
struct CallbackState {
    int call_count = 0;
    std::string last_key;
    std::string last_value;
    std::string last_desc;
    void* last_context = nullptr;
};

static void TestCallback(const char* key, const char* value, const char* description, void* context) {
    if (context != nullptr) {
        CallbackState* state = static_cast<CallbackState*>(context);
        state->call_count++;
        if (key) state->last_key = key;
        if (value) state->last_value = value;
        if (description) state->last_desc = description;
        state->last_context = context;
    }
}

class AweConfigTestFixture: public testing::Test
{
  public:
    void SetUp()
    {
        ASSERT_EQ(aweconfig_create(&config_m), AWECFG_RC_OK);
        ASSERT_TRUE(config_m != NULL);

        // Add keys of different types
        ASSERT_EQ(aweconfig_add(config_m, "bool_true", "Bool True", "true"), AWECFG_RC_OK);
        ASSERT_EQ(aweconfig_add(config_m, "bool_false", "Bool False", "false"), AWECFG_RC_OK);
        ASSERT_EQ(aweconfig_add(config_m, "bool_on", "Bool On", "on"), AWECFG_RC_OK);
        ASSERT_EQ(aweconfig_add(config_m, "bool_off", "Bool Off", "off"), AWECFG_RC_OK);
        ASSERT_EQ(aweconfig_add(config_m, "bool_0", "Bool On", "0"), AWECFG_RC_OK);
        ASSERT_EQ(aweconfig_add(config_m, "bool_1", "Bool Off", "1"), AWECFG_RC_OK);
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
        ASSERT_EQ(aweconfig_destroy(&config_m), AWECFG_RC_OK);
        ASSERT_TRUE(config_m == NULL);
    };

    awe_config* config_m = NULL;
};


/**
```yaml
- id: utest~AWEMGR.AWECONFIG.AddSetAndGet~1
  covers: dsn~AWEMGR.AWECONFIG.Add~1
  description: |
    Checks that the configs can be added and set / get can be performed on the config.
```
*/
TEST_F(AweConfigTestFixture, AddSetAndGet) {
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
- id: utest~AWEMGR.AWECONFIG.PerConfigCallback~1
  covers: dsn~AWEMGR.AWECONFIG.Callback~1
  description: |
    Checks that callbacks can be bound to specific configs and trigger on init, add, and set.
```
*/
TEST_F(AweConfigTestFixture, PerConfigCallback) {
    CallbackState state;

    ASSERT_EQ(aweconfig_add(config_m, "cb_key", "Test cb", "initial"), AWECFG_RC_OK);

    // Bind listener and verify it fires immediately with the current state
    ASSERT_EQ(aweconfig_add_listener(config_m, "cb_key", TestCallback, &state), AWECFG_RC_OK);
    EXPECT_EQ(state.call_count, 1);
    EXPECT_EQ(state.last_key, "cb_key");
    EXPECT_EQ(state.last_value, "initial");
    EXPECT_EQ(state.last_desc, "Test cb");

    // Verify updating via SET triggers the callback
    ASSERT_EQ(aweconfig_set(config_m, "cb_key", "updated"), AWECFG_RC_OK);
    EXPECT_EQ(state.call_count, 2);
    EXPECT_EQ(state.last_value, "updated");

    // Verify updating via ADD (overwrite) triggers the callback
    ASSERT_EQ(aweconfig_add(config_m, "cb_key", "New desc", "overwritten"), AWECFG_RC_OK);
    EXPECT_EQ(state.call_count, 3);
    EXPECT_EQ(state.last_value, "overwritten");
    EXPECT_EQ(state.last_desc, "New desc");

    // Verify modifying a different config DOES NOT trigger this callback
    ASSERT_EQ(aweconfig_set(config_m, "bool_true", "false"), AWECFG_RC_OK);
    EXPECT_EQ(state.call_count, 3);
}

/**
```yaml
- id: utest~AWEMGR.AWECONFIG.AddStringSet~1
  covers:
    - dsn~AWEMGR.AWECONFIG.BatchAdd~1
    - dsn~AWEMGR.AWECONFIG.Get~1
  description: |
    Checks that the configs can be added and set via a config string, and then can be retrieved.
```
*/
TEST_F(AweConfigTestFixture, AddStringSet) {
    ASSERT_EQ(aweconfig_add(config_m, "key", "desc", "none"), AWECFG_RC_OK);
    ASSERT_EQ(aweconfig_add(config_m, "key2", "desc2", "none"), AWECFG_RC_OK);

    ASSERT_EQ(aweconfig_from_string(config_m, "key=value;key2=value2;"), AWECFG_RC_OK);

    const char* description;
    ASSERT_STREQ(aweconfig_get(config_m, "key", &description), "value");
    ASSERT_STREQ(aweconfig_get(config_m, "key2", &description), "value2");
}

/**
```yaml
- id: utest~AWEMGR.AWECONFIG.AddEnvSet~1
  covers:
    - dsn~AWEMGR.AWECONFIG.EnvAdd~1
    - dsn~AWEMGR.AWECONFIG.Get~1
  description: |
    Checks that the configs can be added and set via a config string, and then can be retrieved.
```
*/
TEST_F(AweConfigTestFixture, AddEnvSet) {
    ASSERT_EQ(aweconfig_add(config_m, "key", "desc", "none"), AWECFG_RC_OK);
    ASSERT_EQ(aweconfig_add(config_m, "key2", "desc2", "none"), AWECFG_RC_OK);

#ifdef WIN32
    ASSERT_EQ(_putenv_s("MYTESTVAR123", "key=value;key2=value2;"), 0);
#else
    ASSERT_EQ(setenv("MYTESTVAR123", "key=value;key2=value2;", 1), 0);
#endif

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

    val = true;
    EXPECT_EQ(aweconfig_get_as_bool(config_m, "bool_0", &val), AWECFG_RC_OK);
    EXPECT_FALSE(val);
    val = false;
    EXPECT_EQ(aweconfig_get_as_bool(config_m, "bool_1", &val), AWECFG_RC_OK);
    EXPECT_TRUE(val);
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

    ASSERT_EQ(aweconfig_create(NULL), AWECFG_RC_FAIL);
    ASSERT_EQ(aweconfig_destroy(NULL), AWECFG_RC_FAIL);

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

    // Callback error checks
    CallbackState state;
    ASSERT_EQ(aweconfig_add_listener(NULL, "config", TestCallback, &state), AWECFG_RC_FAIL);
    ASSERT_EQ(aweconfig_add_listener(config_m, NULL, TestCallback, &state), AWECFG_RC_FAIL);
    ASSERT_EQ(aweconfig_add_listener(config_m, "nonexistent_key", TestCallback, &state), AWECFG_RC_NOT_FOUND);

    ASSERT_EQ(aweconfig_from_string(config_m, "blablabla"), AWECFG_RC_CONVERSION_FAIL);
}


/**
```yaml
- id: utest~AWEMGR.AWECONFIG.add_multiple_strings~1
  covers:
    - dsn~AWEMGR.AWECONFIG.BatchAdd~1
  description: |
    Checks a different way of adding multiple configs at once. A structured list of
    entries is provided to the add_multiple function. Both values are checked after addition.
```
*/
TEST_F(AweConfigTestFixture, AddMultiple) {

    aweconfig_init_tuple config_items[] = {
        {"key", "value_1", "desc", NULL, NULL},
        {"key2", "value_2", "desc2", NULL, NULL},
    };

    int nr_cfgs = sizeof(config_items) / sizeof(config_items[0]);
    ASSERT_EQ(aweconfig_add_multiple(config_m, config_items, nr_cfgs), AWECFG_RC_OK);

    const char* description;
    ASSERT_STREQ(aweconfig_get(config_m, "key", &description), "value_1");
    ASSERT_STREQ(aweconfig_get(config_m, "key2", &description), "value_2");
}

/**
```yaml
- id: utest~AWEMGR.AWECONFIG.add_multiple_strings_fail~1
  covers:
    - dsn~AWEMGR.AWECONFIG.BatchAdd~1
  description: |
    Calls adding multiple configs with a null pointer and expects failure.
```
*/
TEST_F(AweConfigTestFixture, AddMultipleFail) {

    aweconfig_init_tuple config_items[] = {
        {"key", "value_1", "desc", NULL, NULL},
    };

    int nr_cfgs = sizeof(config_items) / sizeof(config_items[0]);
    ASSERT_EQ(aweconfig_add_multiple(NULL, config_items, nr_cfgs), AWECFG_RC_FAIL);
    ASSERT_EQ(aweconfig_add_multiple(config_m, NULL, 0), AWECFG_RC_FAIL);

}

/**
```yaml
- id: utest~AWEMGR.AWECONFIG.add_multiple_callbacks~1
  covers:
    - dsn~AWEMGR.AWECONFIG.BatchAdd~1
    - dsn~AWEMGR.AWECONFIG.Callback~1
  description: |
    Checks that adding multiple configs with callbacks provided in the tuple
    properly binds the listeners and triggers them upon addition and update.
```
*/
TEST_F(AweConfigTestFixture, AddMultipleWithCallbacks) {
    CallbackState state1;
    CallbackState state2;

    // Create a mix of configs: two with listeners, one without
    aweconfig_init_tuple config_items[] = {
        {"cb_key_1", "val_1", "desc 1", TestCallback, &state1},
        {"cb_key_2", "val_2", "desc 2", TestCallback, &state2},
        {"no_cb_key", "val_3", "desc 3", NULL, NULL}
    };

    int nr_cfgs = sizeof(config_items) / sizeof(config_items[0]);

    ASSERT_EQ(aweconfig_add_multiple(config_m, config_items, nr_cfgs), AWECFG_RC_OK);

    // Verify callbacks fired exactly once during the binding phase of add_multiple
    EXPECT_EQ(state1.call_count, 1);
    EXPECT_EQ(state1.last_key, "cb_key_1");
    EXPECT_EQ(state1.last_value, "val_1");

    EXPECT_EQ(state2.call_count, 1);
    EXPECT_EQ(state2.last_key, "cb_key_2");
    EXPECT_EQ(state2.last_value, "val_2");

    // Verify that updating a specific config triggers its specific callback
    ASSERT_EQ(aweconfig_set(config_m, "cb_key_1", "val_1_updated"), AWECFG_RC_OK);

    EXPECT_EQ(state1.call_count, 2);
    EXPECT_EQ(state1.last_value, "val_1_updated");

    // Ensure state2 was NOT affected by state1's update
    EXPECT_EQ(state2.call_count, 1);
    EXPECT_EQ(state2.last_value, "val_2");
}

/**
```yaml
- id: utest~AWEMGR.AWECONFIG.remove_callback~1
  covers:
    - dsn~AWEMGR.AWECONFIG.Callback~1
  description: |
    Checks that removing callbacks works as expected and that removed callbacks
    are no longer triggered on updates, while other callbacks remain functional.
```
*/
TEST_F(AweConfigTestFixture, RemoveCallback) {
    CallbackState state1;
    CallbackState state2;

    ASSERT_EQ(aweconfig_add(config_m, "key-1", "any value", "default-val-1"), AWECFG_RC_OK);
    ASSERT_EQ(aweconfig_add(config_m, "key-2", "another value", "default-val-1"), AWECFG_RC_OK);

    // Bind listeners and verify it fires immediately with the current state
    ASSERT_EQ(aweconfig_add_listener(config_m, "key-1", TestCallback, &state1), AWECFG_RC_OK);
    ASSERT_EQ(aweconfig_add_listener(config_m, "key-2", TestCallback, &state2), AWECFG_RC_OK);
    EXPECT_EQ(state1.call_count, 1);
    EXPECT_EQ(state2.call_count, 1);

    // remove the first item
    ASSERT_EQ(aweconfig_remove_listener(config_m, "key-1"), AWECFG_RC_OK);

    // update both items and verify that only the second callback is triggered
    ASSERT_EQ(aweconfig_set(config_m, "key-1", "my-updated-val1"), AWECFG_RC_OK);
    ASSERT_EQ(aweconfig_set(config_m, "key-2", "my-updated-val2"), AWECFG_RC_OK);

    EXPECT_EQ(state1.call_count, 1);  // Should not have changed
    EXPECT_EQ(state2.call_count, 2);  // Should have been incremented

}

