/* MIT License
**
** Copyright (c) 2024 DSP Concepts, Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**/
// Tests for the awemgr_logging component.
//
// When built with -DAWEMGR_LOGGING_AWEQ the customer header "awe_log_util.h"
// is replaced by mocks/awe_log_util.h via include-path ordering (see
// CMakeLists.txt).  All calls to AUDIO_BASE_LOG are forwarded to
// mock_audio_base_log_impl() which records the arguments so tests can assert
// on level, module and message content.
//
// Key difference vs STDIO/SYSLOG modes:
//   - AWEQ macros do NOT check get_loglevel() before calling AUDIO_BASE_LOG;
//     level filtering is delegated entirely to the AWE-Q runtime.
//   - AWEMGR_LOGW and AWEMGR_LOGI both map to BASE_LOG_INFO (AWE-Q has no
//     dedicated WARN level).

#include <gtest/gtest.h>

#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>

extern "C" {
#include "awemgr_logging.h"
}

// ---------------------------------------------------------------------------
// Mock sink - receives every AUDIO_BASE_LOG() call

struct LogCall {
    int         level;
    int         module;
    std::string message;
};

static std::vector<LogCall> g_log_calls;

extern "C" void mock_audio_base_log_impl(int level, int module, const char *format, ...)
{
    char buf[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    g_log_calls.push_back({level, module, std::string(buf)});
}

// ---------------------------------------------------------------------------
// Fixture - clears the call log before each test

class LoggingAweqTest : public ::testing::Test {
protected:
    void SetUp() override { g_log_calls.clear(); }
};

// ---------------------------------------------------------------------------
// Routing: each AWEMGR_LOG* macro must call AUDIO_BASE_LOG with the correct level

/**
```yaml
- id: utest~AWEMGR.AWELOG.Loge_RoutesToErrorLevel~1
  covers: req~AWEMGR.Logging~1
  description: Verifies AWEMGR_LOGE routes to BASE_LOG_ERROR with MODULE_AWE_MGR and formats the message.
```
*/
TEST_F(LoggingAweqTest, Loge_RoutesToErrorLevel)
{
    AWEMGR_LOGE(AWEMGR_LOG_API, "error value %d", 42);
    ASSERT_EQ(g_log_calls.size(), 1u);
    EXPECT_EQ(g_log_calls[0].level,  BASE_LOG_ERROR);
    EXPECT_EQ(g_log_calls[0].module, MODULE_AWE_MGR);
    EXPECT_NE(g_log_calls[0].message.find("42"), std::string::npos);
}

/**
```yaml
- id: utest~AWEMGR.AWELOG.Logw_RoutesToInfoLevel~1
  covers: req~AWEMGR.Logging~1
  description: Verifies AWEMGR_LOGW routes to BASE_LOG_INFO (AWE-Q has no dedicated WARN level).
```
*/
TEST_F(LoggingAweqTest, Logw_RoutesToInfoLevel)
{
    AWEMGR_LOGW(AWEMGR_LOG_AWC, "warn msg");
    ASSERT_EQ(g_log_calls.size(), 1u);
    EXPECT_EQ(g_log_calls[0].level, BASE_LOG_INFO);
}

/**
```yaml
- id: utest~AWEMGR.AWELOG.Logi_RoutesToInfoLevel~1
  covers: req~AWEMGR.Logging~1
  description: Verifies AWEMGR_LOGI routes to BASE_LOG_INFO.
```
*/
TEST_F(LoggingAweqTest, Logi_RoutesToInfoLevel)
{
    AWEMGR_LOGI(AWEMGR_LOG_CMD, "info msg");
    ASSERT_EQ(g_log_calls.size(), 1u);
    EXPECT_EQ(g_log_calls[0].level, BASE_LOG_INFO);
}

/**
```yaml
- id: utest~AWEMGR.AWELOG.Logd_RoutesToDebugLevel~1
  covers: req~AWEMGR.Logging~1
  description: Verifies AWEMGR_LOGD routes to BASE_LOG_DEBUG.
```
*/
TEST_F(LoggingAweqTest, Logd_RoutesToDebugLevel)
{
    AWEMGR_LOGD(AWEMGR_LOG_COMM, "debug msg");
    ASSERT_EQ(g_log_calls.size(), 1u);
    EXPECT_EQ(g_log_calls[0].level, BASE_LOG_DEBUG);
}

/**
```yaml
- id: utest~AWEMGR.AWELOG.AllMacros_UseModuleAweMgr~1
  covers: req~AWEMGR.Logging~1
  description: Verifies that all AWEMGR_LOG* macros tag every call with MODULE_AWE_MGR.
```
*/
TEST_F(LoggingAweqTest, AllMacros_UseModuleAweMgr)
{
    AWEMGR_LOGE(AWEMGR_LOG_API, "e");
    AWEMGR_LOGW(AWEMGR_LOG_API, "w");
    AWEMGR_LOGI(AWEMGR_LOG_API, "i");
    AWEMGR_LOGD(AWEMGR_LOG_API, "d");
    for (const auto &c : g_log_calls)
        EXPECT_EQ(c.module, MODULE_AWE_MGR);
}

// ---------------------------------------------------------------------------
// set_loglevel / get_loglevel are compiled in regardless of logging backend

/**
```yaml
- id: utest~AWEMGR.AWELOG.SetGetLoglevel_RoundTrip~1
  covers: req~AWEMGR.Logging~1
  description: Verifies that set_loglevel/get_loglevel store and retrieve the configured level correctly.
```
*/
TEST_F(LoggingAweqTest, SetGetLoglevel_RoundTrip)
{
    set_loglevel(AWEMGR_LOG_API, AWEMGR_LOG_LEVEL_DEBUG);
    EXPECT_EQ(get_loglevel(AWEMGR_LOG_API), AWEMGR_LOG_LEVEL_DEBUG);
    set_loglevel(AWEMGR_LOG_API, AWEMGR_LOG_LEVEL_ERROR);
    EXPECT_EQ(get_loglevel(AWEMGR_LOG_API), AWEMGR_LOG_LEVEL_ERROR);
}

/**
```yaml
- id: utest~AWEMGR.AWELOG.SetLoglevel_OutOfRangeIgnored~1
  covers: req~AWEMGR.Logging~1
  description: Verifies that set_loglevel ignores an out-of-range level value and leaves the previous level unchanged.
```
*/
TEST_F(LoggingAweqTest, SetLoglevel_OutOfRangeIgnored)
{
    set_loglevel(AWEMGR_LOG_API, AWEMGR_LOG_LEVEL_DEBUG);
    set_loglevel(AWEMGR_LOG_API, AWEMGR_LOG_LEVEL_MAX);   // invalid, must be ignored
    EXPECT_EQ(get_loglevel(AWEMGR_LOG_API), AWEMGR_LOG_LEVEL_DEBUG);
}

/**
```yaml
- id: utest~AWEMGR.AWELOG.Loge_FiresRegardlessOfSetLoglevel~1
  covers: req~AWEMGR.Logging~1
  description: Documents that AWEQ macros call AUDIO_BASE_LOG unconditionally; level filtering is delegated to the AWE-Q runtime, not get_loglevel().
```
*/
TEST_F(LoggingAweqTest, Loge_FiresRegardlessOfSetLoglevel)
{
    set_loglevel(AWEMGR_LOG_API, AWEMGR_LOG_LEVEL_ERROR);
    AWEMGR_LOGE(AWEMGR_LOG_API, "fires unconditionally");
    EXPECT_EQ(g_log_calls.size(), 1u);
}

// ---------------------------------------------------------------------------
// awemgr_log_buffer - callback variant

/**
```yaml
- id: utest~AWEMGR.AWELOG.LogBuffer_Int_HexFormatted~1
  covers: req~AWEMGR.Logging~1
  description: Verifies awemgr_log_buffer formats integer data as lowercase hex (0x...) and delivers it via callback.
```
*/
TEST_F(LoggingAweqTest, LogBuffer_Int_HexFormatted)
{
    unsigned int data[] = {0xDEADBEEF, 0x00000001};
    std::vector<std::string> lines;
    auto cb = [](char *line, int /*idx*/, void *ctx) {
        static_cast<std::vector<std::string>*>(ctx)->emplace_back(line);
    };
    awemgr_log_buffer(data, sizeof(data), AWEMGR_LOG_VARTYPE_INT, cb, &lines);
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_NE(lines[0].find("0xdeadbeef"), std::string::npos);
    EXPECT_NE(lines[0].find("0x00000001"), std::string::npos);
}

/**
```yaml
- id: utest~AWEMGR.AWELOG.LogBuffer_Float_DecimalFormatted~1
  covers: req~AWEMGR.Logging~1
  description: Verifies awemgr_log_buffer formats float data in decimal notation and delivers it via callback.
```
*/
TEST_F(LoggingAweqTest, LogBuffer_Float_DecimalFormatted)
{
    float data[] = {1.0f, 2.5f};
    std::vector<std::string> lines;
    auto cb = [](char *line, int /*idx*/, void *ctx) {
        static_cast<std::vector<std::string>*>(ctx)->emplace_back(line);
    };
    awemgr_log_buffer(data, sizeof(data), AWEMGR_LOG_VARTYPE_FLOAT, cb, &lines);
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_NE(lines[0].find("2.5"), std::string::npos);
}

/**
```yaml
- id: utest~AWEMGR.AWELOG.LogBuffer_NullCallback_DoesNotCrash~1
  covers: req~AWEMGR.Logging~1
  description: Verifies awemgr_log_buffer falls back to printf and does not crash when the callback is NULL.
```
*/
TEST_F(LoggingAweqTest, LogBuffer_NullCallback_DoesNotCrash)
{
    ::testing::internal::CaptureStdout();
    unsigned int data[] = {0x1234};
    // Falls back to printf when cb is NULL
    awemgr_log_buffer(data, sizeof(data), AWEMGR_LOG_VARTYPE_INT, nullptr, nullptr);
    std::string output = ::testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "0x00001234\n");
}
