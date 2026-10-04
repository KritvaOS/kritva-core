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

## Package, release and compatibility boundary (R1.0)

```text
Release / package producer
        |
        v   (artifact; provenance is deployment-specific)
Installed Core package  <--- package search path controlled by the integrator
        |
        v   (find_package version rule: same MAJOR, installed >= requested)
Integrator / Application
        |
        v
Kritva Core contracts (stable classification, compatibility policy)
```

- The version rule selects a package by version only. It does not authenticate the artifact, its producer or its distribution channel; those are deployment decisions outside Core.
- A compatibility, versioning or deprecation statement is not an authorization or authenticity statement, and a stable C++ API creates no wire, IPC or serialization boundary.
- The absence of an ABI promise makes binary mixing unsupported; it does not create or remove a trust boundary.
