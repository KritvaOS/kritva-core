//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : configuration_conformance.hpp
// Description : Reusable conformance checks of the Component configuration contract (test only).
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-CFG-012
// API         : CORE-TEST-CONFIGURATION-CONFORMANCE
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <kritva/core/core.hpp>

#include "reference_configuration.hpp"

namespace kritva::core::runtime::conformance {

/// What a generic check needs to drive ONE component under test. Everything is public API plus a few
/// test-only observations the component's owner supplies (its own applied state and counters).
class ConfigurationFixture {
public:
    virtual ~ConfigurationFixture() = default;
    virtual Component& component() = 0;
    /// Canonical rendering of the component's applied configuration ("" before any was accepted).
    virtual std::string applied() const = 0;
    /// Two distinct configurations the component accepts (n = 0, 1).
    virtual Configuration valid(int n) const = 0;
    /// A structurally valid configuration the component rejects semantically.
    virtual Configuration invalid() const = 0;
    /// Changes every parameter of `c` to a different value (the caller reusing or clobbering its object).
    virtual void clobber(Configuration& c) const = 0;
    /// Semantic-stage evaluations performed so far (a retry shows up as more than one per call).
    virtual int evaluations() const = 0;
    /// Moves the component to FAULT through its own public lifecycle (a failed operation); false when it cannot.
    virtual bool drive_to_fault() = 0;
};

using FixtureFactory = std::function<std::unique_ptr<ConfigurationFixture>()>;

/// Keeps every Configuration handed to a component alive until the scenario ends, so that a component that
/// wrongly retains the caller's object is DETECTED (by what it reads) instead of causing a use-after-free.
class Keeper {
public:
    const Configuration& keep(Configuration c) { pool_.push_back(std::make_unique<Configuration>(std::move(c))); return *pool_.back(); }
private:
    std::vector<std::unique_ptr<Configuration>> pool_;
};

/// CORE-CFG-012 (contract checks): returns the first violation of the Component configuration contract
/// (CORE-CFG-004, 005, 006, 007), or an empty string when `make()` produces conforming components.
inline std::string check_configuration_contract(const FixtureFactory& make) {
    // ---- legal states: UNKNOWN and STOPPED, the lifecycle state never changes ----------------------
    {
        Keeper k;
        auto f = make();
        Component& c = f->component();
        if (c.lifecycle_state() != LifecycleState::UNKNOWN) return "a new component is not UNKNOWN";
        const Configuration& a = k.keep(f->valid(0));
        if (!c.configure(a)) return "configure() was rejected in UNKNOWN";
        if (c.lifecycle_state() != LifecycleState::UNKNOWN) return "a successful configure() changed the lifecycle state in UNKNOWN";
        const std::string applied_a = f->applied();
        if (applied_a.empty()) return "an accepted configuration is not reflected in the applied state";
        if (!c.initialize() || !c.start() || !c.stop()) return "the component could not be driven to STOPPED";
        if (c.lifecycle_state() != LifecycleState::STOPPED) return "the component is not STOPPED after stop()";
        const Configuration& b = k.keep(f->valid(1));
        if (!c.configure(b)) return "configure() was rejected in STOPPED";
        if (c.lifecycle_state() != LifecycleState::STOPPED) return "a successful configure() changed the lifecycle state in STOPPED";
        if (f->applied() == applied_a) return "a new accepted configuration did not replace the applied state";
    }
    // ---- invalid states: READY, RUNNING and FAULT: INVALID_STATE, no other effect --------------------
    for (int where = 0; where < 3; ++where) {
        Keeper k;
        auto f = make();
        Component& c = f->component();
        if (!c.configure(k.keep(f->valid(0)))) return "the initial configuration was rejected";
        const std::string before_applied = f->applied();
        if (where == 2) {
            if (!f->drive_to_fault() || c.lifecycle_state() != LifecycleState::FAULT) return "the component could not be driven to FAULT";
        } else {
            if (!c.initialize()) return "initialize() failed";
            if (where == 1 && !c.start()) return "start() failed";
        }
        const LifecycleState state = c.lifecycle_state();
        const int evaluations = f->evaluations();
        const auto r = c.configure(k.keep(f->valid(1)));
        if (r) return "configure() was accepted in READY, RUNNING or FAULT";
        if (r.error().code != ErrorCode::INVALID_STATE) return "an invalid-state configure() did not fail with INVALID_STATE";
        if (r.error().source != c.info().id()) return "the invalid-state error is not attributed to the component";
        if (c.lifecycle_state() != state) return "an invalid-state configure() changed the lifecycle state";
        if (f->applied() != before_applied) return "an invalid-state configure() changed the applied configuration";
        if (f->evaluations() != evaluations) return "an invalid-state configure() went on to evaluate the configuration";
    }
    // ---- atomic application: a rejected configuration changes nothing ---------------------------------
    {
        Keeper k;
        auto f = make();
        Component& c = f->component();
        if (!c.configure(k.keep(f->valid(0)))) return "the initial configuration was rejected";
        const std::string before = f->applied();
        const int evaluations = f->evaluations();
        const Configuration& bad = k.keep(f->invalid());
        if (!bad.validate()) return "the fixture's invalid configuration is not structurally valid";
        const auto r = c.configure(bad);
        if (r) return "a semantically invalid configuration was accepted";
        if (r.error().source != c.info().id()) return "the rejection is not attributed to the component";
        if (f->applied() != before) return "a rejected configuration partially replaced the applied state";
        if (c.lifecycle_state() != LifecycleState::UNKNOWN) return "a rejected configuration changed the lifecycle state";
        if (f->evaluations() != evaluations + 1) return "a rejected configuration was evaluated more than once (retry)";
        if (!c.configure(k.keep(f->valid(1))) || f->applied() == before) return "a valid configuration after a rejection did not apply";
    }
    // ---- a first-ever rejection leaves the initial state --------------------------------------------------
    {
        Keeper k;
        auto f = make();
        Component& c = f->component();
        if (c.configure(k.keep(f->invalid()))) return "a semantically invalid configuration was accepted";
        if (!f->applied().empty()) return "a first rejected configuration left a partially applied state";
        if (c.lifecycle_state() != LifecycleState::UNKNOWN) return "a first rejected configuration changed the lifecycle state";
    }
    // ---- detachment: the caller's object may be clobbered and destroyed after configure() ---------------
    {
        auto f = make();
        Component& c = f->component();
        auto owned = std::make_unique<Configuration>(f->valid(0));
        if (!c.configure(*owned)) return "a valid configuration was rejected";
        const std::string applied = f->applied();
        f->clobber(*owned);
        if (f->applied() != applied) return "the component reads through the caller's Configuration after the call";
        owned.reset();
        if (f->applied() != applied) return "the applied state changed when the caller's object was destroyed";
    }
    return {};
}

/// CORE-CFG-012 (Runtime forwarding checks): `components` are harness components registered on the Runtime
/// behind `configure` (a callable that makes the Runtime, or a deliberately broken driver, configure them), with
/// `log` the shared call log, `expected_order` the dependency order and `failing` the id (or 0) that was told to fail.
struct ForwardingExpectation {
    std::vector<std::uint64_t> expected_order;     // component ids in the order they must be configured
    std::uint64_t failing{0};                      // 0: nothing fails
};

inline std::string check_runtime_forwarding(const contract::ConfigurationCallLog& log, const Configuration& given,
                                            const ForwardingExpectation& expect, bool configure_succeeded) {
    const std::size_t expected_calls = expect.failing == 0 ? expect.expected_order.size() : [&] {
        std::size_t n = 0;
        for (const std::uint64_t id : expect.expected_order) { ++n; if (id == expect.failing) break; }
        return n;
    }();
    const std::size_t common = log.size() < expected_calls ? log.size() : expected_calls;
    for (std::size_t i = 0; i < common; ++i) {
        if (log[i].component != expect.expected_order[i]) return "components were not configured in dependency order";
        if (log[i].object != &given) return "a component did not receive the caller's own Configuration object";
    }
    for (std::size_t i = 0; i + 1 < log.size(); ++i) {
        if (!log[i].accepted) return "the sequence continued after a failure";
    }
    if (log.size() > expected_calls) return "more configure() calls than the order requires (retry, rollback or a continued sequence)";
    if (log.size() < expected_calls) return "fewer configure() calls than the order requires";
    if (expect.failing == 0 && !configure_succeeded) return "configure() failed although no component failed";
    if (expect.failing != 0 && configure_succeeded) return "configure() succeeded although a component failed";
    return {};
}
} // namespace kritva::core::runtime::conformance
