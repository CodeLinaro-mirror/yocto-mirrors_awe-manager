# AWE OSAL: Requirements

These are the requirements specific to the OSAL layer.

**List of `dsn` and `req` specification items **:

```yaml
- id: dsn~AWEMGR.AWOSAL.Threads~1
  needs: utest
  description: |
    The component shall provide the api to create, start, join and destroy a thread in platform independent way.

- id: dsn~AWEMGR.AWOSAL.Mutex~1
  needs: utest
  description: |
    The component shall provide the api to create, lock, unlock and destroy a mutex in platform independent way.


```
