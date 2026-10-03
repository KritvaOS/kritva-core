//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : watchdog_conformance.hpp
// Description : Conformance checks for platform::IWatchdog.
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

#include "conformance.hpp"

#include <kritva/core/platform/watchdog.hpp>

namespace kritva::core::platform::conformance {

namespace detail {
inline bool failed_with(const Result<void>& result, ErrorCode code) {
    return !result && result.error().code == code;
}
} // namespace detail

/// IWatchdog (CORE-PLAT-003, CORE-PLAT-007) on a FRESH watchdog. The suite never lets a
/// started watchdog expire (Environment::watchdog_timeout is long and no time is passed
/// while it runs). Not checked (adapter-defined): the expiry action, the supported
/// timeout range, and whether stop() is possible. If the adapter cannot stop a running
/// watchdog (stop() returns UNSUPPORTED) the watchdog is left running.
inline void check_watchdog(IWatchdog& watchdog, const Environment& env, Report& report) {
    using detail::failed_with;

    // Initial state STOPPED: stop() is an idempotent success, kick() is INVALID_STATE.
    KRITVA_CONFORMANCE_CHECK(report, watchdog.stop());
    KRITVA_CONFORMANCE_CHECK(report, watchdog.stop());
    KRITVA_CONFORMANCE_CHECK(report, failed_with(watchdog.kick(), ErrorCode::INVALID_STATE));

    // Zero and negative timeouts are INVALID_ARGUMENT and leave the watchdog STOPPED.
    KRITVA_CONFORMANCE_CHECK(report, failed_with(watchdog.start(Duration::from_nanoseconds(0)), ErrorCode::INVALID_ARGUMENT));
    KRITVA_CONFORMANCE_CHECK(report, failed_with(watchdog.start(Duration::from_milliseconds(-5)), ErrorCode::INVALID_ARGUMENT));
    KRITVA_CONFORMANCE_CHECK(report, failed_with(watchdog.kick(), ErrorCode::INVALID_STATE));   // kick() never starts it

    const Result<void> started = watchdog.start(env.watchdog_timeout);
    if (!started) {
        if (is_capability_limit(started.error().code)) {
            report.skip("watchdog: the adapter cannot start with Environment::watchdog_timeout");
        } else {
            KRITVA_CONFORMANCE_CHECK(report, false && "start() failed with a code the contract does not permit");
        }
        KRITVA_CONFORMANCE_CHECK(report, failed_with(watchdog.kick(), ErrorCode::INVALID_STATE));   // failed start: still stopped
        return;
    }

    // RUNNING: start() is INVALID_STATE, kick() succeeds.
    KRITVA_CONFORMANCE_CHECK(report, failed_with(watchdog.start(env.watchdog_timeout), ErrorCode::INVALID_STATE));
    KRITVA_CONFORMANCE_CHECK(report, watchdog.kick());
    KRITVA_CONFORMANCE_CHECK(report, watchdog.kick());

    const Result<void> stopped = watchdog.stop();
    if (!stopped) {
        KRITVA_CONFORMANCE_CHECK(report, failed_with(stopped, ErrorCode::UNSUPPORTED));   // the only permitted failure
        report.skip("watchdog: the adapter cannot stop a running watchdog (UNSUPPORTED); it stays running");
        KRITVA_CONFORMANCE_CHECK(report, watchdog.kick());                                // failure has no effect
        return;
    }

    // STOPPED again: idempotent stop, kick rejected, a fresh start works.
    KRITVA_CONFORMANCE_CHECK(report, watchdog.stop());
    KRITVA_CONFORMANCE_CHECK(report, failed_with(watchdog.kick(), ErrorCode::INVALID_STATE));
    KRITVA_CONFORMANCE_CHECK(report, watchdog.start(env.watchdog_timeout));
    KRITVA_CONFORMANCE_CHECK(report, watchdog.kick());
    KRITVA_CONFORMANCE_CHECK(report, watchdog.stop());
}

} // namespace kritva::core::platform::conformance
