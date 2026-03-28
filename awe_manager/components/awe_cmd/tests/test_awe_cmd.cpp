#include <gtest/gtest.h>
#include "awe_cmd.h"

// A custom comparison function
::testing::AssertionResult ArraysEqual(const unsigned int* expected, const unsigned int* actual, int size) {
    for (int i = 0; i < size; ++i) {
        if (expected[i] != actual[i]) {
            return ::testing::AssertionFailure() << "Arrays differ at index " << i
                                                 << ": expected " << expected[i]
                                                 << " but got " << actual[i];
        }
    }
    return ::testing::AssertionSuccess();
}

class AweCmdTestFixture: public testing::Test
{
	  public:
		    void SetUp()
        {
            ASSERT_EQ(awecmd_init(&msg_buffer, 264, false, 264), AWECMD_RC_OK);
        };
		    void TearDown()
        {
            ASSERT_EQ(awecmd_exit(&msg_buffer), AWECMD_RC_OK);
        };

        struct awecmd_st   msg_buffer;
};



class AweCmdSetupFixture: public testing::Test
{
    public:
        void SetUp() { awecmd_init(&mCmdBuffer, 20, false, 10); };
        void TearDown() { awecmd_exit(&mCmdBuffer); };

    public:
        struct awecmd_st mCmdBuffer;
};

/**
```yaml
- id: utest~AWEMGR.Cmd.Init_Exit~1
  covers: dsn~AWEMGR.AWECMD.MemoryHandle~1
  description: Ensures the awe_CMD component can be initialized and torn down.
```
*/
TEST(CmdTest, InitExit) {
	  struct awecmd_st   msg_buffer;
    ASSERT_EQ(awecmd_init(&msg_buffer, 264, false, 264), 0);
    ASSERT_EQ(awecmd_exit(&msg_buffer), 0);
}

extern "C"
{
  UINT32 ComputeCRC(UINT32 *pMessage, UINT32 len);
}
/**
```yaml
- id: utest~AWEMGR.Cmd.InvalidInputs~1
  covers: dsn~AWEMGR.AWECMD.MemoryHandle~1
  description: Ensures the awe_CMD correctly handles Invalid Inputs such as NULL context
```
*/
TEST(CmdTest, InvalidInputs) {
	  struct awecmd_st   msg_buffer;

    // Try Error cases first
    memset(&msg_buffer, 0 , sizeof(awecmd_st));
    ASSERT_EQ(awecmd_init(NULL, 264, false, 264), AWECMD_RC_ERR);
    ASSERT_EQ(awecmd_exit(NULL), AWECMD_RC_ERR);
    ASSERT_EQ(awecmd_reset(NULL, 0), AWECMD_RC_ERR);
    ASSERT_EQ(awecmd_AudioStart(NULL, 0), AWECMD_RC_ERR);
    ASSERT_EQ(awecmd_AudioStop(NULL, 0), AWECMD_RC_ERR);
    ASSERT_EQ(awecmd_Destroy(NULL, 0), AWECMD_RC_ERR);
    ASSERT_EQ(awecmd_from_stream(NULL, NULL, 0, 0, 0), AWECMD_RC_ERR);
    ASSERT_EQ(awecmd_from_stream(&msg_buffer, NULL, 0, 0, 0), AWECMD_RC_ERR);
    ASSERT_EQ(awecmd_getTargetInfo(NULL, 0, 0), AWECMD_RC_ERR);
    ASSERT_EQ(awecmd_setValue(NULL, 0, 0, NULL, 0, 0, AWECMD_FLOAT, 0), AWECMD_RC_ERR);
    ASSERT_EQ(awecmd_getValue(NULL, 0, 0, 0, 0, 0), AWECMD_RC_ERR);
    ASSERT_EQ(awecmd_setModuleStatus(NULL, 0, 0, 0, 0), AWECMD_RC_ERR);
    ASSERT_EQ(awecmd_getModuleStatus(NULL, 0, 0, 0), AWECMD_RC_ERR);
    ASSERT_EQ(awecmd_getModuleClass(NULL, 0, 0, 0), AWECMD_RC_ERR);
    ASSERT_EQ(awecmd_getNrCores(NULL, 0, 0), AWECMD_RC_ERR);
    ASSERT_EQ(awecmd_getExtendedInfo(NULL, 0, 0), AWECMD_RC_ERR);
    ASSERT_EQ(awecmd_BSPCmd(NULL, 0, 0, 0, NULL, 0), AWECMD_RC_ERR);
    ASSERT_EQ(awecmd_getClassCount(NULL, 0, 0), AWECMD_RC_ERR);
    ASSERT_EQ(awecmd_getClassInfo(NULL, 0, 0, 0), AWECMD_RC_ERR);
    ASSERT_EQ(awecmd_getHeapCount(NULL, 0), AWECMD_RC_ERR);
    ASSERT_EQ(awecmd_getHeapSize(NULL, 0), AWECMD_RC_ERR);
    ASSERT_EQ(awecmd_getSharedHeapSize(NULL, 0), AWECMD_RC_ERR);
    ASSERT_EQ(awecmd_getCpuLoad(NULL, 0, 0), AWECMD_RC_ERR);
    ASSERT_EQ(ComputeCRC(NULL, 0), 0);
    uint32_t val = 5;
    ASSERT_EQ(ComputeCRC(&val, 0), 0);
    ASSERT_EQ(awecmd_exit(&msg_buffer), AWECMD_RC_OK);
}

/**
```yaml
- id: utest~AWEMGR.Cmd.CheckBufferSizes~1
  covers: dsn~AWEMGR.AWECMD.MemoryHandle~1
  description: Checks if buffer sizes (in nr of words) can be retrieved from initialized structure.
```
*/
TEST(CmdTest, CheckMacros) {
	  struct awecmd_st   msg_buffer;
    ASSERT_EQ(awecmd_init(&msg_buffer, 264, false, 222), 0);
    ASSERT_EQ(AWECMD_REQUESTBUFFER_SZ((&msg_buffer)), 264);
    ASSERT_EQ(AWECMD_RESPONSEBUFFER_SZ((&msg_buffer)), 222);
    ASSERT_EQ(awecmd_exit(&msg_buffer), 0);
}


/**
```yaml
- id: utest~AWEMGR.Cmd.GetResponse~1
  covers: dsn~AWEMGR.AWECMD.CentralCheckError~1
  description: Checks whether an AWE response can be parsed and extracted from buffer correctly.
```
*/
TEST_F(AweCmdSetupFixture, GetResponse) {
    unsigned int awe_response[10] = {10<<16, 0, 1, 2, 3, 4, 5, 6, 7, 12345};
    memcpy(mCmdBuffer.response_buffer_p, awe_response, 10 * sizeof(unsigned int));

    unsigned int payload[10];
    unsigned int copied;
    ASSERT_EQ(awecmd_response_getData(&mCmdBuffer, payload, 10, &copied), 0);

    ASSERT_EQ(payload[0], 1);
    ASSERT_EQ(payload[6], 7);
    ASSERT_EQ(copied, 7);
}


/**
```yaml
- id: utest~AWEMGR.Cmd.GetResponseFailBufSize~1
  covers: dsn~AWEMGR.AWECMD.CentralCheckError~1
  description: |
    Checks that too small output buffer is handled correctly and only that amount is copied out
    which fits into the output buffer.
```
*/
TEST_F(AweCmdSetupFixture, GetResponseFailBufSize) {
    unsigned int awe_response[10] = {10<<16, 0, 1, 2, 3, 4, 5, 6, 7, 12345};
    memcpy(mCmdBuffer.response_buffer_p, awe_response, 10 * sizeof(unsigned int));

    unsigned int payload[5];  // too small size!
    unsigned int copied;
    ASSERT_EQ(awecmd_response_getData(&mCmdBuffer, payload, 5, &copied), AWECMD_RC_OK);

    ASSERT_EQ(payload[0], 1);
    ASSERT_EQ(payload[4], 5);
    ASSERT_EQ(copied, 5);

    // "re-send" again
    memcpy(mCmdBuffer.response_buffer_p, awe_response, 10 * sizeof(unsigned int));
    unsigned int scalar_variable;
    ASSERT_EQ(awecmd_response_getData(&mCmdBuffer, &scalar_variable, 1, &copied), AWECMD_RC_OK);

    ASSERT_EQ(scalar_variable, 1);
    ASSERT_EQ(copied, 1);
}


/**
```yaml
- id: utest~AWEMGR.Cmd.GetResponseFailErrorCode~1
  covers: dsn~AWEMGR.AWECMD.CentralCheckError~1
  description: |
    Checks that a buffer with AWE error is handled.
```
*/
TEST_F(AweCmdSetupFixture, GetResponseFailErrorCode) {
    unsigned int awe_response[10] = {10<<16, 0xffffffff, 1, 2, 3, 4, 5, 6, 7, 12345};  // (<0) indicates error
    memcpy(mCmdBuffer.response_buffer_p, awe_response, 10 * sizeof(unsigned int));

    unsigned int payload[10];
    unsigned int copied;
    ASSERT_EQ(awecmd_response_getData(&mCmdBuffer, payload, sizeof(payload), &copied), AWECMD_RC_AWE_ERR);

    ASSERT_EQ(copied, 0);
}

/**
```yaml
- id: utest~AWEMGR.Cmd.TargetInfo~1
  covers: dsn~AWEMGR.AWECMD.TargetInfo~1
  description: Checks a correct cmd buffer for getting target info.
```
*/
TEST_F(AweCmdTestFixture, TargetInfo)
{
    ASSERT_EQ(awecmd_getTargetInfo(&msg_buffer, 0, 0), AWECMD_RC_OK);
    ASSERT_EQ(msg_buffer.words_written, 2);
    ASSERT_EQ(msg_buffer.buffer_p[0], 0x00020029);
    ASSERT_EQ(msg_buffer.buffer_p[1], 0x00020029); // crc is the same!

    // awecmd_reset(&msg_buffer); // not needed actually as current_p member of awecmd_st is not updated anyhow ;-)
    ASSERT_EQ(awecmd_getTargetInfo(&msg_buffer, 2, 0), AWECMD_RC_OK);
    ASSERT_EQ(msg_buffer.words_written, 2);
    ASSERT_EQ(msg_buffer.buffer_p[0], 0x00022029);
    ASSERT_EQ(msg_buffer.buffer_p[1], 0x00022029);
}

/**
```yaml
- id: utest~AWEMGR.Cmd.GetNrCores2~1
  covers: dsn~AWEMGR.AWECMD.TargetInfo_Extended~1
  description: |
    Checks that a command to get the number of cores is created.
```
*/
TEST_F(AweCmdTestFixture, GetNrCores2) {
    ASSERT_EQ(awecmd_getNrCores(&msg_buffer, 0, 0), AWECMD_RC_OK);
    ASSERT_EQ(msg_buffer.words_written, 2);
    ASSERT_EQ(msg_buffer.buffer_p[0], 0x0002007F);
    ASSERT_EQ(msg_buffer.buffer_p[1], 0x0002007F); // crc is the same!
}

/**
```yaml
- id: utest~AWEMGR.Cmd.GetExtInfo~1
  covers: dsn~AWEMGR.AWECMD.TargetInfo_Extended~1
  description: |
    Checks that a command to get extended an info structure.
```
*/
TEST_F(AweCmdTestFixture, GetExtInfo) {
    ASSERT_EQ(awecmd_getExtendedInfo(&msg_buffer, 0, 0), AWECMD_RC_OK);
    ASSERT_EQ(msg_buffer.words_written, 2);
    ASSERT_EQ(msg_buffer.buffer_p[0], 0x0002007E);
    ASSERT_EQ(msg_buffer.buffer_p[1], 0x0002007E); // crc is the same!
}

/**
```yaml
- id: utest~AWEMGR.Cmd.Destroy~1
  covers: dsn~AWEMGR.AWECMD.Destroy~1
  description: Checks that a correct destroy command is created.
```
*/
TEST_F(AweCmdTestFixture, Destroy)
{
    ASSERT_EQ(awecmd_Destroy(&msg_buffer, 0), AWECMD_RC_OK);
    ASSERT_EQ(msg_buffer.words_written, 2);
    ASSERT_EQ(msg_buffer.buffer_p[0], 0x0002000C);
    ASSERT_EQ(msg_buffer.buffer_p[1], 0x0002000C); // crc is the same!

    // awecmd_reset(&msg_buffer); // not needed actually as current_p member of awecmd_st is not updated anyhow ;-)
    ASSERT_EQ(awecmd_Destroy(&msg_buffer, 2), AWECMD_RC_OK);
    ASSERT_EQ(msg_buffer.words_written, 2);
    ASSERT_EQ(msg_buffer.buffer_p[0], 0x0002200C);
    ASSERT_EQ(msg_buffer.buffer_p[1], 0x0002200C);
}

/**
```yaml
- id: utest~AWEMGR.Cmd.ModuleClass~1
  covers: dsn~AWEMGR.AWECMD.ModuleClass~1
  description: Checks that correct commands are created to obtaining the module class information.
```
*/
TEST_F(AweCmdTestFixture, ModuleClass)
{
    ASSERT_EQ(awecmd_getModuleClass(&msg_buffer, 0, 0, 0x1234), AWECMD_RC_OK);
    ASSERT_EQ(msg_buffer.words_written, 3);
    unsigned int expected[3] = {0x0003002E, 0x1234, 0x0003121A};
    EXPECT_TRUE(ArraysEqual(expected, msg_buffer.buffer_p, 3));

    // awecmd_reset(&msg_buffer); // not needed actually as current_p member of awecmd_st is not updated anyhow ;-)
    ASSERT_EQ(awecmd_getModuleClass(&msg_buffer, 2, 0, 0x1234), AWECMD_RC_OK);
    ASSERT_EQ(msg_buffer.words_written, 3);
    unsigned int expected2[3] = {0x0003202E, 0x1234, 0x0003321A};
    EXPECT_TRUE(ArraysEqual(expected2, msg_buffer.buffer_p, 3));
}

/**
```yaml
- id: utest~AWEMGR.Cmd.ModuleOperationState~1
  covers: dsn~AWEMGR.AWECMD.ModuleOperationState~1
  description: Checks that correct commands for getting the module state are generated.
```
*/
TEST_F(AweCmdTestFixture, ModuleOperationState)
{
    ASSERT_EQ(awecmd_getModuleStatus(&msg_buffer, 0, 0, 0x1234), AWECMD_RC_OK);
    ASSERT_EQ(msg_buffer.words_written, 3);
    unsigned int expected[3] = {0x00030014, 0x1234, 0x00031220};
    EXPECT_TRUE(ArraysEqual(expected, msg_buffer.buffer_p, 3));

    // awecmd_reset(&msg_buffer); // not needed actually as current_p member of awecmd_st is not updated anyhow ;-)
    ASSERT_EQ(awecmd_getModuleStatus(&msg_buffer, 2, 0, 0x1234), AWECMD_RC_OK);
    ASSERT_EQ(msg_buffer.words_written, 3);
    unsigned int expected2[3] = {0x00032014, 0x1234, 0x00033220};
    EXPECT_TRUE(ArraysEqual(expected2, msg_buffer.buffer_p, 3));
}

/**
```yaml
- id: utest~AWEMGR.Cmd.ResponseTwiceError~1
  covers: dsn~AWEMGR.AWECMD.CentralCheckError~1
  description: Checks that it is not possible to call the response buffer parsing routine twice.
    Also checks that buffer size is smaller than 3.
```
*/
TEST_F(AweCmdTestFixture, ResponseTwiceError)
{
    unsigned int response[2];
    unsigned int nr_words_obtained;
    unsigned int crc_dummy = 0xaffe;

    unsigned int ref_2[4] = {4 << 16, 0, 0x1234, crc_dummy};
    memcpy (msg_buffer.response_buffer_p, ref_2, sizeof(ref_2));

    ASSERT_EQ(awecmd_response_getData(&msg_buffer, response, 2, &nr_words_obtained), AWECMD_RC_OK);
    ASSERT_EQ(nr_words_obtained, 1);
    ASSERT_EQ(response[0], 0x1234);

    ASSERT_EQ(awecmd_response_getData(&msg_buffer, NULL, 0, &nr_words_obtained), AWECMD_RC_ERR);
    ASSERT_EQ(nr_words_obtained, 0);
}


/**
```yaml
- id: utest~AWEMGR.Cmd.ResponseHandling~2
  covers: dsn~AWEMGR.AWECMD.CentralCheckError~1
  description: Checks the central response handling method,
    tries various cases to retrieve data without causing an error response
```
*/
TEST_F(AweCmdTestFixture, ResponseHandling)
{
    unsigned int response[2];
    unsigned int nr_words_obtained;
    unsigned int crc_dummy = 0xaffe;

    // check 0 length buffer
    unsigned int ref_1[3] = {3 << 16, 0, crc_dummy};  // 3 is nr of words of received buffer; includes status + crc
    memcpy (msg_buffer.response_buffer_p, ref_1, sizeof(ref_1));

    ASSERT_EQ(awecmd_response_getData(&msg_buffer, response, 2, &nr_words_obtained), AWECMD_RC_OK);
    ASSERT_EQ(nr_words_obtained, 0);

    // check 1 word length ("normal")
    unsigned int ref_2[4] = {4 << 16, 0, 0x1234, crc_dummy};
    memcpy (msg_buffer.response_buffer_p, ref_2, sizeof(ref_2));

    ASSERT_EQ(awecmd_response_getData(&msg_buffer, response, 2, &nr_words_obtained), AWECMD_RC_OK);
    ASSERT_EQ(nr_words_obtained, 1);
    ASSERT_EQ(response[0], 0x1234);


    memcpy (msg_buffer.response_buffer_p, ref_2, sizeof(ref_2));  // "re-send" again
    // check 1 word copy out into NULL, should not be an error, but return 0 copied data
    ASSERT_EQ(awecmd_response_getData(&msg_buffer, NULL, 0, &nr_words_obtained), AWECMD_RC_OK);
    ASSERT_EQ(nr_words_obtained, 0);

    // check buffer too long; it should only copy out the size of response buffer
    unsigned int ref_3[6] = {6 << 16, 0, 0x1234, 0x5678, 0x9ABC, crc_dummy};
    memcpy (msg_buffer.response_buffer_p, ref_3, sizeof(ref_3));
    ASSERT_EQ(sizeof(ref_3), 24);

    ASSERT_EQ(awecmd_response_getData(&msg_buffer, response, 2, &nr_words_obtained), AWECMD_RC_OK);
    ASSERT_EQ(nr_words_obtained, 2);
    ASSERT_EQ(response[0], 0x1234);
    ASSERT_EQ(response[1], 0x5678);
}


/**
```yaml
- id: utest~AWEMGR.Cmd.ResponseHandlingCpuCores~1
  covers: dsn~AWEMGR.AWECMD.CentralCheckError~1
  description: Checks reception of PFID_GetCores2 responses
```
*/
TEST_F(AweCmdTestFixture, ResponseHandling_CpuCores)
{
    unsigned int response[3];
    unsigned int nr_words_obtained;
    unsigned int crc_dummy = 0xaffe;

    msg_buffer.buffer_p[0] = 127;  // "simulate" that we sent this command; 127 = PFID_GetCores2

    // 1 CPU core; instance = 0x1234
    unsigned int ref_1[4] = {4 << 16, 1, 0x1234, crc_dummy};
    memcpy (msg_buffer.response_buffer_p, ref_1, sizeof(ref_1));

    ASSERT_EQ(awecmd_response_getData(&msg_buffer, response, 3, &nr_words_obtained), AWECMD_RC_OK);
    ASSERT_EQ(nr_words_obtained, 2);

    ASSERT_EQ(response[0], 1);
    ASSERT_EQ(response[1], 0x1234);

    // 2 CPU cores; instances = [0x1234, 0x5678]
    unsigned int ref_2[5] = {5 << 16, 2, 0x1234, 0x5678, crc_dummy};
    memcpy (msg_buffer.response_buffer_p, ref_2, sizeof(ref_2));

    ASSERT_EQ(awecmd_response_getData(&msg_buffer, response, 3, &nr_words_obtained), AWECMD_RC_OK);
    ASSERT_EQ(nr_words_obtained, 3);

    ASSERT_EQ(response[0], 2);
    ASSERT_EQ(response[1], 0x1234);
    ASSERT_EQ(response[2], 0x5678);
}


/**
```yaml
- id: utest~AWEMGR.Cmd.SetCmds~1
  covers: dsn~AWEMGR.AWECMD.SetGetValueOrValues~1
  description: Checks the tune commands for setting values are correctly formatted.
```
*/
TEST_F(AweCmdTestFixture, SetCmds)
{
    unsigned int data[3] = {0x3145, 0x57721, 0x271828}; // easter egg alert! you know these? ;-)

    // set scalar value (encoded into handle: 0xdead)
    ASSERT_EQ(awecmd_setValue(&msg_buffer, 0, 0xdead, data, 0, 3, AWECMD_INTEGER, 1U), AWECMD_RC_OK);
    ASSERT_EQ(msg_buffer.words_written, 6);
    unsigned int expected[6] = {0x00060D3C, 0xD02D, 0x3145, 0x80000000, 0x0, 0x8006EC54};
    EXPECT_TRUE(ArraysEqual(expected, msg_buffer.buffer_p, 6));

    ASSERT_EQ(awecmd_setValue(&msg_buffer, 0, 0xde10, data, 0, 3, AWECMD_INTEGER, 1U), AWECMD_RC_OK);
    ASSERT_EQ(msg_buffer.words_written, 6);
    unsigned int expected2[6] = {0x00060C3C, 0xd010, 0x3145, 0x00010000, 0x0, 0x7ED69};
    EXPECT_TRUE(ArraysEqual(expected2, msg_buffer.buffer_p, 6));

    // set array value
    ASSERT_EQ(awecmd_setValue(&msg_buffer, 0, 0x8000dead, data, 0, 3, AWECMD_INTEGER, 1U), AWECMD_RC_OK);
    ASSERT_EQ(msg_buffer.words_written, 9);
    unsigned int expected3[9] = {0x00090D3D, 0xD02D, 0, 0x80000000, 3, 0x3145, 0x57721, 0x271828, 0x802B835F };
    EXPECT_TRUE(ArraysEqual(expected3, msg_buffer.buffer_p, 9));

    // same, but without mask
    ASSERT_EQ(awecmd_setValue(&msg_buffer, 0, 0x8000dead, data, 0, 3, AWECMD_INTEGER, 1U), AWECMD_RC_OK);
    ASSERT_EQ(msg_buffer.words_written, 9);
    unsigned int expected4[9] = {0x00090D3D, 0xD02D, 0, 0x80000000, 3, 0x3145, 0x57721, 0x271828, 0x802B835F};
    EXPECT_TRUE(ArraysEqual(expected4, msg_buffer.buffer_p, 9));

}


/**
```yaml
- id: utest~AWEMGR.Cmd.GetCmds~1
  covers: dsn~AWEMGR.AWECMD.SetGetValueOrValues~1
  description: Checks the tune commands for getting values are correctly formatted.
```
*/
TEST_F(AweCmdTestFixture, GetCmds)
{
    // set scalar value (encoded into handle: 0xdead)
    ASSERT_EQ(awecmd_getValue(&msg_buffer, 0, 0xdead, 0, 3, 1U), AWECMD_RC_OK);
    ASSERT_EQ(msg_buffer.words_written, 4);
    unsigned int expected[4] = {0x40D08, 0xD02D, 0, 0x4DD25};
    EXPECT_TRUE(ArraysEqual(expected, msg_buffer.buffer_p, 4));

    // set array value
    ASSERT_EQ(awecmd_getValue(&msg_buffer, 0, 0x8000dead, 0, 3, 1U), AWECMD_RC_OK);
    ASSERT_EQ(msg_buffer.words_written, 6);
    unsigned int expected2[6] = {0x00060D3F, 0xD02D, 0, 0x80000000, 3, 0x8006DD11 };
    EXPECT_TRUE(ArraysEqual(expected2, msg_buffer.buffer_p, 6));
}

/**
```yaml
- id: utest~AWEMGR.Cmd.Classes~1
  covers: dsn~AWEMGR.AWECMD.ModuleClass~1
  description: Checks if (installed) module information can be retrieved.
```
*/
TEST_F(AweCmdTestFixture, GetClasses)
{
    unsigned int response[2];
    unsigned int nr_words_obtained;
    unsigned int crc_dummy = 0xaffe;

    // get number of classes
    ASSERT_EQ(awecmd_getClassCount(&msg_buffer, 0, 0), AWECMD_RC_OK);
    ASSERT_EQ(msg_buffer.words_written, 2);
    unsigned int expected[3] = {0x0002000D, 0x0002000D};
    EXPECT_TRUE(ArraysEqual(expected, msg_buffer.buffer_p, 2));

    // check result
    // 0x00030000, 0x00000214, 0x00030214
    unsigned int ref_nrclasses[3] = {0x00030000, 0x00000214, 0x00030214};
    memcpy (msg_buffer.response_buffer_p, ref_nrclasses, sizeof(ref_nrclasses));

    ASSERT_EQ(awecmd_response_getData(&msg_buffer, response, 3, &nr_words_obtained), AWECMD_RC_OK);
    ASSERT_EQ(nr_words_obtained, 1);
    ASSERT_EQ(response[0], 0x214);

    // get a specific class
    ASSERT_EQ(awecmd_getClassInfo(&msg_buffer, 0, 0, 0x11), AWECMD_RC_OK);
    ASSERT_EQ(msg_buffer.words_written, 3);
    unsigned int expected2[3] = {0x0003000E, 0x11, 0x0003001F };
    EXPECT_TRUE(ArraysEqual(expected2, msg_buffer.buffer_p, 3));

    // check result
    // 0x00050000, 0xbeef0802, 0xbeef0802, 0x00000011, 0x00050011
    unsigned int ref_classinfo[5] = {0x00050000, 0xbeef0802, 0xbeef0802, 0x00000011, 0x00050011};
    memcpy (msg_buffer.response_buffer_p, ref_classinfo, sizeof(ref_classinfo));

    ASSERT_EQ(awecmd_response_getData(&msg_buffer, response, 5, &nr_words_obtained), AWECMD_RC_OK);
    ASSERT_EQ(nr_words_obtained, 2);
    ASSERT_EQ(response[0], 0xbeef0802);
    ASSERT_EQ(response[1], 0x00000011);

}

/**
```yaml
- id: utest~AWEMGR.Cmd.MemInfo~1
  covers: dsn~AWEMGR.AWECMD.MemInfo~1
  description: Checks commands to retrieve (heap) memory size information.
```
*/
TEST_F(AweCmdTestFixture, GetMemories)
{
    unsigned int response[10];
    unsigned int nr_words_obtained;
    unsigned int crc_dummy = 0xaffe;

    // get number of classes
    ASSERT_EQ(awecmd_getHeapCount(&msg_buffer, 0), AWECMD_RC_OK);
    ASSERT_EQ(msg_buffer.words_written, 2);
    unsigned int expected[3] = {0x0002000A, 0x0002000A};
    EXPECT_TRUE(ArraysEqual(expected, msg_buffer.buffer_p, 2));

    // check result
    // 4 heaps
    unsigned int ref_nrheaps[3] = {0x00030000, 0x00000004, 0x00030004};
    memcpy (msg_buffer.response_buffer_p, ref_nrheaps, sizeof(ref_nrheaps));

    ASSERT_EQ(awecmd_response_getData(&msg_buffer, response, 3, &nr_words_obtained), AWECMD_RC_OK);
    ASSERT_EQ(nr_words_obtained, 1);
    ASSERT_EQ(response[0], 4);

    // get heap sizes
    ASSERT_EQ(awecmd_getHeapSize(&msg_buffer, 0), AWECMD_RC_OK);
    ASSERT_EQ(msg_buffer.words_written, 2);
    unsigned int expected_heapsize[3] = {0x0002000B, 0x0002000B};
    EXPECT_TRUE(ArraysEqual(expected_heapsize, msg_buffer.buffer_p, 2));

    // check result
    unsigned int ref_heaps[9] = {0x00090000, 0x00000000, 0x0065b9a9, 0x0065b9a9, 0x0065b9a9, 0x0065b9aa, 0x0065b9aa, 0x0065b9aa, 0x00090003};
    memcpy (msg_buffer.response_buffer_p, ref_heaps, sizeof(ref_heaps));

    ASSERT_EQ(awecmd_response_getData(&msg_buffer, response, 10, &nr_words_obtained), AWECMD_RC_OK);
    ASSERT_EQ(nr_words_obtained, 6);
    EXPECT_TRUE(ArraysEqual(&ref_heaps[2], response, 6));

    // get shared heap (for AWE-Q/MultiCore canvases)
    ASSERT_EQ(awecmd_getSharedHeapSize(&msg_buffer, 0), AWECMD_RC_OK);
    ASSERT_EQ(msg_buffer.words_written, 2);
    unsigned int expected_sharedheap[3] = {0x00020082, 0x00020082};
    EXPECT_TRUE(ArraysEqual(expected_sharedheap, msg_buffer.buffer_p, 2));

    // check result
    unsigned int ref_shared_heaps[5] = {0x00050000, 0x00000000, 0x01312ce8, 0x01312d00, 0x000501e8};
    memcpy (msg_buffer.response_buffer_p, ref_shared_heaps, sizeof(ref_shared_heaps));

    ASSERT_EQ(awecmd_response_getData(&msg_buffer, response, 10, &nr_words_obtained), AWECMD_RC_OK);
    ASSERT_EQ(nr_words_obtained, 2);
    EXPECT_TRUE(ArraysEqual(&ref_shared_heaps[2], response, 2));

}

/**
```yaml
- id: utest~AWEMGR.Cmd.CpuInfo~1
  covers: dsn~AWEMGR.AWECMD.CpuInfo~1
  description: Checks commands to retrieve CPU load information.
```
*/
TEST_F(AweCmdTestFixture, GetCpuLoad)
{
    unsigned int response[10];
    unsigned int nr_words_obtained;
    unsigned int crc_dummy = 0xaffe;

    // get CPU load
    ASSERT_EQ(awecmd_getCpuLoad(&msg_buffer, 0, 0), AWECMD_RC_OK);
    ASSERT_EQ(msg_buffer.words_written, 2);
    unsigned int expected[3] = {0x0002002b, 0x0002002b};
    EXPECT_TRUE(ArraysEqual(expected, msg_buffer.buffer_p, 2));

    // check result (real live 36% IDC :) )
    // 0x00050000, 0x00000000, 0x55410fbb, 0xec3f5b2c, 0xb97b5497
    unsigned int ref_cpuload[5] = {0x00050000, 0x00000000, 0x55410fbb, 0xec3f5b2c, 0xb97b5497};
    memcpy (msg_buffer.response_buffer_p, ref_cpuload, sizeof(ref_cpuload));

    ASSERT_EQ(awecmd_response_getData(&msg_buffer, response, 3, &nr_words_obtained), AWECMD_RC_OK);
    ASSERT_EQ(nr_words_obtained, 2);
    ASSERT_EQ(response[0], 0x55410fbb);
    ASSERT_EQ(response[1], 0xec3f5b2c);
}

