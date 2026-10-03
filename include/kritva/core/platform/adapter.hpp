//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : adapter.hpp
// Description : Platform adapter contract: identity, service discovery and capabilities.
//
// Component   : Kritva Core
// Module      : Platform Contract
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-008
// API         : CORE-API-PLATFORM
//
// Author      : KritvaOS Core Team
// Created     : 04-10-2026
//==============================================================================

#pragma once
#include "scheduler.hpp"
#include "watchdog.hpp"
#include "../capability/capability_set.hpp"
#include "../time/clock.hpp"
#include "../time/timer.hpp"
#include "../types/version.hpp"
#include <cstdint>
#include <string>
namespace kritva::core::platform {

/// The platform services an adapter can provide.
enum class PlatformService : std::uint8_t { SCHEDULER, CLOCK, TIMER, WATCHDOG };

/// Identity of a platform adapter: a name and a version, nothing else. There are
/// no CPU, operating-system, board or vendor fields and no vendor types: such
/// detail is reported, if at all, through capabilities defined by the adapter.
struct PlatformInfo {
    std::string name;    ///< Non-empty label of the platform; a label, not an identity key.
    Version version{};   ///< Version of the adapter.
};

//------------------------------------------------------------------------------
// IPlatformAdapter (CORE-PLAT-008)
//
// One adapter describes one platform to the integrator: who it is, which
// platform services it provides and which capabilities it reports. It follows
// the shared boundary rules in platform/boundary.hpp: the integrator owns the
// adapter and every service it exposes; Core holds only non-owning references; there
// is no singleton, global adapter or service locator, and no adapter registry in
// Core; the adapter outlives every Core object that references it.
//
// IDENTITY
//   info() returns the adapter's identity. It is immutable from the adapter's
//   point of view: it never changes during the adapter's life and the returned
//   reference stays valid until the adapter is destroyed. Two calls return equal
//   values.
//
// SERVICES
//   scheduler(), clock(), timer() and watchdog() each return a non-owning
//   pointer to the service, or nullptr when the adapter does not provide it.
//   - A non-null pointer refers to an object owned by the adapter that stays
//     valid and returns the same object on every call until the adapter is
//     destroyed. The service obeys its own contract (platform/scheduler.hpp,
//     time/clock.hpp, time/timer.hpp, platform/watchdog.hpp).
//   - nullptr means "not supported". It is never an error and an adapter never
//     returns a placeholder that pretends to support the service.
//   - Asking does not activate anything: it never starts a task, timer or
//     watchdog, never opens or instantiates hardware, and has no side effect.
//   - The accessors are const (they only report) yet return mutable service
//     pointers, because the services are the adapter's non-owning handles.
//   supports(service) is not virtual: it is true exactly when the matching
//   accessor returns non-null and false exactly when it returns nullptr, so the
//   two can never disagree. The value of an unknown enumerator is false.
//
// CAPABILITIES
//   capabilities() returns a snapshot by value (like Component::capabilities()):
//   the caller owns the copy, it stays valid and unchanged whatever the adapter
//   does later. It reports what the adapter says it offers beyond the four
//   services, using the existing Core CapabilitySet. Capability identities are
//   chosen by the adapter; Core reserves no identity range for platforms, and an
//   adapter shall not give an identity already used in its own set a different
//   meaning or version. The same adapter reports equal sets (same entries in the
//   same order) unless it documents otherwise. Capabilities describe, they do
//   not enable: reporting one does not instantiate hardware, activate a feature
//   or change Runtime behavior. The call may allocate (control-plane).
//
// RUNTIME
//   The Runtime may be given a reference to an adapter (RuntimeManager::
//   attach_platform, planned for a later R0.4 task), but Runtime operations never call any
//   member of this interface.
//
// THREADS
//   Thread safety is adapter-defined. The members are control-plane queries and
//   Core makes no hard-real-time, latency or jitter claim for them.
//------------------------------------------------------------------------------
class IPlatformAdapter {
public:
    virtual ~IPlatformAdapter() = default;

    [[nodiscard]] virtual const PlatformInfo& info() const noexcept = 0;
    [[nodiscard]] virtual IScheduler* scheduler() const noexcept = 0;
    [[nodiscard]] virtual time::IClock* clock() const noexcept = 0;
    [[nodiscard]] virtual time::ITimer* timer() const noexcept = 0;
    [[nodiscard]] virtual IWatchdog* watchdog() const noexcept = 0;
    [[nodiscard]] virtual CapabilitySet capabilities() const = 0;

    [[nodiscard]] bool supports(PlatformService service) const noexcept {
        switch (service) {
            case PlatformService::SCHEDULER: return scheduler() != nullptr;
            case PlatformService::CLOCK:     return clock() != nullptr;
            case PlatformService::TIMER:     return timer() != nullptr;
            case PlatformService::WATCHDOG:  return watchdog() != nullptr;
        }
        return false;
    }
};
} // namespace kritva::core::platform
