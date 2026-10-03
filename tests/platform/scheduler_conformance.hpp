//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : scheduler_conformance.hpp
// Description : Conformance checks for platform::IScheduler.
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
#include <set>

#include <kritva/core/platform/scheduler.hpp>

namespace kritva::core::platform::conformance {

namespace detail {
inline void count_task(void* context) {
    if (context != nullptr) ++*static_cast<std::atomic<int>*>(context);
}
} // namespace detail

/// IScheduler (CORE-PLAT-001, CORE-PLAT-005) on a FRESH scheduler with capacity for at
/// least two tasks. Checks the mandatory rules: invalid arguments, non-zero distinct
/// ids, atomic failures, idempotent start/stop, restart, no entry before start() and
/// none after stop() returned. Not checked (adapter-defined): priority mapping and
/// range, which CPU masks exist, capacity, whether tasks can be created while RUNNING
/// (both permitted outcomes are accepted), periodic timing, how often an aperiodic task
/// runs. Postcondition: the scheduler is STOPPED.
inline void check_scheduler(IScheduler& scheduler, const Environment& env, Report& report) {
    const auto failed_with = [](const auto& result, ErrorCode code) { return !result && result.error().code == code; };
    std::atomic<int> aperiodic_runs{0};
    std::atomic<int> periodic_runs{0};
    std::set<TaskId> ids;

    // STOPPED at first: stop() is an idempotent success.
    KRITVA_CONFORMANCE_CHECK(report, scheduler.stop());
    KRITVA_CONFORMANCE_CHECK(report, scheduler.stop());

    // Invalid arguments: INVALID_ARGUMENT, and nothing is created.
    TaskConfig config;
    KRITVA_CONFORMANCE_CHECK(report, failed_with(scheduler.create_task(config, nullptr, &aperiodic_runs), ErrorCode::INVALID_ARGUMENT));
    TaskConfig no_name = config;
    no_name.name = nullptr;
    KRITVA_CONFORMANCE_CHECK(report, failed_with(scheduler.create_task(no_name, &detail::count_task, &aperiodic_runs), ErrorCode::INVALID_ARGUMENT));
    TaskConfig negative_period = config;
    negative_period.period = Duration::from_nanoseconds(-1);
    KRITVA_CONFORMANCE_CHECK(report, failed_with(scheduler.create_task(negative_period, &detail::count_task, &aperiodic_runs), ErrorCode::INVALID_ARGUMENT));

    // Registration: non-zero, distinct ids; registering never invokes the entry.
    const auto aperiodic = scheduler.create_task(config, &detail::count_task, &aperiodic_runs);
    KRITVA_CONFORMANCE_CHECK(report, aperiodic.has_value());
    TaskConfig periodic_config = config;
    periodic_config.period = env.task_period;
    const auto periodic = scheduler.create_task(periodic_config, &detail::count_task, &periodic_runs);
    KRITVA_CONFORMANCE_CHECK(report, periodic.has_value());
    if (aperiodic) { KRITVA_CONFORMANCE_CHECK(report, aperiodic.value() != 0); ids.insert(aperiodic.value()); }
    if (periodic) {
        KRITVA_CONFORMANCE_CHECK(report, periodic.value() != 0);
        KRITVA_CONFORMANCE_CHECK(report, ids.insert(periodic.value()).second);   // distinct
    }

    // No entry runs before start().
    env.let_time_pass(times(env.task_period, 5));
    KRITVA_CONFORMANCE_CHECK(report, aperiodic_runs == 0 && periodic_runs == 0);

    // Priority and affinity: only the permitted outcomes. A failure creates nothing (ids stay valid).
    {
        TaskConfig high = config;
        high.priority = 0xFFFFFFFFu;
        const auto r = scheduler.create_task(high, &detail::count_task, nullptr);
        if (r) {
            KRITVA_CONFORMANCE_CHECK(report, r.value() != 0 && ids.insert(r.value()).second);
        } else {
            KRITVA_CONFORMANCE_CHECK(report, r.error().code == ErrorCode::INVALID_ARGUMENT || r.error().code == ErrorCode::RESOURCE_UNAVAILABLE);
        }
        TaskConfig pinned = config;
        pinned.cpu_affinity = 0x1;   // CPU 0
        const auto p = scheduler.create_task(pinned, &detail::count_task, nullptr);
        if (p) {
            KRITVA_CONFORMANCE_CHECK(report, p.value() != 0 && ids.insert(p.value()).second);
        } else {
            KRITVA_CONFORMANCE_CHECK(report, p.error().code == ErrorCode::UNSUPPORTED || p.error().code == ErrorCode::INVALID_ARGUMENT
                                             || p.error().code == ErrorCode::RESOURCE_UNAVAILABLE);
        }
    }

    // start() is idempotent.
    KRITVA_CONFORMANCE_CHECK(report, scheduler.start());
    KRITVA_CONFORMANCE_CHECK(report, scheduler.start());

    // create_task() while RUNNING is adapter policy: success, or an atomic INVALID_STATE (RESOURCE_UNAVAILABLE if full).
    {
        const auto r = scheduler.create_task(config, &detail::count_task, nullptr);
        if (r) {
            KRITVA_CONFORMANCE_CHECK(report, r.value() != 0 && ids.insert(r.value()).second);
        } else {
            KRITVA_CONFORMANCE_CHECK(report, r.error().code == ErrorCode::INVALID_STATE || r.error().code == ErrorCode::RESOURCE_UNAVAILABLE);
        }
    }

    env.let_time_pass(times(env.task_period, 10));

    // After stop() returned, no entry is executing and none begins.
    KRITVA_CONFORMANCE_CHECK(report, scheduler.stop());
    const int aperiodic_at_stop = aperiodic_runs;
    const int periodic_at_stop = periodic_runs;
    env.let_time_pass(times(env.task_period, 10));
    KRITVA_CONFORMANCE_CHECK(report, aperiodic_runs == aperiodic_at_stop && periodic_runs == periodic_at_stop);
    KRITVA_CONFORMANCE_CHECK(report, scheduler.stop());   // idempotent

    // The scheduler can be restarted, and tasks created while STOPPED keep distinct ids.
    KRITVA_CONFORMANCE_CHECK(report, scheduler.start());
    KRITVA_CONFORMANCE_CHECK(report, scheduler.stop());
    const auto later = scheduler.create_task(config, &detail::count_task, nullptr);
    if (later) {
        KRITVA_CONFORMANCE_CHECK(report, later.value() != 0 && ids.insert(later.value()).second);
    } else {
        KRITVA_CONFORMANCE_CHECK(report, later.error().code == ErrorCode::RESOURCE_UNAVAILABLE);   // exhaustion is the only permitted refusal
    }

    // Exhaustion is reported as RESOURCE_UNAVAILABLE (no abort, no exception); ids stay distinct.
    for (int i = 0; i < 64; ++i) {
        const auto r = scheduler.create_task(config, &detail::count_task, nullptr);
        if (!r) {
            KRITVA_CONFORMANCE_CHECK(report, r.error().code == ErrorCode::RESOURCE_UNAVAILABLE);
            break;
        }
        KRITVA_CONFORMANCE_CHECK(report, r.value() != 0 && ids.insert(r.value()).second);
    }
    KRITVA_CONFORMANCE_CHECK(report, scheduler.stop());
}

} // namespace kritva::core::platform::conformance
