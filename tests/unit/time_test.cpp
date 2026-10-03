//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : time_test.cpp
// Description : Kritva Core — Time Contract Tests
//               These tests validate only the contracts exposed by the current Core headers.
//               Platform-specific clock/timer behavior is intentionally outside this suite.
//
// Component   : Kritva Core
// Module      : TIME
// Layer       : Core Foundation
//
// Requirements: CORE-TIME-001 : IClock
//               CORE-TIME-002 : ITimer
// API         : CORE-TEST-TIME
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================

#include <cassert>
#include <cstdint>
#include <type_traits>
#include <utility>

#include "kritva/core/time/clock.hpp"
#include "kritva/core/time/timer.hpp"

using namespace kritva::core;
using namespace kritva::core::time;

namespace {

class FakeClock final : public IClock {
public:
    explicit FakeClock(Timestamp timestamp) noexcept
        : timestamp_(timestamp) {}

    [[nodiscard]] Timestamp now() const noexcept override {
        return timestamp_;
    }

private:
    Timestamp timestamp_{};
};

void noop(void*) {}

class FakeTimer final : public ITimer {
public:
    Result<void> start(Duration period, TimerMode mode, Callback callback) override {
        ++start_count;
        last_period = period;
        last_mode = mode;
        last_callback = callback;
        running = true;
        return Result<void>::success();
    }

    Result<void> stop() override {
        ++stop_count;
        running = false;
        return Result<void>::success();
    }

    Duration last_period{};
    TimerMode last_mode{TimerMode::ONE_SHOT};
    Callback last_callback{};
    std::uint32_t start_count{0};
    std::uint32_t stop_count{0};
    bool running{false};
};

void test_clock_contract_shape() {
    static_assert(std::is_abstract_v<IClock>);
    static_assert(std::is_polymorphic_v<IClock>);
    static_assert(std::has_virtual_destructor_v<IClock>);
}

void test_clock_now_returns_timestamp() {
    const Timestamp expected{
        1234567890LL,
        ClockDomain::MONOTONIC
    };

    const FakeClock clock(expected);
    const Timestamp actual = clock.now();

    assert(actual == expected);
    assert(actual.nanoseconds() == 1234567890LL);
    assert(actual.domain() == ClockDomain::MONOTONIC);
}

void test_clock_preserves_clock_domain() {
    const Timestamp monotonic{
        1000LL,
        ClockDomain::MONOTONIC
    };
    const Timestamp realtime{
        2000LL,
        ClockDomain::REALTIME
    };

    const FakeClock monotonic_clock(monotonic);
    const FakeClock realtime_clock(realtime);

    const Timestamp monotonic_now = monotonic_clock.now();
    const Timestamp realtime_now = realtime_clock.now();

    assert(monotonic_now.domain() == ClockDomain::MONOTONIC);
    assert(realtime_now.domain() == ClockDomain::REALTIME);
    assert(monotonic_now != realtime_now);
}

void test_clock_now_is_const_callable() {
    const Timestamp expected{
        42LL,
        ClockDomain::REALTIME
    };

    const FakeClock implementation(expected);
    const IClock& clock = implementation;

    assert(clock.now() == expected);
}

void test_timer_contract_shape() {
    static_assert(std::is_abstract_v<ITimer>);
    static_assert(std::is_polymorphic_v<ITimer>);
    static_assert(std::has_virtual_destructor_v<ITimer>);
}

void test_timer_starts_with_period() {
    FakeTimer timer;

    const Duration period = Duration::from_milliseconds(10);
    const Result<void> result = timer.start(period, TimerMode::PERIODIC, Callback{&noop, nullptr});

    assert(result);
    assert(result.has_value());
    assert(timer.start_count == 1);
    assert(timer.running);
    assert(timer.last_period == period);
    assert(timer.last_mode == TimerMode::PERIODIC);
    assert(timer.last_callback.valid());
}

void test_timer_stops() {
    FakeTimer timer;

    const Duration period = Duration::from_milliseconds(10);
    assert(timer.start(period, TimerMode::PERIODIC, Callback{&noop, nullptr}));
    assert(timer.running);

    const Result<void> result = timer.stop();

    assert(result);
    assert(result.has_value());
    assert(timer.stop_count == 1);
    assert(!timer.running);
}

// Timestamps are not orderable or subtractable by design: this keeps mixed
// clock domains from being compared or combined silently.
template<class T, class = void> struct has_less : std::false_type {};
template<class T> struct has_less<T, std::void_t<decltype(std::declval<T>() < std::declval<T>())>> : std::true_type {};
template<class T, class = void> struct has_minus : std::false_type {};
template<class T> struct has_minus<T, std::void_t<decltype(std::declval<T>() - std::declval<T>())>> : std::true_type {};

void test_timestamp_domains_are_not_silently_comparable() {
    static_assert(!has_less<Timestamp>::value);
    static_assert(!has_minus<Timestamp>::value);
    static_assert(std::is_trivially_copyable_v<Timestamp>);

    const Timestamp mono{500, ClockDomain::MONOTONIC};
    const Timestamp real{500, ClockDomain::REALTIME};

    // Same nanoseconds, different domain: never equal.
    assert(mono.nanoseconds() == real.nanoseconds());
    assert(mono != real);
    assert(!(mono == real));
    // Same nanoseconds and domain: equal.
    assert(mono == Timestamp(500, ClockDomain::MONOTONIC));

    // Default is 0 ns, MONOTONIC.
    assert(Timestamp{}.nanoseconds() == 0);
    assert(Timestamp{}.domain() == ClockDomain::MONOTONIC);
    // Negative nanoseconds are representable (REALTIME before its epoch).
    assert(Timestamp(-1, ClockDomain::REALTIME).nanoseconds() == -1);
}

// An instance reports one fixed domain on every call.
void test_clock_domain_is_stable_per_instance() {
    const FakeClock clock(Timestamp{7, ClockDomain::REALTIME});
    for (int i = 0; i < 3; ++i) {
        assert(clock.now().domain() == ClockDomain::REALTIME);
    }
    static_assert(noexcept(std::declval<const IClock&>().now()));
}

// IClock and ITimer are independent contracts that can be implemented together
// without conflict, and neither depends on the other.
class ClockAndTimer final : public IClock, public ITimer {
public:
    [[nodiscard]] Timestamp now() const noexcept override { return Timestamp{1}; }
    Result<void> start(Duration, TimerMode, Callback) override { return Result<void>::success(); }
    Result<void> stop() override { return Result<void>::success(); }
};

void test_clock_and_timer_are_independent_contracts() {
    ClockAndTimer both;
    const IClock& clock = both;
    ITimer& timer = both;
    assert(clock.now() == Timestamp{1});
    assert(timer.start(Duration::from_milliseconds(1), TimerMode::ONE_SHOT, Callback{&noop, nullptr}));
    assert(timer.stop());
    static_assert(!std::is_base_of_v<IClock, ITimer>);
    static_assert(!std::is_base_of_v<ITimer, IClock>);
}

} // namespace

int main() {
    test_timestamp_domains_are_not_silently_comparable();
    test_clock_domain_is_stable_per_instance();
    test_clock_and_timer_are_independent_contracts();

    test_clock_contract_shape();
    test_clock_now_returns_timestamp();
    test_clock_preserves_clock_domain();
    test_clock_now_is_const_callable();

    test_timer_contract_shape();
    test_timer_starts_with_period();
    test_timer_stops();

    return 0;
}
