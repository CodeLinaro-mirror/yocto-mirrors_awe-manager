# Overview

{{name.awe_mgr}} is a component whose primary purpose is to allow applications the control of a running {{name.awe_sf}} at runtime.

It ...

- ... is a component used by {{name.controller}} inside an audio control service - this is the "calling application" from {{name.awe_mgr}}'s point of view
- ... provides an interface to {{name.controller}} to ...

    - use name based access to modules and variables inside {{name.awe_sf}} - it internally computes appropriate {{name.awe_tune_cmd}} messages (named access)
    - pass through correctly structured {{name.awe_tune_cmd}} packets from an application (raw access)
    - load and unload a specific {{name.awe_sf}} (AWB file)
    - apply a batch of value changes (applying so-called preset AWB files)
    - informs {{name.controller}} about asynchronuously occurring events in the {{name.awe_sf}} and in  {{name.awe_host}} (DSP/BSP code) via callback functions

- ... is configured by information from a specific file and does not require re-compilation when {{name.awe_sf}} is updated
- ... communicates (sends) {{name.awe_tune_cmd}} messages to {{name.awe_host}} (like DSP)
- ... can be (optionally) used in an ["interactive debug shell"][awe-manager-shell] application, developed together with {{name.awe_mgr}}

More information is available in the [Context View](1_context_view.md).
