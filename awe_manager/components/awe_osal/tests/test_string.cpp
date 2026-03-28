#include <gtest/gtest.h>
#include "awosal_string.h"

class AweOSALStrlcpyTestFixture : public ::testing::Test {
protected:
    char buffer[20];
};

TEST_F(AweOSALStrlcpyTestFixture, NormalCopy) {
    size_t ret = awosal_strlcpy(buffer, "hello", sizeof(buffer));
    EXPECT_EQ(ret, 5);
    EXPECT_STREQ(buffer, "hello");
}

TEST_F(AweOSALStrlcpyTestFixture, Truncation) {
    size_t ret = awosal_strlcpy(buffer, "hello world", 6);
    EXPECT_EQ(ret, 11); // length of src
    EXPECT_STREQ(buffer, "hello"); // truncated copy
}

TEST_F(AweOSALStrlcpyTestFixture, ZeroSize) {
    strcpy(buffer, "initial");
    size_t ret = awosal_strlcpy(buffer, "test", 0);
    EXPECT_EQ(ret, 4);
    EXPECT_STREQ(buffer, "initial"); // buffer unchanged
}

TEST_F(AweOSALStrlcpyTestFixture, SrcDstSamePointer) {
    strcpy(buffer, "samebuffer");
    size_t ret = awosal_strlcpy(buffer, buffer, sizeof(buffer));
    EXPECT_EQ(ret, strlen(buffer));
    EXPECT_STREQ(buffer, "samebuffer");
}

TEST_F(AweOSALStrlcpyTestFixture, NullSrc) {
    size_t ret = awosal_strlcpy(buffer, nullptr, sizeof(buffer));
    EXPECT_EQ(ret, 0);
}

TEST_F(AweOSALStrlcpyTestFixture, NullDst) {
    size_t ret = awosal_strlcpy(nullptr, "test", 5);
    EXPECT_EQ(ret, 0);
}

TEST_F(AweOSALStrlcpyTestFixture, OverlappingBuffers) {
    char buffer[30] = "1234567890abcdefghij";

    // Copy starting from buffer[0] to buffer[5], i.e. overlapping
    size_t ret = awosal_strlcpy(buffer + 5, buffer, 15);

    // ret should be length of src string (20)
    EXPECT_EQ(ret, 20);

    // buffer now should have copied the first 14 chars + null terminator starting at buffer[5]
    // original first 5 chars untouched: "12345"
    // then copied chars starting at buffer[5]: "1234567890abcd"
    EXPECT_STREQ(buffer + 5, "1234567890abcd");

    // Full buffer expected start: "12345" + "1234567890abcd"
    EXPECT_STREQ(buffer, "123451234567890abcd");
}