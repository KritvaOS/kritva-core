//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : timer_conformance.hpp
// Description : Conformance checks for time::ITimer.
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

#include <atomic>

#include <kritva/core/time/timer.hpp>

namespace kritva::core::platform::conformance {

namespace detail {

struct TimerProbe {
    std::atomic<int> fired{0};
    time::ITimer* timer{nullptr};
    std::atomic<int> stop_code{-1};     // ErrorCode seen by a stop() made from the callback
    std::atomic<int> start_code{-1};    // ErrorCode seen by a start() made from the callback
    Duration period{Duration::from_milliseconds(50)};
};

inline void count_fire(void* context) {
    if (context != nullptr) ++static_cast<TimerProbe*>(context)->fired;
}

inline void reenter(void* context) {
    auto* probe = static_cast<TimerProbe*>(context);
    ++probe->fired;
    const Result<void> stopped = probe->timer->stop();
    probe->stop_code = stopped ? static_cast<int>(ErrorCode::NONE) : static_cast<int>(stopped.error().code);
    const Result<void> started = probe->timer->start(probe->period, time::TimerMode::ONE_SHOT, Callback{&count_fire, nullptr});
    probe->start_code = started ? static_cast<int>(ErrorCode::NONE) : static_cast<int>(started.error().code);
}

inline bool timer_failed_with(const Result<void>& result, ErrorCode code) {
    return !result && result.error().code == code;
}

} // namespace detail

/// ITimer (CORE-TIME-002, CORE-PLAT-006) on a FRESH timer. Checks the mandatory rules:
/// argument validation, STOPPED/RUNNING states, one-shot fires exactly once and ends
/// STOPPED, no callback after stop() returned, idempotent stop, restart, and
/// INVALID_STATE for start()/stop() from the timer's own callback. Not checked
/// (adapter-defined): accuracy, jitter, overrun handling, how many times a periodic
/// timer fires, resolution, maximum period. A mode or period the adapter cannot provide
/// (UNSUPPORTED / RESOURCE_UNAVAILABLE) is recorded as a skip.
inline void check_timer(time::ITimer& timer, const Environment& env, Report& report) {
    using detail::timer_failed_with;
    using time::TimerMode;
    const Duration period = env.timer_period;
    detail::TimerProbe probe;
    const Callback counting{&detail::count_fire, &probe};

    // STOPPED at first: stop() is an idempotent success.
    KRITVA_CONFORMANCE_CHECK(report, timer.stop());
    KRITVA_CONFORMANCE_CHECK(report, timer.stop());

    // Invalid arguments are INVALID_ARGUMENT, for every mode, and leave the timer STOPPED.
    for (const TimerMode mode : {TimerMode::ONE_SHOT, TimerMode::PERIODIC}) {
        KRITVA_CONFORMANCE_CHECK(report, timer_failed_with(timer.start(Duration::from_nanoseconds(0), mode, counting), ErrorCode::INVALID_ARGUMENT));
        KRITVA_CONFORMANCE_CHECK(report, timer_failed_with(timer.start(Duration::from_milliseconds(-5), mode, counting), ErrorCode::INVALID_ARGUMENT));
        KRITVA_CONFORMANCE_CHECK(report, timer_failed_with(timer.start(period, mode, Callback{}), ErrorCode::INVALID_ARGUMENT));
    }
    env.let_time_pass(times(period, 3));
    KRITVA_CONFORMANCE_CHECK(report, probe.fired == 0);                   // nothing was activated by the rejected requests

    // ONE_SHOT: fires exactly once, then the timer is STOPPED and restartable.
    const Result<void> one_shot = timer.start(period, TimerMode::ONE_SHOT, counting);
    if (!one_shot) {
        if (is_capability_limit(one_shot.error().code)) {
            report.skip("timer: the adapter cannot start a one-shot timer with Environment::timer_period");
        } else {
            KRITVA_CONFORMANCE_CHECK(report, false && "start(ONE_SHOT) failed with a code the contract does not permit");
        }
        return;
    }
    KRITVA_CONFORMANCE_CHECK(report, timer_failed_with(timer.start(period, TimerMode::ONE_SHOT, counting), ErrorCode::INVALID_STATE));   // running
    env.let_time_pass(times(period, 5));
    KRITVA_CONFORMANCE_CHECK(report, probe.fired == 1);                   // exactly once
    env.let_time_pass(times(period, 5));
    KRITVA_CONFORMANCE_CHECK(report, probe.fired == 1);                   // and not again
    KRITVA_CONFORMANCE_CHECK(report, timer.stop());                       // already stopped: success
    const Result<void> again = timer.start(period, TimerMode::ONE_SHOT, counting);   // restartable
    KRITVA_CONFORMANCE_CHECK(report, again.has_value());
    if (again) {
        env.let_time_pass(times(period, 5));
        KRITVA_CONFORMANCE_CHECK(report, probe.fired == 2);               // a fresh activation fires once more
    }

    // A one-shot stopped before it fires never fires.
    probe.fired = 0;
    if (timer.start(period, TimerMode::ONE_SHOT, counting)) {
        KRITVA_CONFORMANCE_CHECK(report, timer.stop());
        env.let_time_pass(times(period, 5));
        KRITVA_CONFORMANCE_CHECK(report, probe.fired == 0);
    }

    // PERIODIC: running rejects start(); after stop() returned no callback is executed or begins.
    probe.fired = 0;
    const Result<void> periodic = timer.start(period, TimerMode::PERIODIC, counting);
    if (!periodic) {
        if (is_capability_limit(periodic.error().code)) {
            report.skip("timer: the adapter does not provide periodic timers");
        } else {
            KRITVA_CONFORMANCE_CHECK(report, false && "start(PERIODIC) failed with a code the contract does not permit");
        }
    } else {
        KRITVA_CONFORMANCE_CHECK(report, timer_failed_with(timer.start(period, TimerMode::PERIODIC, counting), ErrorCode::INVALID_STATE));
        env.let_time_pass(times(period, 5));                              // zero or more firings: not counted
        KRITVA_CONFORMANCE_CHECK(report, timer.stop());
        const int at_stop = probe.fired;
        env.let_time_pass(times(period, 5));
        KRITVA_CONFORMANCE_CHECK(report, probe.fired == at_stop);         // nothing after stop() returned
        KRITVA_CONFORMANCE_CHECK(report, timer.stop());                   // idempotent
    }

    // start()/stop() from the timer's own callback fail with INVALID_STATE (no deadlock).
    detail::TimerProbe reentrant;
    reentrant.timer = &timer;
    reentrant.period = period;
    if (timer.start(period, TimerMode::ONE_SHOT, Callback{&detail::reenter, &reentrant})) {
        env.let_time_pass(times(period, 5));
        KRITVA_CONFORMANCE_CHECK(report, reentrant.fired == 1);
        KRITVA_CONFORMANCE_CHECK(report, reentrant.stop_code == static_cast<int>(ErrorCode::INVALID_STATE));
        KRITVA_CONFORMANCE_CHECK(report, reentrant.start_code == static_cast<int>(ErrorCode::INVALID_STATE));
        KRITVA_CONFORMANCE_CHECK(report, timer.stop());
    }

    // A null context is valid.
    if (timer.start(period, TimerMode::ONE_SHOT, Callback{&detail::count_fire, nullptr})) {
        env.let_time_pass(times(period, 5));
    }
    KRITVA_CONFORMANCE_CHECK(report, timer.stop());
}

} // namespace kritva::core::platform::conformance
