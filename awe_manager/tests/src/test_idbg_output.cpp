/* MIT License
**
** Copyright (c) 2026 DSP Concepts, Inc.
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

/**
 * @file test_idbg_output.cpp
 * @brief Tests for the deferred output facility of the idbg command engine
 *        (idbg_output_hold() / idbg_output_flush() / idbg_print_direct()).
 *
 * This is what keeps AWEMgr-Shell command output contiguous while the comm
 * trace tap emits data from inside the AWE Manager API calls of a command.
 * No AWE server is needed for these tests.
 */

#include "idbg.h"

#include <gtest/gtest.h>

#include <stdarg.h>
#include <stdio.h>
#include <string>

namespace {

/** Alternative output sink; the backend pointer is the collecting string. */
void print_to_string(idbg_t *p, const char *fmt, va_list ap)
{
    std::string *out_p = (std::string *) idbg_get_printfct_backend(p);
    char         buf[1024];

    (void) vsnprintf(buf, sizeof(buf), fmt, ap);
    if (out_p != nullptr)
        out_p->append(buf);
}

/** Sink recording how much data it gets per call; ctx is a SinkStats. */
struct SinkStats
{
    std::string text;
    size_t      max_call_size = 0;
    int         calls = 0;
};

void print_to_stats(idbg_t *p, const char *fmt, va_list ap)
{
    SinkStats *st_p = (SinkStats *) idbg_get_printfct_backend(p);
    char       buf[4096];

    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    if (st_p != nullptr && n > 0)
    {
        st_p->text.append(buf, (size_t)n);
        if ((size_t)n > st_p->max_call_size)
            st_p->max_call_size = (size_t)n;
        st_p->calls++;
    }
}

class IdbgOutputTest : public ::testing::Test
{
protected:
    void SetUp() override { ASSERT_EQ(idbg_init(&m_p, nullptr), 0); }
    void TearDown() override { idbg_exit(&m_p); }

    idbg_t *m_p = nullptr;
};

} // namespace

/**
```yaml
- id: utest~AWEMGR.ADDON.SHELL.HoldDefersOutput~1
  covers: dsn~AWEMGR.ADDON.SHELL.AtomicCommandOutput~1
  description: |
    Checks that output printed while a hold is active is not written to the
    sink before the matching flush, and that the flush emits it unchanged.
```
*/
TEST_F(IdbgOutputTest, HoldDefersOutputUntilFlush)
{
    ::testing::internal::CaptureStdout();
    EXPECT_EQ(idbg_output_hold(m_p), 0);
    idbg_print(m_p, "cpu_info:\n");
    idbg_print(m_p, "  - name: %s\n", "AWE1");
    EXPECT_EQ(::testing::internal::GetCapturedStdout(), "");

    ::testing::internal::CaptureStdout();
    EXPECT_EQ(idbg_output_flush(m_p), 0);
    EXPECT_EQ(::testing::internal::GetCapturedStdout(), "cpu_info:\n  - name: AWE1\n");
}

/**
```yaml
- id: utest~AWEMGR.ADDON.SHELL.DirectPrintBypassesHold~1
  covers: dsn~AWEMGR.ADDON.SHELL.CommTraceStreaming~1
  description: |
    Checks that idbg_print_direct() output - used by the comm trace tap -
    bypasses an active hold and therefore never appears inside the output
    block collected by the running command.
```
*/
TEST_F(IdbgOutputTest, DirectPrintBypassesHold)
{
    ::testing::internal::CaptureStdout();
    EXPECT_EQ(idbg_output_hold(m_p), 0);
    idbg_print(m_p, "cpu_info:\n");
    idbg_print_direct(m_p, "TX: [ 0x%08x ]\n", 1);
    idbg_print(m_p, "  - cpu: 1.25\n");
    idbg_print_direct(m_p, "RX: [ 0x%08x ]\n", 2);
    EXPECT_EQ(idbg_output_flush(m_p), 0);

    /* traces first (as they happen), then the command output as one block */
    EXPECT_EQ(::testing::internal::GetCapturedStdout(),
              "TX: [ 0x00000001 ]\n"
              "RX: [ 0x00000002 ]\n"
              "cpu_info:\n"
              "  - cpu: 1.25\n");
}

/**
```yaml
- id: utest~AWEMGR.ADDON.SHELL.NestedHold~1
  covers: dsn~AWEMGR.ADDON.SHELL.AtomicCommandOutput~1
  description: |
    Checks that holds nest: an inner flush keeps collecting and only the
    outermost flush emits the collected output.
```
*/
TEST_F(IdbgOutputTest, NestedHoldsEmitOnOutermostFlush)
{
    ::testing::internal::CaptureStdout();
    EXPECT_EQ(idbg_output_hold(m_p), 0);
    idbg_print(m_p, "outer\n");
    EXPECT_EQ(idbg_output_hold(m_p), 0);
    idbg_print(m_p, "inner\n");
    EXPECT_EQ(idbg_output_flush(m_p), 0);
    EXPECT_EQ(::testing::internal::GetCapturedStdout(), "");

    ::testing::internal::CaptureStdout();
    EXPECT_EQ(idbg_output_flush(m_p), 0);
    EXPECT_EQ(::testing::internal::GetCapturedStdout(), "outer\ninner\n");
}

/**
```yaml
- id: utest~AWEMGR.ADDON.SHELL.FlushWithoutHold~1
  covers: dsn~AWEMGR.ADDON.SHELL.AtomicCommandOutput~1
  description: |
    Checks that a flush without a matching hold is reported as an error and
    leaves normal printing intact.
```
*/
TEST_F(IdbgOutputTest, FlushWithoutHoldReportsError)
{
    EXPECT_EQ(idbg_output_flush(m_p), -1);

    ::testing::internal::CaptureStdout();
    idbg_print(m_p, "still printing\n");
    EXPECT_EQ(::testing::internal::GetCapturedStdout(), "still printing\n");
}

/**
```yaml
- id: utest~AWEMGR.ADDON.SHELL.HoldLongOutput~1
  covers: dsn~AWEMGR.ADDON.SHELL.AtomicCommandOutput~1
  description: |
    Checks that a single print larger than the internal formatting buffer is
    collected completely, i.e. is not truncated.
```
*/
TEST_F(IdbgOutputTest, HoldCollectsLongOutputWithoutTruncation)
{
    const std::string long_line(3000, 'x');

    ::testing::internal::CaptureStdout();
    EXPECT_EQ(idbg_output_hold(m_p), 0);
    idbg_print(m_p, "%s\n", long_line.c_str());
    EXPECT_EQ(idbg_output_flush(m_p), 0);
    EXPECT_EQ(::testing::internal::GetCapturedStdout(), long_line + "\n");
}

/**
```yaml
- id: utest~AWEMGR.ADDON.SHELL.HoldWithAlternativeSink~1
  covers: dsn~AWEMGR.ADDON.SHELL.AtomicCommandOutput~1
  description: |
    Checks that the collected output goes to the sink installed with
    idbg_set_printfct() - as used for socket sessions - and that installing a
    sink while a hold is active does not defeat the collection.
```
*/
TEST_F(IdbgOutputTest, HoldEmitsToInstalledSink)
{
    std::string sink;

    idbg_set_printfct(m_p, print_to_string, &sink);
    EXPECT_EQ(idbg_output_hold(m_p), 0);
    idbg_print(m_p, "traces:\n");

    /* a sink exchange during the hold must not release the collected output */
    std::string sink2;
    idbg_set_printfct(m_p, print_to_string, &sink2);
    idbg_print(m_p, "  state: on\n");
    EXPECT_TRUE(sink.empty());
    EXPECT_TRUE(sink2.empty());

    EXPECT_EQ(idbg_output_flush(m_p), 0);
    EXPECT_TRUE(sink.empty());
    EXPECT_EQ(sink2, "traces:\n  state: on\n");

    idbg_reset_printfct(m_p);
}

/**
```yaml
- id: utest~AWEMGR.ADDON.SHELL.HoldChunkedFlush~1
  covers: dsn~AWEMGR.ADDON.SHELL.AtomicCommandOutput~1
  description: |
    Checks that a collected block larger than the fixed buffers of the output
    sinks is handed over in chunks small enough for them, and arrives
    completely.
```
*/
TEST_F(IdbgOutputTest, HoldFlushesLargeBlockInSinkSizedChunks)
{
    SinkStats   stats;
    std::string expected;

    idbg_set_printfct(m_p, print_to_stats, &stats);
    EXPECT_EQ(idbg_output_hold(m_p), 0);
    for (int i = 0; i < 200; ++i)
    {
        char line[64];
        (void) snprintf(line, sizeof(line), "  - name: entry_%03d\n", i);
        expected.append(line);
        idbg_print(m_p, "%s", line);
    }
    EXPECT_EQ(idbg_output_flush(m_p), 0);
    idbg_reset_printfct(m_p);

    EXPECT_GT(expected.size(), 1024U);   /* larger than MAX_LINE_LENGTH */
    EXPECT_EQ(stats.text, expected);     /* nothing lost */
    EXPECT_GT(stats.calls, 1);           /* was chunked */
    EXPECT_LE(stats.max_call_size, 1024U);
}

/**
```yaml
- id: utest~AWEMGR.ADDON.SHELL.DropDiscardsOutput~1
  covers: dsn~AWEMGR.ADDON.SHELL.OutputSuppression~1
  description: |
    Checks that output printed while idbg_output_disable() is active is
    discarded, and that idbg_output_enable() puts the sink installed before -
    as used for socket sessions - back in place together with its backend.
```
*/
TEST_F(IdbgOutputTest, DropDiscardsOutputAndRestoresSink)
{
    std::string sink;

    idbg_set_printfct(m_p, print_to_string, &sink);

    EXPECT_EQ(idbg_output_disable(m_p), 0);
    idbg_print(m_p, "repeated command output\n");
    EXPECT_TRUE(sink.empty()) << sink;

    EXPECT_EQ(idbg_output_enable(m_p), 0);
    idbg_print(m_p, "repeat:\n");
    EXPECT_EQ(sink, "repeat:\n");

    idbg_reset_printfct(m_p);
}

/**
```yaml
- id: utest~AWEMGR.ADDON.SHELL.NestedDrop~1
  covers: dsn~AWEMGR.ADDON.SHELL.OutputSuppression~1
  description: |
    Checks that dropping the output nests - as happens when a command that
    drops the output repeats another command doing the same: the output stays
    dropped until the outermost enable, which restores the real sink instead
    of the discarding one.
```
*/
TEST_F(IdbgOutputTest, NestedDropRestoresOnOutermostEnable)
{
    std::string sink;

    idbg_set_printfct(m_p, print_to_string, &sink);

    EXPECT_EQ(idbg_output_disable(m_p), 0);
    EXPECT_EQ(idbg_output_disable(m_p), 0);
    idbg_print(m_p, "inner\n");
    EXPECT_EQ(idbg_output_enable(m_p), 0);

    /* still inside the outer drop */
    idbg_print(m_p, "outer\n");
    EXPECT_TRUE(sink.empty()) << sink;

    EXPECT_EQ(idbg_output_enable(m_p), 0);
    idbg_print(m_p, "back\n");
    EXPECT_EQ(sink, "back\n");

    idbg_reset_printfct(m_p);
}

/**
```yaml
- id: utest~AWEMGR.ADDON.SHELL.EnableWithoutDrop~1
  covers: dsn~AWEMGR.ADDON.SHELL.OutputSuppression~1
  description: |
    Checks that enabling the output without a preceding disable is reported as
    an error and leaves the installed output sink untouched.
```
*/
TEST_F(IdbgOutputTest, EnableWithoutDropReportsError)
{
    std::string sink;

    idbg_set_printfct(m_p, print_to_string, &sink);

    EXPECT_EQ(idbg_output_enable(m_p), -1);

    idbg_print(m_p, "still printing\n");
    EXPECT_EQ(sink, "still printing\n");

    idbg_reset_printfct(m_p);
}

/**
```yaml
- id: utest~AWEMGR.ADDON.SHELL.DropInsideHold~1
  covers: dsn~AWEMGR.ADDON.SHELL.OutputSuppression~1
  description: |
    Checks that dropping the output inside an active hold keeps the collection
    intact: only what is printed outside the drop is emitted by the flush.
```
*/
TEST_F(IdbgOutputTest, DropInsideHoldKeepsCollectedOutput)
{
    ::testing::internal::CaptureStdout();
    EXPECT_EQ(idbg_output_hold(m_p), 0);
    idbg_print(m_p, "repeat:\n");

    EXPECT_EQ(idbg_output_disable(m_p), 0);
    idbg_print(m_p, "repeated command output\n");
    EXPECT_EQ(idbg_output_enable(m_p), 0);

    idbg_print(m_p, "  calls: 42\n");
    EXPECT_EQ(idbg_output_flush(m_p), 0);
    EXPECT_EQ(::testing::internal::GetCapturedStdout(), "repeat:\n  calls: 42\n");
}
