//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component_context.hpp
// Description : ComponentContext: a Component's explicit, non-owning execution context.
//
// Component   : Kritva Core
// Module      : Runtime
// Layer       : Core Foundation
//
// Requirements: CORE-CTX-001, CORE-CTX-002, CORE-CTX-003, CORE-CTX-004
// API         : CORE-API-COMPONENT-CONTEXT
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#pragma once
#include "component.hpp"
#include "component_id.hpp"
#include "component_info.hpp"
#include "../platform/context.hpp"
#include "../platform/requirements.hpp"
#include "../error/result.hpp"
namespace kritva::core::runtime {

//------------------------------------------------------------------------------
// ComponentContext (CORE-CTX-001)
//
// The explicit, deterministic, NON-OWNING context an integrator-written
// Component holds: who the component is (its immutable ComponentInfo) and which
// platform it was given (the R0.5 platform::PlatformContext view). "Context
// carries access; Runtime retains control."
//
// SHAPE
//   A small copyable value of exactly two non-owning pointers: a
//   const ComponentInfo* and a platform::PlatformContext (itself one pointer).
//   It is trivially destructible, nothrow default-constructible, and has no
//   cache, no registry, no static, global or thread-local state, and no
//   inference from any name or version.
//     ComponentContext()                       UNBOUND: no identity, no platform
//     explicit ComponentContext(const ComponentInfo&, platform::PlatformContext = {})
//                                              BOUND to that identity and platform
//   bound() is true exactly for the second form. info() returns the identity
//   or nullptr when unbound; id() returns the component's ComponentId or the
//   invalid id (ComponentId{}) when unbound. platform() returns the R0.5 view BY
//   CONST REFERENCE ONLY: a default-constructed (unattached) PlatformContext when
//   none was given.
//
// IMMUTABLE AFTER CONSTRUCTION
//   A context is fixed when it is constructed. It has no setter, no reset and no
//   rebinding of any kind, and it is not assignable (copy and move assignment are
//   deleted; copy and move construction are available): a component stores its
//   context, built once, in its constructor, so a context can never become
//   mutable shared state.
//
// OWNERSHIP AND LIFETIME
//   The context owns nothing, creates nothing and destroys nothing. The
//   ComponentInfo it refers to must outlive the context (it lives as long as its
//   Component, and Component::info() is stable until destruction); the adapter
//   behind the platform view must outlive every context, exactly as for
//   PlatformContext. Copying a ComponentContext copies the two pointers: it
//   neither extends the lifetime of the identity or the adapter nor transfers or
//   duplicates ownership. Using a context after what it refers to is destroyed
//   is undefined behavior; it cannot detect that. Destroying a context never
//   touches the identity or the adapter.
//
//
// ACCESS POLICY (CORE-CTX-002)
//   The ONLY operational access a ComponentContext offers, each with its side
//   effects and its errors:
//
//     path                              side effects   result
//     --------------------------------  -------------  ---------------------------------------
//     bound(), info(), id(), platform() none           the values described above
//     require_scheduler/clock/timer/    none (a query) Result<T*>: the adapter-owned pointer, or
//       watchdog()                                      UNSUPPORTED (see below)
//     supports(PlatformService)         none (a query) true exactly when the service is provided;
//                                                       false when unattached or for an unknown one
//     has_capability(CapabilityId)      none (a query) true exactly when the platform reports a
//                                                       capability with that IDENTITY
//     attribute(Error)                  none           the same Error with only its source replaced
//
//   Nothing else is reachable through a context: no Runtime, registry, other
//   component, Configuration, Statistics or Health, no raw adapter, and no lookup
//   by name or by a runtime-chosen service key. There is deliberately no generic
//   "require(service)" or "get(name)": each approved service has its own typed
//   query. A context never starts, stops, configures, creates or owns a service,
//   never reads a clock and never changes the Runtime; every query above makes
//   at most one adapter accessor call (has_capability copies one capability
//   snapshot) and has no other effect.
//
//   require_scheduler(), require_clock(), require_timer() and require_watchdog()
//   forward to the R0.5 PlatformContext::require_*() queries (platform/context.hpp)
//   and return exactly its result, with ONE deliberate refinement of the
//   Component-facing layer: when the context is BOUND, the Core availability error
//   (ErrorCode::UNSUPPORTED, produced when the platform is unattached or does not
//   provide the service) carries source = the component's ComponentId, so the
//   component can return it directly as its own failed Result, as the Component
//   contract requires of every Error a component operation returns. Its code,
//   severity, timestamp and message are exactly those of PlatformContext. An
//   UNBOUND context has no identity to attribute and returns the R0.5 error
//   unchanged. PlatformContext itself is unchanged: its error still has no source.
//
//   PLATFORM SERVICE ERRORS ARE NEVER TOUCHED. After a successful require_*() the
//   caller uses the service directly through its own contract; an Error that
//   service returns keeps its code, severity, source, timestamp and message. A
//   component that returns such an Error from one of its lifecycle operations
//   calls attribute() to set the source to its own id.
//
//   attribute(error) is ATTRIBUTION, not translation or wrapping. It returns the
//   input with ONLY `source` replaced by this component's ComponentId: code,
//   severity, timestamp, message and every other field are unchanged, an Error
//   that already carries another source is attributed to this component, and
//   nothing is created, retried, logged or recovered. For an unbound context it
//   returns the original error unchanged. It is noexcept: it takes the Error by
//   value (any copy happens at the call) and changes one field.
//
//
// INJECTION (CORE-CTX-003)
//   A component obtains its context by CONSTRUCTION, and only by construction:
//   the integrator builds the context from the component's own identity and a
//   platform view (from an adapter, or from runtime::RuntimeManager::platform())
//   and the component stores it, usually as a const member initialized in its own
//   constructor:
//
//       Worker(ComponentInfo info, platform::PlatformContext platform)
//           : Component(std::move(info)), context_(*this, platform) {}
//
//   ComponentContext(const Component&, platform::PlatformContext = {}) is that
//   spelling: it uses component.info(), which is valid as soon as the Component
//   base is constructed and stays valid until the component is destroyed. A
//   temporary Component would dangle and is refused at compile time.
//
//   Nothing else injects, finds, creates or passes a context:
//   - The Runtime never creates, stores, passes or probes a ComponentContext. There
//     is no RuntimeManager member for it, no automatic injection and no
//     registration hook, and RuntimeManager::attach_platform()/platform() keep their
//     R0.4/R0.5 meaning (the integrator may build a platform view from platform()).
//   - The Component base class is unchanged: it has no context member and its
//     lifecycle operations keep exactly their R0.3 signatures; a component's
//     context is its own private business.
//   - Because the Runtime never sees the context, Runtime ordering, failure
//     propagation, fail-fast, FAULT, reset() and statistics are exactly those of
//     R0.3 whether or not components hold contexts, and whatever they do with them.
//   - The context carries no lifecycle: it is not created, destroyed, configured,
//     started, stopped or recovered by the Runtime or by Core; its lifetime is its
//     owner's, which must be shorter than the identity's and the adapter's.
//
//
// REQUIREMENT BINDING (CORE-CTX-004)
//   A component states what it needs with the R0.5 platform::PlatformRequirements
//   (platform/requirements.hpp) and checks it against its context:
//     evaluate(requirements)        the R0.5 platform::PlatformRequirementReport,
//                                   UNCHANGED: platform::evaluate(requirements,
//                                   platform()). It never fails, treats an
//                                   unattached platform as providing nothing, and
//                                   is the authoritative structured result.
//     check_required(requirements)  the R0.5 platform::check_required(requirements,
//                                   platform()): success when no REQUIRED item is
//                                   missing (optional items never matter),
//                                   otherwise ErrorCode::UNSUPPORTED naming the
//                                   first missing required declaration. When the
//                                   context is BOUND that error carries source =
//                                   the component's ComponentId (the same rule as
//                                   require_*(), so the component can return it
//                                   directly as its own failed Result); its code,
//                                   severity, timestamp and message are exactly
//                                   R0.5's, and an unbound context returns the R0.5
//                                   error unchanged.
//   BINDING, NOT STORING. The context stores no requirements: it is stateless
//   (still two pointers) and every call takes the requirements as an argument, so
//   the component keeps its own PlatformRequirements (declared with the R0.5
//   add_service()/add_capability(), whose INVALID_ARGUMENT rules and atomicity
//   are unchanged) wherever it likes. Matching stays by service and capability
//   IDENTITY only: never the platform's name or version, never a capability's
//   name or version, never a string. Evaluating is a query: PlatformContext's
//   rules apply (supports() once per declared service, capabilities() at most
//   once), nothing is started, stopped, configured or created, and neither the
//   requirements, the context, the adapter nor the Runtime changes. The same
//   requirements and the same platform give the same answer. Checking
//   requirements does not make the Runtime check them: when and whether a
//   component checks (typically in initialize()) is the component's own decision.
//
// NO CONTEXT AMPLIFICATION
//   The context never returns, directly or indirectly, a broader authority than
//   it was given: no RuntimeManager, ComponentRegistry, other Component,
//   Configuration, Statistics, Health, another component's context, or a raw
//   IPlatformAdapter. It cannot be used to look anything up by name.
//
// SIDE EFFECTS, THREADS, REAL TIME
//   Constructing, copying and querying identity or the platform view are plain
//   pointer operations: no allocation, no blocking, no call to the adapter or to
//   any service, and no effect on the Runtime. Core makes no thread-safety or
//   real-time claim beyond that of the adapter and services reached through it.
//------------------------------------------------------------------------------
class ComponentContext {
public:
    /// Unbound: no identity and an unattached platform view.
    ComponentContext() noexcept = default;

    /// Bound to `info` (which must outlive the context) and to the platform view `platform`.
    explicit ComponentContext(const ComponentInfo& info, platform::PlatformContext platform = {}) noexcept
        : info_(&info), platform_(platform) {}

    /// Bound to the identity of `component` (which must outlive the context): the construction-time injection.
    explicit ComponentContext(const Component& component, platform::PlatformContext platform = {}) noexcept
        : ComponentContext(component.info(), platform) {}

    /// A temporary component would dangle: refused at compile time.
    ComponentContext(const Component&&, platform::PlatformContext = {}) = delete;

    /// A temporary identity would dangle: refused at compile time.
    ComponentContext(const ComponentInfo&&, platform::PlatformContext = {}) = delete;

    ComponentContext(const ComponentContext&) noexcept = default;
    ComponentContext(ComponentContext&&) noexcept = default;
    ComponentContext& operator=(const ComponentContext&) = delete;      // immutable after construction
    ComponentContext& operator=(ComponentContext&&) = delete;
    ~ComponentContext() = default;

    /// True when the context was constructed with a ComponentInfo.
    [[nodiscard]] bool bound() const noexcept { return info_ != nullptr; }

    /// The component's identity, or nullptr when unbound.
    [[nodiscard]] const ComponentInfo* info() const noexcept { return info_; }

    /// The component's ComponentId, or the invalid id when unbound.
    [[nodiscard]] ComponentId id() const noexcept { return info_ != nullptr ? info_->id() : ComponentId{}; }

    /// The R0.5 platform view (unattached when none was given). Const reference only.
    [[nodiscard]] const platform::PlatformContext& platform() const noexcept { return platform_; }

    // --- access policy (see ACCESS POLICY above) -----------------------------------------------

    /// The adapter-owned service, or UNSUPPORTED (attributed to the component when bound).
    [[nodiscard]] Result<platform::IScheduler*> require_scheduler() const { return attributed(platform_.require_scheduler()); }
    [[nodiscard]] Result<time::IClock*> require_clock() const { return attributed(platform_.require_clock()); }
    [[nodiscard]] Result<time::ITimer*> require_timer() const { return attributed(platform_.require_timer()); }
    [[nodiscard]] Result<platform::IWatchdog*> require_watchdog() const { return attributed(platform_.require_watchdog()); }

    /// True when the platform provides the service; false when unattached or for an unknown enumerator.
    [[nodiscard]] bool supports(platform::PlatformService service) const noexcept { return platform_.supports(service); }

    /// True when the platform reports a capability with this identity (identity alone decides).
    [[nodiscard]] bool has_capability(CapabilityId id) const { return platform_.has_capability(id); }

    /// The R0.5 report of what `requirements` still lacks on this platform, unchanged. A query.
    [[nodiscard]] platform::PlatformRequirementReport evaluate(const platform::PlatformRequirements& requirements) const {
        return platform::evaluate(requirements, platform_);
    }

    /// R0.5 check_required: success when no REQUIRED item is missing, else UNSUPPORTED (attributed to the component when bound).
    [[nodiscard]] Result<void> check_required(const platform::PlatformRequirements& requirements) const {
        return attributed(platform::check_required(requirements, platform_));
    }

    /// `error` with ONLY its source replaced by this component's id; unchanged when unbound.
    [[nodiscard]] Error attribute(Error error) const noexcept {
        if (info_ != nullptr) error.source = info_->id();
        return error;
    }

private:
    // A Core availability error of a bound context is attributed to the component; a success and an
    // unbound context's error pass through unchanged.
    template<class T> Result<T> attributed(Result<T> result) const {
        if (result.has_value() || info_ == nullptr) return result;
        return Result<T>::failure(attribute(result.error()));
    }

    const ComponentInfo* info_{nullptr};      // non-owning
    platform::PlatformContext platform_{};    // non-owning view
};
} // namespace kritva::core::runtime
