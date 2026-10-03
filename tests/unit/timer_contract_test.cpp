//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : timer_contract_test.cpp
// Description : ITimer contract tests against a reference implementation.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-006
// API         : CORE-TEST-TIMER-CONTRACT
//
// Author      : KritvaOS Core Team
// Created     : 04-10-2026
//==============================================================================

#include <cassert>
#include <cstdint>
#include <type_traits>

#include <kritva/core/core.hpp>

#include "../contract/reference_timer.hpp"

using namespace kritva::core;
using namespace kritva::core::time;
using kritva::core::time::contract::ReferenceTimer;

namespace {

constexpr std::int64_t MS = 1'000'000;

struct Counter { int value{0}; };
void count(void* context) { if (context != nullptr) ++static_cast<Counter*>(context)->value; }
void noop(void*) {}

Duration ms(std::int64_t n) { return Duration::from_milliseconds(n); }

void expect_error(const Result<void>& result, ErrorCode code) {
    assert(!result);
    assert(result.error().code == code);
}

void test_shape() {
    static_assert(std::is_abstract_v<ITimer>);
    static_assert(std::has_virtual_destructor_v<ITimer>);
    static_assert(std::is_same_v<std::underlying_type_t<TimerMode>, std::uint8_t>);
    assert(TimerMode::ONE_SHOT != TimerMode::PERIODIC);
}

void test_invalid_arguments_leave_timer_stopped() {
    ReferenceTimer timer;
    expect_error(timer.start(Duration::from_nanoseconds(0), TimerMode::ONE_SHOT, Callback{&noop, nullptr}), ErrorCode::INVALID_ARGUMENT);
    expect_error(timer.start(Duration::from_nanoseconds(-5 * MS), TimerMode::PERIODIC, Callback{&noop, nullptr}), ErrorCode::INVALID_ARGUMENT);
    expect_error(timer.start(ms(1), TimerMode::ONE_SHOT, Callback{}), ErrorCode::INVALID_ARGUMENT);
    assert(!timer.running());
    // A null context is valid.
    assert(timer.start(ms(1), TimerMode::ONE_SHOT, Callback{&noop, nullptr}));
}

void test_one_shot_fires_once_then_stops() {
    ReferenceTimer timer;
    Counter counter;
    assert(timer.start(ms(5), TimerMode::ONE_SHOT, Callback{&count, &counter}));
    timer.advance(4 * MS);
    assert(counter.value == 0 && timer.running());            // not before one period
    timer.advance(1 * MS);
    assert(counter.value == 1 && !timer.running());           // fired, now STOPPED
    timer.advance(50 * MS);
    assert(counter.value == 1);                               // exactly once
    assert(timer.start(ms(5), TimerMode::ONE_SHOT, Callback{&count, &counter}));   // may be started again
}

void test_periodic_fires_until_stop() {
    ReferenceTimer timer;
    Counter counter;
    assert(timer.start(ms(2), TimerMode::PERIODIC, Callback{&count, &counter}));
    timer.advance(6 * MS);
    assert(counter.value == 3 && timer.running());
    assert(timer.stop());
    timer.advance(100 * MS);
    assert(counter.value == 3);                               // nothing after stop() returned
}

void test_start_while_running_is_rejected_and_has_no_effect() {
    ReferenceTimer timer;
    Counter first, second;
    assert(timer.start(ms(2), TimerMode::PERIODIC, Callback{&count, &first}));
    expect_error(timer.start(ms(1), TimerMode::ONE_SHOT, Callback{&count, &second}), ErrorCode::INVALID_STATE);
    timer.advance(2 * MS);
    assert(first.value == 1 && second.value == 0);            // the original activation is untouched
}

void test_stop_is_idempotent_and_restart_is_clean() {
    ReferenceTimer timer;
    assert(timer.stop());                                     // STOPPED timer: success
    assert(timer.stop());
    Counter old_activation, new_activation;
    assert(timer.start(ms(2), TimerMode::PERIODIC, Callback{&count, &old_activation}));
    timer.advance(1 * MS);                                    // partial period of the old activation
    assert(timer.stop());
    assert(timer.start(ms(2), TimerMode::PERIODIC, Callback{&count, &new_activation}));
    timer.advance(1 * MS);
    assert(old_activation.value == 0 && new_activation.value == 0);   // no leftover elapsed time or callback
    timer.advance(1 * MS);
    assert(old_activation.value == 0 && new_activation.value == 1);
}

// A callback that re-enters its own timer.
struct Reentrant { ReferenceTimer* timer{nullptr}; ErrorCode stop_code{ErrorCode::UNKNOWN}; ErrorCode start_code{ErrorCode::UNKNOWN}; };
void reenter(void* context) {
    auto* r = static_cast<Reentrant*>(context);
    const auto stopped = r->timer->stop();
    r->stop_code = stopped ? ErrorCode::UNKNOWN : stopped.error().code;
    const auto started = r->timer->start(Duration::from_milliseconds(1), TimerMode::ONE_SHOT, Callback{&noop, nullptr});
    r->start_code = started ? ErrorCode::UNKNOWN : started.error().code;
}

void test_reentrant_calls_fail_without_deadlock() {
    ReferenceTimer timer;
    Reentrant reentrant{&timer};
    assert(timer.start(ms(1), TimerMode::ONE_SHOT, Callback{&reenter, &reentrant}));
    timer.advance(1 * MS);
    assert(reentrant.stop_code == ErrorCode::INVALID_STATE);
    assert(reentrant.start_code == ErrorCode::INVALID_STATE);
}

void test_adapter_defined_limits_use_defined_codes_and_are_atomic() {
    ReferenceTimer timer;
    timer.policy.resolution_ns = 10 * MS;
    expect_error(timer.start(ms(5), TimerMode::ONE_SHOT, Callback{&noop, nullptr}), ErrorCode::UNSUPPORTED);
    assert(!timer.running());
    timer.policy.periodic_supported = false;
    expect_error(timer.start(ms(20), TimerMode::PERIODIC, Callback{&noop, nullptr}), ErrorCode::UNSUPPORTED);
    assert(timer.start(ms(20), TimerMode::ONE_SHOT, Callback{&noop, nullptr}));   // the supported mode still works
    assert(timer.stop());
    timer.policy.resource_available = false;
    expect_error(timer.start(ms(20), TimerMode::ONE_SHOT, Callback{&noop, nullptr}), ErrorCode::RESOURCE_UNAVAILABLE);
    assert(!timer.running());
}

void test_context_is_opaque_and_never_owned() {
    ReferenceTimer timer;
    int sentinel = 0;
    assert(timer.start(ms(1), TimerMode::ONE_SHOT, Callback{&noop, &sentinel}));
    timer.advance(1 * MS);
    assert(sentinel == 0);                                    // the timer never touches the context
}

} // namespace

int main() {
    test_shape();
    test_invalid_arguments_leave_timer_stopped();
    test_one_shot_fires_once_then_stops();
    test_periodic_fires_until_stop();
    test_start_while_running_is_rejected_and_has_no_effect();
    test_stop_is_idempotent_and_restart_is_clean();
    test_reentrant_calls_fail_without_deadlock();
    test_adapter_defined_limits_use_defined_codes_and_are_atomic();
    test_context_is_opaque_and_never_owned();
    return 0;
}
