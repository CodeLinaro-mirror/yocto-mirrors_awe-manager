# AWE OSAL: Internal Architecture

There is no further sub-component included by awe_OSAL component.

All API methods work the same regardless of the underlying platform.

Exception:

Acquiring a mutex under Windows again from the same(!) thread does not return a timeout error. A Windows mutex is recursive (re-entrant) for the owning thread.

