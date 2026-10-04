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

## Capability and requirement boundary (R0.9)

```text
Provider (Component / platform adapter)  --declares-->  CapabilitySet snapshot (a claim)
                                                              |
                          explicit identity check by the consumer or integrator
                                                              |
                                                              v
                                   PlatformRequirements report (descriptive; not authorization)
```

- A provider's declaration is a **claim** Core never verifies; the trust placed in it is the integrator's deployment decision.
- A satisfied requirement is **not** evidence that the provider is authentic, authorized or working.
- Component dependency ordering (`ComponentId` edges), lifecycle, readiness and health are separate and carry no trust or authorization meaning.
