//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : clock_conformance.hpp
// Description : Conformance checks for time::IClock.
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

#include <kritva/core/time/clock.hpp>

namespace kritva::core::platform::conformance {

/// IClock (CORE-TIME-001): one fixed domain per instance on every returned timestamp,
/// and a MONOTONIC clock never decreases. Not checked (adapter-defined): epoch,
/// resolution, whether the clock advances at all between two calls.
inline void check_clock(const time::IClock& clock, const Environment& env, Report& report) {
    const ClockDomain domain = clock.now().domain();
    Timestamp previous = clock.now();
    KRITVA_CONFORMANCE_CHECK(report, previous.domain() == domain);

    for (int i = 0; i < 20; ++i) {
        if (i % 5 == 4 && env.let_time_pass) env.let_time_pass(Duration::from_milliseconds(1));
        const Timestamp now = clock.now();
        KRITVA_CONFORMANCE_CHECK(report, now.domain() == domain);
        if (domain == ClockDomain::MONOTONIC) KRITVA_CONFORMANCE_CHECK(report, now.nanoseconds() >= previous.nanoseconds());
        previous = now;
    }
}

} // namespace kritva::core::platform::conformance
