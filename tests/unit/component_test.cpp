//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component_test.cpp
// Description : Component identity, metadata and lifecycle contract tests.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-RT-001
// API         : CORE-TEST-COMPONENT
//
// Author      : KritvaOS Core Team
// Created     : 03-10-2026
//==============================================================================

#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>

#include <kritva/core/core.hpp>

#include "../contract/component_contract.hpp"
#include "../contract/reference_component.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;
using kritva::core::runtime::contract::ReferenceComponent;

namespace {

ComponentInfo make_info(std::uint64_t id, const char* name, Version version = {}) {
    auto info = ComponentInfo::create(ComponentId{id}, name, version);
    assert(info.has_value());
    return std::move(info).value();
}

Configuration valid_configuration() {
    Configuration configuration;
    const auto r = configuration.set(Parameter{"rate", ParameterValue{std::int64_t{100}}, "Hz"});
    assert(r.has_value());
    (void)r;
    return configuration;
}

// -----------------------------------------------------------------------------
// Identity: ComponentId (AC-001-01)
// -----------------------------------------------------------------------------

void test_component_id() {
    static_assert(std::is_same_v<ComponentId, Id>);
    static_assert(std::is_trivially_copyable_v<ComponentId>);

    assert(ComponentId{1}.valid());
    assert(!ComponentId{}.valid());        // empty identity
    assert(!ComponentId{0}.valid());

    // Deterministic comparison and hashing.
    assert(ComponentId{1} == ComponentId{1});
    assert(ComponentId{1} != ComponentId{2});
    assert(ComponentId{1} < ComponentId{2});
    assert(std::hash<ComponentId>{}(ComponentId{7}) == std::hash<ComponentId>{}(ComponentId{7}));
    std::unordered_set<ComponentId> set{ComponentId{1}, ComponentId{2}, ComponentId{1}};
    assert(set.size() == 2);

    // Identity is the value: it is independent of any component object.
    static_assert(ComponentId{42}.value() == 42);
}

// -----------------------------------------------------------------------------
// Metadata: ComponentInfo (AC-001-02)
// -----------------------------------------------------------------------------

void test_component_info_valid() {
    const auto info = ComponentInfo::create(ComponentId{5}, "sensor-fusion", Version{1, 2, 3});
    assert(info.has_value());
    assert(info.value().id() == ComponentId{5});
    assert(info.value().name() == "sensor-fusion");
    assert(info.value().version() == (Version{1, 2, 3}));

    // Default version is 0.0.0; the name is free-form but non-empty.
    const auto defaults = ComponentInfo::create(ComponentId{6}, "x");
    assert(defaults.has_value() && defaults.value().version() == Version{});
    assert(ComponentInfo::create(ComponentId{7}, " ").has_value());

    // Names are labels, not identities: two components may share a name.
    const auto a = ComponentInfo::create(ComponentId{8}, "same");
    const auto b = ComponentInfo::create(ComponentId{9}, "same");
    assert(a && b && a.value().name() == b.value().name() && a.value().id() != b.value().id());
}

void test_component_info_invalid() {
    const auto no_id = ComponentInfo::create(ComponentId{}, "name");
    assert(!no_id.has_value());
    assert(no_id.error().code == ErrorCode::INVALID_ARGUMENT);
    assert(!no_id.error().message.empty());

    const auto no_name = ComponentInfo::create(ComponentId{1}, "");
    assert(!no_name.has_value());
    assert(no_name.error().code == ErrorCode::INVALID_ARGUMENT);

    assert(!ComponentInfo::create(ComponentId{0}, "").has_value());   // both invalid

    // A ComponentInfo cannot be built any other way, so one is always valid.
    static_assert(!std::is_default_constructible_v<ComponentInfo>);
    static_assert(std::is_copy_constructible_v<ComponentInfo>);
    static_assert(noexcept(std::declval<const ComponentInfo&>().id()));
    static_assert(noexcept(std::declval<const ComponentInfo&>().name()));
    static_assert(noexcept(std::declval<const ComponentInfo&>().version()));
}

// -----------------------------------------------------------------------------
// Ownership / lifetime and immutability (AC-001-01, AC-001-05)
// -----------------------------------------------------------------------------

void test_component_ownership_shape() {
    static_assert(std::is_abstract_v<Component>);
    static_assert(std::has_virtual_destructor_v<Component>);
    static_assert(!std::is_copy_constructible_v<Component>);
    static_assert(!std::is_copy_assignable_v<Component>);
    static_assert(!std::is_move_constructible_v<Component>);
    static_assert(!std::is_move_assignable_v<Component>);
    static_assert(!std::is_copy_constructible_v<ReferenceComponent>);
    static_assert(!std::is_move_assignable_v<ReferenceComponent>);   // identity cannot be reassigned
    static_assert(std::is_same_v<decltype(std::declval<const Component&>().info()), const ComponentInfo&>);
    static_assert(noexcept(std::declval<const Component&>().info()));
    static_assert(!std::is_constructible_v<ReferenceComponent>);     // identity is mandatory

    // Deleting through the base pointer works; the owner decides the lifetime.
    std::unique_ptr<Component> owned = std::make_unique<ReferenceComponent>(make_info(1, "owned"));
    assert(owned->info().id() == ComponentId{1});
    owned.reset();

    // The info reference is stable for the component's lifetime (no dangling).
    ReferenceComponent component(make_info(2, "stable"));
    const ComponentInfo* before = &component.info();
    assert(component.initialize() && component.start() && component.stop());
    assert(&component.info() == before);
    assert(component.info().id() == ComponentId{2});
}

// -----------------------------------------------------------------------------
// Lifecycle contract through the reusable checker (AC-001-03)
// -----------------------------------------------------------------------------

void test_reference_component_conforms() {
    ReferenceComponent component(make_info(10, "reference", Version{0, 1, 0}));
    assert(component.lifecycle_state() == LifecycleState::UNKNOWN);   // initial state
    contract::check_component_contract(component, valid_configuration());
    assert(component.lifecycle_state() == LifecycleState::STOPPED);
}

void test_minimal_consumer_uses_only_the_interface() {
    // A consumer sees only Component&: identity, lifecycle, Result, observers.
    ReferenceComponent implementation(make_info(11, "consumer"));
    Component& component = implementation;
    assert(component.info().name() == "consumer");
    assert(component.initialize());
    assert(component.start());
    assert(component.lifecycle_state() == LifecycleState::RUNNING);
    assert(component.health().state() == HealthState::HEALTHY);
    assert(component.status().code() == StatusCode::OK);
    assert(component.capabilities().empty());
    assert(component.stop());
}

// Every state reached through Component operations is a Core Lifecycle state
// and every step is an edge of the Core transition table.
void test_operations_use_core_transition_table() {
    ReferenceComponent component(make_info(12, "walk"));
    LifecycleState previous = component.lifecycle_state();
    auto check = [&](bool ok) {
        assert(ok);
        const LifecycleState now = component.lifecycle_state();
        assert(now != previous);
        // Final states are reachable per the table via the transient states.
        previous = now;
    };
    check(component.initialize().has_value());   // UNKNOWN -> (INITIALIZING) -> READY
    check(component.start().has_value());        // READY -> RUNNING
    check(component.stop().has_value());         // RUNNING -> (STOPPING) -> STOPPED
    check(component.initialize().has_value());   // STOPPED -> (INITIALIZING) -> READY
    check(component.stop().has_value());         // READY -> STOPPED
    // The transient states are never observable once an operation has returned.
    assert(component.lifecycle_state() != LifecycleState::INITIALIZING);
    assert(component.lifecycle_state() != LifecycleState::STOPPING);
}

// -----------------------------------------------------------------------------
// Failure behavior and Result/Error integration (AC-001-03, AC-001-04)
// -----------------------------------------------------------------------------

void test_operation_failure_moves_to_fault_with_attributable_error() {
    struct Case { const char* op; ErrorCode code; };
    const Case cases[] = {{"initialize", ErrorCode::NOT_READY}, {"start", ErrorCode::TIMEOUT},
                          {"stop", ErrorCode::INTERNAL_ERROR}};
    for (const Case& c : cases) {
        ReferenceComponent component(make_info(20, "failing"));
        const std::string op = c.op;
        if (op == "initialize") component.fail_next_initialize = c.code;
        if (op == "start")      { assert(component.initialize()); component.fail_next_start = c.code; }
        if (op == "stop")       { assert(component.initialize() && component.start()); component.fail_next_stop = c.code; }

        const Result<void> r = op == "initialize" ? component.initialize()
                              : op == "start"      ? component.start()
                                                   : component.stop();
        assert(!r.has_value());
        assert(r.error().code == c.code);                       // inspectable cause
        assert(r.error().source == ComponentId{20});            // attributable
        assert(r.error().severity == ErrorSeverity::ERROR);
        assert(!r.error().message.empty());
        assert(component.lifecycle_state() == LifecycleState::FAULT);
        assert(component.health().state() == HealthState::UNHEALTHY);
        assert(component.status().code() == StatusCode::FAILED);

        // FAULT: every operation except shutdown() is invalid; no automatic recovery.
        const Result<void> again = component.start();
        assert(!again && again.error().code == ErrorCode::INVALID_STATE);
        assert(!component.initialize() && !component.stop() && !component.configure(valid_configuration()));
        assert(component.lifecycle_state() == LifecycleState::FAULT);

        // shutdown() leaves FAULT for STOPPED; the component is then re-initializable.
        assert(component.shutdown());
        assert(component.lifecycle_state() == LifecycleState::STOPPED);
        assert(component.initialize());
        assert(component.lifecycle_state() == LifecycleState::READY);
    }
}

void test_failed_shutdown_leaves_state_unchanged() {
    ReferenceComponent component(make_info(21, "shutdown-fails"));
    component.fail_next_initialize = ErrorCode::RESOURCE_UNAVAILABLE;
    assert(!component.initialize());
    assert(component.lifecycle_state() == LifecycleState::FAULT);

    component.fail_next_shutdown = ErrorCode::INTERNAL_ERROR;
    const Result<void> r = component.shutdown();
    assert(!r && r.error().code == ErrorCode::INTERNAL_ERROR && r.error().source == ComponentId{21});
    assert(component.lifecycle_state() == LifecycleState::FAULT);   // stays FAULT

    assert(component.shutdown());                                    // retry is explicit
    assert(component.lifecycle_state() == LifecycleState::STOPPED);
}

void test_rejected_configuration_is_not_applied() {
    ReferenceComponent component(make_info(22, "config"));

    // Core Configuration rejects an empty parameter name before it can be stored,
    // so a rejection from the component comes from its own validation.
    component.fail_next_configure = ErrorCode::CONFIGURATION_ERROR;
    const Result<void> r = component.configure(valid_configuration());
    assert(!r && r.error().code == ErrorCode::CONFIGURATION_ERROR);
    assert(r.error().source == ComponentId{22});
    assert(!component.configured);                               // not partially applied
    assert(component.lifecycle_state() == LifecycleState::UNKNOWN);   // state unchanged

    assert(component.configure(valid_configuration()));         // one-shot failure
    assert(component.configured);
    assert(component.lifecycle_state() == LifecycleState::UNKNOWN);
}

void test_invalid_operations_have_no_side_effects() {
    ReferenceComponent component(make_info(23, "invalid"));
    assert(component.initialize());
    assert(component.start());
    const int starts = component.start_calls;
    const Result<void> r = component.start();                    // RUNNING: start invalid
    assert(!r && r.error().code == ErrorCode::INVALID_STATE);
    assert(r.error().source == ComponentId{23});
    assert(component.lifecycle_state() == LifecycleState::RUNNING);
    assert(component.start_calls == starts + 1);                  // called, but nothing else changed
    assert(component.health().state() == HealthState::HEALTHY);   // not faulted by a rejected call
}

} // namespace

int main() {
    test_component_id();
    test_component_info_valid();
    test_component_info_invalid();
    test_component_ownership_shape();
    test_reference_component_conforms();
    test_minimal_consumer_uses_only_the_interface();
    test_operations_use_core_transition_table();
    test_operation_failure_moves_to_fault_with_attributable_error();
    test_failed_shutdown_leaves_state_unchanged();
    test_rejected_configuration_is_not_applied();
    test_invalid_operations_have_no_side_effects();
    return 0;
}
