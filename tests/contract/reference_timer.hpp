//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : reference_timer.hpp
// Description : Reference ITimer implementing the documented contract (test double).
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-006
// API         : CORE-TEST-TIMER-REFERENCE
//
// Author      : KritvaOS Core Team
// Created     : 04-10-2026
//==============================================================================

#pragma once

#include <cstdint>

#include <kritva/core/platform/boundary.hpp>
#include <kritva/core/time/timer.hpp>

namespace kritva::core::time::contract {

/// A conforming ITimer built only on Core types. It is a test double, not a timer:
/// elapsed time is simulated by advance(), which makes the contract rules
/// observable without threads or an operating system. Adapter-defined behavior
/// (resolution, supported modes, maximum period) is configured through Policy.
class ReferenceTimer final : public ITimer {
public:
    struct Policy {
        std::int64_t resolution_ns{1'000'000};        // periods below this are UNSUPPORTED
        bool periodic_supported{true};
        bool resource_available{true};
    };
    Policy policy;

    Result<void> start(Duration period, TimerMode mode, Callback callback) override {
        ++start_calls;
        if (period.nanoseconds() <= 0) return fail(ErrorCode::INVALID_ARGUMENT, "period must be positive");
        if (!callback.valid()) return fail(ErrorCode::INVALID_ARGUMENT, "callback is null");
        if (running_ || in_callback_) return fail(ErrorCode::INVALID_STATE, "timer is running");
        if (mode == TimerMode::PERIODIC && !policy.periodic_supported) return fail(ErrorCode::UNSUPPORTED, "periodic unsupported");
        if (period.nanoseconds() < policy.resolution_ns) return fail(ErrorCode::UNSUPPORTED, "period below resolution");
        if (!policy.resource_available) return fail(ErrorCode::RESOURCE_UNAVAILABLE, "no timer resource");
        // Every check passed: only now is state changed (atomicity).
        period_ns_ = period.nanoseconds();
        mode_ = mode;
        callback_ = callback;
        elapsed_ns_ = 0;
        running_ = true;
        return Result<void>::success();
    }

    Result<void> stop() override {
        ++stop_calls;
        if (in_callback_) return fail(ErrorCode::INVALID_STATE, "stop() from the timer callback");
        running_ = false;                                // idempotent
        return Result<void>::success();
    }

    /// Simulate elapsed time; fires the callback once per whole period (never overlapping).
    void advance(std::int64_t nanoseconds) {
        if (!running_) return;
        elapsed_ns_ += nanoseconds;
        while (running_ && elapsed_ns_ >= period_ns_) {
            elapsed_ns_ -= period_ns_;
            in_callback_ = true;
            callback_.function(callback_.context);
            in_callback_ = false;
            ++fired_;
            if (mode_ == TimerMode::ONE_SHOT) running_ = false;   // one-shot ends after firing
        }
    }

    [[nodiscard]] bool running() const noexcept { return running_; }
    [[nodiscard]] std::uint64_t fired() const noexcept { return fired_; }

    int start_calls{0}, stop_calls{0};

private:
    static Result<void> fail(ErrorCode code, const char* message) {
        return Result<void>::failure(platform::make_error(code, message));
    }

    std::int64_t period_ns_{0};
    std::int64_t elapsed_ns_{0};
    TimerMode mode_{TimerMode::ONE_SHOT};
    Callback callback_{};
    std::uint64_t fired_{0};
    bool running_{false};
    bool in_callback_{false};
};

} // namespace kritva::core::time::contract
