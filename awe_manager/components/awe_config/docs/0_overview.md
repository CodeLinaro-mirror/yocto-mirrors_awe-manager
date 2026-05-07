# AWE CONFIG: Overview

The component is responsible for storing and accessing configuration data.

The component can either be instantiated outside of {{name.awe_mgr}} and passed in to the initialization methods. {{name.awe_mgr}} internally keeps a handle to the awe_CONFIG object. Alternatively Null can be passed, and the default config values are used.

The component provides methods to add and retrieve single configuration items. Each configuration item stores the config under a name (key), with a description and a value. The value is a simple string. See [structural view](2_structural_view.md).

