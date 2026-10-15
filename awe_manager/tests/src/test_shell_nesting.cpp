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
 * @file test_shell_nesting.cpp
 * @brief Tests for the nesting limits of the AWEMgr-Shell add-on commands.
 *
 * "repeat" and "script" dispatch further command lines on the same idbg handle
 * and can therefore end up invoking themselves. The commands used here are
 * "version" and "script", which do not talk to AWE Core - no AWE server is
 * needed.
 */

#include "awemgr_shell.h"

#include <gtest/gtest.h>

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

namespace {

class ShellNestingTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_shell_p = awemgr_shell_create(nullptr, nullptr);
        ASSERT_NE(m_shell_p, nullptr);
    }
    void TearDown() override
    {
        awemgr_shell_destroy(m_shell_p);
        for (const std::string &name : m_files)
            (void) remove(name.c_str());
    }

    /** Run *cmd* and return everything it printed to stdout. */
    std::string run(const char *cmd)
    {
        ::testing::internal::CaptureStdout();
        (void) awemgr_shell_execute(m_shell_p, "%s", cmd);
        return ::testing::internal::GetCapturedStdout();
    }

    /** Write *content* to a script file *name*, removed again on teardown. */
    void write_script(const std::string &name, const std::string &content)
    {
        FILE *fp = fopen(name.c_str(), "w");
        ASSERT_NE(fp, nullptr) << name;
        ASSERT_GT(fputs(content.c_str(), fp), 0) << name;
        (void) fclose(fp);
        m_files.push_back(name);
    }

    awemgr_shell_ctx        *m_shell_p = nullptr;
    std::vector<std::string> m_files;
};

} // namespace

/**
```yaml
- id: utest~AWEMGR.ADDON.SHELL.RepeatNoNesting~1
  covers: dsn~AWEMGR.ADDON.SHELL.CommandNestingLimit~1
  description: |
    Checks that "repeat" refuses to run inside another "repeat" instead of
    counting the inner runs as executions of the repeated command. The outer
    run keeps working and still reports its summary, and a "repeat" that is not
    nested is not affected.
```
*/
TEST_F(ShellNestingTest, RepeatRefusesToBeNestedInARepeat)
{
    /* -verbose keeps the output of the inner run, so its rejection is visible */
    std::string out = run("repeat -cmd \"repeat -cmd version -sec 1\" -sec 1 -verbose");

    EXPECT_NE(out.find("repeat cannot be nested"), std::string::npos) << out;

    /* the outer run is not affected and still reports its summary */
    EXPECT_NE(out.find("repeat:"), std::string::npos) << out;

    /* the guard is released again, so the next repeat is not rejected */
    out = run("repeat -cmd version -sec 1");
    EXPECT_NE(out.find("repeat:"), std::string::npos) << out;
    EXPECT_EQ(out.find("cannot be nested"), std::string::npos) << out;
}

/**
```yaml
- id: utest~AWEMGR.ADDON.SHELL.RepeatNoNestingViaScript~1
  covers: dsn~AWEMGR.ADDON.SHELL.CommandNestingLimit~1
  description: |
    Checks that a "repeat" reached through a script file executed by an outer
    "repeat" is rejected as well, while a "repeat" executed from a script that
    is not itself repeated stays allowed - running a repeat from a script is a
    regular use case.
```
*/
TEST_F(ShellNestingTest, RepeatRefusesToBeNestedThroughAScript)
{
    write_script("test_nesting_repeat.txt", "repeat -cmd version -sec 1\n");

    /* a repeat inside a script is allowed as long as nothing repeats the script */
    std::string out = run("script -file test_nesting_repeat.txt");
    EXPECT_NE(out.find("repeat:"), std::string::npos) << out;
    EXPECT_EQ(out.find("cannot be nested"), std::string::npos) << out;

    /* but repeating that script makes the repeat a nested one */
    out = run("repeat -cmd \"script -file test_nesting_repeat.txt\" -sec 1 -verbose");
    EXPECT_NE(out.find("repeat cannot be nested"), std::string::npos) << out;
}

/**
```yaml
- id: utest~AWEMGR.ADDON.SHELL.ScriptNestingLimit~1
  covers: dsn~AWEMGR.ADDON.SHELL.CommandNestingLimit~1
  description: |
    Checks that a script file which includes itself is stopped with an error
    once the nesting limit is reached, instead of recursing until the stack is
    exhausted, and that a script including another script - the regular use
    case - still works.
```
*/
TEST_F(ShellNestingTest, ScriptStopsRecursiveInclude)
{
    /* the regular case: one script includes another one */
    write_script("test_nesting_inner.txt", "version\n");
    write_script("test_nesting_outer.txt", "script -file test_nesting_inner.txt\n");

    std::string out = run("script -file test_nesting_outer.txt");
    EXPECT_NE(out.find("awemgr_version:"), std::string::npos) << out;
    EXPECT_EQ(out.find("recursive include"), std::string::npos) << out;

    /* the fatal case: a script which includes itself */
    write_script("test_nesting_self.txt", "script -file test_nesting_self.txt\n");

    out = run("script -file test_nesting_self.txt");
    EXPECT_NE(out.find("recursive include"), std::string::npos) << out;

    /* the shell survived it and the guard is released again */
    EXPECT_NE(run("version").find("awemgr_version:"), std::string::npos);
    EXPECT_NE(run("script -file test_nesting_outer.txt").find("awemgr_version:"),
              std::string::npos);
}
