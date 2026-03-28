#include "test_fixtures.h"


/**
```yaml
- id: itest~AWEMGR.VariableReadWrite~1
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
	unsigned int new_value = 0xdeadaffe;
	ASSERT_EQ(awemgr_control_write(m_ctx, "SourceInt_1.value", 0, &new_value, 1, AWEMGR_VARTYPE_INTEGER), awemgr_RC_OK);
	
	// give AWE pumping a little bit
	delay_ms(20);

	// read back from down stream module
	ASSERT_EQ(awemgr_control_read(m_ctx, "SinkInt_1.value", response_buffer, 10, &nr_words_in_buf, &typ), awemgr_RC_OK);
	EXPECT_EQ(response_buffer[0], 0xdeadaffe);

	// repeat for an array
	ASSERT_EQ(awemgr_control_read(m_ctx, "SourceInt_10.value", response_buffer, 10, &nr_words_in_buf, &typ), awemgr_RC_OK);
	unsigned int expected[10] = {10, 170, 57005, 0, 0, 0, 0, 10, 170, 57005};
	EXPECT_TRUE(ArraysEqual(response_buffer, expected, 10));

	unsigned int new_values[10] = {57005, 170, 57005, 0, 0xdeadaffe, 0xdeadaffe, 0, 57005, 170, 57005};
	ASSERT_EQ(awemgr_control_write(m_ctx, "SourceInt_10.value", 0, &new_values, 10, AWEMGR_VARTYPE_INTEGER), awemgr_RC_OK);

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
	ASSERT_EQ(awemgr_control_write(m_ctx, "SourceInt_2000.value", 0, request_buffer, 2000, AWEMGR_VARTYPE_INTEGER), awemgr_RC_OK);

	// give AWE pumping a little bit
	delay_ms(20);

	// read back into response buffer
	ASSERT_EQ(awemgr_control_read(m_ctx, "SourceInt_2000.value", response_buffer, 2000, &nr_words_in_buf, &typ), awemgr_RC_OK);

	// original request and response buffer should be the same now
	EXPECT_TRUE(ArraysEqual(response_buffer, request_buffer, 2000));
};


/**
```yaml
- id: itest~AWEMGR.VariableWritePartial~1
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

	unsigned int modify_buf[2] = {0xaffe, 0xdead};
	ASSERT_EQ(awemgr_control_write(m_ctx, "SourceInt_10.value", 4, modify_buf, 2, typ), awemgr_RC_OK);

	// give AWE pumping a little bit
	delay_ms(20);

	ASSERT_EQ(awemgr_control_read(m_ctx, "SinkInt_10.value", response_buffer, 10, &nr_words_in_buf, &typ), awemgr_RC_OK);
	unsigned int expected_partial[10] = {10, 170, 57005, 0, 0xaffe, 0xdead, 0, 10, 170, 57005};
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
	ASSERT_EQ(awemgr_control_write(NULL, "SourceInt_10.value", 0, modify_buf, 1, typ), awemgr_RC_ERR);
	ASSERT_EQ(awemgr_control_write(m_ctx, "Wrong Name", 0, modify_buf, 1, typ), awemgr_RC_ERR);
	ASSERT_EQ(awemgr_control_write(m_ctx, "SourceInt_10.value", 9, modify_buf, 2, typ), awemgr_RC_ERR);

	ASSERT_EQ(awemgr_control_write(m_ctx, "SourceInt_10.value", 12, modify_buf, 2, typ), awemgr_RC_ERR);
};


/**
```yaml
- id: itest~AWEMGR.VariableReadWriteSetMask~1
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
		new_values[x] = 0.01f;

	// the whole buffer is being written in 2 tune packages, the first will not contain the mask information
	//   TX:    0 : 0x0108003d, 0x0753a00c, 0x00000000, 0x00000000, 0x00000102, ...
	// but the second will:
	//   TX:    0 : 0x0104003d, 0x0753a00c, 0x00000102, 0x00001000, 0x000000fe, ...
	// and will call the SetFunction which will update nonZeroGainFract32
	ASSERT_EQ(awemgr_control_write(m_ctx, "MixerFract.gain", 0, new_values, 512, AWEMGR_VARTYPE_FLOAT), awemgr_RC_OK);
	
	// this should immediately be updated
	ASSERT_EQ(awemgr_control_read(m_ctx, "MixerFract.nonZeroGainFract32", response_buffer, 512, &nr_words_in_buf, &typ), awemgr_RC_OK);

	// give AWE pumping a little bit
	delay_ms(10);

	ASSERT_EQ(awemgr_control_read(m_ctx, "SinkFract_MixOut.value", response_buffer, 32, &nr_words_in_buf, &typ), awemgr_RC_OK);
	EXPECT_FALSE(ArraysEqual(expected_zero, response_buffer, 32));
};
