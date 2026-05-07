#include "test_fixtures.h"
#include "awe_awc.h"
#include "awemgr_util.h"
#include <cmath>

/**
```yaml
- id: itest~AWEMGR.VariableReadWrite~2
  covers: req~AWEMGR.Named_Access~1
  description: Reads and Writes variable values, for scaler and array
```
*/
TEST_F(AweMgrTestFixtureSetGetAWCLoaded, VariableReadWrite) {
	unsigned int response_buffer[10];
	unsigned int nr_words_in_buf;
	enum awemgr_vartype typ;
	ASSERT_EQ(awemgr_control_read(m_ctx, "SourceInt_1.value", response_buffer, 10, &nr_words_in_buf, &typ), awemgr_RC_OK);
	EXPECT_EQ(response_buffer[0], 0xdead);

	// update with new value
	unsigned int new_value = 1;
	ASSERT_EQ(awemgr_control_write(m_ctx, "SourceInt_1.value", 0, &new_value, 1), awemgr_RC_OK);

	// give AWE pumping a little bit
	delay_ms(20);

	// read back from down stream module
	ASSERT_EQ(awemgr_control_read(m_ctx, "SinkInt_1.value", response_buffer, 10, &nr_words_in_buf, &typ), awemgr_RC_OK);
	EXPECT_EQ(response_buffer[0], 1);

	// repeat for an array
	ASSERT_EQ(awemgr_control_read(m_ctx, "SourceInt_10.value", response_buffer, 10, &nr_words_in_buf, &typ), awemgr_RC_OK);
	unsigned int expected[10] = {10, 170, 57005, 0, 0, 0, 0, 10, 170, 57005};
	EXPECT_TRUE(ArraysEqual(response_buffer, expected, 10));

	unsigned int new_values[10] = {0, 0, 2, 2, 4, 4, 6, 6, 4, 2};
	ASSERT_EQ(awemgr_control_write(m_ctx, "SourceInt_10.value", 0, &new_values, 10), awemgr_RC_OK);

	// give AWE pumping a little bit
	delay_ms(20);

	ASSERT_EQ(awemgr_control_read(m_ctx, "SinkInt_10.value", response_buffer, 10, &nr_words_in_buf, &typ), awemgr_RC_OK);
	EXPECT_TRUE(ArraysEqual(response_buffer, new_values, 10));
};


/**
```yaml
- id: itest~AWEMGR.VariableReadPartial~1
  covers: req~AWEMGR.Named_Access~1
  description: Reads only partial sections of a variable.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWCLoaded, VariablePartialRead) {
	unsigned int response_buffer[10];
	unsigned int nr_words_in_buf;
	enum awemgr_vartype typ;
	ASSERT_EQ(awemgr_control_read(m_ctx, "SourceInt_10.value", response_buffer, 10, &nr_words_in_buf, &typ), awemgr_RC_OK);
	unsigned int expected[10] = {10, 170, 57005, 0, 0, 0, 0, 10, 170, 57005};
	EXPECT_TRUE(ArraysEqual(response_buffer, expected, 10));

	ASSERT_EQ(awemgr_control_read_partial(m_ctx, "SourceInt_10.value", 1, 2, &response_buffer[4], 6, &nr_words_in_buf, &typ), awemgr_RC_OK);
	unsigned int expected_partial[10] = {10, 170, 57005, 0, 170, 57005, 0, 10, 170, 57005};
	EXPECT_TRUE(ArraysEqual(response_buffer, expected_partial, 10));
};


/**
```yaml
- id: itest~AWEMGR.VariableReadPartialFail~1
  covers: req~AWEMGR.Named_Access~1
  description: Checks error handling for incorrect partial reads.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWCLoaded, VariablePartialReadFail) {
	unsigned int response_buffer[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned int nr_words_in_buf;
	enum awemgr_vartype typ;

	// too many words requested
	ASSERT_EQ(awemgr_control_read_partial(m_ctx, "SourceInt_10.value", 0, 222, &response_buffer[4], 6, &nr_words_in_buf, &typ), awemgr_RC_ERR);

	// too high offset
	ASSERT_EQ(awemgr_control_read_partial(m_ctx, "SourceInt_10.value", 222, 10, &response_buffer[4], 6, &nr_words_in_buf, &typ), awemgr_RC_ERR);

	// too many words due to negative number
	ASSERT_EQ(awemgr_control_read_partial(m_ctx, "SourceInt_10.value", 0, -10, &response_buffer[4], 6, &nr_words_in_buf, &typ), awemgr_RC_ERR);

	// too high offset due to negative number
	ASSERT_EQ(awemgr_control_read_partial(m_ctx, "SourceInt_10.value", -222, 10, &response_buffer[4], 6, &nr_words_in_buf, &typ), awemgr_RC_ERR);
};


/**
```yaml
- id: itest~AWEMGR.VariableReadBigBuffer~1
  covers: req~AWEMGR.Named_Access~1
  description: |
    Reads and Writes a variable array of a size bigger than the tuning message buffer.
    It checks how the partial write/reads happen.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWCLoaded, VariableReadBigBuffer) {
	unsigned int response_buffer[2000];
	unsigned int request_buffer[2000];
	unsigned int nr_words_in_buf;
	enum awemgr_vartype typ;

	// first get "original" content
	ASSERT_EQ(awemgr_control_read(m_ctx, "SourceInt_2000.value", response_buffer, 2000, &nr_words_in_buf, &typ), awemgr_RC_OK);
	ASSERT_EQ(nr_words_in_buf, 2000);

	// now copy a multiple of every value into request buffer
	for(int i=0; i < 2000; i++) {
		request_buffer[i] = response_buffer[i] * 2;
	}

	// send request buffer content
	ASSERT_EQ(awemgr_control_write(m_ctx, "SourceInt_2000.value", 0, request_buffer, 2000), awemgr_RC_OK);

	// give AWE pumping a little bit
	delay_ms(20);

	// read back into response buffer
	ASSERT_EQ(awemgr_control_read(m_ctx, "SourceInt_2000.value", response_buffer, 2000, &nr_words_in_buf, &typ), awemgr_RC_OK);

	// original request and response buffer should be the same now
	EXPECT_TRUE(ArraysEqual(response_buffer, request_buffer, 2000));
};


/**
```yaml
- id: itest~AWEMGR.VariableWritePartial~2
  covers: req~AWEMGR.Named_Access~1
  description: Writes parts of the source variable and checks modified buffer in sink variable.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWCLoaded, VariableWritePartial) {
	unsigned int response_buffer[10];
	unsigned int nr_words_in_buf;
	enum awemgr_vartype typ;
	ASSERT_EQ(awemgr_control_read(m_ctx, "SinkInt_10.value", response_buffer, 10, &nr_words_in_buf, &typ), awemgr_RC_OK);
	unsigned int expected[10] = {10, 170, 57005, 0, 0, 0, 0, 10, 170, 57005};
	EXPECT_TRUE(ArraysEqual(response_buffer, expected, 10));

	unsigned int modify_buf[2] = {2, 6};
	ASSERT_EQ(awemgr_control_write(m_ctx, "SourceInt_10.value", 4, modify_buf, 2), awemgr_RC_OK);

	// give AWE pumping a little bit
	delay_ms(20);

	ASSERT_EQ(awemgr_control_read(m_ctx, "SinkInt_10.value", response_buffer, 10, &nr_words_in_buf, &typ), awemgr_RC_OK);
	unsigned int expected_partial[10] = {10, 170, 57005, 0, 2, 6, 0, 10, 170, 57005};
	EXPECT_TRUE(ArraysEqual(response_buffer, expected_partial, 10));
};

/**
```yaml
- id: itest~AWEMGR.VariablePartialWriteFail~1
  covers: req~AWEMGR.Named_Access~1
  description: Checks error handling for incorrect partial reads.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWCLoaded, VariablePartialWriteFail) {
	unsigned int response_buffer[10];
	unsigned int nr_words_in_buf;
	enum awemgr_vartype typ;
	ASSERT_EQ(awemgr_control_read(NULL, "SinkInt_10.value", response_buffer, 10, &nr_words_in_buf, &typ), awemgr_RC_ERR);
	ASSERT_EQ(awemgr_control_read(NULL, "WrongName", response_buffer, 10, &nr_words_in_buf, &typ), awemgr_RC_ERR);
	ASSERT_EQ(awemgr_control_read(m_ctx, "SinkInt_10.value", response_buffer, 10, &nr_words_in_buf, &typ), awemgr_RC_OK);
	unsigned int expected[10] = {10, 170, 57005, 0, 0, 0, 0, 10, 170, 57005};
	EXPECT_TRUE(ArraysEqual(response_buffer, expected, 10));

	unsigned int modify_buf[2] = {0xaffe, 0xdead};
	ASSERT_EQ(awemgr_control_write(NULL, "SourceInt_10.value", 0, modify_buf, 1), awemgr_RC_ERR);
	ASSERT_EQ(awemgr_control_write(m_ctx, "Wrong Name", 0, modify_buf, 1), awemgr_RC_ERR);
	ASSERT_EQ(awemgr_control_write(m_ctx, "SourceInt_10.value", 9, modify_buf, 2), awemgr_RC_ERR);

	ASSERT_EQ(awemgr_control_write(m_ctx, "SourceInt_10.value", 12, modify_buf, 2), awemgr_RC_ERR);
};


/**
```yaml
- id: itest~AWEMGR.VariableReadWriteSetMask~2
  covers: req~AWEMGR.Named_Access~1
  description: |
    Checks that variables of modules that are written to, do not change the output until the last
    buffer content has been sent.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWCLoaded, VariableReadWriteSetMask) {
	unsigned int response_buffer[512];
	unsigned int nr_words_in_buf;
	enum awemgr_vartype typ;
	ASSERT_EQ(awemgr_control_read(m_ctx, "MixerFract.nonZeroGainFract32", response_buffer, 512, &nr_words_in_buf, &typ), awemgr_RC_OK);
    unsigned int expected_zero[512] = {0};
    EXPECT_TRUE(ArraysEqual(expected_zero, response_buffer, 512));

	// update gain values of mixer
	float new_values[512];
	for (int x=0; x<512; x++)
		new_values[x] = 2.0f; // awc index file has step size set as 2.0

	// the whole buffer is being written in 2 tune packages, the first will not contain the mask information
	//   TX:    0 : 0x0108003d, 0x0753a00c, 0x00000000, 0x00000000, 0x00000102, ...
	// but the second will:
	//   TX:    0 : 0x0104003d, 0x0753a00c, 0x00000102, 0x00001000, 0x000000fe, ...
	// and will call the SetFunction which will update nonZeroGainFract32
	ASSERT_EQ(awemgr_control_write(m_ctx, "MixerFract.gain", 0, new_values, 512), awemgr_RC_OK);

	// this should immediately be updated
	ASSERT_EQ(awemgr_control_read(m_ctx, "MixerFract.nonZeroGainFract32", response_buffer, 512, &nr_words_in_buf, &typ), awemgr_RC_OK);

	// give AWE pumping a little bit
	delay_ms(10);

	ASSERT_EQ(awemgr_control_read(m_ctx, "SinkFract_MixOut.value", response_buffer, 32, &nr_words_in_buf, &typ), awemgr_RC_OK);
	EXPECT_FALSE(ArraysEqual(expected_zero, response_buffer, 32));
};

/**
```yaml
- id: itest~AWEMGR.WriteInvalidRange~1
  covers: req~AWEMGR.ControlRangeCheck~1
  description: Writes incorrect values to variables and expects error codes.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWCLoaded, WriteInvalidRange) {

	// check scalar INT value
	unsigned int int_value = 2;
	ASSERT_EQ(awemgr_control_write(m_ctx, "SourceInt_1.value", 0, (void *)&int_value, 1), awemgr_RC_ERR_INVALID_VAL);

	// check scalar fract value
	float f_value = -0.6f;
	int32_t fract_value = float_to_fract32(f_value);
	ASSERT_EQ(awemgr_control_write(m_ctx, "MixerFract.nonZeroGainFract32", 0, &fract_value, 1), awemgr_RC_ERR_INVALID_VAL);

	// check float value in array out of range
	float f_array_outofrange[10] = {0, 0, 0, 30, 0, 0, 0, 0, 0, 0};  // only values allowed -20 - 20; with 2.0 steps
	ASSERT_EQ(awemgr_control_write(m_ctx, "MixerFract.gain", 0, &f_array_outofrange, 10), awemgr_RC_ERR_INVALID_VAL);

	// check INT value in
	unsigned int int_array[10] = {0, 5, 0, 0, 0, 0, 0, 0, 0, 0};  // only even values allowed, -6 - 6 with step 2
	ASSERT_EQ(awemgr_control_write(m_ctx, "SourceInt_10.value", 0, (void *)&int_array, 10), awemgr_RC_ERR_INVALID_VAL);

	// check incorrect step
	float f_array_wrong_step[10] = {0, 0, 0, 11.5, 0, 0, 0, 0, 0, 0};  // only values allowed -20 - 20; with 2.0 steps
	ASSERT_EQ(awemgr_control_write(m_ctx, "MixerFract.gain", 0, &f_array_wrong_step, 10), awemgr_RC_ERR_INVALID_VAL);
};

/**
```yaml
- id: itest~AWEMGR.WriteInvalidRangeCheckOff~1
  covers: req~AWEMGR.ControlRangeCheckArraysSetting~1
  description: Disables range check and writes non-allowed value without getting an error.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWCLoaded, WriteInvalidRangeCheckOff) {

	awemgr_config_set(cfg_p, CFG_MGR_RANGECHECK_ENABLED, "false");

	unsigned int int_value = 2;
	ASSERT_EQ(awemgr_control_write(m_ctx, "SourceInt_1.value", 0, (void *)&int_value, 1), awemgr_RC_OK);

}

/**
```yaml
- id: itest~AWEMGR.Int32RangeCheck~1
  covers: req~AWEMGR.ControlRangeCheck~1
  description: Checks range checking on int32 data types.
```
*/
TEST(CheckValueRange, Int32)
{
    // Case 1: default min/max=0, step=2
    awc_ctl_t ctl;
    ctl.fullname = (char*)"IntCtrl";
    ctl.type = AWC_CTL_INT32;
    ctl.range = {0, 0, 0, 2}; // min=0, max=0, step=2

    int32_t data1[] = {0, 2, -2, 10};
    // Only 0 is valid for min=max=0
    EXPECT_EQ(check_value_range(data1, 0, &ctl), awemgr_RC_OK);
    EXPECT_EQ(check_value_range(data1, 1, &ctl), awemgr_RC_ERR_INVALID_VAL);
    EXPECT_EQ(check_value_range(data1, 2, &ctl), awemgr_RC_ERR_INVALID_VAL);

    // Case 2: positive range 0..10, step=2
    ctl.range = {0, 0, 10, 2};
    int32_t data2[] = {0, 2, 4, 10, 11};
    EXPECT_EQ(check_value_range(data2, 0, &ctl), awemgr_RC_OK);
    EXPECT_EQ(check_value_range(data2, 1, &ctl), awemgr_RC_OK);
    EXPECT_EQ(check_value_range(data2, 3, &ctl), awemgr_RC_OK);
    EXPECT_EQ(check_value_range(data2, 4, &ctl), awemgr_RC_ERR_INVALID_VAL); // 11 out of range

    // Case 3: negative range -10..0, step=2
    ctl.range = {0, -10, 0, 2};
    int32_t data3[] = {-10, -8, -2, 0, 1};
    EXPECT_EQ(check_value_range(data3, 0, &ctl), awemgr_RC_OK);
    EXPECT_EQ(check_value_range(data3, 3, &ctl), awemgr_RC_OK);
    EXPECT_EQ(check_value_range(data3, 4, &ctl), awemgr_RC_ERR_INVALID_VAL); // 1 not aligned to step

    // Case 4: mixed negative/positive range -5..5, step=1
    ctl.range = {0, -5, 5, 1};
    int32_t data4[] = {-5, 0, 5, 6, -6};
    EXPECT_EQ(check_value_range(data4, 0, &ctl), awemgr_RC_OK);
    EXPECT_EQ(check_value_range(data4, 1, &ctl), awemgr_RC_OK);
    EXPECT_EQ(check_value_range(data4, 2, &ctl), awemgr_RC_OK);
    EXPECT_EQ(check_value_range(data4, 3, &ctl), awemgr_RC_ERR_INVALID_VAL); // 6 > max
    EXPECT_EQ(check_value_range(data4, 4, &ctl), awemgr_RC_ERR_INVALID_VAL); // -6 < min

    // Case 5: step=0 (any value within min/max allowed)
    ctl.range = {0, 0, 10, 0};
    int32_t data5[] = {0, 1, 5, 10, 11};
    EXPECT_EQ(check_value_range(data5, 0, &ctl), awemgr_RC_OK);
    EXPECT_EQ(check_value_range(data5, 3, &ctl), awemgr_RC_OK);
    EXPECT_EQ(check_value_range(data5, 4, &ctl), awemgr_RC_ERR_INVALID_VAL); // 11 > max
}

/**
```yaml
- id: itest~AWEMGR.UInt32RangeCheck~1
  covers: req~AWEMGR.ControlRangeCheck~1
  description: Checks range checking on uint32 data types.
```
*/
TEST(CheckValueRange, UInt32)
{
    awc_ctl_t ctl;
    ctl.fullname = (char*)"UIntCtrl";
    ctl.type = AWC_CTL_UINT32;

    // Case 1: default=0, min=0, max=0, step=2
    ctl.range = {0, 0, 0, 2}; // default=0, min=0, max=0, step=2
    uint32_t data1[] = {0, 2, 10};
    EXPECT_EQ(check_value_range(data1, 0, &ctl), awemgr_RC_OK);   // 0 ok
    EXPECT_EQ(check_value_range(data1, 1, &ctl), awemgr_RC_ERR_INVALID_VAL); // 2 > max

    // Case 2: default=0, min=0, max=10, step=2
    ctl.range = {0, 0, 10, 2};
    uint32_t data2[] = {0, 2, 4, 10, 11};
    EXPECT_EQ(check_value_range(data2, 0, &ctl), awemgr_RC_OK);
    EXPECT_EQ(check_value_range(data2, 1, &ctl), awemgr_RC_OK);
    EXPECT_EQ(check_value_range(data2, 3, &ctl), awemgr_RC_OK); // 10 aligned to step

    // Case 3: default=1, min=1, max=10, step=3
    ctl.range = {1, 1, 10, 3}; // default=1
    uint32_t data4[] = {1, 4, 7, 10, 2};
    EXPECT_EQ(check_value_range(data4, 0, &ctl), awemgr_RC_OK); // 1 aligned
    EXPECT_EQ(check_value_range(data4, 1, &ctl), awemgr_RC_OK); // 4 aligned
    EXPECT_EQ(check_value_range(data4, 2, &ctl), awemgr_RC_OK); // 7 aligned
    EXPECT_EQ(check_value_range(data4, 3, &ctl), awemgr_RC_OK); // 10 aligned
    EXPECT_EQ(check_value_range(data4, 4, &ctl), awemgr_RC_ERR_INVALID_VAL); // 2 not aligned
}

/**
```yaml
- id: itest~AWEMGR.FloatRangeCheck~1
  covers: req~AWEMGR.ControlRangeCheck~1
  description: Checks range checking on float data types.
```
*/
TEST(CheckValueRange, Float)
{
	awc_ctl_t ctl;
	ctl.fullname = (char*)"FloatCtrl"; ctl.type = AWC_CTL_FLOAT;
	ctl.range = {0.0, 0.0, 1.0, 0.1};

    float data[] = {0.0f, 0.1f, 0.2f, 0.15f};
    EXPECT_EQ(check_value_range(data, 0, &ctl), awemgr_RC_OK);
    EXPECT_EQ(check_value_range(data, 3, &ctl), awemgr_RC_ERR_INVALID_VAL);
}

/**
```yaml
- id: itest~AWEMGR.Fract32RangeCheck~1
  covers: req~AWEMGR.ControlRangeCheck~1
  description: Checks range checking on fract32 data types.
```
*/
TEST(CheckValueRange, Fract32)
{
	awc_ctl_t ctl;
	ctl.fullname = (char*)"Fract32Ctrl"; ctl.type = AWC_CTL_FRACT32;
	ctl.range = {0.0, 0.0, 1.0, 0.1};
    int32_t data[] = {0, float_to_fract32(0.1f), 2147483647}; // 0.0, ~0.1, ~1.0
    EXPECT_EQ(check_value_range(data, 0, &ctl), awemgr_RC_OK);
    EXPECT_EQ(check_value_range(data, 1, &ctl), awemgr_RC_OK);
    EXPECT_EQ(check_value_range(data, 2, &ctl), awemgr_RC_OK);
}

/**
```yaml
- id: itest~AWEMGR.Fract16RangeCheck~1
  covers: req~AWEMGR.ControlRangeCheck~1
  description: Checks range checking on fract16 data types.
```
*/
TEST(CheckValueRange, Fract16)
{
	awc_ctl_t ctl;
	ctl.fullname = (char*)"Fract16Ctrl"; ctl.type = AWC_CTL_FRACT16;
	ctl.range = {0.0, 0.0, 1.0, 0.1};
    int32_t data[] = {0, float_to_fract16(0.1f), 32768}; // 0.0, ~0.1, 1.0
    EXPECT_EQ(check_value_range(data, 0, &ctl), awemgr_RC_OK);
    EXPECT_EQ(check_value_range(data, 1, &ctl), awemgr_RC_OK);
    EXPECT_EQ(check_value_range(data, 2, &ctl), awemgr_RC_OK);
}

TEST(CheckValueRange, Fract32_AlignmentAndRangeErrors)
{
	awc_ctl_t ctl;
	ctl.fullname = (char*)"Fract32Ctrl"; ctl.type = AWC_CTL_FRACT32;
	ctl.range = {0.0, 0.0, 0.9, 0.1};
    int32_t data_valid[] = {0, 214748365, 429496731, float_to_fract32(0.9f)};
    int32_t data_step_error = 107374182;   // ~0.05, misaligned to 0.1
	int32_t data_range_error = static_cast<int32_t>(2147483647); // maximum valid

    // Valid values
    for (int i = 0; i < 4; ++i)
        EXPECT_EQ(check_value_range(data_valid, i, &ctl), awemgr_RC_OK);

    // Step misalignment
    EXPECT_EQ(check_value_range(&data_step_error, 0, &ctl), awemgr_RC_ERR_INVALID_VAL);

    // Range violation
    EXPECT_EQ(check_value_range(&data_range_error, 0, &ctl), awemgr_RC_ERR_INVALID_VAL);
}

TEST(CheckValueRange, Fract16_AlignmentAndRangeErrors)
{
	awc_ctl_t ctl;
	ctl.fullname = (char*)"Fract16Ctrl"; ctl.type = AWC_CTL_FRACT16;
	ctl.range = {0.0, 0.0, 1.0, 0.1};

    // 0.0, ~0.1, ~0.2, ~1.0
    int32_t data_valid[] = {0, 3277, 6554, 32768};
    int32_t data_step_error[] = {1638};  // ~0.05
    int32_t data_range_error[] = {32769}; // >1.0

    // Valid values
    for (int i = 0; i < 4; ++i)
        EXPECT_EQ(check_value_range(data_valid, i, &ctl), awemgr_RC_OK);

    // Step misalignment
    EXPECT_EQ(check_value_range(data_step_error, 0, &ctl), awemgr_RC_ERR_INVALID_VAL);

    // Range violation
    EXPECT_EQ(check_value_range(data_range_error, 0, &ctl), awemgr_RC_ERR_INVALID_VAL);
}
