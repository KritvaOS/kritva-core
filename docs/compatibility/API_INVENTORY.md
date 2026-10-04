# Kritva Core Public API Inventory (R1.0 baseline)

**Status:** baseline established by KF-CORE-R10-001 from the released R0.9 tree (`kritva-core-r0.9`, version 0.9.0).
**Authority:** this page is the *inventory and classification* of the installed public header surface. It is not the compatibility policy. The rules that say what a class means and what may change are defined by later R1.0 tasks (source/semantic: R10-002, ABI: R10-003, versioning: R10-004, deprecation: R10-005). Nothing here is an ABI promise and nothing here changes any header or behavior.
**Mechanical audit:** `scripts/audit/check_api_docs.py` checks `docs/api`; `scripts/audit/check_api_inventory.py` checks this page against `include/kritva/core/` (run by `make check` and two CTests).

## Public surface

The public surface is every header installed from `include/kritva/core/` (`install(DIRECTORY include/ ...)`), plus the exported CMake package `kritva_core` and its target `kritva_core::kritva_core`. Anything not installed (`src/`, `tests/`, `scripts/`, `docs/`) is not part of the public surface. Test-only support under `tests/` is not installed and is not contractual.

## Stability classes

| Class | Meaning in this inventory |
|---|---|
| `stable` | Part of the supported surface. The compatibility rules defined by R10-002..R10-005 apply to it. |
| `experimental` | Public but explicitly not yet covered by those rules. |
| `deprecated` | Public, scheduled for removal under the deprecation policy (R10-005). |
| `internal` | Visible because it is a header but not part of the supported surface. |
| `test-only` | Support for tests; must not be installed. |

R0.9 shipped every header below as part of its documented API and no header was released as experimental, deprecated or internal, so **every header is classified `stable`**. No other class is used: classes are not manufactured to populate the table. A header leaves `stable` only through the evolution and deprecation process defined by R1.0.

## Sensitivity flags

`Y` = the header has the property, `-` = it does not. The flags mark where a later compatibility rule must be applied with extra care; they are not themselves rules.

| Flag | Column | Meaning |
|---|---|---|
| Ownership / lifetime | Own | The header documents ownership or lifetime, or exposes non-owning access (references, pointers, views, callables) whose validity depends on the owner. |
| Thread safety | Thr | The header documents a thread-safety, synchronization or real-time guarantee. |
| Enum / error values | Enum | The header declares a public enumeration whose enumerators and numeric values are observable (for example `ErrorCode`, `LifecycleState`). Private enumerations (for example in `RuntimeManager`) are not counted. |
| Virtual interface | Virt | The header declares a virtual interface that clients implement or call through. |

`Enum`, `Virt` and `Thr` are audited against the header text (a `Y` requires the header to contain `enum`, `virtual` or `thread` respectively, and a header containing `virtual` must carry `Y`). `Own` is a reviewed classification and is not mechanically derivable.

## Inventory

Domain is the first path component under `include/kritva/core/` (`core` for the umbrella header). `Owning documentation` is the page under `docs/api/` that owns the header's contract and its status in `docs/api/API_INDEX.md`; `none` means no page owns it yet.

| Header | Domain | Stability | Own | Thr | Enum | Virt | Owning documentation | Doc status |
|---|---|---|---|---|---|---|---|---|
| `capability/capability.hpp` | capability | stable | Y | Y | - | - | `capability/CAPABILITY.md` | maintained |
| `capability/capability_id.hpp` | capability | stable | - | - | - | - | `capability/CAPABILITY.md` | maintained |
| `capability/capability_set.hpp` | capability | stable | Y | Y | - | - | `capability/CAPABILITY_SET.md` | maintained |
| `configuration/configuration.hpp` | configuration | stable | Y | Y | - | - | `configuration/CONFIGURATION.md` | stub |
| `configuration/configuration_version.hpp` | configuration | stable | - | Y | - | - | `configuration/CONFIGURATION_VERSION.md` | stub |
| `configuration/parameter.hpp` | configuration | stable | - | - | - | - | `configuration/CONFIGURATION.md` | stub |
| `core.hpp` | core | stable | - | - | - | - | none | none |
| `error/error.hpp` | error | stable | - | - | - | - | `error/ERROR.md` | stub |
| `error/error_code.hpp` | error | stable | - | - | Y | - | `error/ERROR_CODES.md` | stub |
| `error/result.hpp` | error | stable | Y | Y | - | - | `error/ERROR.md` | stub |
| `event/event.hpp` | event | stable | - | - | - | - | none | none |
| `event/event_type.hpp` | event | stable | - | - | Y | - | none | none |
| `health/health.hpp` | health | stable | Y | - | - | - | none | none |
| `health/health_state.hpp` | health | stable | - | - | Y | - | none | none |
| `lifecycle/lifecycle.hpp` | lifecycle | stable | - | - | - | - | `lifecycle/LIFECYCLE.md` | maintained |
| `lifecycle/lifecycle_state.hpp` | lifecycle | stable | - | - | Y | - | `lifecycle/LIFECYCLE.md` | maintained |
| `messaging/message.hpp` | messaging | stable | - | - | - | - | none | none |
| `messaging/topic.hpp` | messaging | stable | Y | - | - | - | none | none |
| `platform/adapter.hpp` | platform | stable | Y | Y | Y | Y | `platform/PLATFORM_ADAPTER.md` | stub |
| `platform/boundary.hpp` | platform | stable | Y | Y | - | - | none | none |
| `platform/clock.hpp` | platform | stable | - | - | - | - | none | none |
| `platform/context.hpp` | platform | stable | Y | Y | - | - | `context/PLATFORM_CONTEXT.md` | stub |
| `platform/requirements.hpp` | platform | stable | Y | Y | Y | - | `platform/PLATFORM_REQUIREMENTS.md` | maintained |
| `platform/scheduler.hpp` | platform | stable | Y | Y | - | Y | none | none |
| `platform/watchdog.hpp` | platform | stable | Y | Y | - | Y | none | none |
| `runtime/component.hpp` | runtime | stable | Y | Y | - | Y | `runtime/COMPONENT.md` | stub |
| `runtime/component_context.hpp` | runtime | stable | Y | Y | - | - | `context/COMPONENT_CONTEXT.md` | stub |
| `runtime/component_events.hpp` | runtime | stable | Y | Y | - | Y | none | none |
| `runtime/component_id.hpp` | runtime | stable | - | Y | - | - | none | none |
| `runtime/component_info.hpp` | runtime | stable | Y | Y | - | - | none | none |
| `runtime/component_observation.hpp` | runtime | stable | Y | Y | - | Y | none | none |
| `runtime/component_registry.hpp` | runtime | stable | Y | Y | - | - | `runtime/DEPENDENCY_GRAPH.md` | maintained |
| `runtime/component_statistics.hpp` | runtime | stable | Y | Y | - | Y | none | none |
| `runtime/dependency_graph.hpp` | runtime | stable | Y | Y | - | - | `runtime/DEPENDENCY_GRAPH.md` | maintained |
| `runtime/runtime.hpp` | runtime | stable | - | - | - | Y | `runtime/RUNTIME.md` | stub |
| `runtime/runtime_manager.hpp` | runtime | stable | Y | Y | - | - | `runtime/RUNTIME.md` | stub |
| `statistics/counter.hpp` | statistics | stable | - | Y | - | - | none | none |
| `statistics/gauge.hpp` | statistics | stable | Y | Y | - | - | none | none |
| `statistics/statistics.hpp` | statistics | stable | Y | Y | - | - | none | none |
| `status/status.hpp` | status | stable | Y | Y | - | - | none | none |
| `status/status_code.hpp` | status | stable | - | Y | Y | - | none | none |
| `time/clock.hpp` | time | stable | Y | Y | - | Y | none | none |
| `time/timer.hpp` | time | stable | Y | Y | Y | Y | none | none |
| `types/callback.hpp` | types | stable | Y | - | - | - | none | none |
| `types/duration.hpp` | types | stable | - | - | - | - | none | none |
| `types/id.hpp` | types | stable | - | - | - | - | none | none |
| `types/metadata.hpp` | types | stable | Y | - | - | - | none | none |
| `types/timestamp.hpp` | types | stable | - | Y | Y | - | none | none |
| `types/version.hpp` | types | stable | - | - | - | - | none | none |

**Total public headers: 49**

## Documentation decisions

- **D-INV-1 — stub pages.** Nine pages in `docs/api` are `stub`s (R0.9 policy, decision D13 of R1.0). A stub names its owning headers only through this inventory; it is not promoted to `maintained` by R1.0 classification. A stub becomes maintained when its public contract changes or when a later R1.0 task records that compatibility classification requires it.
- **D-INV-2 — headers without a page.** 29 of the 49 headers have no owning page (`none`). Their authoritative contract today is the header's own contract comment together with the requirement rows in `REQUIREMENTS.md`. R1.0 does not create pages for them by default; whether any of them needs a maintained page for compatibility classification is decided by KF-CORE-R10-002 and, for final reconciliation, KF-CORE-R10-008. Until then the `stable` classification applies to them exactly as to documented headers.
- **D-INV-3 — umbrella header.** `core.hpp` is classified `stable` as an include convenience; it carries no contract of its own beyond including the headers it names.
- **D-INV-4 — package.** The CMake package `kritva_core` and target `kritva_core::kritva_core` are part of the public surface. Their version-selection behavior is classified by R10-004 and tested by R10-007; this inventory only records that they are in scope.
- **D-INV-5 — no ABI claim.** Inventory and `stable` classification say nothing about binary compatibility. The ABI posture is decided by R10-003.

## Requirement

CORE-COMPAT-001 — inventory and classify the installed public Core API boundary.
