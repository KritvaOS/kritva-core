//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : clock.hpp
// Description : Platform-neutral clock contract exposed by Core.
//
// Component   : Kritva Core
// Module      : Time
// Layer       : Core Foundation
//
// Requirements: CORE-TIME-001
// API         : CORE-API-TIME
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================

#pragma once

#include "../types/timestamp.hpp"

namespace kritva::core::time {

//------------------------------------------------------------------------------
// IClock (CORE-TIME-001)
//
// The canonical Core clock abstraction: a platform-neutral source of
// Timestamps. Core owns this contract; Linux/RTOS/MCU/PTP/vendor clock
// implementations are provided by platform adapters outside Core.
// (platform::IClock in platform/clock.hpp is only a compatibility alias of this
// type, not a second contract.)
//
// Clock domain (see ClockDomain in types/timestamp.hpp):
//   - Every IClock instance belongs to exactly one ClockDomain for its whole
//     lifetime, and every Timestamp it returns carries that domain.
//   - MONOTONIC: never decreases between successive now() calls and is not
//     affected by wall-clock adjustments. Its epoch is unspecified, so only
//     differences between timestamps of the same clock are meaningful.
//   - REALTIME: wall-clock time. It may step forward or backward when the
//     system time is adjusted, so successive values are NOT guaranteed
//     ordered. The adapter documents the epoch it uses.
//
// Comparing timestamps:
//   - Timestamps from different domains are incomparable and must not be
//     ordered, subtracted, or mixed in arithmetic. Core deliberately provides
//     no ordering or subtraction for Timestamp; callers that compute with
//     nanoseconds() must first check domain().
//   - Equal domains are necessary but not sufficient: two distinct clock
//     sources in the same domain (for example two MONOTONIC clocks on
//     different devices) need not share an epoch. Only compare timestamps
//     from the same clock source unless an adapter documents otherwise.
//
// Real-time / thread-safety:
//   - now() is const and noexcept: it must not throw. Whether it allocates,
//     blocks, or has bounded latency is an adapter property that the adapter
//     documents; Core makes no hard-real-time claim.
//   - Implementations should be safe for concurrent now() calls from multiple
//     threads and must document it if they are not.
//
// Out of scope: timers, scheduling and callbacks (see ITimer, IScheduler).
//------------------------------------------------------------------------------
class IClock {
public:
    virtual ~IClock() = default;

    [[nodiscard]] virtual Timestamp now() const noexcept = 0;
};

} // namespace kritva::core::time
