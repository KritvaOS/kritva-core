//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : reference_component.hpp
// Description : Reference Component implementing the documented contract.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-RT-001
// API         : CORE-TEST-COMPONENT-REFERENCE
//
// Author      : KritvaOS Core Team
// Created     : 03-10-2026
//==============================================================================

#pragma once

#include <cassert>
#include <string>
#include <utility>
#include <vector>

#include <kritva/core/core.hpp>

namespace kritva::core::runtime::contract {

/// A conforming Component built on the Core Lifecycle (component.hpp contract).
/// It is a test double, not a runtime: it exists to show the contract can be
/// implemented with Core primitives only and to give later tasks (registry,
/// dependency, runtime) a deterministic reference component.
///
/// Failure injection: set `fail_next_*` before calling an operation; that single
/// call fails with the given ErrorCode after the operation was otherwise valid.
class ReferenceComponent : public Component {
public:
    explicit ReferenceComponent(ComponentInfo info) : Component(std::move(info)) {}

    Result<void> configure(const Configuration& configuration) override {
        ++configure_calls;
        record("configure");
        if (state() != LifecycleState::UNKNOWN && state() != LifecycleState::STOPPED) {
            return invalid_state("configure");
        }
        if (auto valid = configuration.validate(); !valid) {
            return fail(valid.error().code, valid.error().message);
        }
        if (fail_next_configure != ErrorCode::NONE) return inject(fail_next_configure, "configure failed");
        configuration_ = configuration;  // applied only after every check passed
        configured = true;
        return Result<void>::success();
    }

    Result<void> initialize() override {
        ++initialize_calls;
        record("initialize");
        if (state() != LifecycleState::UNKNOWN && state() != LifecycleState::STOPPED) {
            return invalid_state("initialize");
        }
        go(LifecycleState::INITIALIZING);
        if (fail_next_initialize != ErrorCode::NONE) return fault(fail_next_initialize, "initialize failed");
        go(LifecycleState::READY);
        return Result<void>::success();
    }

    Result<void> start() override {
        ++start_calls;
        record("start");
        if (state() != LifecycleState::READY) return invalid_state("start");
        if (fail_next_start != ErrorCode::NONE) return fault(fail_next_start, "start failed");
        go(LifecycleState::RUNNING);
        return Result<void>::success();
    }

    Result<void> stop() override {
        ++stop_calls;
        record("stop");
        if (state() != LifecycleState::READY && state() != LifecycleState::RUNNING) {
            return invalid_state("stop");
        }
        if (state() == LifecycleState::RUNNING) go(LifecycleState::STOPPING);
        if (fail_next_stop != ErrorCode::NONE) return fault(fail_next_stop, "stop failed");
        go(LifecycleState::STOPPED);
        return Result<void>::success();
    }

    Result<void> shutdown() override {
        ++shutdown_calls;
        record("shutdown");
        const LifecycleState s = state();
        if (s != LifecycleState::UNKNOWN && s != LifecycleState::STOPPED && s != LifecycleState::FAULT) {
            return invalid_state("shutdown");
        }
        // A failed shutdown leaves the state unchanged (a FAULT component stays FAULT).
        if (fail_next_shutdown != ErrorCode::NONE) return inject(fail_next_shutdown, "shutdown failed");
        if (s == LifecycleState::FAULT) go(LifecycleState::STOPPED);
        return Result<void>::success();         // UNKNOWN / STOPPED: nothing to release
    }

    [[nodiscard]] LifecycleState lifecycle_state() const noexcept override { return state(); }
    [[nodiscard]] Status status() const override {
        Status s(state() == LifecycleState::FAULT ? StatusCode::FAILED : StatusCode::OK);
        s.set_message(last_message_);
        return s;
    }
    [[nodiscard]] Health health() const override {
        return Health(state() == LifecycleState::FAULT ? HealthState::UNHEALTHY : HealthState::HEALTHY);
    }
    [[nodiscard]] CapabilitySet capabilities() const override { return capabilities_; }

    /// Optional shared invocation trace: each operation call appends "<id>:<operation>".
    std::vector<std::string>* trace{nullptr};

    // Failure injection (one-shot) and call counters, public for test convenience.
    ErrorCode fail_next_configure{ErrorCode::NONE};
    ErrorCode fail_next_initialize{ErrorCode::NONE};
    ErrorCode fail_next_start{ErrorCode::NONE};
    ErrorCode fail_next_stop{ErrorCode::NONE};
    ErrorCode fail_next_shutdown{ErrorCode::NONE};
    int configure_calls{0}, initialize_calls{0}, start_calls{0}, stop_calls{0}, shutdown_calls{0};
    bool configured{false};

private:
    void record(const char* operation) {
        if (trace != nullptr) trace->push_back(std::to_string(info().id().value()) + ":" + operation);
    }
    [[nodiscard]] LifecycleState state() const noexcept { return lifecycle_.state(); }

    Result<void> fail(ErrorCode code, std::string message) {
        last_message_ = message;
        // Contract: every Error carries source == info().id().
        return Result<void>::failure(Error{code, ErrorSeverity::ERROR, info().id(), {}, std::move(message)});
    }
    Result<void> invalid_state(const char* operation) {
        return fail(ErrorCode::INVALID_STATE, std::string(operation) + " is not valid in this lifecycle state");
    }
    Result<void> inject(ErrorCode& injected, const char* message) {
        const ErrorCode code = injected;
        injected = ErrorCode::NONE;
        return fail(code, message);
    }
    Result<void> fault(ErrorCode& injected, const char* message) {
        go(LifecycleState::FAULT);  // an operation failure moves the component to FAULT
        return inject(injected, message);
    }
    void go(LifecycleState target) {
        const auto r = lifecycle_.transition_to(target);  // reuse the Core transition table
        assert(r.has_value());
        (void)r;
    }

    Lifecycle lifecycle_{};
    Configuration configuration_{};
    CapabilitySet capabilities_{};
    std::string last_message_;
};

} // namespace kritva::core::runtime::contract
