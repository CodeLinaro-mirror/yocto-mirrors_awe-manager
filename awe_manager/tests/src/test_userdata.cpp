#include "test_fixtures.h"

/**
```yaml
- id: itest~AWEMGR.AWC.UserData~1
  covers: req~AWEMGR.UserData~1
  description: |
    Ensures the user data can be accessed by index
```
*/
TEST_F(AweMgrTestUserData, UserDataByIndex)
{
  awemgr_userdata data;
  memset(&data, 0, sizeof(awemgr_userdata));
  ASSERT_EQ(awemgr_get_control_userdata_count(m_ctx, "Scaler1.gain"), 1);
  ASSERT_EQ(awemgr_get_control_userdata_by_index(m_ctx, "Scaler1.gain", 0, &data), awemgr_RC_OK);
  ASSERT_STREQ(data.key, "helptext");
  ASSERT_EQ(data.type, AWEMGR_USRDATA_STR);
  ASSERT_STREQ(awemgr_userdata_type_to_string(data.type), "STRING");
  ASSERT_STREQ(data.value.str, "controls the mastergain");

  memset(&data, 0, sizeof(awemgr_userdata));

  ASSERT_EQ(awemgr_get_control_userdata_count(m_ctx, "Scaler1.smoothingTime"), 1);
  ASSERT_EQ(awemgr_get_control_userdata_by_index(m_ctx, "Scaler1.smoothingTime", 0, &data), awemgr_RC_OK);
  ASSERT_STREQ(data.key, "vm_mask");
  ASSERT_EQ(data.type, AWEMGR_USRDATA_UINT);
  ASSERT_STREQ(awemgr_userdata_type_to_string(data.type), "UNSIGNED_INTEGER");
  ASSERT_EQ(data.value.u32, 7);

  memset(&data, 0, sizeof(awemgr_userdata));

  ASSERT_EQ(awemgr_get_control_userdata_count(m_ctx, "Scaler1.isDB"), 4);
  ASSERT_EQ(awemgr_get_control_userdata_by_index(m_ctx, "Scaler1.isDB", 0, &data), awemgr_RC_OK);
  ASSERT_STREQ(data.key, "floatData");
  ASSERT_EQ(data.type, AWEMGR_USRDATA_FLOAT);
  ASSERT_STREQ(awemgr_userdata_type_to_string(data.type), "FLOAT");
  ASSERT_EQ(data.value.f32, 9.0f);

  memset(&data, 0, sizeof(awemgr_userdata));

  ASSERT_EQ(awemgr_get_module_userdata_count(m_ctx, "Scaler1"), 2);
  ASSERT_EQ(awemgr_get_module_userdata_by_index(m_ctx, "Scaler1", 0, &data), awemgr_RC_OK);
  ASSERT_STREQ(data.key, "vm_mask");
  ASSERT_EQ(data.type, AWEMGR_USRDATA_UINT);
  ASSERT_STREQ(awemgr_userdata_type_to_string(data.type), "UNSIGNED_INTEGER");
  ASSERT_EQ(data.value.u32, 15);

  // also checking INT case missed so far
  ASSERT_STREQ(awemgr_userdata_type_to_string(AWEMGR_USRDATA_INT), "INTEGER");

  // Error cases
  ASSERT_EQ(awemgr_get_control_userdata_count(NULL, NULL), awemgr_RC_ERR);
  ASSERT_EQ(awemgr_get_control_userdata_count(NULL, "Scaler1.gain"), awemgr_RC_ERR);
  ASSERT_EQ(awemgr_get_control_userdata_count(m_ctx, NULL), awemgr_RC_ERR);
  ASSERT_EQ(awemgr_get_control_userdata_count(m_ctx, "Scaler1.wrongname"), awemgr_RC_ERR);

  ASSERT_EQ(awemgr_get_control_userdata_by_index(m_ctx, "Scaler1.smoothingTime", 2344, &data), awemgr_RC_ERR); // Invalid index
  ASSERT_EQ(awemgr_get_control_userdata_by_index(m_ctx, "Scaler1.erwerw", 0, &data), awemgr_RC_ERR);           // Invalid control name
  ASSERT_EQ(awemgr_get_control_userdata_by_index(m_ctx, "Scaler1.smoothingTime", 0, NULL), awemgr_RC_ERR);
  ASSERT_EQ(awemgr_get_control_userdata_by_index(NULL, "Scaler1.smoothingTime", 0, &data), awemgr_RC_ERR);
  ASSERT_EQ(awemgr_get_control_userdata_by_index(m_ctx, NULL, 0, &data), awemgr_RC_ERR);
  ASSERT_EQ(awemgr_get_control_userdata_by_index(NULL, NULL, 0, NULL), awemgr_RC_ERR);

  ASSERT_EQ(awemgr_get_module_userdata_count(NULL, NULL), awemgr_RC_ERR);
  ASSERT_EQ(awemgr_get_module_userdata_count(NULL, "Scaler1"), awemgr_RC_ERR);
  ASSERT_EQ(awemgr_get_module_userdata_count(m_ctx, NULL), awemgr_RC_ERR);
  ASSERT_EQ(awemgr_get_module_userdata_count(m_ctx, "invalidName"), awemgr_RC_ERR);

  ASSERT_EQ(awemgr_get_module_userdata_by_index(m_ctx, "Scaler1", 3432, &data), awemgr_RC_ERR);
  ASSERT_EQ(awemgr_get_module_userdata_by_index(m_ctx, "Scaler1234", 0, &data), awemgr_RC_ERR);
  ASSERT_EQ(awemgr_get_module_userdata_by_index(NULL, "Scaler1", 0, &data), awemgr_RC_ERR);
  ASSERT_EQ(awemgr_get_module_userdata_by_index(m_ctx, NULL, 0, NULL), awemgr_RC_ERR);
  ASSERT_EQ(awemgr_get_module_userdata_by_index(NULL, NULL, -1, NULL), awemgr_RC_ERR);

  ASSERT_STREQ(awemgr_userdata_type_to_string(AWEMGR_USRDATA_TYPE_MAX), "UNDEFINED");
}

/**
```yaml
- id: itest~AWEMGR.UserDataNamedAccess~1
  covers: req~AWEMGR.UserData~1
  description: |
    Ensures the user data can be accessed by key
```
*/
TEST_F(AweMgrTestUserData, UserDataNamedAccess)
{
  awemgr_userdata data;
  memset(&data, 0, sizeof(awemgr_userdata));

  ASSERT_EQ(awemgr_get_module_userdata_by_key(m_ctx, "Scaler1", "vm_mask", &data), awemgr_RC_OK);
  ASSERT_STREQ(data.key, "vm_mask");
  ASSERT_EQ(data.type, AWEMGR_USRDATA_UINT);
  ASSERT_EQ(data.value.u32, 15);

  memset(&data, 0, sizeof(awemgr_userdata));
  ASSERT_EQ(awemgr_get_control_userdata_by_key(m_ctx, "Scaler1.gain", "helptext", &data), awemgr_RC_OK);
  ASSERT_STREQ(data.key, "helptext");
  ASSERT_EQ(data.type, AWEMGR_USRDATA_STR);
  ASSERT_STREQ(data.value.str, "controls the mastergain");

  memset(&data, 0, sizeof(awemgr_userdata));
  ASSERT_EQ(awemgr_get_control_userdata_by_key(m_ctx, "Scaler1.smoothingTime", "vm_mask", &data), awemgr_RC_OK);
  ASSERT_STREQ(data.key, "vm_mask");
  ASSERT_EQ(data.type, AWEMGR_USRDATA_UINT);
  ASSERT_EQ(data.value.u32, 7);

  // Error cases
  ASSERT_EQ(awemgr_get_control_userdata_by_key(m_ctx, "Scaler1.gain", "wrongkey", &data), awemgr_RC_ERR);
  ASSERT_EQ(awemgr_get_control_userdata_by_key(m_ctx, "Scaler1.wrongname", "helptext", &data), awemgr_RC_ERR);
  ASSERT_EQ(awemgr_get_control_userdata_by_key(m_ctx, NULL, NULL, &data), awemgr_RC_ERR);
  ASSERT_EQ(awemgr_get_control_userdata_by_key(NULL, "Scaler1.gain", "helptext", NULL), awemgr_RC_ERR);
  ASSERT_EQ(awemgr_get_control_userdata_by_key(NULL, NULL, NULL, NULL), awemgr_RC_ERR);

  ASSERT_EQ(awemgr_get_module_userdata_by_key(m_ctx, "Scaler1", "wrongkey", &data), awemgr_RC_ERR);
  ASSERT_EQ(awemgr_get_module_userdata_by_key(m_ctx, "wrongname", "vm_mask", &data), awemgr_RC_ERR);
  ASSERT_EQ(awemgr_get_module_userdata_by_key(m_ctx, NULL, NULL, &data), awemgr_RC_ERR);
  ASSERT_EQ(awemgr_get_module_userdata_by_key(NULL, "Scaler1", "vm_mask", NULL), awemgr_RC_ERR);
  ASSERT_EQ(awemgr_get_module_userdata_by_key(NULL, NULL, NULL, NULL), awemgr_RC_ERR);
}
