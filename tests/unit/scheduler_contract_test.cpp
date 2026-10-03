//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : scheduler_contract_test.cpp
// Description : IScheduler contract tests against a reference implementation.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-005
// API         : CORE-TEST-SCHEDULER-CONTRACT
//
// Author      : KritvaOS Core Team
// Created     : 04-10-2026
//==============================================================================

#include <cassert>
#include <cstdint>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

#include <kritva/core/core.hpp>

#include "../contract/reference_scheduler.hpp"
#include "../contract/scheduler_contract.hpp"

using namespace kritva::core;
using namespace kritva::core::platform;
using kritva::core::platform::contract::ReferenceScheduler;

namespace {

struct Counter { int value{0}; };
void count(void* context) { if (context != nullptr) ++static_cast<Counter*>(context)->value; }
void noop(void*) {}

// -----------------------------------------------------------------------------
// Shape and defaults
// -----------------------------------------------------------------------------

void test_contract_shape_and_defaults() {
    static_assert(std::is_abstract_v<IScheduler>);
    static_assert(std::has_virtual_destructor_v<IScheduler>);
    static_assert(std::is_same_v<TaskId, std::uint64_t>);

    const TaskConfig config;
    assert(std::string(config.name) == "kritva");
    assert(config.priority == 0 && config.cpu_affinity == 0 && config.period == Duration{});   // zero values mean: default, unconstrained, aperiodic
}

void test_reference_conforms_to_both_adapter_policies() {
    ReferenceScheduler rejecting;                             // dynamic creation unsupported
    contract::check_scheduler_contract(rejecting);
    ReferenceScheduler dynamic;
    dynamic.policy.dynamic_creation = true;                   // dynamic creation supported
    contract::check_scheduler_contract(dynamic);
}

// -----------------------------------------------------------------------------
// TaskId: zero invalid, unique, atomic failure consumes nothing
// -----------------------------------------------------------------------------

void test_task_ids_are_non_zero_unique_and_not_consumed_by_failures() {
    ReferenceScheduler scheduler;
    std::set<TaskId> seen;
    const auto first = scheduler.create_task(TaskConfig{}, noop, nullptr);
    assert(first && first.value() != 0);
    seen.insert(first.value());

    // Failed requests change nothing and consume no id.
    assert(!scheduler.create_task(TaskConfig{}, nullptr, nullptr));
    TaskConfig bad;
    bad.period = Duration::from_nanoseconds(-5);
    assert(!scheduler.create_task(bad, noop, nullptr));
    TaskConfig no_name;
    no_name.name = nullptr;
    assert(!scheduler.create_task(no_name, noop, nullptr));
    assert(scheduler.task_count() == 1);

    const auto second = scheduler.create_task(TaskConfig{}, noop, nullptr);
    assert(second && second.value() != 0 && seen.insert(second.value()).second);
    assert(second.value() == first.value() + 1);              // the reference numbering shows nothing was consumed
}

// -----------------------------------------------------------------------------
// TaskConfig semantics
// -----------------------------------------------------------------------------

void test_priority_affinity_and_period_semantics() {
    ReferenceScheduler s;
    TaskConfig config;

    // Affinity: 0 = unconstrained, a valid mask is honoured, an invalid mask and unsupported affinity differ.
    config.cpu_affinity = 0;
    assert(s.create_task(config, noop, nullptr));
    config.cpu_affinity = 0x1;
    assert(s.create_task(config, noop, nullptr));
    config.cpu_affinity = 0x10;                                // selects no existing CPU: malformed
    const auto malformed = s.create_task(config, noop, nullptr);
    assert(!malformed && malformed.error().code == ErrorCode::INVALID_ARGUMENT);

    ReferenceScheduler no_affinity;
    no_affinity.policy.affinity_supported = false;
    config.cpu_affinity = 0x1;                                 // well formed but unsupported
    const auto unsupported = no_affinity.create_task(config, noop, nullptr);
    assert(!unsupported && unsupported.error().code == ErrorCode::UNSUPPORTED);
    config.cpu_affinity = 0;
    assert(no_affinity.create_task(config, noop, nullptr));    // 0 never needs affinity support

    // Priority is relative and its range is adapter-defined (here: rejected above the adapter maximum).
    ReferenceScheduler prio;
    prio.policy.max_priority = 10;
    config.priority = 10;
    assert(prio.create_task(config, noop, nullptr));
    config.priority = 11;
    const auto too_high = prio.create_task(config, noop, nullptr);
    assert(!too_high && too_high.error().code == ErrorCode::INVALID_ARGUMENT);
    ReferenceScheduler wide;                                   // another adapter, another range: still conforming
    wide.policy.max_priority = 1000;
    config.priority = 500;
    assert(wide.create_task(config, noop, nullptr));

    // Period: zero = aperiodic (once per start), positive = periodic, negative invalid.
    ReferenceScheduler timing;
    Counter aperiodic, periodic;
    TaskConfig once;
    TaskConfig repeating;
    repeating.period = Duration::from_milliseconds(10);
    assert(timing.create_task(once, count, &aperiodic) && timing.create_task(repeating, count, &periodic));
    assert(timing.start());
    assert(aperiodic.value == 1 && periodic.value == 0);       // periodic waits for its period
    timing.tick(); timing.tick();
    assert(aperiodic.value == 1 && periodic.value == 2);
    assert(timing.stop() && timing.start());
    assert(aperiodic.value == 2);                              // aperiodic runs once per start()
}

// -----------------------------------------------------------------------------
// start / stop, dynamic creation, exhaustion, failure atomicity
// -----------------------------------------------------------------------------

void test_start_and_stop_are_idempotent_and_failure_is_atomic() {
    ReferenceScheduler s;
    Counter c;
    assert(s.create_task(TaskConfig{}, count, &c));
    assert(s.start() && s.start());                            // idempotent
    assert(c.value == 1 && s.running());                       // the second start() ran nothing again
    assert(s.stop() && s.stop() && !s.running());             // idempotent
    ReferenceScheduler never_started;
    assert(never_started.stop() && !never_started.running());  // stop before start

    // All-or-nothing activation: a task that cannot be activated leaves the scheduler STOPPED.
    ReferenceScheduler failing;
    Counter a, b;
    assert(failing.create_task(TaskConfig{}, count, &a) && failing.create_task(TaskConfig{}, count, &b));
    failing.policy.fail_start_at = 1;
    const auto r = failing.start();
    assert(!r && r.error().code == ErrorCode::RESOURCE_UNAVAILABLE);
    assert(!failing.running() && a.value == 0 && b.value == 0);   // nothing was left running or invoked
    failing.policy.fail_start_at = static_cast<std::size_t>(-1);
    assert(failing.start() && a.value == 1 && b.value == 1);      // and it can be started afterwards
}

void test_dynamic_creation_is_adapter_policy() {
    for (const bool dynamic : {false, true}) {
        ReferenceScheduler s;
        s.policy.dynamic_creation = dynamic;
        Counter c;
        assert(s.start());
        const auto r = s.create_task(TaskConfig{}, count, &c);
        if (dynamic) {
            assert(r && r.value() != 0 && s.task_count() == 1);   // (a) joins the running set
            assert(c.value == 1);
        } else {
            assert(!r && r.error().code == ErrorCode::INVALID_STATE);   // (b) refused, atomically
            assert(s.task_count() == 0 && c.value == 0);
            assert(s.stop());
            assert(s.create_task(TaskConfig{}, count, &c));       // portable: create while STOPPED
        }
    }
}

void test_resource_exhaustion_is_reported_and_atomic() {
    ReferenceScheduler s;
    s.policy.capacity = 2;
    assert(s.create_task(TaskConfig{}, noop, nullptr) && s.create_task(TaskConfig{}, noop, nullptr));
    const auto full = s.create_task(TaskConfig{}, noop, nullptr);
    assert(!full && full.error().code == ErrorCode::RESOURCE_UNAVAILABLE);
    assert(s.task_count() == 2);                               // unchanged, no exception, no abort
    assert(s.start() && s.stop());                             // still usable
}

// -----------------------------------------------------------------------------
// Entry / context lifetime and execution rules
// -----------------------------------------------------------------------------

struct Observer {
    ReferenceScheduler* scheduler{nullptr};
    int calls{0};
    ErrorCode stop_result{ErrorCode::NONE};
    int nested_ticks{0};
    bool running_inside{false};
};

void stop_from_entry(void* context) {
    auto* o = static_cast<Observer*>(context);
    ++o->calls;
    o->running_inside = o->scheduler->running();
    const auto r = o->scheduler->stop();                       // prohibited: would wait on itself
    o->stop_result = r ? ErrorCode::NONE : r.error().code;
}

void overlapping_entry(void* context) {
    auto* o = static_cast<Observer*>(context);
    ++o->calls;
    const int before = o->scheduler->overruns_skipped;
    o->scheduler->tick();                                      // the same task is due again while still executing
    o->nested_ticks += o->scheduler->overruns_skipped - before;
}

void test_entries_run_only_while_running_with_the_callers_context() {
    ReferenceScheduler s;
    Counter counter;
    TaskConfig periodic;
    periodic.period = Duration::from_milliseconds(1);
    assert(s.create_task(periodic, count, &counter));
    s.tick();
    assert(counter.value == 0);                                // not started: no invocation
    assert(s.start());
    s.tick(); s.tick();
    assert(counter.value == 2);                                // the very context pointer, unchanged
    assert(s.stop());
    s.tick(); s.tick();
    assert(counter.value == 2);                                // after stop() returned: never again

    // The caller owns the context; the scheduler never touched or freed it.
    Counter* heap = new Counter;
    {
        ReferenceScheduler local;
        assert(local.create_task(TaskConfig{}, count, heap) && local.start() && local.stop());
    }
    assert(heap->value == 1);
    delete heap;
}

void test_stop_from_an_entry_is_rejected_not_deadlocked() {
    ReferenceScheduler s;
    Observer o;
    o.scheduler = &s;
    assert(s.create_task(TaskConfig{}, stop_from_entry, &o));
    assert(s.start());
    assert(o.calls == 1 && o.running_inside);
    assert(o.stop_result == ErrorCode::INVALID_STATE);         // INVALID_STATE instead of waiting on itself
    assert(s.running());                                       // and the scheduler is still RUNNING
    assert(s.stop() && !s.running());                          // an external stop works
}

void test_a_task_never_overlaps_itself() {
    ReferenceScheduler s;
    Observer o;
    o.scheduler = &s;
    TaskConfig periodic;
    periodic.period = Duration::from_milliseconds(1);
    assert(s.create_task(periodic, overlapping_entry, &o));
    assert(s.start());
    s.tick();                                                  // runs the entry; inside it a nested tick is due
    assert(o.calls == 1);                                      // the nested activation did not re-enter the entry
    assert(o.nested_ticks == 1 && s.overruns_skipped == 1);    // the overrun was skipped (adapter policy), not overlapped
    assert(s.invocations(1) == 1);
}

} // namespace

int main() {
    test_contract_shape_and_defaults();
    test_reference_conforms_to_both_adapter_policies();
    test_task_ids_are_non_zero_unique_and_not_consumed_by_failures();
    test_priority_affinity_and_period_semantics();
    test_start_and_stop_are_idempotent_and_failure_is_atomic();
    test_dynamic_creation_is_adapter_policy();
    test_resource_exhaustion_is_reported_and_atomic();
    test_entries_run_only_while_running_with_the_callers_context();
    test_stop_from_an_entry_is_rejected_not_deadlocked();
    test_a_task_never_overlaps_itself();
    return 0;
}
