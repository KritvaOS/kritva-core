//==============================================================================
// Kritva Core — Time Contract Tests
// SPDX-License-Identifier: Apache-2.0
//
// Requirements:
//   CORE-TIME-001 : IClock
//   CORE-TIME-002 : ITimer
//
// These tests validate only the contracts exposed by the current Core headers.
// Platform-specific clock/timer behavior is intentionally outside this suite.
//==============================================================================

#include <cassert>
#include <cstdint>
#include <type_traits>

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

class FakeTimer final : public ITimer {
public:
    Result<void> start(Duration period) override {
        ++start_count;
        last_period = period;
        running = true;
        return Result<void>::success();
    }

    Result<void> stop() override {
        ++stop_count;
        running = false;
        return Result<void>::success();
    }

    Duration last_period{};
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
    const Result<void> result = timer.start(period);

    assert(result);
    assert(result.has_value());
    assert(timer.start_count == 1);
    assert(timer.running);
    assert(timer.last_period == period);
}

void test_timer_stops() {
    FakeTimer timer;

    const Duration period = Duration::from_milliseconds(10);
    assert(timer.start(period));
    assert(timer.running);

    const Result<void> result = timer.stop();

    assert(result);
    assert(result.has_value());
    assert(timer.stop_count == 1);
    assert(!timer.running);
}

void test_timer_preserves_zero_duration_value() {
    // The current ITimer contract does not define period validation.
    // Therefore this test checks only that a zero Duration can be passed
    // through the interface. Validation policy belongs to an implementation.
    FakeTimer timer;

    const Duration zero = Duration::from_nanoseconds(0);
    const Result<void> result = timer.start(zero);

    assert(result);
    assert(timer.last_period == zero);
}

} // namespace

int main() {
    test_clock_contract_shape();
    test_clock_now_returns_timestamp();
    test_clock_preserves_clock_domain();
    test_clock_now_is_const_callable();

    test_timer_contract_shape();
    test_timer_starts_with_period();
    test_timer_stops();
    test_timer_preserves_zero_duration_value();

    return 0;
}
