//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : context_conformance.hpp
// Description : Reusable contract checks for a ComponentContext against the platform it was given.
//
// Component   : Kritva Core
// Module      : Component Context Test Support
// Layer       : Core Foundation
//
// Requirements: CORE-CTX-005
// API         : CORE-TEST-CONTEXT-CONFORMANCE
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#pragma once

// Checks the implementation-independent rules of ComponentContext (runtime/component_context.hpp)
// for ONE context and the adapter (or none) it was built over, using only public APIs. Failures are
// collected in a conformance::Report. It is test support: it never starts a service and it
// treats the adapter as read-only.
//
// Checked: identity (bound/unbound), the platform view, the typed queries (success is the adapter's
// own object; an unavailable service is UNSUPPORTED with the component as source when bound and no
// source when unbound), supports and capability identity, attribution (only the source changes),
// requirement binding (evaluate equals the R0.5 report, check_required equals the R0.5 result with
// the source rule), and determinism. Not checked here: side effects on a counting adapter (the
// caller observes those), lifetimes and the Runtime.

#include <cstdint>
#include <string>

#include <kritva/core/core.hpp>

#include "conformance.hpp"

namespace kritva::core::platform::conformance {

namespace detail {

inline bool same_services(const std::vector<ServiceRequirement>& x, const std::vector<ServiceRequirement>& y) {
    if (x.size() != y.size()) return false;
    for (std::size_t i = 0; i < x.size(); ++i) if (x[i].service != y[i].service || x[i].level != y[i].level) return false;
    return true;
}
inline bool same_capabilities(const std::vector<CapabilityRequirement>& x, const std::vector<CapabilityRequirement>& y) {
    if (x.size() != y.size()) return false;
    for (std::size_t i = 0; i < x.size(); ++i) if (x[i].id != y[i].id || x[i].level != y[i].level) return false;
    return true;
}
inline bool same_report(const PlatformRequirementReport& a, const PlatformRequirementReport& b) {
    return same_services(a.missing_required_services, b.missing_required_services) && same_services(a.missing_optional_services, b.missing_optional_services)
        && same_capabilities(a.missing_required_capabilities, b.missing_required_capabilities) && same_capabilities(a.missing_optional_capabilities, b.missing_optional_capabilities);
}

template<class T> void check_service_query(const runtime::ComponentContext& context, const Result<T*>& result, T* adapter_service,
                                           const char* name, Report& report) {
    if (adapter_service != nullptr) {
        KRITVA_CONFORMANCE_CHECK(report, result.has_value());
        if (result) KRITVA_CONFORMANCE_CHECK(report, result.value() == adapter_service);        // the adapter's own object
    } else {
        KRITVA_CONFORMANCE_CHECK(report, !result.has_value());
        if (!result) {
            KRITVA_CONFORMANCE_CHECK(report, result.error().code == ErrorCode::UNSUPPORTED);
            KRITVA_CONFORMANCE_CHECK(report, result.error().severity == ErrorSeverity::ERROR);
            KRITVA_CONFORMANCE_CHECK(report, result.error().message.find(name) != std::string::npos);
            if (context.bound()) KRITVA_CONFORMANCE_CHECK(report, result.error().source == context.id());   // attributed to the component
            else KRITVA_CONFORMANCE_CHECK(report, !result.error().source.valid());                         // nothing to attribute to
        }
    }
}

} // namespace detail

/// Contract checks for `context`, which must have been built over `adapter` (nullptr: no platform).
inline void check_component_context(const runtime::ComponentContext& context, const IPlatformAdapter* adapter, Report& report) {
    // Identity.
    KRITVA_CONFORMANCE_CHECK(report, context.bound() == (context.info() != nullptr));
    if (context.bound()) {
        KRITVA_CONFORMANCE_CHECK(report, context.id().valid());
        KRITVA_CONFORMANCE_CHECK(report, context.id() == context.info()->id());
    } else {
        KRITVA_CONFORMANCE_CHECK(report, !context.id().valid());
    }

    // The platform view is the adapter's (or unattached when none was given).
    KRITVA_CONFORMANCE_CHECK(report, context.platform().attached() == (adapter != nullptr));
    if (adapter == nullptr) {
        KRITVA_CONFORMANCE_CHECK(report, context.platform().info() == nullptr);
        KRITVA_CONFORMANCE_CHECK(report, context.platform().scheduler() == nullptr && context.platform().clock() == nullptr);
        KRITVA_CONFORMANCE_CHECK(report, context.platform().timer() == nullptr && context.platform().watchdog() == nullptr);
    } else {
        KRITVA_CONFORMANCE_CHECK(report, context.platform().info() == &adapter->info());
    }

    // The typed queries.
    IScheduler* scheduler = adapter ? adapter->scheduler() : nullptr;
    time::IClock* clock = adapter ? adapter->clock() : nullptr;
    time::ITimer* timer = adapter ? adapter->timer() : nullptr;
    IWatchdog* watchdog = adapter ? adapter->watchdog() : nullptr;
    detail::check_service_query(context, context.require_scheduler(), scheduler, "scheduler", report);
    detail::check_service_query(context, context.require_clock(), clock, "clock", report);
    detail::check_service_query(context, context.require_timer(), timer, "timer", report);
    detail::check_service_query(context, context.require_watchdog(), watchdog, "watchdog", report);
    KRITVA_CONFORMANCE_CHECK(report, context.supports(PlatformService::SCHEDULER) == (scheduler != nullptr));
    KRITVA_CONFORMANCE_CHECK(report, context.supports(PlatformService::CLOCK) == (clock != nullptr));
    KRITVA_CONFORMANCE_CHECK(report, context.supports(PlatformService::TIMER) == (timer != nullptr));
    KRITVA_CONFORMANCE_CHECK(report, context.supports(PlatformService::WATCHDOG) == (watchdog != nullptr));
    KRITVA_CONFORMANCE_CHECK(report, !context.supports(static_cast<PlatformService>(200)));

    // Capability identity.
    const CapabilitySet capabilities = adapter ? adapter->capabilities() : CapabilitySet{};
    for (const Capability& capability : capabilities.all()) KRITVA_CONFORMANCE_CHECK(report, context.has_capability(capability.id));
    KRITVA_CONFORMANCE_CHECK(report, !context.has_capability(CapabilityId{}));
    CapabilityId absent{1};
    while (capabilities.contains(absent)) absent = CapabilityId{absent.value() + 1};
    KRITVA_CONFORMANCE_CHECK(report, !context.has_capability(absent));

    // Identity is authoritative: nothing about the platform's name or version is ever a capability.
    if (adapter != nullptr) {
        const PlatformInfo& info = adapter->info();
        for (const std::uint64_t candidate : {static_cast<std::uint64_t>(info.name.size()), static_cast<std::uint64_t>(info.version.major),
                                              static_cast<std::uint64_t>(info.version.minor), static_cast<std::uint64_t>(info.version.patch)}) {
            if (candidate != 0 && !capabilities.contains(CapabilityId{candidate})) KRITVA_CONFORMANCE_CHECK(report, !context.has_capability(CapabilityId{candidate}));
        }
    }

    // Attribution: only the source changes (bound), nothing changes (unbound).
    Error probe;
    probe.code = ErrorCode::RESOURCE_UNAVAILABLE;
    probe.severity = ErrorSeverity::WARNING;
    probe.source = Id{987654};
    probe.timestamp = Timestamp(42, ClockDomain::REALTIME);
    probe.message = "probe";
    const Error attributed = context.attribute(probe);
    KRITVA_CONFORMANCE_CHECK(report, attributed.code == probe.code && attributed.severity == probe.severity);
    KRITVA_CONFORMANCE_CHECK(report, attributed.message == probe.message && attributed.timestamp == probe.timestamp);
    KRITVA_CONFORMANCE_CHECK(report, context.bound() ? attributed.source == context.id() : attributed.source == probe.source);

    // Requirement binding equals R0.5, with the source rule for the failing check.
    PlatformRequirements satisfiable, unsatisfiable;
    for (const PlatformService service : {PlatformService::SCHEDULER, PlatformService::CLOCK, PlatformService::TIMER, PlatformService::WATCHDOG}) {
        const bool provided = adapter && adapter->supports(service);
        if (provided) (void)satisfiable.add_service(service, Requirement::REQUIRED);
        else (void)unsatisfiable.add_service(service, Requirement::REQUIRED);
    }
    (void)unsatisfiable.add_capability(absent, Requirement::REQUIRED);                        // never provided
    // OPTIONAL items: reported as missing when absent, and they never fail a check.
    PlatformRequirements optional_only;
    for (const PlatformService service : {PlatformService::SCHEDULER, PlatformService::CLOCK, PlatformService::TIMER, PlatformService::WATCHDOG}) {
        (void)optional_only.add_service(service, Requirement::OPTIONAL);
    }
    (void)optional_only.add_capability(absent, Requirement::OPTIONAL);
    PlatformRequirements mixed = unsatisfiable;
    (void)mixed.add_capability(CapabilityId{absent.value() + 1}, Requirement::OPTIONAL);
    const PlatformContext& view = context.platform();
    KRITVA_CONFORMANCE_CHECK(report, detail::same_report(context.evaluate(satisfiable), evaluate(satisfiable, view)));
    KRITVA_CONFORMANCE_CHECK(report, detail::same_report(context.evaluate(unsatisfiable), evaluate(unsatisfiable, view)));
    KRITVA_CONFORMANCE_CHECK(report, detail::same_report(context.evaluate(optional_only), evaluate(optional_only, view)));
    KRITVA_CONFORMANCE_CHECK(report, detail::same_report(context.evaluate(mixed), evaluate(mixed, view)));
    KRITVA_CONFORMANCE_CHECK(report, !context.evaluate(optional_only).missing_optional_capabilities.empty());   // the absent optional capability is reported
    KRITVA_CONFORMANCE_CHECK(report, context.evaluate(optional_only).missing_required_services.empty());       // and is never a required one
    KRITVA_CONFORMANCE_CHECK(report, context.check_required(optional_only).has_value());                       // optional items never fail a check
    KRITVA_CONFORMANCE_CHECK(report, context.check_required(satisfiable).has_value());
    const auto failed = context.check_required(unsatisfiable);
    KRITVA_CONFORMANCE_CHECK(report, !failed.has_value());
    if (!failed) {
        const Error r05 = check_required(unsatisfiable, view).error();
        KRITVA_CONFORMANCE_CHECK(report, failed.error().code == ErrorCode::UNSUPPORTED && failed.error().code == r05.code);
        KRITVA_CONFORMANCE_CHECK(report, failed.error().severity == r05.severity && failed.error().message == r05.message && failed.error().timestamp == r05.timestamp);
        KRITVA_CONFORMANCE_CHECK(report, context.bound() ? failed.error().source == context.id() : !failed.error().source.valid());
    }

    // Determinism.
    KRITVA_CONFORMANCE_CHECK(report, detail::same_report(context.evaluate(unsatisfiable), context.evaluate(unsatisfiable)));
    KRITVA_CONFORMANCE_CHECK(report, context.has_capability(absent) == context.has_capability(absent));
}

} // namespace kritva::core::platform::conformance
