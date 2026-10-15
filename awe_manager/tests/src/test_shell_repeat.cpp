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
 * @file test_shell_repeat.cpp
 * @brief Tests for the "repeat" command of the AWEMgr-Shell add-on.
 *
 * The command repeats another shell command for a given number of seconds and
 * reports how many executions it managed. The command used for repetition here
 * is "version", which does not talk to AWE Core - no AWE server is needed.
 */

#include "awemgr_shell.h"

#include <gtest/gtest.h>

#include <stdio.h>
#include <stdlib.h>
#include <string>

namespace {

class ShellRepeatTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_shell_p = awemgr_shell_create(nullptr, nullptr);
        ASSERT_NE(m_shell_p, nullptr);
    }
    void TearDown() override { awemgr_shell_destroy(m_shell_p); }

    /** Run *cmd* and return everything it printed to stdout. */
    std::string run(const char *cmd)
    {
        ::testing::internal::CaptureStdout();
        (void) awemgr_shell_execute(m_shell_p, "%s", cmd);
        return ::testing::internal::GetCapturedStdout();
    }

    /** Value of the "  <key>: " entry in the YAML *out*, empty when missing. */
    static std::string yaml_value(const std::string &out, const std::string &key)
    {
        size_t pos = out.find("  " + key + ": ");
        if (pos == std::string::npos)
            return "";
        pos += key.size() + 4U;
        return out.substr(pos, out.find('\n', pos) - pos);
    }

    awemgr_shell_ctx *m_shell_p = nullptr;
};

} // namespace

/**
```yaml
- id: utest~AWEMGR.ADDON.SHELL.RepeatCountsExecutions~2
  covers: dsn~AWEMGR.ADDON.SHELL.RepeatCommand~2
  description: |
    Checks that "repeat" executes the command given with -cmd for the time
    given with -sec and reports the number of executions performed, and that
    the output of the repeated command is suppressed by default so that only
    the summary of the run is printed. The output sink used before the run has
    to be usable afterwards.
```
*/
TEST_F(ShellRepeatTest, RepeatsCommandAndCountsExecutions)
{
    std::string out = run("repeat -cmd version -sec 1");

    ASSERT_NE(out.find("repeat:"), std::string::npos) << out;
    EXPECT_EQ(yaml_value(out, "command"), "\"version\"") << out;

    /* without -verbose the output of the repeated command is dropped */
    EXPECT_EQ(out.find("awemgr_version:"), std::string::npos) << out;

    EXPECT_GT(atol(yaml_value(out, "calls").c_str()), 0) << out;
    EXPECT_GE(atof(yaml_value(out, "duration_sec").c_str()), 1.0) << out;

    /* the sink has to be usable again after the run */
    EXPECT_NE(run("version").find("awemgr_version:"), std::string::npos);
}

/**
```yaml
- id: utest~AWEMGR.ADDON.SHELL.RepeatVerbose~1
  covers: dsn~AWEMGR.ADDON.SHELL.RepeatCommand~2
  description: |
    Checks that "repeat -verbose" prints the output of the repeated command -
    one block per execution - in addition to the summary of the repeat run.
```
*/
TEST_F(ShellRepeatTest, VerbosePrintsOutputOfRepeatedCommand)
{
    std::string out = run("repeat -cmd version -sec 1 -verbose");

    ASSERT_NE(out.find("repeat:"), std::string::npos) << out;
    ASSERT_NE(out.find("awemgr_version:"), std::string::npos) << out;

    long calls = atol(yaml_value(out, "calls").c_str());
    EXPECT_GT(calls, 0) << out;

    /* "version" prints one block per execution */
    size_t printed = 0, pos = 0;
    while ((pos = out.find("awemgr_version:", pos)) != std::string::npos) { printed++; pos++; }
    EXPECT_EQ((long)printed, calls) << out;
}

/**
```yaml
- id: utest~AWEMGR.ADDON.SHELL.RepeatNested~1
  covers: dsn~AWEMGR.ADDON.SHELL.OutputSuppression~1
  description: |
    Checks that a "repeat" run that repeats another "repeat" - which is
    rejected, see AWEMGR.ADDON.SHELL.CommandNestingLimit - leaves the shell
    output intact: the summary of the outer run is printed and the sink still
    works afterwards.
```
*/
TEST_F(ShellRepeatTest, NestedRepeatKeepsOutputSinkUsable)
{
    std::string out = run("repeat -cmd \"repeat -cmd version -sec 1\" -sec 1");

    /* the summary of the outer run is printed although the output was dropped
     * while the rejected inner run was dispatched */
    ASSERT_NE(out.find("repeat:"), std::string::npos) << out;
    EXPECT_EQ(yaml_value(out, "command"), "\"repeat -cmd version -sec 1\"") << out;
    EXPECT_GT(atol(yaml_value(out, "calls").c_str()), 0) << out;

    /* and the sink is not left dropping output */
    EXPECT_NE(run("version").find("awemgr_version:"), std::string::npos);
}

/**
```yaml
- id: utest~AWEMGR.ADDON.SHELL.RepeatThrottle~1
  covers: dsn~AWEMGR.ADDON.SHELL.RepeatThrottle~1
  description: |
    Checks that "repeat -throttle" inserts the requested delay between the
    executions, so that the number of executions stays below the limit the
    delay implies and clearly below an unthrottled run of the same length,
    while the run is still ended by -sec. A throttle of 0 does not delay.
```
*/
TEST_F(ShellRepeatTest, ThrottleLimitsTheExecutionRate)
{
    const long seconds     = 1;
    /* keep this at 1 ms or more: the Windows OSAL maps usleep() to Sleep(ms) */
    const long throttle_us = 50000;

    std::string unthrottled = run("repeat -cmd version -sec 1");
    long unthrottled_calls  = atol(yaml_value(unthrottled, "calls").c_str());
    ASSERT_GT(unthrottled_calls, 0) << unthrottled;

    std::string out = run("repeat -cmd version -sec 1 -throttle 50000");
    ASSERT_NE(out.find("repeat:"), std::string::npos) << out;

    long calls = atol(yaml_value(out, "calls").c_str());
    EXPECT_GT(calls, 0) << out;

    /* The delay is inserted between the executions, so the run cannot do more
     * repetitions than the repeat time allows - the command itself and the
     * scheduling granularity take time on top of the delay. The few calls of
     * tolerance keep the check away from the timer resolution; the run is
     * still orders of magnitude below the unthrottled one checked below. */
    const long max_calls = ((seconds * 1000000L) / throttle_us) + 1L + 4L;
    EXPECT_LE(calls, max_calls) << out;
    EXPECT_LT(calls, unthrottled_calls) << "throttled:\n" << out
                                        << "unthrottled:\n" << unthrottled;

    /* -sec still ends the run; the delay does not stretch it beyond the last
     * repetition. */
    double duration = atof(yaml_value(out, "duration_sec").c_str());
    EXPECT_GE(duration, (double)seconds) << out;
    EXPECT_LT(duration, (double)seconds + 1.0) << out;

    /* -throttle is optional, and 0 means "no delay" */
    out = run("repeat -cmd version -sec 1 -throttle 0");
    EXPECT_GT(atol(yaml_value(out, "calls").c_str()), calls) << out;
}

/**
```yaml
- id: utest~AWEMGR.ADDON.SHELL.RepeatArguments~1
  covers: dsn~AWEMGR.ADDON.SHELL.RepeatCommand~2
  description: |
    Checks that "repeat" passes a quoted command line including its parameters
    to the repeated command, and that a missing -cmd or a non-positive -sec is
    rejected with an error instead of being executed.
```
*/
TEST_F(ShellRepeatTest, RejectsInvalidArgumentsAndForwardsParameters)
{
    /* parameters of the repeated command survive the repetition */
    std::string out = run("repeat -cmd \"echo hello\" -sec 1");
    EXPECT_EQ(yaml_value(out, "command"), "\"echo hello\"") << out;

    /* -cmd is mandatory */
    out = run("repeat -sec 1");
    EXPECT_NE(out.find("error"), std::string::npos) << out;
    EXPECT_EQ(out.find("repeat:"), std::string::npos) << out;

    /* a repeat time of zero or less has no meaning */
    out = run("repeat -cmd version -sec 0");
    EXPECT_NE(out.find("error"), std::string::npos) << out;
    EXPECT_EQ(out.find("repeat:"), std::string::npos) << out;
}

/**
```yaml
- id: utest~AWEMGR.ADDON.SHELL.RepeatCount~1
  covers: dsn~AWEMGR.ADDON.SHELL.RepeatCount~1
  description: |
    Checks that "repeat -count" performs exactly the requested number of
    executions and ends the run afterwards, that it takes precedence over -sec
    instead of waiting for the repeat time to pass, that a count of 0 leaves
    -sec in charge, and that a negative count is rejected with an error.
```
*/
TEST_F(ShellRepeatTest, CountLimitsTheNumberOfExecutions)
{
    /* -count performs exactly the requested number of executions ... */
    std::string out = run("repeat -cmd version -count 5 -verbose");
    ASSERT_NE(out.find("repeat:"), std::string::npos) << out;
    EXPECT_EQ(atol(yaml_value(out, "calls").c_str()), 5) << out;

    /* ... and the repeated command really did run that often */
    size_t printed = 0U;
    size_t pos     = 0U;
    while ((pos = out.find("awemgr_version:", pos)) != std::string::npos) { printed++; pos++; }
    EXPECT_EQ(printed, 5U) << out;

    /* -count takes precedence over -sec: the run ends with the last execution
     * instead of after the repeat time, which would take a minute here */
    out = run("repeat -cmd version -count 3 -sec 60");
    EXPECT_EQ(atol(yaml_value(out, "calls").c_str()), 3) << out;
    EXPECT_LT(atof(yaml_value(out, "duration_sec").c_str()), 10.0) << out;

    /* a single execution is a valid request */
    out = run("repeat -cmd version -count 1");
    EXPECT_EQ(atol(yaml_value(out, "calls").c_str()), 1) << out;

    /* -count is optional, and 0 leaves -sec in charge */
    out = run("repeat -cmd version -sec 1 -count 0");
    ASSERT_NE(out.find("repeat:"), std::string::npos) << out;
    EXPECT_GT(atol(yaml_value(out, "calls").c_str()), 1) << out;
    EXPECT_GE(atof(yaml_value(out, "duration_sec").c_str()), 1.0) << out;

    /* a negative number of executions has no meaning */
    out = run("repeat -cmd version -count -1");
    EXPECT_NE(out.find("error"), std::string::npos) << out;
    EXPECT_EQ(out.find("repeat:"), std::string::npos) << out;
}

/**
```yaml
- id: utest~AWEMGR.ADDON.SHELL.RepeatCountThrottle~1
  covers: dsn~AWEMGR.ADDON.SHELL.RepeatCount~1
  description: |
    Checks that -count and -throttle work together: the requested number of
    executions is performed with the delay inserted between them, and the delay
    is not applied after the last execution, so the run is not stretched by one
    extra delay.
```
*/
TEST_F(ShellRepeatTest, CountAndThrottleCombine)
{
    /* few executions on purpose: the check below bounds the runtime by one
     * delay, so the fewer delays are involved the less their overshoot on a
     * loaded machine matters */
    const long calls       = 3;
    /* keep this at 1 ms or more: the Windows OSAL maps usleep() to Sleep(ms) */
    const long throttle_us = 50000;

    std::string out = run("repeat -cmd version -count 3 -throttle 50000");
    ASSERT_NE(out.find("repeat:"), std::string::npos) << out;
    EXPECT_EQ(atol(yaml_value(out, "calls").c_str()), calls) << out;

    /* the delay sits between the executions, so it is applied one time less
     * than the number of executions */
    const double min_sec = (double)((calls - 1L) * throttle_us) / 1000000.0;
    double duration = atof(yaml_value(out, "duration_sec").c_str());
    EXPECT_GE(duration, min_sec) << out;

    /* and it is not applied after the last one, which would add one delay */
    EXPECT_LT(duration, min_sec + ((double)throttle_us / 1000000.0)) << out;
}
