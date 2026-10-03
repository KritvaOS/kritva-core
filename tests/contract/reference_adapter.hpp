//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : reference_adapter.hpp
// Description : Reference IPlatformAdapter composed of the reference services (test double).
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-008
// API         : CORE-TEST-ADAPTER-REFERENCE
//
// Author      : KritvaOS Core Team
// Created     : 04-10-2026
//==============================================================================

#pragma once

#include <string>
#include <utility>

#include <kritva/core/platform/adapter.hpp>

#include "reference_scheduler.hpp"
#include "reference_timer.hpp"
#include "reference_watchdog.hpp"

namespace kritva::core::platform::contract {

/// A fixed clock used by the reference adapter.
class ReferenceClock : public time::IClock {
public:
    [[nodiscard]] Timestamp now() const noexcept override { return Timestamp{1, ClockDomain::MONOTONIC}; }
};

/// A conforming IPlatformAdapter. Which services it provides is chosen at
/// construction; each service the adapter provides is owned by the adapter, the
/// others report nullptr. Capabilities are whatever the test hands in.
class ReferenceAdapter : public IPlatformAdapter {
public:
    struct Provides { bool scheduler{true}, clock{true}, timer{true}, watchdog{true}; };

    ReferenceAdapter(PlatformInfo info, Provides provides, CapabilitySet capabilities = {})
        : info_(std::move(info)), provides_(provides), capabilities_(std::move(capabilities)) {}

    [[nodiscard]] const PlatformInfo& info() const noexcept override { return info_; }
    [[nodiscard]] IScheduler* scheduler() const noexcept override { return provides_.scheduler ? &scheduler_ : nullptr; }
    [[nodiscard]] time::IClock* clock() const noexcept override { return provides_.clock ? &clock_ : nullptr; }
    [[nodiscard]] time::ITimer* timer() const noexcept override { return provides_.timer ? &timer_ : nullptr; }
    [[nodiscard]] IWatchdog* watchdog() const noexcept override { return provides_.watchdog ? &watchdog_ : nullptr; }
    [[nodiscard]] CapabilitySet capabilities() const override { return capabilities_; }

    /// Number of times a service was started by a query: must stay zero (queries never activate).
    [[nodiscard]] int activations() const noexcept {
        return scheduler_.start_calls + timer_.start_calls + watchdog_.start_calls;
    }

private:
    PlatformInfo info_;
    Provides provides_;
    CapabilitySet capabilities_;
    mutable ReferenceScheduler scheduler_;
    mutable ReferenceClock clock_;
    mutable time::contract::ReferenceTimer timer_;
    mutable ReferenceWatchdog watchdog_;
};

} // namespace kritva::core::platform::contract
