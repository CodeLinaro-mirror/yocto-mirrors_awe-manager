# AWE CMD: Overview

awe_CMD component is responsible for creating {{name.awe_tune_cmd}} messages.

It provides methods to encapsulate and hide the internal structure of such command messages from code. Only inside the awe_CMD component references to DSPC {{name.awe_lib}} code are implemented. 

The component uses a "buffer handle" describing the memory locations in which to place the constructed messages. 

For more information on the {{name.awe_tune_cmd}} syntax, refer to the [tuning syntax documentation](https://w.dspconcepts.com/hubfs/Docs-AWECoreOS/AWECoreOS_UserGuide/a00075.html).
