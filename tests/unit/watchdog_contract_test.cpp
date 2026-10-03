//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : watchdog_contract_test.cpp
// Description : IWatchdog contract tests against a reference implementation.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-007
// API         : CORE-TEST-WATCHDOG-CONTRACT
//
// Author      : KritvaOS Core Team
// Created     : 04-10-2026
//==============================================================================

#include <cassert>
#include <cstdint>
#include <type_traits>
#include <utility>

#include <kritva/core/core.hpp>

#include "../contract/reference_watchdog.hpp"

using namespace kritva::core;
using namespace kritva::core::platform;
using kritva::core::platform::contract::ReferenceWatchdog;

namespace {

constexpr std::int64_t MS = 1'000'000;

Duration ms(std::int64_t n) { return Duration::from_milliseconds(n); }

void expect_error(const Result<void>& result, ErrorCode code) {
    assert(!result);
    assert(result.error().code == code);
}

void test_shape() {
    static_assert(std::is_abstract_v<IWatchdog>);
    static_assert(std::has_virtual_destructor_v<IWatchdog>);
}

void test_initial_state_is_stopped() {
    ReferenceWatchdog watchdog;
    assert(!watchdog.running());
    assert(watchdog.stop());                                  // stopping a stopped watchdog succeeds
    assert(!watchdog.running());
}

void test_timeout_validation_leaves_watchdog_stopped() {
    ReferenceWatchdog watchdog;
    expect_error(watchdog.start(Duration::from_nanoseconds(0)), ErrorCode::INVALID_ARGUMENT);
    expect_error(watchdog.start(Duration::from_nanoseconds(-10 * MS)), ErrorCode::INVALID_ARGUMENT);
    assert(!watchdog.running());
    assert(watchdog.start(ms(10)));                           // still startable after rejected requests
    assert(watchdog.running());
}

void test_adapter_limits_use_defined_codes_atomically() {
    ReferenceWatchdog watchdog;
    watchdog.policy.min_timeout_ns = 10 * MS;
    watchdog.policy.max_timeout_ns = 100 * MS;
    expect_error(watchdog.start(ms(5)), ErrorCode::UNSUPPORTED);
    expect_error(watchdog.start(ms(500)), ErrorCode::UNSUPPORTED);
    assert(!watchdog.running());
    watchdog.policy.resource_available = false;
    expect_error(watchdog.start(ms(50)), ErrorCode::RESOURCE_UNAVAILABLE);
    assert(!watchdog.running());
}

void test_start_while_running_is_rejected_without_reconfiguring() {
    ReferenceWatchdog watchdog;
    assert(watchdog.start(ms(10)));
    expect_error(watchdog.start(ms(1000)), ErrorCode::INVALID_STATE);
    assert(watchdog.running());
    watchdog.advance(10 * MS);
    assert(watchdog.expired());                               // the original 10 ms timeout still applies
}

void test_kick_restarts_the_window() {
    ReferenceWatchdog watchdog;
    assert(watchdog.start(ms(10)));
    watchdog.advance(9 * MS);
    assert(watchdog.kick());
    watchdog.advance(9 * MS);
    assert(!watchdog.expired());                              // the window restarted at the kick
    watchdog.advance(1 * MS);
    assert(watchdog.expired());
    assert(watchdog.expiries() == 1);
}

void test_kick_outside_running_fails_and_never_starts() {
    ReferenceWatchdog watchdog;
    expect_error(watchdog.kick(), ErrorCode::INVALID_STATE);
    assert(!watchdog.running());                              // kick() never starts the watchdog
    assert(watchdog.start(ms(10)));
    assert(watchdog.stop());
    expect_error(watchdog.kick(), ErrorCode::INVALID_STATE);
}

void test_stop_is_idempotent_and_restart_is_fresh() {
    ReferenceWatchdog watchdog;
    assert(watchdog.start(ms(10)));
    watchdog.advance(8 * MS);
    assert(watchdog.stop());
    assert(watchdog.stop());
    watchdog.advance(100 * MS);
    assert(watchdog.expiries() == 0);                         // no expiry after stop
    assert(watchdog.start(ms(20)));                           // any valid timeout
    watchdog.advance(19 * MS);
    assert(!watchdog.expired());                              // nothing carried over from the old activation
    watchdog.advance(1 * MS);
    assert(watchdog.expired());

    // A watchdog that has expired can be stopped and restarted; the expiry is not carried over.
    assert(watchdog.stop());
    assert(watchdog.start(ms(20)));
    assert(!watchdog.expired());
    watchdog.advance(19 * MS);
    assert(!watchdog.expired());
    watchdog.advance(1 * MS);
    assert(watchdog.expired());
    assert(watchdog.expiries() == 2);                         // one expiry per activation
}

void test_stop_unsupported_keeps_running() {
    ReferenceWatchdog watchdog;
    watchdog.policy.can_stop = false;                         // a hardware-style watchdog
    assert(watchdog.stop());                                  // stopped already: still succeeds
    assert(watchdog.start(ms(10)));
    expect_error(watchdog.stop(), ErrorCode::UNSUPPORTED);
    assert(watchdog.running());                               // failure has no effect
    watchdog.advance(10 * MS);
    assert(watchdog.expired());
}

// Minimal Runtime component used to observe that expiry never touches the Runtime.
class Probe final : public runtime::Component {
public:
    explicit Probe(runtime::ComponentInfo info) : Component(std::move(info)) {}
    Result<void> configure(const Configuration&) override { return Result<void>::success(); }
    Result<void> initialize() override { return Result<void>::success(); }
    Result<void> start() override { return Result<void>::success(); }
    Result<void> stop() override { return Result<void>::success(); }
    Result<void> shutdown() override { return Result<void>::success(); }
    LifecycleState lifecycle_state() const noexcept override { return LifecycleState::UNKNOWN; }
    Status status() const override { return Status{}; }
    Health health() const override { return Health{}; }
    CapabilitySet capabilities() const override { return CapabilitySet{}; }
};

void test_expiry_has_no_runtime_effect() {
    auto info = runtime::ComponentInfo::create(runtime::ComponentId{1}, "probe");
    assert(info);
    Probe probe(std::move(info).value());
    runtime::RuntimeManager manager;
    assert(manager.register_component(probe));
    assert(manager.initialize() && manager.start());
    assert(manager.state() == LifecycleState::RUNNING);
    const auto errors_before = manager.statistics().error_count.value();

    ReferenceWatchdog watchdog;
    assert(watchdog.start(ms(10)));
    watchdog.advance(1000 * MS);                              // far past the timeout, never kicked
    assert(watchdog.expired());

    assert(manager.state() == LifecycleState::RUNNING);       // no reset, no fault, no recovery
    assert(manager.fault_error() == nullptr);
    assert(manager.statistics().error_count.value() == errors_before);
    assert(!manager.reset());                                 // reset() is still only valid in FAULT
}

} // namespace

int main() {
    test_shape();
    test_initial_state_is_stopped();
    test_timeout_validation_leaves_watchdog_stopped();
    test_adapter_limits_use_defined_codes_atomically();
    test_start_while_running_is_rejected_without_reconfiguring();
    test_kick_restarts_the_window();
    test_kick_outside_running_fails_and_never_starts();
    test_stop_is_idempotent_and_restart_is_fresh();
    test_stop_unsupported_keeps_running();
    test_expiry_has_no_runtime_effect();
    return 0;
}
