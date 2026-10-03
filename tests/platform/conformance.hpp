//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : conformance.hpp
// Description : Platform conformance suite: report, environment and umbrella header.
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

// Reusable conformance suite for platform adapters (CORE-PLAT-009).
//
// WHAT IT IS. Header-only checks that an external adapter (Linux, RTOS, MCU,
// simulator, ...) runs against its own IScheduler, time::IClock, time::ITimer,
// IWatchdog and IPlatformAdapter implementations. Everything here uses only the
// public Core headers in include/kritva/core/ and the C++ standard library: no
// hardware, network or vendor SDK, no Core production helper added for testing.
//
// HOW TO USE IT. Construct a FRESH object (services start STOPPED), build an
// Environment, and run the check; failures are collected in a Report instead of
// aborting, so an adapter's own test can print them:
//
//     kritva::core::platform::conformance::Report report;
//     kritva::core::platform::conformance::Environment env;
//     env.let_time_pass = [](kritva::core::Duration d) { /* sleep or advance a fake clock */ };
//     kritva::core::platform::conformance::check_timer(my_timer, env, report);
//     assert(report.ok());
//
// WHAT IT CHECKS. Only MANDATORY Core semantics (the contracts in
// platform/scheduler.hpp, time/clock.hpp, time/timer.hpp, platform/watchdog.hpp,
// platform/adapter.hpp). Adapter-defined behavior is never asserted as
// universal: where the contract lets an adapter choose (affinity, priority
// range, capacity, dynamic task creation, supported timer modes, resolution and
// ranges, whether a watchdog can be stopped) the suite accepts every permitted
// outcome and, when an adapter legitimately cannot provide a feature
// (UNSUPPORTED / RESOURCE_UNAVAILABLE), records a skip rather than a failure.
// Anything outside the permitted set of outcomes is a failure.
//
// LIMITS. The suite cannot observe what a hardware watchdog does on expiry, and
// timing accuracy, jitter and overrun handling are adapter-defined and not
// measured. Time-dependent checks use Environment::let_time_pass, which the
// adapter maps to real waiting or to a simulated clock.

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include <kritva/core/error/error_code.hpp>
#include <kritva/core/types/duration.hpp>

namespace kritva::core::platform::conformance {

/// Outcome of a conformance run: counts checks, collects failures and skips.
class Report {
public:
    void check(bool condition, const char* expression, const char* file, int line) {
        ++checks_;
        if (!condition) failures_.push_back(std::string(file) + ":" + std::to_string(line) + ": " + expression);
    }
    void skip(const std::string& reason) { skips_.push_back(reason); }

    [[nodiscard]] bool ok() const noexcept { return failures_.empty(); }
    [[nodiscard]] std::size_t checks() const noexcept { return checks_; }
    [[nodiscard]] const std::vector<std::string>& failures() const noexcept { return failures_; }
    [[nodiscard]] const std::vector<std::string>& skips() const noexcept { return skips_; }

private:
    std::size_t checks_{0};
    std::vector<std::string> failures_;
    std::vector<std::string> skips_;
};

#define KRITVA_CONFORMANCE_CHECK(report, condition) (report).check(static_cast<bool>(condition), #condition, __FILE__, __LINE__)

/// What the adapter supplies to the suite.
struct Environment {
    /// Lets the given amount of time pass for the adapter's services: a real adapter
    /// sleeps, a simulated adapter advances its fake clock. Must be set. May be
    /// called from the thread running the suite only.
    std::function<void(Duration)> let_time_pass;

    /// Period used for timers. Must be one the adapter supports (>= its resolution)
    /// and long enough that two consecutive calls do not race it (the suite checks
    /// "start while running" right after a start).
    Duration timer_period{Duration::from_milliseconds(50)};

    /// Period used for periodic scheduler tasks.
    Duration task_period{Duration::from_milliseconds(5)};

    /// Watchdog timeout: long enough that no check can let it expire.
    Duration watchdog_timeout{Duration::from_milliseconds(1000)};
};

/// True when `code` is one of the codes the contract permits for "the adapter cannot
/// provide this" (a skip, not a failure).
inline bool is_capability_limit(ErrorCode code) noexcept {
    return code == ErrorCode::UNSUPPORTED || code == ErrorCode::RESOURCE_UNAVAILABLE;
}

inline Duration times(Duration d, int n) noexcept { return Duration::from_nanoseconds(d.nanoseconds() * n); }

} // namespace kritva::core::platform::conformance
