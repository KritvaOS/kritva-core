//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : reference_watchdog.hpp
// Description : Reference IWatchdog implementing the documented contract (test double).
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-007
// API         : CORE-TEST-WATCHDOG-REFERENCE
//
// Author      : KritvaOS Core Team
// Created     : 04-10-2026
//==============================================================================

#pragma once

#include <cstdint>

#include <kritva/core/platform/boundary.hpp>
#include <kritva/core/platform/watchdog.hpp>

namespace kritva::core::platform::contract {

/// A conforming IWatchdog built only on Core types. It is a test double, not a
/// watchdog: elapsed time is simulated by advance(). The expiry action is
/// adapter-defined, so the reference only records that expiry happened.
/// Adapter-defined behavior is configured through Policy.
class ReferenceWatchdog final : public IWatchdog {
public:
    struct Policy {
        std::int64_t min_timeout_ns{1'000'000};
        std::int64_t max_timeout_ns{60'000'000'000};
        bool can_stop{true};                         // false: stop() of a running watchdog is UNSUPPORTED
        bool resource_available{true};
    };
    Policy policy;

    Result<void> start(Duration timeout) override {
        ++start_calls;
        if (timeout.nanoseconds() <= 0) return fail(ErrorCode::INVALID_ARGUMENT, "timeout must be positive");
        if (running_) return fail(ErrorCode::INVALID_STATE, "watchdog is running");
        if (timeout.nanoseconds() < policy.min_timeout_ns || timeout.nanoseconds() > policy.max_timeout_ns) {
            return fail(ErrorCode::UNSUPPORTED, "timeout outside supported range");
        }
        if (!policy.resource_available) return fail(ErrorCode::RESOURCE_UNAVAILABLE, "no watchdog resource");
        timeout_ns_ = timeout.nanoseconds();
        elapsed_ns_ = 0;
        expired_ = false;
        running_ = true;
        return Result<void>::success();
    }

    Result<void> kick() override {
        ++kick_calls;
        if (!running_) return fail(ErrorCode::INVALID_STATE, "watchdog is not running");
        elapsed_ns_ = 0;
        return Result<void>::success();
    }

    Result<void> stop() override {
        ++stop_calls;
        if (!running_) return Result<void>::success();             // idempotent
        if (!policy.can_stop) return fail(ErrorCode::UNSUPPORTED, "watchdog cannot be stopped");
        running_ = false;
        return Result<void>::success();
    }

    /// Simulate elapsed time. On expiry the reference records it and stops counting; the
    /// action an adapter takes is outside the contract.
    void advance(std::int64_t nanoseconds) {
        if (!running_ || expired_) return;
        elapsed_ns_ += nanoseconds;
        if (elapsed_ns_ >= timeout_ns_) { expired_ = true; ++expiries_; }
    }

    [[nodiscard]] bool running() const noexcept { return running_; }
    [[nodiscard]] bool expired() const noexcept { return expired_; }
    [[nodiscard]] std::uint64_t expiries() const noexcept { return expiries_; }

    int start_calls{0}, kick_calls{0}, stop_calls{0};

private:
    static Result<void> fail(ErrorCode code, const char* message) {
        return Result<void>::failure(make_error(code, message));
    }

    std::int64_t timeout_ns_{0};
    std::int64_t elapsed_ns_{0};
    std::uint64_t expiries_{0};
    bool running_{false};
    bool expired_{false};
};

} // namespace kritva::core::platform::contract
