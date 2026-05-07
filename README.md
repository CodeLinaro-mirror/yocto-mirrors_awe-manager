# AWE-Manager

AWE-Manager is a C library that acts as a high-level control plane for DSP Audio Weaver (AWE) instances, connecting a host application to an AWECore engine. It handles design loading, control, events, and configuration, abstracting the transport layer (TCP socket or shared memory) between the host and the AWECore engine.

## Versioning outside of GIT workspaces

AWE-Manager version is directly derived from GIT.

In case the workspace for the AWE-Manager compilation is outside GIT control, a `VERSION` file can be used to set the build version **manually**.

In this case please create a `VERSION` file (on workspace toplevel) with the following content:

```sh
# this should be the version you downloaded - don't use own versions
AWEMGR_VERSION=0.9.0
# this is a label which is added behind the "official" version
# when 'const char *awemgr_get_version()' is used
AWEMGR_CUSTOM_VERSION=patch-0.1
```

## Preparing For (binary/source) Distribution

To build the distribution package, run the following commands from the repository root.

```sh
cmake --preset distribution
cmake --build --preset distribution --target package package_source --parallel $(nproc)
```

From a Windows prompt:

```sh
cmake --preset windows-distribution
cmake --build --preset windows-distribution --config Release --target package
// or:
cmake --build --preset windows-distribution --config Debug
```

Your package will be under the `build/Windows/distribution` directory. Note that while the cmake preset defines default values for certain cache variables, you can also override/augment the configuration by passing additional `-D` arguments during the configuration step. E.g. `cmake --preset distribution -DCMAKE_TOOLCHAIN_FILE=/path/to/toolchain.cmake -DCMAKE_BUILD_TYPE=Debug`.


### Build Configuration

The shown values (OFF/ON) mention the non-default values!

- `-D AWEMGR_BUILD_TESTS=OFF` - disable building of test code; when it is ON then:
    - `-D AWE_AWC_BUILD_TESTS=OFF` - disables aweAWC component tests
    - `-D AWE_COMM_BUILD_TESTS=OFF` - disables aweCOMM component tests
    - `-D AWE_CMD_BUILD_TESTS=OFF` - disables aweCMD component tests
    - `-D AWE_OSAL_BUILD_TESTS=OFF` - disables aweOSAL component tests
    - `-D AWE_LOGGING_BUILD_TESTS=OFF` - disables aweLogging component tests
    - `-D AWE_LOGGING_AWEQ_MOCK=ON` - in case logging via AWE-Q is enabled, replace it with a mocking implementation
    - `-D AWEMGR_TESTS_COVERAGE=ON` - enables GCOV based code coverage for Linux x64 builds of AWE Manager
- `-D AWEMGR_STATIC_ANALYSIS=ON` - Enable targets for running static analysis.
- `-D AWEMGR_BUILD_ADDONS=OFF` - disable building of all additional targets, like e.g. AWEMgr-Shell; when it is ON then:
    - `-D IDBG_HAVE_ISOCLINE=OFF` - disables command completion in AWEMgr-Shell
    - `-D AWE_AWC_BUILD_EXAMPLES=OFF` - disable building AWC example program
- `-D AWEMGR_BUILD_SHARED_LIB=ON` - build dynamic library instead of static


## Preparation Steps for Testing/Documentation/Reporting

Several Python related tools are used. Create local Python environment first.

- install UV tool - https://docs.astral.sh/uv/getting-started/installation/
- `uv venv`
- then activate environment (follow instructions from former command)
- `uv pip install -r requirements.txt`

## Development Within Docker Containers

This project uses some pre-configured Docker set ups, stored in `.devcontainer` subdirectory.

Either open the workspace (this directory) in VSCode, which will suggest re-opening the workspace in one of the docker containers, or activate and use a docker image on the command line.

Shown here for the AWE-Q development container:

- `npm install -g @devcontainers/cli` - for devcontainer tool (execute only once)
- `devcontainer up --workspace-folder . --config .devcontainer/qualcomm-dev/devcontainer.json` - bring up container
- `devcontainer  exec --workspace-folder . --config .devcontainer/qualcomm-dev/devcontainer.json /bin/bash` - execute it

Note: when running on WSL don't forget to potentially start Docker-Desktop on Windows first.

## Running Tests

### GTest

The test scripts are only fully supported on Linux x64, see below for how Windows GTests are performed.

**Linux/WSL:**

- perform build steps, using the `testing` preset (see above, replace `distribution` with `testing`)
- in top directory: `python -m awe_manager.tests.run_ctest_with_aweserver --cwd build/Linux/testing results.xml`
    - this script starts several (python) socket servers and a Linux based app including AWECore library
    - ctest is started as well, with AWE-Manager then connecting to test scripts and AWECore via socket backend
    - NOTE: in case testing is not stably exexcuted, the system (Linux) may not correctly release socket port addresses and only completely removes them after some grace period; check with e.g. `netstat -a | grep 1501` if there are dangling sockets around; this command should not return any LISTEN or TIME_WAIT statements.
- Manual steps to run tests with more granular control (those tests work on PC (WSL) only):
    - in terminal 1 - in top directory: `./awe_manager/tests/bin/linux_x86-64/LinuxApp -bsize:48`
    - in terminal 2 - in top directory: `python -m awe_manager.tests.event_socket_simulator`
    - in other terminal: in `build/Linux/testing` dir run `ctest` possibly with `--rerun-failed --output-on-failure` or other parameters
    - or change into respective sub-build dirs and execute directly, e.g., `cd awe_manager/tests/`, `./awe_mgr_test --gtest_list_tests` or `./awe_mgr_test --gtest_filter="*PresetLoad"`

**Windows:**

You will still need WSL to run the test cases under Windows.

Perform the manual steps mentioned above in WSL, i.e. start LinuxApp and event socket simulator.

In a Windows terminal, compile Windows code including test cases using the `windows-testing` preset (do not use the distribution preset):

- `cmake --preset windows-testing`
- `cmake --build --preset windows-testing --config Debug`

Run test cases from the build directory with `ctest`, like or use preset selection:

- `ctest --preset windows-testing -R AWOSALThreadTests --output-on-failure`

The software on Windows will connect to the LinuxApp and event simulator running on WSL.

### Stress Test

- Perform build steps (see above)

- Manual steps to run tests with more granular control:
- Start `LinuxApp` in Background: `./awe_manager/tests/bin/linux_x86-64/LinuxApp -bsize:48 &` or start it in another terminal
- Start  `mgr_stress_test` with `./awe_manager/tests/stress_test/mgr_stress_test`
- The test runs for about 10 seconds by default, you can control the duration with command line argumenent  `-duration:` while launching the stress test. e.g. to run test for 1 min, `./awe_manager/tests/stress_test/mgr_stress_test -duration:60`


## Documentation

Documentation is maintained as Markdown content in doc folders. This documentation includes:

- design documentation - how is AWE-Manager structured etc
- requirements - a list of (software) requirements is maintained as part of the documentation
- API documentation - how is AWE-Manager supposed to be used

To view or to render the documentation mkdocs and doxygen tool is used.
The tools are (partly) installed into a Python environment first.

**NOTE:** It is assumed that DSPC devpi access is configured (to install one specific Python dependency). In case this access is not configured or possible, please remove the entry `req-tracer` from `requirements.txt` file. The documentation generation will just work fine, except it does not contain a requirement traceability report - a report showing which (customer) requirement was implemented by which test case.

Preparation in WSL/Linux:

- Have Python environment enabled (see above)
- `sudo apt install doxygen`

To start rendering and viewing locally in a web browser application:

- `mkdocs serve` - serves documentation under http://127.0.0.1:8000

To create a distributable static-HTML folder:

- `mkdocs build --site-dir dist/documentation`
- `ENABLE_PDF_EXPORT=1 mkdocs build --site-dir dist/documentation` - to also include a PDF version


## Quality Reports

Those reports use tools available only on Linux for the moment.

**Cyclomatic Complexity**:

- Have Python environment enabled (see above)

- From the toplevel directory run:

```sh
./scripts/calculate_cyclomatic_complexity.sh -s awe_manager -o reports/cyclomatic_complexity -n "awemanager-cyclomatic-complexity"
```

**Code Coverage**:

The `testing` preset also generates code coverage data to be used with [lcov](https://github.com/linux-test-project/lcov).

Install `lcov` on WSL/Linux first - this will also install the `genhtml` to render HTML output too:

```sh
sudo apt install lcov
```

From toplevel directory, first run the unit tests and collect coverage reports:

```sh
# possibly remove ./build directory first to start from a clean slate
cmake --preset testing
cmake --build --preset testing --target coverage-html
cmake --build --preset testing --target coverage  # use this if only xml output is needed
```

Reports are generated in the build directory, but can be "installed" to output directories.

```sh
# XML reports Junit/lcov
cmake --install build/Linux/testing --prefix reports --component Reports

# HTML report
cmake --install build/Linux/testing --prefix reports/lcov-html --component HtmlReports
```

**Requirement Traceability**

The requirements and test cases are traced and reported via a page in the documentation automatically.
To get the report in a different format, please run either:

- `(venv) req-tracer  --config scripts/reqtracing-cfg.yml`
- `(venv) req-tracer  --config scripts/reqtracing-cfg-graph.yml`

Check for errors on the command line to spot incorrectly formatted "SpecItem-comments".
Read req-tracer documentation for more details.

## Publication

To publish the source to the public DSP Concepts repository for AWE-Manager execute:

- `./scripts/publish-tag.sh <tag>`

## Addons and Applications

An addon is an optional part of AWE-Manager. They can be enabled or disabled by compile flags, like AWEMGR_BUILD_ADDONS.

Applications are provided as example code and are not meant for production (yet).

### Addon - AWE Manager Shell

The shell addon provides an (interactive) text based interface to test and validate AWE-Manager's API.

This access is also suitable to support automated test cases using AWE-Manager.

This addon consists of:

* *shell-server* - as part of the AWE-Manager API running on the target, this is used when AWE-Manager is included by a HLOS system service
* *shell-client* - a Python script connecting to the *shell-server*. This provides a user friendly interface with tab-completion and color coding. The client is typically used on PC and connects to the server running on the target.
* *shell-executable* - this is the former AWEmgr-Shell example. This includes AWE-Manager library directly. It can be used when the *shell-server* is **not** up and running as it may interfere with the event communication system.

See also [README-awemgrshell.md](awe_manager/apps/awemgr_shell/README-awemgrshell.md) for details.

### Addon - Server Tuning Socket

The addon provides a tuning server socket implementation. This socket is (typically) used by AWE Designer or tuning tools from PC. AWE tuning commands are received and passed on to AWE-Manager's API.

### Application - AWE-Manager Service

This is an example for how AWE-Manager API (and its addons) can be used in an application suitable to be executed in a Linux systemd environment.

## Cross Compilation

### AARCH64

This is a generic ARM cross-compilation, not targeted for a specific target.
It works well under WSL (or on Raspi) using the socket backend for the communication to AWE audio processing.

Preparation in WSL/Linux:

- `sudo apt install gcc make gcc-aarch64-linux-gnu g++-aarch64-linux-gnu binutils-aarch64-linux-gnu`
- `sudo apt install qemu-user qemu-user-static`

Then configure and compile:

```sh
cmake --preset aarm64-cross
cmake --build --preset aarm64-cross --target package --parallel $(nproc)
```

You can then even run the applications in WSL:

- `qemu-aarch64 -L /usr/aarch64-linux-gnu ./build/Linux/aarm64-cross/awe_manager/apps/awemgr_shell/awemgr_shell`
- `qemu-aarch64 -L /usr/aarch64-linux-gnu ./build/Linux/aarm64-cross/awe_manager/tests/awe_mgr_test --gtest_filter=AweMgrTestEvents.AsyncTestEventCallbacks` - note that this TC fails most of the time due to different timing behavior under QEMU
- or even the complete GTest suite: `ctest` (ie. `/usr/bin/ctest` !) will pick up the AARCH executables and execute them automatically in QEMU


### AWE-Q

To compile AWE-Manager specifically for AWE-Q you will need external libraries included in the source tree. Inside DSP Concepts build environment this is guaranteed. This compilation setup is not tested anywhere else.

Please note that this compilation here was based on Lemans precompiled libraries, but they should be compatible with Nordy systems.

Prerequisits for local build:

- have Docker installed
- have access to DSP Concepts build images
- have docker image started (see above)

To compile locally inside the container:

- `cmake --preset aweq-build`
- `cmake --build --preset aweq-build --parallel $(nproc)`
- `cmake --install build/Linux/aweq-build --prefix dist/Linux/aweq`

Inside the distribution folder you will find AWE-Manager library and test executables.
You can side-load these onto an AWE-Q target.

Example commands:

- `adb push dist\Linux\aweq\ /tmp`  - push complete AWE-Manager install folder
- `adb shell chmod +x "/tmp/aweq/bin/*"` - make all binaries executable
- `# ./bin/awemgr_shell  -awc /etc/awc_index.txt` - hook to "system config" (command issued in an ADB shell)


## Sample Designs Included

Some AWE Designer test signal flows and their target configuration files are included.

- [SetGet](awe_manager/tests/data/designs/set_get/)
- [Events](awe_manager/tests/data/designs/events/)
- [SubCanvas](awe_manager/tests/data/designs/subcanvas/)

They have been generated with AWE-Target Configurator.

A batch script is used to upgrade those target files.

- change into `./awe_manager/tests/data/designs`
- run `./run_awcgen_all.cmd`
