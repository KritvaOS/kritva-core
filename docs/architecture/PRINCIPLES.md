# Kritva Core Architectural Principles

## Genericity

Kritva Core defines platform-independent contracts. It must not encode assumptions specific to Kritva Nexus, Kritva Edge, Linux, RTOS, MCU, vendor SDKs, EtherCAT, ROS 2/DDS, or concrete hardware.

## Boundary ownership

Core provides contracts and deterministic orchestration. Integrators and platform layers own concrete services, drivers, hardware, and platform lifecycle.

## Explicit access

Platform and component services are accessed through explicit contracts. Core does not provide a generic service locator, registry, or hidden dependency mechanism.

## Lifecycle authority

Runtime and Component lifecycle semantics remain explicit and deterministic. Health, capability, configuration, dependency ordering, and readiness are distinct concepts unless an accepted contract explicitly connects them.

## API governance

Public API changes require architecture review, contract tests, documentation updates, and traceability updates.

## Security governance

Security impact is assessed for architecture and public API changes. Security mechanisms are introduced only when justified by explicit requirements.
