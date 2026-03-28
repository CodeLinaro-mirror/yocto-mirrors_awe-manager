#include "test_fixtures.h"

static const char *awc_file = TEST_DATA_DIR "/designs/subcanvas/target_files/awc_index.txt";

/**
```yaml
- id: itest~AWEMGR.Subcanvas~1
  covers:
    - req~AWEMGR.SubcanvasLoadUnloadDesign~1
    - req~AWEMGR.SubcanvasAccess~1
  description: |
    This test shows:
      - A multi-instance design with 1 subcanvas per core can be loaded.
      - A design can be loaded to a subcanvas using tunneling mechanism
      - Controls from the subcanvas can be accessed (Set/Get) via the main canvas
      - Designs can be unloaded
```
*/
TEST_F(AweMgrTestFixture, Subcanvas)
{
    uint32_t sink1[8];
    load_awc(awc_file);
    m_ctx = awemgr_get_awc_context(m_mgr_p, 0);
    ASSERT_TRUE(m_ctx != NULL);

    ASSERT_EQ(awemgr_load_design(m_ctx, "Main"), awemgr_RC_OK);
    delay_ms(10);

    // Before the subcanvases designs are loaded the sink1 should report Samples = 0
    ASSERT_EQ(readArray("Sink1.value", sink1, 8, 0), true);
    for (uint32_t i = 0; i < 8; i++)
    {
        ASSERT_EQ(sink1[i], 0);
    }

    ASSERT_EQ(awemgr_load_design(m_ctx, "Multiplier0"), awemgr_RC_OK);
    ASSERT_EQ(awemgr_load_design(m_ctx, "Multiplier1"), awemgr_RC_OK);
    ASSERT_EQ(awemgr_load_design(m_ctx, "Multiplier2"), awemgr_RC_OK);
    ASSERT_EQ(awemgr_load_design(m_ctx, "Multiplier3"), awemgr_RC_OK);
 
    delay_ms(10);
    // After the subcanvases designs are loaded the sink1 should report Samples != 0
    ASSERT_EQ(readArray("Sink1.value", sink1, 8, 0), true);
    for (uint32_t i = 0; i < 8; i++)
    {
        ASSERT_NE(sink1[i], 0);
    }

    // Try to read a parameter in a subcanvas (1 to 4), change its value and read it again.
    ASSERT_EQ(getValue<uint32_t>("Subcanvas_Inst0/DC.value"), 666);
    ASSERT_EQ(setValue<uint32_t>("Subcanvas_Inst0/DC.value", 777), true);
    ASSERT_EQ(getValue<uint32_t>("Subcanvas_Inst0/DC.value"), 777);

    ASSERT_EQ(getValue<uint32_t>("Subcanvas_Inst1/DC.value"), 666);
    ASSERT_EQ(setValue<uint32_t>("Subcanvas_Inst1/DC.value", 777), true);
    ASSERT_EQ(getValue<uint32_t>("Subcanvas_Inst1/DC.value"), 777);

    ASSERT_EQ(getValue<uint32_t>("Subcanvas_Inst2/DC.value"), 666);
    ASSERT_EQ(setValue<uint32_t>("Subcanvas_Inst2/DC.value", 777), true);
    ASSERT_EQ(getValue<uint32_t>("Subcanvas_Inst2/DC.value"), 777);

    ASSERT_EQ(getValue<uint32_t>("Subcanvas_Inst3/DC.value"), 666);
    ASSERT_EQ(setValue<uint32_t>("Subcanvas_Inst3/DC.value", 777), true);
    ASSERT_EQ(getValue<uint32_t>("Subcanvas_Inst3/DC.value"), 777);

    awemgr_module_runtimestate state;
    awemgr_module mod;

    const char* modulePaths[8] = {
      "Subcanvas_Inst0/DC",
      "Subcanvas_Inst1/DC",
      "Subcanvas_Inst2/DC",
      "Subcanvas_Inst3/DC",
      "Subcanvas_Inst0/SC",
      "Subcanvas_Inst1/SC",
      "Subcanvas_Inst2/SC",
      "Subcanvas_Inst3/SC"
    };

    for(int i = 0 ; i < 8; i++)
    {
      awemgr_module_runtimestate state;
      awemgr_module mod;
      ASSERT_EQ(awemgr_get_module_by_name(m_ctx, modulePaths[i], &mod), awemgr_RC_OK);
      ASSERT_EQ(awemgr_module_get_state(m_ctx, mod, &state), awemgr_RC_OK);
      ASSERT_EQ(state, MODULE_ACTIVE);
      ASSERT_EQ(awemgr_module_set_state(m_ctx, mod, MODULE_BYPASS), awemgr_RC_OK);
      ASSERT_EQ(awemgr_module_get_state(m_ctx, mod, &state), awemgr_RC_OK);
      ASSERT_EQ(state, MODULE_BYPASS);
      ASSERT_EQ(awemgr_module_set_state(m_ctx, mod, MODULE_ACTIVE), awemgr_RC_OK);
      ASSERT_EQ(awemgr_module_get_state(m_ctx, mod, &state), awemgr_RC_OK);
      ASSERT_EQ(state, MODULE_ACTIVE);
    }

    ASSERT_EQ(awemgr_unload_design(m_ctx, "Multiplier0"), awemgr_RC_OK);
    ASSERT_EQ(awemgr_unload_design(m_ctx, "Multiplier1"), awemgr_RC_OK);
    ASSERT_EQ(awemgr_unload_design(m_ctx, "Multiplier2"), awemgr_RC_OK);
    ASSERT_EQ(awemgr_unload_design(m_ctx, "Multiplier3"), awemgr_RC_OK);

    // After the subcanvases designs are unloaded the sink1 should report Samples = 0
    delay_ms(10);
    ASSERT_EQ(readArray("Sink1.value", sink1, 8, 0), true);
    for (uint32_t i = 0; i < 8; i++)
    {
        ASSERT_EQ(sink1[i], 0);
    }

    ASSERT_EQ(awemgr_unload_design(m_ctx, "Main"), awemgr_RC_OK);
}

/**
```yaml
- id: itest~AWEMGR.SubCanvasPresets~1
  covers:
    - req~AWEMGR.SubcanvasLoadUnloadDesign~1
    - req~AWEMGR.SubcanvasAccess~1
  description: |
    This test shows a preset can be loaded in a subcanvas
```
*/
TEST_F(AweMgrTestFixture, SubCanvasPresets)
{
    uint32_t sink1[8];
    load_awc(awc_file);
    m_ctx = awemgr_get_awc_context(m_mgr_p, 0);
    ASSERT_TRUE(m_ctx != NULL);

    ASSERT_EQ(awemgr_load_design(m_ctx, "Main"), awemgr_RC_OK);
    delay_ms(10);

    // Before the subcanvases designs are loaded the sink1 should report Samples = 0
    ASSERT_EQ(readArray("Sink1.value", sink1, 8, 0), true);
    for (uint32_t i = 0; i < 8; i++)
    {
        ASSERT_EQ(sink1[i], 0);
    }

    ASSERT_EQ(awemgr_load_design(m_ctx, "Multiplier0"), awemgr_RC_OK);
    ASSERT_EQ(awemgr_load_design(m_ctx, "Multiplier1"), awemgr_RC_OK);
    ASSERT_EQ(awemgr_load_design(m_ctx, "Multiplier2"), awemgr_RC_OK);
    ASSERT_EQ(awemgr_load_design(m_ctx, "Multiplier3"), awemgr_RC_OK);
 
    delay_ms(10);

    ASSERT_EQ(getValue<uint32_t>("Subcanvas_Inst0/DC.value"), 666);
    ASSERT_EQ(getValue<uint32_t>("Subcanvas_Inst0/ShiftBits1.numBits"), 1); 

    ASSERT_EQ(getValue<uint32_t>("Subcanvas_Inst1/DC.value"), 666);
    ASSERT_EQ(getValue<uint32_t>("Subcanvas_Inst1/ShiftBits1.numBits"), 1); 

    ASSERT_EQ(getValue<uint32_t>("Subcanvas_Inst2/DC.value"), 666);
    ASSERT_EQ(getValue<uint32_t>("Subcanvas_Inst2/ShiftBits1.numBits"), 1); 

    ASSERT_EQ(getValue<uint32_t>("Subcanvas_Inst3/DC.value"), 666);
    ASSERT_EQ(getValue<uint32_t>("Subcanvas_Inst3/ShiftBits1.numBits"), 1); 

    ASSERT_EQ(awemgr_load_design(m_ctx, "Multiplier0Preset1"), awemgr_RC_OK);
    ASSERT_EQ(awemgr_load_design(m_ctx, "Multiplier1Preset1"), awemgr_RC_OK);
    ASSERT_EQ(awemgr_load_design(m_ctx, "Multiplier2Preset1"), awemgr_RC_OK);
    ASSERT_EQ(awemgr_load_design(m_ctx, "Multiplier3Preset1"), awemgr_RC_OK);

    ASSERT_EQ(getValue<uint32_t>("Subcanvas_Inst0/DC.value"), 777);
    ASSERT_EQ(getValue<uint32_t>("Subcanvas_Inst0/ShiftBits1.numBits"), 2); 

    ASSERT_EQ(getValue<uint32_t>("Subcanvas_Inst1/DC.value"), 777);
    ASSERT_EQ(getValue<uint32_t>("Subcanvas_Inst1/ShiftBits1.numBits"), 2); 

    ASSERT_EQ(getValue<uint32_t>("Subcanvas_Inst2/DC.value"), 777);
    ASSERT_EQ(getValue<uint32_t>("Subcanvas_Inst2/ShiftBits1.numBits"), 2); 

    ASSERT_EQ(getValue<uint32_t>("Subcanvas_Inst3/DC.value"), 777);
    ASSERT_EQ(getValue<uint32_t>("Subcanvas_Inst3/ShiftBits1.numBits"), 2);
}