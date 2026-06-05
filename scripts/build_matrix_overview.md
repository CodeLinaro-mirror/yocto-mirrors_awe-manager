# AWE Manager — Build Matrix Overview

## Build Matrix — All PASSED ✅

### Available CMake flags

| Flag | Default | Values | Description |
|------|---------|--------|-------------|
| `AWEMGR_AWECORE_CONNECTION` | `SOCKET` | `SOCKET`, `CSHMEM` | Communication backend to AWECore |
| `AWEMGR_LOGGING` | `STDIO` | `STDIO`, `SYSLOG`, `AWEQ` | Logging/tracing backend |
| `AWEMGR_BUILD_SHARED_LIB` | `OFF` | `ON`/`OFF` | Build `libawe_manager.so` instead of `.a` |
| `AWEMGR_BUILD_SINGLE_LIB` | `OFF` | `ON`/`OFF` | Bundle all component archives into one `libawe_manager.a` |
| `AWEMGR_BUILD_ADDONS` | `ON` | `ON`/`OFF` | Build shell, tuning server, and related addons |
| `AWEMGR_BUILD_SHELL` | `ON` | `ON`/`OFF` | Build `libawemgr_shell_lib.a` + `awemgr_shell` binary |
| `AWEMGR_BUILD_TUNING_SERVER` | `ON` | `ON`/`OFF` | Build `libawemgr_tuning_server_lib.a` |
| `AWEMGR_BUILD_APPS` | `ON` | `ON`/`OFF` | Build application programs (service, examples) |
| `AWEMGR_BUILD_SERVICE` | `ON` | `ON`/`OFF` | Build `awemgr_service` system service app |
| `AWEMGR_BUILD_TESTS` | `ON` | `ON`/`OFF` | Build unit tests (pulls in googletest) |
| `AWEMGR_ENABLE_ALL_WARNINGS` | `OFF` | `ON`/`OFF` | Compile with `-Wall` |
| `AWEMGR_ENABLE_WARNINGS_ARE_ERRORS` | `OFF` | `ON`/`OFF` | Compile with `-Wall -Werror` |

### Installed artifacts per combination

> All combinations install the 6 public headers (`awe_manager.h`, `awemgr_classinfo.h`,
> `awemgr_cpuinfo.h`, `awemgr_heapinfo.h`, `awemgr_layoutinfo.h`, `awemgr_targetinfo.h`)
> plus `include/externals/Errors.h` and `share/doc/awe_manager/README.md`.

---

#### 1. `combo_default`
**Flags:** `TESTS=ON`, `ADDONS=ON`, `SHELL=ON`, `TUNING_SERVER=ON`, `APPS=ON`, `SERVICE=ON`, `SOCKET`, `STDIO`, `Debug`, `-Werror`

| Destination | Files |
|-------------|-------|
| `lib/` | `libawe_manager.a`, `libawemgr_shell_lib.a`, `libawemgr_tuning_server_lib.a`, `libidbg_lib.a` |
| `bin/` | `awemgr_service`, `awemgr_shell`, `awemgr_minimal`, `awemgr_events`, `awemgr_event`, `mgr_stress_test` |
| `include/` | 6 public headers + `externals/Errors.h` |
| `share/doc/` | `README.md`, `README-awemgrshell.md`, `awemgr.service` |
| `share/awe_manager/data/` | Test design data (AWC/AWB files for passthrough, set_get, subcanvas, events, dtmf_gen, minimal designs) |

---

#### 2. `combo_no_addons`
**Flags:** `TESTS=OFF`, `ADDONS=OFF`, `APPS=OFF`, `SOCKET`, `STDIO`, `Debug`, `-Werror`

Minimal library-only install.

| Destination | Files |
|-------------|-------|
| `lib/` | `libawe_manager.a` |
| `bin/` | — |
| `include/` | 6 public headers + `externals/Errors.h` |
| `share/doc/` | `README.md` |

---

#### 3. `combo_no_apps`
**Flags:** `TESTS=OFF`, `ADDONS=ON`, `SHELL=ON`, `TUNING_SERVER=ON`, `APPS=OFF`, `SOCKET`, `STDIO`, `Debug`, `-Werror`

Full addon stack but no application programs (service, examples).

| Destination | Files |
|-------------|-------|
| `lib/` | `libawe_manager.a`, `libawemgr_shell_lib.a`, `libawemgr_tuning_server_lib.a`, `libidbg_lib.a` |
| `bin/` | — |
| `include/` | 6 public headers + `externals/Errors.h` |
| `share/doc/` | `README.md`, `README-awemgrshell.md` |

---

#### 4. `combo_no_shell`
**Flags:** `TESTS=OFF`, `ADDONS=ON`, `SHELL=OFF`, `TUNING_SERVER=ON`, `APPS=ON`, `SERVICE=ON`, `SOCKET`, `STDIO`, `Debug`, `-Werror`

No shell library or interactive shell binary; tuning server and applications still built.

| Destination | Files |
|-------------|-------|
| `lib/` | `libawe_manager.a`, `libawemgr_tuning_server_lib.a` |
| `bin/` | `awemgr_service`, `awemgr_minimal`, `awemgr_events`, `awemgr_event` |
| `include/` | 6 public headers + `externals/Errors.h` |
| `share/doc/` | `README.md`, `awemgr.service` |

---

#### 5. `combo_no_tuning`
**Flags:** `TESTS=OFF`, `ADDONS=ON`, `SHELL=ON`, `TUNING_SERVER=OFF`, `APPS=ON`, `SERVICE=ON`, `SOCKET`, `STDIO`, `Debug`, `-Werror`

No AWE tuning socket bridge; shell and applications still built.

| Destination | Files |
|-------------|-------|
| `lib/` | `libawe_manager.a`, `libawemgr_shell_lib.a`, `libidbg_lib.a` |
| `bin/` | `awemgr_service`, `awemgr_shell`, `awemgr_minimal`, `awemgr_events`, `awemgr_event` |
| `include/` | 6 public headers + `externals/Errors.h` |
| `share/doc/` | `README.md`, `README-awemgrshell.md`, `awemgr.service` |

---

#### 6. `combo_single_lib`
**Flags:** `TESTS=OFF`, `ADDONS=ON`, `SHELL=ON`, `TUNING_SERVER=ON`, `APPS=ON`, `SERVICE=ON`, `SINGLE_LIB=ON`, `SOCKET`, `STDIO`, `Debug`, `-Werror`

All component archives (`awe_config_lib`, `awe_comm_lib`, `awe_cmd_lib`, `awe_awc`, `awosal_lib`, `awe_manager_lib`) bundled into a single `libawe_manager.a`.

| Destination | Files |
|-------------|-------|
| `lib/` | `libawe_manager.a` *(all components bundled)*, `libawemgr_shell_lib.a`, `libawemgr_tuning_server_lib.a`, `libidbg_lib.a` |
| `bin/` | `awemgr_service`, `awemgr_shell`, `awemgr_minimal`, `awemgr_events`, `awemgr_event` |
| `include/` | 6 public headers + `externals/Errors.h` |
| `share/doc/` | `README.md`, `README-awemgrshell.md`, `awemgr.service` |

---

#### 7. `combo_shared_lib`
**Flags:** `TESTS=OFF`, `ADDONS=ON`, `SHELL=ON`, `TUNING_SERVER=ON`, `APPS=ON`, `SERVICE=ON`, `SHARED_LIB=ON`, `SOCKET`, `STDIO`, `Debug`, `-Werror`

`awe_manager` built as a shared library.

| Destination | Files |
|-------------|-------|
| `lib/` | `libawe_manager.so`, `libawemgr_shell_lib.a`, `libawemgr_tuning_server_lib.a`, `libidbg_lib.a` |
| `bin/` | `awemgr_service`, `awemgr_shell`, `awemgr_minimal`, `awemgr_events`, `awemgr_event` |
| `include/` | 6 public headers + `externals/Errors.h` |
| `share/doc/` | `README.md`, `README-awemgrshell.md`, `awemgr.service` |

---

#### 8. `combo_syslog`
**Flags:** `TESTS=OFF`, `ADDONS=ON`, `SHELL=ON`, `TUNING_SERVER=ON`, `APPS=ON`, `SERVICE=ON`, `SOCKET`, `LOGGING=SYSLOG`, `Debug`, `-Werror`

Identical installed artifacts to the default combination; only compile-time preprocessor difference (`-DAWEMGR_LOGGING_SYSLOG`).

| Destination | Files |
|-------------|-------|
| `lib/` | `libawe_manager.a`, `libawemgr_shell_lib.a`, `libawemgr_tuning_server_lib.a`, `libidbg_lib.a` |
| `bin/` | `awemgr_service`, `awemgr_shell`, `awemgr_minimal`, `awemgr_events`, `awemgr_event` |
| `include/` | 6 public headers + `externals/Errors.h` |
| `share/doc/` | `README.md`, `README-awemgrshell.md`, `awemgr.service` |

---

#### 9. `combo_distribution`
**Flags:** `TESTS=OFF`, `ADDONS=ON`, `SHELL=ON`, `TUNING_SERVER=ON`, `APPS=ON`, `SERVICE=ON`, `SOCKET`, `STDIO`, `Release`, `-Werror`

Release-optimised build matching the `distribution` CMake preset intent.

| Destination | Files |
|-------------|-------|
| `lib/` | `libawe_manager.a`, `libawemgr_shell_lib.a`, `libawemgr_tuning_server_lib.a`, `libidbg_lib.a` |
| `bin/` | `awemgr_service`, `awemgr_shell`, `awemgr_minimal`, `awemgr_events`, `awemgr_event` |
| `include/` | 6 public headers + `externals/Errors.h` |
| `share/doc/` | `README.md`, `README-awemgrshell.md`, `awemgr.service` |

---

## Key Observations

- **`AWEMGR_BUILD_ADDONS=OFF`** gives the minimal library-only install (just `libawe_manager.a` + headers).
- **`AWEMGR_BUILD_SHELL=OFF`** drops `libawemgr_shell_lib.a`, `libidbg_lib.a`, `awemgr_shell`, and `README-awemgrshell.md`.
- **`AWEMGR_BUILD_TUNING_SERVER=OFF`** drops `libawemgr_tuning_server_lib.a`.
- **`AWEMGR_BUILD_APPS=OFF`** drops all application programs (`awemgr_service`, `awemgr_minimal`, `awemgr_events`, `awemgr_event`) and `awemgr.service`.
- **`AWEMGR_BUILD_SINGLE_LIB=ON`** bundles all component `.a` files into one `libawe_manager.a` (requires `-lm` propagation fix).
- **`AWEMGR_BUILD_SHARED_LIB=ON`** produces `libawe_manager.so` instead of `.a`.
- **`AWEMGR_LOGGING=SYSLOG`** vs `STDIO` produces identical installed artifacts — only a compile-time preprocessor flag differs.
- **`AWEMGR_BUILD_TESTS=ON`** additionally installs `bin/mgr_stress_test` and `share/awe_manager/data/designs/` (test AWC/AWB design data).
- `AWEMGR_AWECORE_CONNECTION=CSHMEM` and `AWEMGR_BUILD_AWEQ=ON` require a Snapdragon-specific toolchain and were not tested on this Linux x86_64 host.
