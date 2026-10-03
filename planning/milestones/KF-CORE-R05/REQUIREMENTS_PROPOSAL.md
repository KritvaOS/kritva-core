# KF-CORE-R05 — Requirements Proposal

These IDs are proposals. They enter authoritative `REQUIREMENTS.md` only when the implementing task is accepted.

| ID | Proposed requirement |
|---|---|
| CORE-PLAT-012 | Define a non-owning `PlatformContext` view over the attached platform adapter without introducing service ownership, singleton state, service locator behavior or a generic service registry. |
| CORE-PLAT-013 | Define platform service requirement semantics for required/optional services using capability identity rather than platform-name/version inference. |
| CORE-PLAT-014 | Define explicit consumption semantics for scheduler, clock, timer and watchdog services, including unavailable-service and error-propagation behavior. |
| CORE-PLAT-015 | Preserve separation between Runtime lifecycle and platform-service lifecycle; attaching a platform does not implicitly start, stop, create or recover platform services. |
| CORE-PLAT-016 | Provide a reusable test-only reference platform and fake services capable of deterministic service availability, capability, callback, lifetime and failure testing without hardware. |
| CORE-PLAT-017 | Demonstrate Runtime/platform integration without changing the frozen R0.3 Runtime lifecycle, failure, reset, dependency ordering or statistics semantics. |
| CORE-PLAT-018 | Validate the R0.5 platform runtime integration foundation through unit tests, integration tests, full regression, quality analysis, traceability, install-consumer and prohibited-dependency checks. |

## Requirement Entry Rule

A proposed requirement is not authoritative until:

1. its implementing task is accepted;
2. the requirement is traced to the production/public artifact;
3. required tests are registered;
4. the traceability audit passes.

Do not renumber `CORE-PLAT-001` through `CORE-PLAT-011`.
