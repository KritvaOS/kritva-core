//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : scheduler_contract.hpp
// Description : Reusable conformance checks for platform::IScheduler.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-001
// API         : CORE-TEST-SCHEDULER
//
// Author      : KritvaOS Core Team
// Created     : 02-10-2026
//==============================================================================

#pragma once

#include <cassert>
#include <kritva/core/platform/scheduler.hpp>

namespace kritva::core::platform::contract {

namespace detail {
inline void count_entry(void* context) {
    if (context != nullptr) {
        ++*static_cast<int*>(context);
    }
}
} // namespace detail

/// Checks the implementation-independent rules of the IScheduler contract
/// (scheduler.hpp). Preconditions for `scheduler`: freshly constructed
/// (STOPPED, no tasks), capacity for at least two tasks, and no requirement
/// on CPU affinity or priority support (the default TaskConfig is used).
/// Postcondition: the scheduler is STOPPED.
///
/// Not checked here because they are adapter-specific: capacity exhaustion,
/// affinity support, the priority maximum, and periodic timing.
inline void check_scheduler_contract(IScheduler& scheduler) {
    int runs = 0;
    const TaskConfig config{};

    // stop() on a never-started scheduler is an idempotent success.
    assert(scheduler.stop());
    assert(scheduler.stop());

    // Invalid arguments are rejected with INVALID_ARGUMENT and create nothing.
    {
        const auto no_entry = scheduler.create_task(config, nullptr, &runs);
        assert(!no_entry);
        assert(no_entry.error().code == ErrorCode::INVALID_ARGUMENT);

        TaskConfig no_name = config;
        no_name.name = nullptr;
        const auto r = scheduler.create_task(no_name, detail::count_entry, &runs);
        assert(!r);
        assert(r.error().code == ErrorCode::INVALID_ARGUMENT);

        TaskConfig negative_period = config;
        negative_period.period = Duration::from_nanoseconds(-1);
        const auto n = scheduler.create_task(negative_period, detail::count_entry, &runs);
        assert(!n);
        assert(n.error().code == ErrorCode::INVALID_ARGUMENT);
    }

    // create_task() registers a stopped task: non-zero id, entry not invoked,
    // null context allowed, ids distinct.
    const auto first = scheduler.create_task(config, detail::count_entry, &runs);
    assert(first);
    assert(first.value() != 0);
    assert(runs == 0);

    const auto second = scheduler.create_task(config, detail::count_entry, nullptr);
    assert(second);
    assert(second.value() != 0);
    assert(second.value() != first.value());
    assert(runs == 0);

    // start() is idempotent.
    assert(scheduler.start());
    assert(scheduler.start());

    // create_task() while RUNNING is INVALID_STATE and creates nothing.
    {
        const auto r = scheduler.create_task(config, detail::count_entry, &runs);
        assert(!r);
        assert(r.error().code == ErrorCode::INVALID_STATE);
    }

    // stop() is idempotent; the scheduler can be restarted; ids stay valid.
    assert(scheduler.stop());
    assert(scheduler.stop());
    assert(scheduler.start());
    assert(scheduler.stop());

    // A later task never reuses an earlier id.
    const auto third = scheduler.create_task(config, detail::count_entry, nullptr);
    assert(third);
    assert(third.value() != first.value());
    assert(third.value() != second.value());
}

} // namespace kritva::core::platform::contract
