# AWE-Manager Component - AWE QC Integration

[![codecov](https://codecov.ops.dspconcepts.com/bb/dspconcepts/qc-audiolite-integration-awemanager/graph/badge.svg?token=4RTFHAAQTV)](https://codecov.ops.dspconcepts.com/bb/dspconcepts/qc-audiolite-integration-awemanager)

The component provides an API to control and administer running AWE instances on the platform.

It is the library called/used by Qualcomm's control service running on PVM (RHL).

## Content

- awe_manager - main component, provides the interface to QC
- awe_awc - component inside awe_manager to convert text->mod/var handle; potentially this dir should become a sub-dir of awe_manager

## Compiling

To build the distribution package, run the following commands from the repository root.

```sh
cmake --preset distribution
cmake --build --preset distribution --target package --parallel $(nproc)
```

From a Windows prompt:

```sh
cmake --preset windows-distribution
cmake --build --preset windows-distribution --config Release --target package
// or:
cmake --build --preset windows-distribution --config Debug
```


Your package will be under the `build/distribution` directory. Note that while the cmake preset defines default values for certain cache variables, you can also override/augment the configuration by passing additional `-D` arguments during the configuration step. E.g. `cmake --preset distribution -DCMAKE_TOOLCHAIN_FILE=/path/to/toolchain.cmake -DCMAKE_BUILD_TYPE=Debug`.

### Build Configuration

The shown values (OFF/ON) mention the non-default values!

- `-D AWEMGR_BUILD_TESTS=OFF` - disable building of test code; when it is ON then:
    - `-D AWE_AWC_BUILD_TESTS=OFF` - specifically disable AWC component tests
    - `-D AWE_CTRL_BUILD_TESTS=OFF` - ditto for aweCTRL component
    - `-D AWE_CMD_BUILD_TESTS=OFF` - ditto for aweCMD component
    - `-D AWE_OSAL_BUILD_TESTS=OFF` - ditto for aweOSAL component
    - `-D AWEMGR_TESTS_COVERAGE=ON` - enables GCOV based code coverage for Linux x64 builds of AWE Manager
- `-D AWEMGR_STATIC_ANALYSIS=ON` - Enable targets for running static analysis.
- `-D AWEMGR_BUILD_ADDONS=OFF` - disable building of all additional targets, like e.g. AWEMgr-Shell; when it is ON then:
    - `-D IDBG_HAVE_ISOCLINE=OFF` - disables command completion in AWEMgr-Shell
    - `-D AWE_AWC_BUILD_EXAMPLES=OFF` - disable building AWC example program
- `-D AWEMGR_BUILD_SHARED_LIB=ON` - build dynamic library instead of static

## Running Tests

**GTest**:

- perform build steps, using the `testing` preset (see above, replace `distribution` with `testing`)
- in top directory: `python -m awe_manager.tests.run_ctest_with_aweserver --cwd build/testing results.xml`
    - NOTE: due to the nature of sockets being possibly torn down ungracefully, the system (Linux) might force you to wait a bit; check with e.g. `netstat -a | grep 1501` if there are dangling sockets around; this command should not return any LISTEN or TIME_WAIT statements.
- Manual steps to run tests with more granular control (those tests work on PC (WSL) only):
    - in terminal 1 - in top directory: `./awe_manager/tests/bin/linux_x86-64/LinuxApp -bsize:48`
    - in terminal 2 - in top directory: `python -m awe_manager.tests.event_socket_simulator`
    - in other terminal: in `_build` dir run `ctest` possibly with `--rerun-failed --output-on-failure` or other parameters
    - or change into respective sub-build dirs and execute directly, e.g., `cd awe_manager/tests/`, `./awe_mgr_test --gtest_list_tests` or `./awe_mgr_test --gtest_filter="*PresetLoad"`

## Stress Test

- Perform build steps (see above)

- Manual steps to run tests with more granular control:
- Start `LinuxApp` in Background: `../awe_manager/tests/bin/linux_x86-64/LinuxApp -bsize:48 &` or start it in another terminal
- Start  `mgr_stress_test` with `./awe_manager/tests/stress_test/mgr_stress_test`
- The test runs for about 10 seconds by default, you can control the duration with command line argumenent  `-duration:` while launching the stress test. e.g. to run test for 1 min, `./awe_manager/tests/stress_test/mgr_stress_test -duration:60`

## Quality Reports

Those reports use tools available only on Linux for the moment. They are the same as used by AWE Core library product development.

**Cyclomatic Complexity**:

In the above mentioned build directory:

- `cd build/testing`
- `../../scripts/calculate_cyclomatic_complexity.sh  -s ../../awe_manager -o cyc_comp -n "awemanager-cyc_comp"`

This will have created a directory `build/testing/cyc_comp` containing the report in HTML and CSV file.

**Code Coverage**:

Run the unit tests and collect coverage reports as follows.

```sh
cmake --build --preset testing --target coverage
```

To generate an HTML report:

```sh
cmake --build --preset testing --target coverage-html
```

Reports are generated in the build tree, but can be "installed" to output directories.

```sh
# XML report
cmake --install build/testing --prefix reports --component Reports

# HTML report
cmake --install build/testing --prefix html --component HtmlReports
```

**Requirement Traceability**

The requirements and test cases are traced and reported via a page in the documentation.
To get the report (a markdown document) updated please run:

- `(venv) req-tracer  --config scripts/reqtracing-cfg-markdown.yml`

Check for errors on the command line to spot incorrectly formatted "SpecItem-comments".
Read req-tracer documentation for more details.

The file `awe_manager/docs/traceability_report.md` gets updated then. Put it under VC.

## Interactive AWE Manager Shell

A small shell program was created to test and validate the concepts of AWE-Manager's API.

See [README-awemgrshell.md](awe_manager/examples/cmdline/README-awemgrshell.md) for details.

## Cross Compilation

### AARCH64

This is a generic ARM cross-compilation, not targeted for a specific target.
It works well under WSL (or on Raspi) using the socket backend for the communication to AWE audio processing.

Preparation in WSL/Linux:

- `sudo apt install gcc make gcc-aarch64-linux-gnu g++-aarch64-linux-gnu binutils-aarch64-linux-gnu`
- `sudo apt install qemu-user qemu-user-static`

Then configure and compile:

- `mkdir _build-aarch64`
- `cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/generic-aarch64-linux.cmake ..`
- `make -j 16 package`

You can then run the applications:

- `qemu-aarch64 -L /usr/aarch64-linux-gnu ./awe_manager/examples/cmdline/awemgr_shell`
- `qemu-aarch64 -L /usr/aarch64-linux-gnu ./awe_manager/tests/awe_mgr_test --gtest_filter=AweMgrTestEvents.AsyncTestEventCallbacks` - note that this TC fails most of the time due to different timing behavior under QEMU
- or even the complete GTest suite: `ctest` (ie. `/usr/bin/ctest` !) will pick up the AARCH executables and execute them automatically in QEMU


## Sample Designs Included

- [DTMF Generator](awe_manager/tests/data/designs/dtmf_gen/)
- [SetGet](awe_manager/tests/data/designs/set_get/)

## Upgrade AWC Files

It is required that AWC-Tooling has been installed. The AWC-Tooling package contains information on how to do that. Basically, it is just installing a few Python packages into the above created Python environment (see Running Tests).

Then...

- change into `./awe_manager/tests/data/designs`
- run `./run_awcgen_all.cmd`
