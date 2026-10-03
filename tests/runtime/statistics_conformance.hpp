//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : statistics_conformance.hpp
// Description : Reusable conformance check of a Component statistics provider (test only).
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-OPS-004
// API         : CORE-TEST-OPERATIONAL-CONFORMANCE
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#pragma once
#include <concepts>
#include <cstdint>
#include <string>

#include <kritva/core/core.hpp>

namespace kritva::core::runtime::conformance {

/// What the owner of a provider must let the check drive: the provider's own numbers.
template <class Fixture>
concept StatisticsFixture = requires(Fixture& f, std::uint64_t n, std::int64_t g) {
    { f.provider() } -> std::same_as<const IComponentStatistics&>;
    f.add_samples(n);
    f.add_errors(n);
    f.set_queue_depth(g);
};

/// CORE-OPS-004: a conforming provider reports its own numbers by value.
///  - reading is a pure query: repeated reads are equal and never reset or change anything;
///  - a returned Statistics is an independent snapshot: later updates never change it;
///  - fields are independent: updating one field leaves the others as they were;
///  - the values are exactly the provider's (Core derives and adds nothing).
/// Returns the first violation, or an empty string.
template <StatisticsFixture Fixture>
std::string check_statistics_provider(Fixture& fixture) {
    const IComponentStatistics& p = fixture.provider();
    const Statistics start = p.statistics();
    const Statistics again = p.statistics();
    if (start.sample_count.value() != again.sample_count.value() || start.error_count.value() != again.error_count.value() ||
        start.queue_depth.value() != again.queue_depth.value()) return "two consecutive reads differ without an update";

    fixture.add_samples(5);
    const Statistics a = p.statistics();
    if (a.sample_count.value() != start.sample_count.value() + 5) return "a sample update was not reported exactly";
    if (a.error_count.value() != start.error_count.value() || a.queue_depth.value() != start.queue_depth.value() ||
        a.drop_count.value() != start.drop_count.value() || a.retry_count.value() != start.retry_count.value() ||
        a.utilization.value() != start.utilization.value()) return "updating samples changed another field";
    (void)p.statistics(); (void)p.statistics();
    if (p.statistics().sample_count.value() != a.sample_count.value()) return "reading changed or reset a counter";

    fixture.add_errors(2);
    const Statistics b = p.statistics();
    if (b.error_count.value() != a.error_count.value() + 2) return "an error update was not reported exactly";
    if (b.sample_count.value() != a.sample_count.value() || b.queue_depth.value() != a.queue_depth.value()) return "updating errors changed another field";

    fixture.set_queue_depth(-7);
    const Statistics c = p.statistics();
    if (c.queue_depth.value() != -7) return "a gauge value was not reported exactly";
    if (c.sample_count.value() != b.sample_count.value() || c.error_count.value() != b.error_count.value()) return "updating a gauge changed a counter";

    // Snapshots by value: nothing taken earlier changes after later updates.
    fixture.add_samples(100);
    fixture.add_errors(100);
    fixture.set_queue_depth(123);
    if (a.sample_count.value() != start.sample_count.value() + 5) return "an earlier snapshot changed after later updates";
    if (b.error_count.value() != a.error_count.value() + 2) return "an earlier snapshot changed after later updates";
    if (c.queue_depth.value() != -7) return "an earlier gauge snapshot changed after later updates";
    return {};
}
} // namespace kritva::core::runtime::conformance
