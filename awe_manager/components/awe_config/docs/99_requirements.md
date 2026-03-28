# AWE CONFIG: Requirements

These are the requirements specific to the CMD handling component.

**List of `dsn` and `req` specification items **:

```yaml

- id: dsn~AWEMGR.AWECONFIG.Add~1
  needs: utest
  description: |
    There must be an API to add "default" values.

- id: dsn~AWEMGR.AWECONFIG.Get~1
  needs: utest
  description: |
    The Component shouw allow to get the value by the key,
    either as string or apropriate type.

- id: dsn~AWEMGR.AWECONFIG.Errorhandling~1
  needs: utest
  description: |
    Component should handle erros such as conversion errors, key not found, invalid handles

- id: dsn~AWEMGR.AWECONFIG.BatchAdd~1
  needs: utest
  description: |
    There must be an API to set/configure multiple values at once.

- id: dsn~AWEMGR.AWECONFIG.EnvAdd~1
  needs: utest
  description: |
    There must be an API to add values via an environment variable.

```

