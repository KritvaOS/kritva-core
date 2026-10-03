//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component_configuration_test.cpp
// Description : Contract tests for Component configuration lifecycle eligibility.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-CFG-004, CORE-CFG-011
// API         : CORE-API-CONFIGURATION
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#include <cassert>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include <kritva/core/core.hpp>

#include "../contract/reference_component.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;
using kritva::core::runtime::contract::ReferenceComponent;

namespace {

ComponentInfo make_info(std::uint64_t id) {
    auto info = ComponentInfo::create(ComponentId{id}, "cfg");
    assert(info);
    return std::move(info).value();
}

Configuration sample() {
    Configuration c;
    assert(c.set(Parameter{"rate", std::int64_t{10}, "hz"}));
    return c;
}

// Records on which thread configure() ran.
class ThreadProbe final : public ReferenceComponent {
public:
    using ReferenceComponent::ReferenceComponent;
    Result<void> configure(const Configuration& c) override { thread = std::this_thread::get_id(); return ReferenceComponent::configure(c); }
    std::thread::id thread{};
};

template <class T> concept HasReconfigure = requires(T t, const Configuration& c) { t.reconfigure(c); };
template <class T> concept HasSetParameter = requires(T t) { t.set_parameter("a", 1); };
template <class T> concept HasGetParameter = requires(T t) { t.get_parameter("a"); };
template <class T> concept HasConfigurationAccessor = requires(const T t) { t.configuration(); };
template <class T> concept HasApply = requires(T t, const Configuration& c) { t.apply(c); };
template <class T> concept HasUpdate = requires(T t, const Configuration& c) { t.update_configuration(c); };

void test_shape_and_absence_of_dynamic_configuration_api() {
    static_assert(std::is_same_v<decltype(&Component::configure), Result<void> (Component::*)(const Configuration&)>);
    static_assert(std::is_same_v<decltype(&RuntimeManager::configure), Result<void> (RuntimeManager::*)(const Configuration&)>);
    static_assert(!HasReconfigure<Component> && !HasReconfigure<RuntimeManager> && !HasReconfigure<ComponentContext>);
    static_assert(!HasSetParameter<Component> && !HasSetParameter<RuntimeManager> && !HasSetParameter<ComponentContext>);
    static_assert(!HasGetParameter<Component> && !HasGetParameter<RuntimeManager> && !HasGetParameter<ComponentContext>);
    static_assert(!HasConfigurationAccessor<Component> && !HasConfigurationAccessor<RuntimeManager> && !HasConfigurationAccessor<ComponentContext>);
    static_assert(!HasApply<Component> && !HasApply<RuntimeManager> && !HasApply<ComponentContext>);
    static_assert(!HasUpdate<Component> && !HasUpdate<RuntimeManager> && !HasUpdate<ComponentContext>);
    // The lifecycle has exactly the eight R0.1 states: no CONFIGURED / RECONFIGURING is added.
    static_assert(static_cast<int>(LifecycleState::UNKNOWN) == 0 && static_cast<int>(LifecycleState::INITIALIZING) == 1 &&
                  static_cast<int>(LifecycleState::READY) == 2 && static_cast<int>(LifecycleState::RUNNING) == 3 &&
                  static_cast<int>(LifecycleState::STOPPING) == 4 && static_cast<int>(LifecycleState::STOPPED) == 5 &&
                  static_cast<int>(LifecycleState::FAULT) == 6 && static_cast<int>(LifecycleState::RECOVERING) == 7);
}

void test_configure_is_valid_from_unknown_and_stopped_and_leaves_the_state_unchanged() {
    ReferenceComponent c(make_info(1));
    assert(c.lifecycle_state() == LifecycleState::UNKNOWN);
    assert(c.configure(sample()));
    assert(c.lifecycle_state() == LifecycleState::UNKNOWN && c.configured);          // applied, still UNKNOWN
    assert(c.configure(sample()));                                                   // repeatable while UNKNOWN
    assert(c.initialize() && c.start() && c.stop());
    assert(c.lifecycle_state() == LifecycleState::STOPPED);
    c.configured = false;
    assert(c.configure(sample()));
    assert(c.lifecycle_state() == LifecycleState::STOPPED && c.configured);          // applied, still STOPPED (not READY, not CONFIGURED)
    assert(c.initialize() && c.lifecycle_state() == LifecycleState::READY);          // and the component can then be initialized again
}

void expect_invalid_state_without_effect(ReferenceComponent& c, ComponentId id) {
    const LifecycleState before = c.lifecycle_state();
    const int calls = c.configure_calls;
    c.configured = false;
    c.fail_next_configure = ErrorCode::TIMEOUT;                                      // would be consumed if anything past the state check ran
    const auto r = c.configure(sample());
    assert(!r && r.error().code == ErrorCode::INVALID_STATE);
    assert(r.error().source == id);
    assert(c.lifecycle_state() == before);                                           // state unchanged
    assert(!c.configured);                                                           // nothing applied
    assert(c.fail_next_configure == ErrorCode::TIMEOUT);                             // nothing validated or consumed
    assert(c.configure_calls == calls + 1);
    c.fail_next_configure = ErrorCode::NONE;
}

void test_configure_is_invalid_in_ready_running_and_fault_without_effect() {
    ReferenceComponent c(make_info(2));
    assert(c.initialize());
    expect_invalid_state_without_effect(c, ComponentId{2});                          // READY
    assert(c.start());
    expect_invalid_state_without_effect(c, ComponentId{2});                          // RUNNING
    assert(c.stop() && c.shutdown());
    ReferenceComponent f(make_info(3));
    f.fail_next_initialize = ErrorCode::INTERNAL_ERROR;
    assert(!f.initialize() && f.lifecycle_state() == LifecycleState::FAULT);
    expect_invalid_state_without_effect(f, ComponentId{3});                          // FAULT
    assert(f.shutdown() && f.lifecycle_state() == LifecycleState::STOPPED);          // the way back is shutdown(), then configure() works
    assert(f.configure(sample()));
}

void test_configure_never_changes_state_in_any_reachable_state() {
    // Every observable (state, success/failure) pair: the state afterwards equals the state before.
    for (int variant = 0; variant < 5; ++variant) {  // UNKNOWN, READY, RUNNING, STOPPED, FAULT
        ReferenceComponent c(make_info(4));
        if (variant >= 1) assert(c.initialize());
        if (variant >= 2) assert(c.start());
        if (variant == 3) assert(c.stop());
        if (variant == 4) { c.fail_next_stop = ErrorCode::INTERNAL_ERROR; (void)c.stop(); assert(c.lifecycle_state() == LifecycleState::FAULT); }
        const LifecycleState before = c.lifecycle_state();
        (void)c.configure(sample());
        assert(c.lifecycle_state() == before);
    }
}

void test_a_rejected_valid_state_configuration_also_leaves_the_state_unchanged() {
    ReferenceComponent c(make_info(5));
    c.fail_next_configure = ErrorCode::CONFIGURATION_ERROR;
    const auto r = c.configure(sample());
    assert(!r && r.error().code == ErrorCode::CONFIGURATION_ERROR && r.error().source == ComponentId{5});
    assert(c.lifecycle_state() == LifecycleState::UNKNOWN && !c.configured);
    assert(c.configure(sample()) && c.configured);                                   // and a later explicit attempt works (no retry happened)
}

void test_configure_is_synchronous_on_the_callers_thread() {
    ThreadProbe c(make_info(6));
    assert(c.configure(sample()));
    assert(c.thread == std::this_thread::get_id());
    RuntimeManager runtime;
    ThreadProbe a(make_info(7)), b(make_info(8));
    assert(runtime.register_component(a) && runtime.register_component(b));
    assert(runtime.configure(sample()));
    assert(a.thread == std::this_thread::get_id() && b.thread == std::this_thread::get_id());   // no Core thread or executor
}

void test_runtime_configure_eligibility_in_every_runtime_state() {
    std::vector<std::string> trace;
    ReferenceComponent a(make_info(1)), b(make_info(2));
    a.trace = &trace; b.trace = &trace;
    RuntimeManager runtime;
    assert(runtime.register_component(a) && runtime.register_component(b));
    auto expect_invalid = [&](LifecycleState state) {
        assert(runtime.state() == state);
        trace.clear();
        const auto errors = runtime.statistics().error_count.value();
        const auto samples = runtime.statistics().sample_count.value();
        const auto r = runtime.configure(sample());
        assert(!r && r.error().code == ErrorCode::INVALID_STATE);
        assert(trace.empty());                                                       // no component was invoked
        assert(runtime.state() == state);                                            // state unchanged
        assert(runtime.statistics().error_count.value() == errors && runtime.statistics().sample_count.value() == samples);
    };
    auto expect_valid = [&](LifecycleState state) {
        assert(runtime.state() == state);
        trace.clear();
        assert(runtime.configure(sample()));
        assert(trace.size() == 2);                                                   // every component configured
        assert(runtime.state() == state);                                            // success leaves the Runtime state unchanged
        assert(runtime.fault_error() == nullptr);
    };
    expect_valid(LifecycleState::UNKNOWN);
    assert(runtime.initialize());
    expect_invalid(LifecycleState::READY);
    assert(runtime.start());
    expect_invalid(LifecycleState::RUNNING);
    assert(runtime.stop());
    expect_valid(LifecycleState::STOPPED);
    assert(runtime.initialize() && runtime.start());
    b.fail_next_stop = ErrorCode::INTERNAL_ERROR;
    assert(!runtime.stop() && runtime.state() == LifecycleState::FAULT);
    expect_invalid(LifecycleState::FAULT);
    assert(runtime.reset());
    expect_valid(LifecycleState::STOPPED);                                           // reset() then configure() is the way back
}

void test_a_failed_runtime_configure_leaves_the_runtime_state_unchanged() {
    ReferenceComponent a(make_info(1));
    RuntimeManager runtime;
    assert(runtime.register_component(a));
    a.fail_next_configure = ErrorCode::CONFIGURATION_ERROR;
    const auto r = runtime.configure(sample());
    assert(!r && r.error().code == ErrorCode::CONFIGURATION_ERROR && r.error().source == ComponentId{1});
    assert(runtime.state() == LifecycleState::UNKNOWN && runtime.fault_error() == nullptr);   // a rejected configuration is not a Runtime FAULT
    assert(runtime.configure(sample()) && runtime.initialize());                              // an explicit new attempt, then normal operation
}

} // namespace

int main() {
    test_shape_and_absence_of_dynamic_configuration_api();
    test_configure_is_valid_from_unknown_and_stopped_and_leaves_the_state_unchanged();
    test_configure_is_invalid_in_ready_running_and_fault_without_effect();
    test_configure_never_changes_state_in_any_reachable_state();
    test_a_rejected_valid_state_configuration_also_leaves_the_state_unchanged();
    test_configure_is_synchronous_on_the_callers_thread();
    test_runtime_configure_eligibility_in_every_runtime_state();
    test_a_failed_runtime_configure_leaves_the_runtime_state_unchanged();
    return 0;
}
