//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component_contract.hpp
// Description : Reusable conformance checks for runtime::Component.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-RT-001
// API         : CORE-TEST-COMPONENT-CONTRACT
//
// Author      : KritvaOS Core Team
// Created     : 03-10-2026
//==============================================================================

#pragma once

#include <cassert>
#include <string>

#include <kritva/core/core.hpp>

namespace kritva::core::runtime::contract {

namespace detail {
inline void expect_ok(const Result<void>& r) { assert(r.has_value()); (void)r; }

/// Invalid operation: INVALID_STATE, attributable to the component, state unchanged.
inline void expect_invalid_state(const Component& c, const Result<void>& r, LifecycleState before) {
    assert(!r.has_value());
    assert(r.error().code == ErrorCode::INVALID_STATE);
    assert(r.error().source == c.info().id());
    assert(c.lifecycle_state() == before);
    (void)c; (void)r; (void)before;
}
} // namespace detail

/// Checks the implementation-independent rules of the Component contract
/// (runtime/component.hpp) over the states reachable through Component
/// operations: UNKNOWN, READY, RUNNING, STOPPED. FAULT is reachable only by
/// operation failure and is checked against implementations that can inject one.
///
/// Preconditions: `c` is freshly constructed (UNKNOWN) and `configuration` is
/// valid. Postcondition: `c` is STOPPED.
inline void check_component_contract(Component& c, const Configuration& configuration) {
    using detail::expect_invalid_state;
    using detail::expect_ok;
    using S = LifecycleState;

    // Identity snapshot: must be unchanged, at the same address, after everything.
    const ComponentInfo* info_address = &c.info();
    const ComponentId id = c.info().id();
    const std::string name = c.info().name();
    const Version version = c.info().version();
    assert(id.valid());
    assert(!name.empty());

    // --- UNKNOWN -----------------------------------------------------------
    assert(c.lifecycle_state() == S::UNKNOWN);
    expect_invalid_state(c, c.start(), S::UNKNOWN);
    expect_invalid_state(c, c.stop(), S::UNKNOWN);
    expect_ok(c.shutdown());                         // idempotent no-op, state unchanged
    assert(c.lifecycle_state() == S::UNKNOWN);
    expect_ok(c.configure(configuration));           // does not change the state
    assert(c.lifecycle_state() == S::UNKNOWN);
    expect_ok(c.configure(configuration));
    expect_ok(c.initialize());                       // UNKNOWN -> READY
    assert(c.lifecycle_state() == S::READY);

    // --- READY -------------------------------------------------------------
    expect_invalid_state(c, c.configure(configuration), S::READY);
    expect_invalid_state(c, c.initialize(), S::READY);
    expect_invalid_state(c, c.shutdown(), S::READY);  // stop() first
    expect_ok(c.start());                            // READY -> RUNNING
    assert(c.lifecycle_state() == S::RUNNING);

    // --- RUNNING -----------------------------------------------------------
    expect_invalid_state(c, c.configure(configuration), S::RUNNING);
    expect_invalid_state(c, c.initialize(), S::RUNNING);
    expect_invalid_state(c, c.start(), S::RUNNING);
    expect_invalid_state(c, c.shutdown(), S::RUNNING);
    expect_ok(c.stop());                             // RUNNING -> STOPPED
    assert(c.lifecycle_state() == S::STOPPED);

    // --- STOPPED -----------------------------------------------------------
    expect_invalid_state(c, c.start(), S::STOPPED);
    expect_invalid_state(c, c.stop(), S::STOPPED);
    expect_ok(c.shutdown());                         // release; stays STOPPED
    expect_ok(c.shutdown());                         // idempotent
    assert(c.lifecycle_state() == S::STOPPED);
    expect_ok(c.configure(configuration));           // reconfigure while stopped
    expect_ok(c.initialize());                       // STOPPED -> READY (re-initialize)
    assert(c.lifecycle_state() == S::READY);
    expect_ok(c.stop());                             // READY -> STOPPED
    assert(c.lifecycle_state() == S::STOPPED);

    // --- observers are snapshots by value ------------------------------------
    {
        const Status st = c.status();
        const Health h = c.health();
        CapabilitySet caps = c.capabilities();
        const std::size_t before = caps.size();
        caps.add(Capability{CapabilityId{999}, "added-by-caller", Version{}});
        assert(c.capabilities().size() == before);   // the copy does not alias the component
        (void)st; (void)h;
    }

    // --- identity is immutable and unaffected by lifecycle ------------------
    assert(&c.info() == info_address);
    assert(c.info().id() == id);
    assert(c.info().name() == name);
    assert(c.info().version() == version);
    assert(c.lifecycle_state() == S::STOPPED);
}

} // namespace kritva::core::runtime::contract
