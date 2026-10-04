# Error Codes

Contract of `ErrorCode`, `ErrorSeverity` and the `Error` record (`CORE-ERR-001`, `CORE-ERR-002`), and of the rule that a failed `Result` never carries `NONE` (`CORE-ERR-004`). `error/error_code.hpp` is the authoritative enumeration. This page restates what the public headers and `REQUIREMENTS.md` already state; where they are silent, this page says so and Core assigns no further meaning.

## 1. Purpose

To give every Core operation one shared, closed vocabulary for reporting why an operation failed (`ErrorCode`), how serious the report is (`ErrorSeverity`), and a structured record carrying them (`Error`), for use inside `Result<T>`/`Result<void>`.

## 2. API surface

| Item | Header |
|---|---|
| `ErrorSeverity` (`std::uint8_t`), `ErrorCode` (`std::uint32_t`) | `error/error_code.hpp` |
| `Error { code, severity, source, timestamp, message }` | `error/error.hpp` |
| `Result<T>::failure(Error)`, `Result<void>::failure(Error)` | `error/result.hpp` |

`ErrorSeverity`: `INFO` = 0, `WARNING` = 1, `ERROR` = 2, `CRITICAL` = 3. The headers state no ordering semantics, escalation rule or per-severity behavior beyond these names.

`Error` fields: `code` (default `ErrorCode::NONE`), `severity` (default `ErrorSeverity::ERROR`), `source` (`Id`, default-constructed), `timestamp` (`Timestamp`, default-constructed), `message` (`std::string`). The headers state which contracts set `source` (for example a Component operation sets the component id); they state nothing about `timestamp`, which the contract leaves unspecified.

`ErrorCode` values (numeric values are contract; `NONE` = 0, then consecutive):

| Value | Code | Documented Core usage |
|---|---|---|
| 0 | `NONE` | Default / "no error" value of `Error::code`; never the code of a failed `Result`. |
| 1 | `UNKNOWN` | No Core-defined producer at the 1.0 baseline. |
| 2 | `INVALID_ARGUMENT` | Empty parameter name rejected by `Configuration::set()`; invalid or empty `ComponentInfo::create()` arguments; duplicate component registration (source = the id); invalid `DependencyGraph::add_dependency()` edge (source = dependent); invalid requirement declaration; an event naming a different source than the reporting component. Also named by `Component` for a rejected configuration. |
| 3 | `INVALID_STATE` | An operation invalid in the current lifecycle state (component, `Lifecycle::transition_to`, `configure()`, `RuntimeManager`), with no other effect; setup operations after the topology is fixed; a second `attach_platform()`; an unbound event reporter. |
| 4 | `NOT_INITIALIZED` | No Core-defined producer at the 1.0 baseline. |
| 5 | `NOT_READY` | No Core-defined producer at the 1.0 baseline. |
| 6 | `ALREADY_RUNNING` | No Core-defined producer at the 1.0 baseline. |
| 7 | `TIMEOUT` | No Core-defined producer at the 1.0 baseline. |
| 8 | `RESOURCE_UNAVAILABLE` | No Core-defined producer at the 1.0 baseline. |
| 9 | `CONFIGURATION_ERROR` | `DependencyGraph::order()` when an edge endpoint is not registered; a component's rejected or schema-incompatible configuration (`configuration/configuration_version.hpp`, `runtime/component.hpp`). |
| 10 | `UNSUPPORTED` | A platform service or required requirement that is unavailable (`platform/context.hpp`, `platform/requirements.hpp`, `runtime/component_context.hpp`). |
| 11 | `INTERNAL_ERROR` | No Core-defined producer at the 1.0 baseline. |

"No Core-defined producer" is a factual statement about the 1.0 baseline: Core itself returns no such code. The headers assign these names no further meaning, and this page does not either. A Component or integrator may return any `ErrorCode` from its own operations; `runtime/component.hpp` asks for `INVALID_STATE` for invalid operations, `INVALID_ARGUMENT` or `CONFIGURATION_ERROR` for a rejected configuration, and "the most specific Core `ErrorCode`" otherwise.

## 3. Semantics and invariants

- `NONE` is only the default/no-error value. `Result::failure(Error)` has the precondition `error.code != ErrorCode::NONE` (`CORE-ERR-004`), so a failure never carries `NONE`.
- The code, not the message, is the contract; messages are for humans.
- `ErrorSeverity` classifies a report; its effect on any Core behavior is not stated by the headers.
- Relation to `StatusCode` (`status/status_code.hpp`, `CORE-STA-001`): a separate enumeration with its own names and `std::uint8_t` values; no mapping between the two is defined by Core. `status_code.hpp` says its numeric values must not be persisted or transmitted (use the names), while the 1.0 compatibility policy still protects those values against change.

## 4. Ownership and lifetime

`ErrorCode` and `ErrorSeverity` are plain values. `Error` is a value that owns its `message` string; a `Result` owns its `Error`. Nothing is shared or borrowed.

## 5. Lifecycle interaction

None of its own. Several codes are returned by lifecycle and configuration operations as stated in their contracts (`INVALID_STATE` for an invalid operation, which has no other effect). A failed valid component operation returns the component's `Error` and moves it to `FAULT` (see `docs/api/lifecycle/LIFECYCLE.md`). An `ErrorCode` never changes a lifecycle state by itself.

## 6. Error behavior

These types report errors and do not fail. The only precondition is `Result::failure()` with a non-`NONE` code; violating it is asserted when `NDEBUG` is not defined and undefined behavior in release builds (`CORE-ERR-004`).

## 7. Thread safety

`ErrorCode` and `ErrorSeverity` are plain enumerations, safe to copy and read concurrently. `Error` and `Result` are not thread-safe: do not access one object concurrently without external synchronization (`error/result.hpp`).

## 8. Allocation, blocking and real time

The enumerations are trivially copyable and never allocate or block. `Error` holds a `std::string`, so constructing or copying an `Error` with a message may allocate; `Result` itself does not allocate beyond what `T` or `Error::message` do. No hard-real-time claim is made for creating errors with messages.

## 9. Compatibility

The numeric value, underlying type and meaning of every `ErrorCode` and `ErrorSeverity` enumerator are contract (`docs/compatibility/COMPATIBILITY_POLICY.md`, `docs/compatibility/VERSIONING_POLICY.md`; pinned by `tests/unit/api_compat_boundary_test.cpp`). Adding an `ErrorCode` or enumerator is Review-required and released in a MINOR release after evolution review. An existing value is never renumbered, reused for a different meaning or removed outside a MAJOR release, and a retired value is not reassigned. Changing the `ErrorCode` returned for a documented condition is incompatible. Clients must tolerate an unknown enumerator value when they switch over these enumerations. Behavior that no contract source states, including the unproduced codes above, is not promised.

## 10. Security considerations

Error reporting is not a security event, and an `ErrorCode` is not an authorization decision or a trust signal. `Error::message` is human-readable text and may name internal identifiers; treat it accordingly before exposing it. See `docs/security/TRUST_BOUNDARIES.md`.

## 11. Examples

```cpp
auto r = registry.register_component(component);
if (!r) {
    switch (r.error().code) {
        case ErrorCode::INVALID_ARGUMENT: /* e.g. duplicate id; source names it */ break;
        case ErrorCode::INVALID_STATE:    /* setup closed */                       break;
        default:                          /* tolerate unknown / other values */   break;
    }
}
```

## 12. Requirements traceability

`CORE-ERR-001` (codes and severities), `CORE-ERR-002` (`Error` record), `CORE-ERR-004` (`Result`, non-`NONE` failure), `CORE-STA-001` (related `StatusCode`), `CORE-COMPAT-006` (enumeration and `ErrorCode` evolution rules).

## 13. Related headers

`error/error_code.hpp`, `error/error.hpp`, `error/result.hpp`, `status/status_code.hpp`, `runtime/component.hpp`, `runtime/runtime_manager.hpp`, `runtime/dependency_graph.hpp`, `configuration/configuration.hpp`, `platform/context.hpp`.

## 14. Related tests

`tests/unit/error_test.cpp`, `tests/unit/result_test.cpp`, `tests/unit/status_test.cpp`, `tests/contract/foundation_contract_test.cpp`, `tests/unit/api_compat_boundary_test.cpp`.

## 15. Explicit exclusions

No mapping from `ErrorCode` to `StatusCode`, to a lifecycle state, to health, or to an authorization outcome; no automatic retry, recovery or escalation driven by a code or severity; no meaning assigned to the unproduced codes beyond their names; no exceptions, error categories, error chains or user-defined codes; no persistence or wire encoding of codes is defined by Core.
