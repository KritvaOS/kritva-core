//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : context.hpp
// Description : PlatformContext: a non-owning view over a platform adapter.
//
// Component   : Kritva Core
// Module      : Platform Contract
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-012, CORE-PLAT-014
// API         : CORE-API-PLATFORM
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#pragma once
#include "adapter.hpp"
#include "../capability/capability_id.hpp"
#include "../capability/capability_set.hpp"
#include "../error/result.hpp"
#include <string>
namespace kritva::core::platform {

//------------------------------------------------------------------------------
// PlatformContext (CORE-PLAT-012)
//
// A small value type through which Core-facing code, written by the integrator,
// reaches the platform it was given. It is a VIEW over one IPlatformAdapter
// (platform/adapter.hpp, unchanged): IPlatformAdapter remains the authoritative
// platform boundary and PlatformContext adds no second abstraction of a platform.
//
// WHAT IT IS NOT
//   - It owns nothing, creates nothing and destroys nothing: no adapter, no
//     service, no memory kept beyond one pointer.
//   - It is not a service registry or service locator. It registers nothing,
//     finds nothing by name, and has no static, global or thread-local state:
//     every context holds exactly the adapter it was constructed with, and a
//     default-constructed context holds none.
//   - It infers nothing. Whether a service or capability exists is exactly what
//     the adapter reports; the platform's name and version are never consulted.
//
// CONSTRUCTION AND COPYING
//   PlatformContext()                      unattached: no platform
//   explicit PlatformContext(IPlatformAdapter&)   attached to that adapter
//   explicit PlatformContext(IPlatformAdapter*)   attached, or unattached if the
//       pointer is null (so runtime::RuntimeManager::platform(), which returns a
//       nullable pointer, can be passed directly)
//   A PlatformContext is a cheap, copyable value. Copying it copies the view,
//   not the platform: every copy refers to the same adapter. Copying a
//   PlatformContext does not extend the lifetime of the adapter and does not
//   transfer or duplicate ownership.
//
// OWNERSHIP AND LIFETIME
//   The integrator owns the adapter and every service it exposes (see
//   platform/boundary.hpp). The adapter must outlive every PlatformContext that
//   refers to it and every pointer obtained through one; using a context after
//   its adapter has been destroyed is undefined behavior, exactly as using the
//   adapter would be. A context never checks, and cannot detect, that the
//   adapter has gone. Destroying a context never touches the adapter.
//
// SERVICE ACCESS
//   scheduler(), clock(), timer() and watchdog() forward to the adapter and
//   return its non-owning pointer, or nullptr when the context is unattached or
//   the adapter does not provide that service (nullptr means "not supported",
//   as in IPlatformAdapter). The same adapter-owned object is returned on every
//   call. Asking has no side effect: it never starts, stops, configures or
//   creates a service, and never opens or instantiates hardware.
//   supports(service) is true exactly when the matching accessor is non-null;
//   it is false for an unattached context and for an unknown enumerator.
//
// IDENTITY AND CAPABILITIES
//   info() returns the adapter's PlatformInfo, or nullptr when unattached; the
//   pointer stays valid as long as the adapter does (IPlatformAdapter::info()).
//   capabilities() returns the adapter's snapshot by value (an empty set when
//   unattached), owned by the caller. has_capability(id) is exactly
//   CapabilitySet::contains(id): capability IDENTITY decides, never the
//   capability's version, name or the platform's name or version. Capability
//   queries do not instantiate hardware, activate a feature or change Runtime
//   behavior.
//
//
// EXPLICIT SERVICE CONSUMPTION (CORE-PLAT-014)
//   require_scheduler(), require_clock(), require_timer() and require_watchdog()
//   are the explicit way to consume a platform service. Each returns a
//   Result<T*>:
//     success  the adapter-owned, non-owning, non-null service pointer (the same
//              pointer the matching accessor returns)
//     failure  ErrorCode::UNSUPPORTED with ErrorSeverity::ERROR and no component
//              source, when the context is unattached or the adapter does not
//              provide the service. The message says which service and whether
//              no platform is attached or the platform does not provide it; the
//              code, not the message, is the contract.
//   Requiring a service is a query: it never starts, stops, configures, creates
//   or owns anything, never opens hardware, and has no side effect. An
//   unavailable service is a normal, deterministic result of the platform the
//   integrator chose, never an exception and never an abort. Check several needs
//   at once, before using any, with PlatformRequirements and check_required()
//   (platform/requirements.hpp).
//
//   After a successful require_*() the caller uses the service directly, through
//   its own contract (platform/scheduler.hpp, time/clock.hpp, time/timer.hpp,
//   platform/watchdog.hpp), exactly as for an adapter's accessor:
//   - SERVICE ERRORS ARE NEVER TRANSLATED. Core wraps, rewrites and adds nothing:
//     an Error a service returns keeps its code, severity and message. The only
//     error Core itself produces here is the UNSUPPORTED above.
//   - A Component that propagates a platform Error through one of its lifecycle
//     operations sets the Error's source to its own ComponentId, as every Error
//     returned by a Component operation does (runtime/component.hpp), and leaves
//     the code and message unchanged; the Runtime then propagates it unchanged
//     (R0.3 failure semantics). Core offers no error-wrapping helper.
//   - Callback, context-lifetime and re-entry rules are exactly those of the
//     service contracts (R0.4): obtaining a service through a context changes
//     none of them, and a callback re-entering its own service through a
//     pointer obtained here is treated as it is for any other pointer.
//   - Nothing is started or stopped implicitly: the service's state is whatever
//     the integrator or adapter made it; destroying or copying the context
//     leaves it untouched.
//
// THREADS, ALLOCATION, REAL TIME
//   A context holds no state of its own beyond one pointer and is as
//   thread-safe as the adapter it forwards to (adapter-defined). Only
//   capabilities() and has_capability() allocate (they copy a snapshot); they
//   are control-plane queries. Core makes no hard-real-time claim for any of
//   these members.
//------------------------------------------------------------------------------
class PlatformContext {
public:
    PlatformContext() noexcept = default;
    explicit PlatformContext(IPlatformAdapter& adapter) noexcept : adapter_(&adapter) {}
    explicit PlatformContext(IPlatformAdapter* adapter) noexcept : adapter_(adapter) {}

    /// True when the context refers to an adapter.
    [[nodiscard]] bool attached() const noexcept { return adapter_ != nullptr; }

    /// The adapter's identity, or nullptr when unattached.
    [[nodiscard]] const PlatformInfo* info() const noexcept { return adapter_ ? &adapter_->info() : nullptr; }

    [[nodiscard]] IScheduler* scheduler() const noexcept { return adapter_ ? adapter_->scheduler() : nullptr; }
    [[nodiscard]] time::IClock* clock() const noexcept { return adapter_ ? adapter_->clock() : nullptr; }
    [[nodiscard]] time::ITimer* timer() const noexcept { return adapter_ ? adapter_->timer() : nullptr; }
    [[nodiscard]] IWatchdog* watchdog() const noexcept { return adapter_ ? adapter_->watchdog() : nullptr; }

    /// True when the service is provided; false when unattached, unsupported or unknown.
    [[nodiscard]] bool supports(PlatformService service) const noexcept { return adapter_ != nullptr && adapter_->supports(service); }

    /// The adapter's capability snapshot (empty when unattached), owned by the caller.
    [[nodiscard]] CapabilitySet capabilities() const { return adapter_ ? adapter_->capabilities() : CapabilitySet{}; }

    /// Explicit consumption: the adapter-owned service, or UNSUPPORTED when unattached or unsupported.
    [[nodiscard]] Result<IScheduler*> require_scheduler() const { return require(scheduler(), "scheduler"); }
    [[nodiscard]] Result<time::IClock*> require_clock() const { return require(clock(), "clock"); }
    [[nodiscard]] Result<time::ITimer*> require_timer() const { return require(timer(), "timer"); }
    [[nodiscard]] Result<IWatchdog*> require_watchdog() const { return require(watchdog(), "watchdog"); }

    /// True when the adapter reports a capability with this identity.
    [[nodiscard]] bool has_capability(CapabilityId id) const { return capabilities().contains(id); }

private:
    template<class Service> Result<Service*> require(Service* service, const char* name) const {
        if (service != nullptr) return Result<Service*>::success(service);
        return Result<Service*>::failure(Error{ErrorCode::UNSUPPORTED, ErrorSeverity::ERROR, {}, {},
            std::string("platform service is not available: ") + name + (adapter_ == nullptr ? " (no platform is attached)" : " (not provided by the platform)")});
    }

    IPlatformAdapter* adapter_{nullptr};   // non-owning; the integrator owns the adapter
};
} // namespace kritva::core::platform
