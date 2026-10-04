# Trust Boundaries

Current architectural boundaries:

```text
Integrator / Application
        |
        v
Kritva Core
        |
   +----+----+
   |         |
Component  Platform Adapter
   |         |
   v         v
External resources / services
```

Core does not authenticate the entities behind these interfaces.
