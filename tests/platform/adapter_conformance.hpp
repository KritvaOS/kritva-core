//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : adapter_conformance.hpp
// Description : Conformance checks for platform::IPlatformAdapter and the services it provides.
//
// Component   : Kritva Core
// Module      : Platform Conformance
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-009
// API         : CORE-TEST-PLATFORM-CONFORMANCE
//
// Author      : KritvaOS Core Team
// Created     : 04-10-2026
//==============================================================================

#pragma once

#include "clock_conformance.hpp"
#include "conformance.hpp"
#include "scheduler_conformance.hpp"
#include "timer_conformance.hpp"
#include "watchdog_conformance.hpp"

#include <kritva/core/platform/adapter.hpp>

namespace kritva::core::platform::conformance {

/// The identity, discovery and capability rules of IPlatformAdapter (CORE-PLAT-008) only;
/// it does not exercise the services (see check_platform_adapter). Not checked
/// (adapter-defined): which services and capabilities exist, the name and version values.
inline void check_adapter_description(const IPlatformAdapter& adapter, Report& report) {
    // Identity: non-empty name, immutable, same object and values on every call.
    const PlatformInfo& info = adapter.info();
    KRITVA_CONFORMANCE_CHECK(report, !info.name.empty());
    const std::string name = info.name;
    const Version version = info.version;
    KRITVA_CONFORMANCE_CHECK(report, &adapter.info() == &info);
    KRITVA_CONFORMANCE_CHECK(report, adapter.info().name == name && adapter.info().version == version);

    // Discovery: supports() equals "accessor non-null"; the same object on every call; unknown enumerator unsupported.
    KRITVA_CONFORMANCE_CHECK(report, adapter.supports(PlatformService::SCHEDULER) == (adapter.scheduler() != nullptr));
    KRITVA_CONFORMANCE_CHECK(report, adapter.supports(PlatformService::CLOCK) == (adapter.clock() != nullptr));
    KRITVA_CONFORMANCE_CHECK(report, adapter.supports(PlatformService::TIMER) == (adapter.timer() != nullptr));
    KRITVA_CONFORMANCE_CHECK(report, adapter.supports(PlatformService::WATCHDOG) == (adapter.watchdog() != nullptr));
    KRITVA_CONFORMANCE_CHECK(report, adapter.scheduler() == adapter.scheduler());
    KRITVA_CONFORMANCE_CHECK(report, adapter.clock() == adapter.clock());
    KRITVA_CONFORMANCE_CHECK(report, adapter.timer() == adapter.timer());
    KRITVA_CONFORMANCE_CHECK(report, adapter.watchdog() == adapter.watchdog());
    KRITVA_CONFORMANCE_CHECK(report, !adapter.supports(static_cast<PlatformService>(200)));

    // Capabilities: an owned snapshot, deterministic (same entries in the same order).
    const CapabilitySet first = adapter.capabilities();
    const CapabilitySet second = adapter.capabilities();
    KRITVA_CONFORMANCE_CHECK(report, first.size() == second.size());
    if (first.size() == second.size()) {
        for (std::size_t i = 0; i < first.size(); ++i) {
            KRITVA_CONFORMANCE_CHECK(report, first.all()[i].id == second.all()[i].id);
            KRITVA_CONFORMANCE_CHECK(report, first.all()[i].name == second.all()[i].name);
            KRITVA_CONFORMANCE_CHECK(report, first.all()[i].version == second.all()[i].version);
        }
    }
    for (const Capability& capability : first.all()) KRITVA_CONFORMANCE_CHECK(report, second.contains(capability.id));
}

/// Everything above plus the conformance of every service the adapter provides.
/// The adapter's services must be FRESH (never started). Services the adapter does not
/// provide are not checked.
inline void check_platform_adapter(const IPlatformAdapter& adapter, const Environment& env, Report& report) {
    check_adapter_description(adapter, report);
    if (IScheduler* scheduler = adapter.scheduler()) check_scheduler(*scheduler, env, report);
    if (time::IClock* clock = adapter.clock()) check_clock(*clock, env, report);
    if (time::ITimer* timer = adapter.timer()) check_timer(*timer, env, report);
    if (IWatchdog* watchdog = adapter.watchdog()) check_watchdog(*watchdog, env, report);
}

} // namespace kritva::core::platform::conformance
