//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : statistics_test.cpp
// Description : Statistics, Counter, and Gauge API contract tests.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-STS-001, CORE-STS-002, CORE-STS-003
// API         : CORE-TEST-STATISTICS
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================

#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>

#include <kritva/core/statistics/counter.hpp>
#include <kritva/core/statistics/gauge.hpp>
#include <kritva/core/statistics/statistics.hpp>

int main() {
    using namespace kritva::core;

    //--------------------------------------------------------------------------
    // 1. Counter default state
    //--------------------------------------------------------------------------

    Counter counter{};

    assert(counter.value() == 0);

    //--------------------------------------------------------------------------
    // 2. Counter increment by default amount
    //--------------------------------------------------------------------------

    counter.increment();

    assert(counter.value() == 1);

    counter.increment();

    assert(counter.value() == 2);

    //--------------------------------------------------------------------------
    // 3. Counter increment by explicit amount
    //--------------------------------------------------------------------------

    counter.increment(5);

    assert(counter.value() == 7);

    counter.increment(10);

    assert(counter.value() == 17);

    //--------------------------------------------------------------------------
    // 4. Counter reset
    //--------------------------------------------------------------------------

    counter.reset();

    assert(counter.value() == 0);

    // Reset should be idempotent.
    counter.reset();

    assert(counter.value() == 0);

    //--------------------------------------------------------------------------
    // 5. Gauge default state
    //--------------------------------------------------------------------------

    Gauge gauge{};

    assert(gauge.value() == 0);

    //--------------------------------------------------------------------------
    // 6. Gauge positive, zero, and negative values
    //--------------------------------------------------------------------------

    gauge.set(100);

    assert(gauge.value() == 100);

    gauge.set(0);

    assert(gauge.value() == 0);

    gauge.set(-25);

    assert(gauge.value() == -25);

    //--------------------------------------------------------------------------
    // 7. Gauge update replaces the previous value
    //--------------------------------------------------------------------------

    gauge.set(500);

    assert(gauge.value() == 500);

    gauge.set(250);

    assert(gauge.value() == 250);

    //--------------------------------------------------------------------------
    // 8. Statistics default state
    //--------------------------------------------------------------------------

    Statistics statistics{};

    assert(statistics.sample_count.value() == 0);
    assert(statistics.error_count.value() == 0);
    assert(statistics.retry_count.value() == 0);
    assert(statistics.drop_count.value() == 0);

    assert(statistics.queue_depth.value() == 0);
    assert(statistics.utilization.value() == 0);

    //--------------------------------------------------------------------------
    // 9. Statistics fields operate independently
    //--------------------------------------------------------------------------

    statistics.sample_count.increment(100);
    statistics.error_count.increment(3);
    statistics.retry_count.increment(7);
    statistics.drop_count.increment(2);

    statistics.queue_depth.set(16);
    statistics.utilization.set(75);

    assert(statistics.sample_count.value() == 100);
    assert(statistics.error_count.value() == 3);
    assert(statistics.retry_count.value() == 7);
    assert(statistics.drop_count.value() == 2);

    assert(statistics.queue_depth.value() == 16);
    assert(statistics.utilization.value() == 75);

    // Updating one statistic must not change the others.
    statistics.error_count.increment(1);
    statistics.queue_depth.set(32);

    assert(statistics.sample_count.value() == 100);
    assert(statistics.error_count.value() == 4);
    assert(statistics.retry_count.value() == 7);
    assert(statistics.drop_count.value() == 2);

    assert(statistics.queue_depth.value() == 32);
    assert(statistics.utilization.value() == 75);

    //--------------------------------------------------------------------------
    // 10. Statistics copy/value semantics
    //--------------------------------------------------------------------------

    Statistics original{};

    original.sample_count.increment(1000);
    original.error_count.increment(10);
    original.retry_count.increment(20);
    original.drop_count.increment(5);
    original.queue_depth.set(64);
    original.utilization.set(80);

    Statistics copy = original;

    assert(copy.sample_count.value() == 1000);
    assert(copy.error_count.value() == 10);
    assert(copy.retry_count.value() == 20);
    assert(copy.drop_count.value() == 5);
    assert(copy.queue_depth.value() == 64);
    assert(copy.utilization.value() == 80);

    // Modifying the copy must not modify the original.
    copy.sample_count.increment(1);
    copy.error_count.reset();
    copy.queue_depth.set(128);
    copy.utilization.set(90);

    assert(original.sample_count.value() == 1000);
    assert(original.error_count.value() == 10);
    assert(original.queue_depth.value() == 64);
    assert(original.utilization.value() == 80);

    assert(copy.sample_count.value() == 1001);
    assert(copy.error_count.value() == 0);
    assert(copy.queue_depth.value() == 128);
    assert(copy.utilization.value() == 90);

    //--------------------------------------------------------------------------
    // R0.2 contract: Counter wraps modulo 2^64 on overflow
    //--------------------------------------------------------------------------

    Counter wrap;
    wrap.increment(std::numeric_limits<Counter::value_type>::max());
    assert(wrap.value() == std::numeric_limits<Counter::value_type>::max());
    wrap.increment();
    assert(wrap.value() == 0);
    wrap.increment(5);
    wrap.increment(std::numeric_limits<Counter::value_type>::max());
    assert(wrap.value() == 4);

    //--------------------------------------------------------------------------
    // R0.2 contract: Counter/Gauge are constexpr and noexcept, no allocation
    //--------------------------------------------------------------------------

    constexpr auto constexpr_counter = [] {
        Counter c;
        c.increment(3);
        return c.value();
    }();
    static_assert(constexpr_counter == 3);

    constexpr auto constexpr_gauge = [] {
        Gauge g;
        g.set(-7);
        return g.value();
    }();
    static_assert(constexpr_gauge == -7);

    static_assert(noexcept(wrap.increment()));
    static_assert(noexcept(wrap.reset()));
    static_assert(noexcept(Gauge{}.set(1)));

    //--------------------------------------------------------------------------
    // R0.2 contract: Gauge holds the last value set, including int64 extremes
    //--------------------------------------------------------------------------

    Gauge extreme;
    extreme.set(std::numeric_limits<Gauge::value_type>::min());
    assert(extreme.value() == std::numeric_limits<Gauge::value_type>::min());
    extreme.set(std::numeric_limits<Gauge::value_type>::max());
    assert(extreme.value() == std::numeric_limits<Gauge::value_type>::max());
    extreme.set(0);
    assert(extreme.value() == 0);

    //--------------------------------------------------------------------------
    // R0.2 contract: increment(0) is a no-op; reset() after wrap restarts at 0
    //--------------------------------------------------------------------------

    Counter noop;
    noop.increment(9);
    noop.increment(0);
    assert(noop.value() == 9);
    wrap.reset();
    assert(wrap.value() == 0);
    wrap.increment();
    assert(wrap.value() == 1);

    //--------------------------------------------------------------------------
    // R0.2 contract: Gauge/Statistics store values as-is (no clamping)
    //--------------------------------------------------------------------------

    Statistics unclamped{};
    unclamped.queue_depth.set(-1);
    unclamped.utilization.set(250);
    assert(unclamped.queue_depth.value() == -1);
    assert(unclamped.utilization.value() == 250);

    //--------------------------------------------------------------------------
    // R0.2 contract: constexpr value-initialization and plain-value traits
    //--------------------------------------------------------------------------

    constexpr Statistics zero{};
    static_assert(zero.sample_count.value() == 0);
    static_assert(zero.utilization.value() == 0);

    static_assert(std::is_trivially_copyable_v<Counter>);
    static_assert(std::is_trivially_copyable_v<Gauge>);
    static_assert(std::is_trivially_copyable_v<Statistics>);
    static_assert(std::is_nothrow_default_constructible_v<Statistics>);
    static_assert(std::is_nothrow_copy_constructible_v<Statistics>);
    static_assert(!std::is_same_v<Counter::value_type, Gauge::value_type>);
    static_assert(std::is_unsigned_v<Counter::value_type>);
    static_assert(std::is_signed_v<Gauge::value_type>);

    return 0;
}
