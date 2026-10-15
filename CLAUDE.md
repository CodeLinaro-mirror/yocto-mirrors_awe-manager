# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

C/C++ library providing an API to control and administer running AWE (Audio Weaver Engine) instances on embedded audio processing platforms. Used by Qualcomm's control service running on PVM (RHL). Additional components are available to provide more interfaces to talk to the API (see Addons). Applications are implemented as examples on how Addons and the library may be used, e.g. as a platform service.

## Build System

CMake 3.8+ with Ninja. All build configurations are in `CMakePresets.json`.

```bash
# Development build (debug + tests + coverage)
cmake --preset testing
cmake --build --preset testing --parallel $(nproc)

# Release build
cmake --preset distribution
cmake --build --preset distribution --target package --parallel $(nproc)

# ARM64 cross-compile
cmake --preset aarm64-cross
cmake --build --preset aarm64-cross --target package --parallel $(nproc)
```

Key CMake options: `AWEMGR_BUILD_TESTS`, `AWEMGR_BUILD_ADDONS`, `AWEMGR_BUILD_SHELL`, `AWEMGR_BUILD_TUNING_SERVER`, `AWEMGR_AWECORE_CONNECTION` (SOCKET|CSHMEM), `AWEMGR_LOGGING` (STDIO|SYSLOG).

## Running Tests

Tests use Google Test (GTest). A running AWE server (LinuxApp) and event socket simulator are required for most tests.

```bash
# Automated (starts servers automatically)
python -m awe_manager.tests.run_ctest_with_aweserver --cwd build/Linux/testing results.xml

# Manual setup (three terminals)
./awe_manager/tests/bin/linux_x86-64/LinuxApp -bsize:48          # Terminal 1
python -m awe_manager.tests.event_socket_simulator                # Terminal 2 (Ctrl+C to stop)
cd build/Linux/testing && ctest                                   # Terminal 3

# Single test
cd build/Linux/testing/awe_manager/tests/
./awe_mgr_test --gtest_filter="*PresetLoad"

# Stress test
./awe_manager/tests/bin/linux_x86-64/LinuxApp -bsize:48 &
./awe_manager/tests/stress_test/mgr_stress_test -duration:60
```

Coverage HTML reports are generated at `build/Linux/testing/lcov-html/` when using the `testing` preset.

## Python Environment

```bash
uv venv
source .venv/bin/activate
uv pip install -r requirements.txt
```

## Documentation

```bash
mkdocs serve          # Serve locally at http://127.0.0.1:8000
mkdocs build --site-dir dist/documentation
ENABLE_PDF_EXPORT=1 mkdocs build --site-dir dist/documentation  # With PDF
```

## Architecture

```
awe_manager/
├── include/          # Public API (awe_manager.h and info headers)
├── src/              # Core manager implementation (awemgr_*.c)
├── components/       # Reusable sub-libraries
│   ├── awe_awc/      # AWC container file management
│   ├── awe_cmd/      # AWE tuning command construction/parsing
│   ├── awe_comm/     # Backend communication (socket/shared memory)
│   ├── awe_config/   # Key-value configuration management
│   ├── awe_osal/     # OS abstraction layer (threads, mutexes, sockets)
│   └── logging/      # Tracing and logging infrastructure
├── addons/           # Optional extensions
│   ├── idbg/         # Interactive debug command completion
│   ├── shell/        # Interactive CLI for AWE Manager
│   └── serversocket/ # Socket-based tuning/control proxy
├── apps/
│   ├── awemgr_shell/ # Standalone shell app
│   ├── service/      # Qualcomm control service
│   └── awemgr_client/# Client library for the service
├── tests/            # GTest unit tests + test data in tests/data/designs/
├── examples/         # minimal/, events/, awemgr_event/
└── docs/             # Design docs (overview, context, structural, behavioral views)
```

Each component is self-contained with `include/`, `src/`, `tests/`, and `docs/` subdirectories.

**Communication backends**: SOCKET (TCP/IP, default, port 15002) or CSHMEM (circular shared memory).
**Event socket**: Port 15010 by default.
**Config keys**: Runtime behavior is controlled via `mgr.*` keys (e.g., `mgr.comm.socket.port`, `mgr.api.log.level`).

## Key Notes

- `shellhandler` is a Python **package** (`shellhandler/__init__.py`). Edits to a `shellhandler.py` file have no effect due to Python package precedence.
- Version is derived from `git describe`; a `VERSION` file provides fallback for non-git builds.
- Static analysis: `cppcheck` with suppressions in `cppcheck_suppressions.txt`.
- Cyclomatic complexity: `scripts/calculate_cyclomatic_complexity.sh`.

## Critical Coding Requirements

### 1. Code Quality - MANDATORY

- please check misra rules if possible
- The codebase has zero issues by default, so any issue is from your changes
- DO NOT commit or consider work complete until tests pass cleanly

### 2. Component Reuse - MANDATORY

Always try to reuse existing components or models

- Use a subagent to search for existing components or models

### 3. Documentation Sync - MANDATORY

Keep documentation documents synchronized with code changes.

- When modifying code, also update associated design and user documents
- Check for related files in awe_manager/docs or in the corresponding docs directory of a component or addon
- Ensure documentation reflects the current implementation
- Update examples in docs if behavior changes

### 4. Changelog Sync - MANDATORY

Keep changelog updated.

- When modifying code, a brief user facing information shall be added to `changelog.md`
- For future unreleased versions propose a release number and use placeholder as release date

### 5. Requirement Sync - MANDATORY

Keep the requirement lists updated.

- When code has been added, derive a requirement to be stored in 99_requirements.md inside the component
- Follow the naming convention of the requirement tracer
- When adding new test cases, add a test case YML description to "cover" a requirement

## Commit & PR Guidelines

### Pre-commit checklist (run before every commit)

- critical: test cases must run ok

### Commit rules

These rules apply to BOTH commit messages AND pull request descriptions

- Messages must contain JIRA ticket number
- Keep messages CONCISE (no walls of text)
- Subject line under 72 chars (no body text unless critical)
- NO emojis
- NO promotional text or links
- NO Co-Authored-By lines

Additional guidelines

- check `git status` before committing to avoid adding temporary/binary files
- never commit to main branch
- All CI checks must pass - run the checklist commands above before committing or creating PR
