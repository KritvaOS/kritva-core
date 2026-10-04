//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : capability_readiness_integration_test.cpp
// Description : Capability availability, Component readiness, lifecycle and dependency order stay distinct (public APIs only).
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-CAP-009
// API         : CORE-API-CAPABILITY
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

// Public APIs only: RuntimeManager, Component, ComponentContext, PlatformContext, PlatformRequirements, CapabilitySet and the
// test-only reference component. A "consumer" Component checks its OWN prerequisite inside its own initialize() through its
// ComponentContext and fails with the existing Error; the Runtime only propagates that failure through the existing
// lifecycle semantics. The proofs: no readiness state exists, the Runtime never evaluates, resolves, retries or recovers
// because of a capability, capability presence never changes the dependency order, Health never changes the lifecycle,
// and a seeded model predicts every outcome from the dependency order and the provision alone.

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <random>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <kritva/core/core.hpp>

#include "../contract/reference_component.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;

namespace {

ComponentInfo make_info(std::uint64_t id) {
    auto info = ComponentInfo::create(ComponentId{id}, "c");
    assert(info);
    return std::move(info).value();
}

// An adapter with no services and a capability declaration the test can change; it counts every capability snapshot taken.
class ProvisionAdapter final : public platform::IPlatformAdapter {
public:
    [[nodiscard]] const platform::PlatformInfo& info() const noexcept override { return info_; }
    [[nodiscard]] platform::IScheduler* scheduler() const noexcept override { return nullptr; }
    [[nodiscard]] time::IClock* clock() const noexcept override { return nullptr; }
    [[nodiscard]] time::ITimer* timer() const noexcept override { return nullptr; }
    [[nodiscard]] platform::IWatchdog* watchdog() const noexcept override { return nullptr; }
    [[nodiscard]] CapabilitySet capabilities() const override { ++snapshots; return provided; }
    CapabilitySet provided;
    mutable std::size_t snapshots{0};
private:
    platform::PlatformInfo info_{"provision", Version{1, 0, 0}};
};

void provide(ProvisionAdapter& a, std::initializer_list<std::uint64_t> ids) {
    a.provided = CapabilitySet{};
    for (const std::uint64_t id : ids) a.provided.add(Capability{CapabilityId{id}, "c", Version{1, 0, 0}});
}

// A Component that needs capabilities. It decides for ITSELF, inside its own initialize(), whether its prerequisite is
// sufficient, using the existing requirement check through its ComponentContext, and fails with that existing Error.
class ConsumerComponent final : public contract::ReferenceComponent {
public:
    ConsumerComponent(ComponentInfo info, platform::IPlatformAdapter& adapter, std::vector<std::uint64_t> required, std::vector<std::uint64_t> optional = {})
        : ReferenceComponent(std::move(info)), context_(*this, platform::PlatformContext(adapter)) {
        for (const std::uint64_t id : required) assert(needs_.add_capability(CapabilityId{id}, platform::Requirement::REQUIRED));
        for (const std::uint64_t id : optional) assert(needs_.add_capability(CapabilityId{id}, platform::Requirement::OPTIONAL));
    }
    Result<void> initialize() override {
        ++initialize_attempts;
        const Result<void> check = context_.check_required(needs_);
        if (!check) {
            fail_next_initialize = ErrorCode::UNSUPPORTED;               // the component leaves FAULT as a failed initialize() does ...
            (void)ReferenceComponent::initialize();
            return check;                                                // ... and reports the context's own, already attributed Error
        }
        return ReferenceComponent::initialize();
    }
    [[nodiscard]] Health health() const override { return forced_health ? *forced_health : ReferenceComponent::health(); }
    const ComponentContext& context() const { return context_; }
    int initialize_attempts{0};
    std::optional<Health> forced_health;
private:
    ComponentContext context_;
    platform::PlatformRequirements needs_;
};

const std::set<LifecycleState> kStates{LifecycleState::UNKNOWN, LifecycleState::READY, LifecycleState::RUNNING, LifecycleState::STOPPED, LifecycleState::FAULT};

void test_a_missing_prerequisite_is_surfaced_by_the_component_without_a_new_state() {
    ProvisionAdapter adapter;
    provide(adapter, {1});
    ConsumerComponent a(make_info(1), adapter, {1});
    ConsumerComponent b(make_info(2), adapter, {1, 2});                            // needs 2: not provided
    ConsumerComponent c(make_info(3), adapter, {1});
    RuntimeManager runtime;
    assert(runtime.register_component(a) && runtime.register_component(b) && runtime.register_component(c));
    assert(runtime.add_dependency(ComponentId{2}, ComponentId{1}) && runtime.add_dependency(ComponentId{3}, ComponentId{2}));
    assert(runtime.configure(Configuration{}));
    const auto r = runtime.initialize();
    assert(!r && r.error().code == ErrorCode::UNSUPPORTED && r.error().source == ComponentId{2});   // the component's own Error, attributed to it
    assert(runtime.state() == LifecycleState::FAULT && runtime.fault_error() != nullptr);             // the EXISTING failed-initialize semantics
    assert(runtime.fault_error()->source == ComponentId{2});
    assert(kStates.count(runtime.state()) == 1);                                                      // no "waiting" or "not ready" state exists
    assert(a.initialize_attempts == 1 && b.initialize_attempts == 1 && c.initialize_attempts == 0);   // fail-fast: the dependent was never invoked
    assert(a.lifecycle_state() == LifecycleState::READY && b.lifecycle_state() == LifecycleState::FAULT && c.lifecycle_state() == LifecycleState::UNKNOWN);
}

void test_nothing_retries_or_recovers_because_a_capability_appears_later() {
    ProvisionAdapter adapter;
    provide(adapter, {1});
    ConsumerComponent a(make_info(1), adapter, {1}), b(make_info(2), adapter, {2});
    RuntimeManager runtime;
    assert(runtime.register_component(a) && runtime.register_component(b) && runtime.add_dependency(ComponentId{2}, ComponentId{1}));
    assert(!runtime.initialize() && runtime.state() == LifecycleState::FAULT);
    const std::size_t snapshots = adapter.snapshots;
    provide(adapter, {1, 2});                                                       // the missing capability now exists ...
    for (int i = 0; i < 50; ++i) { (void)runtime.state(); (void)runtime.fault_error(); }
    assert(runtime.state() == LifecycleState::FAULT);                               // ... and nothing noticed, retried or recovered
    assert(a.initialize_attempts == 1 && b.initialize_attempts == 1 && adapter.snapshots == snapshots);   // no poll, no re-evaluation, no call
    assert(runtime.reset() && runtime.state() == LifecycleState::STOPPED);          // only the explicit, caller-requested reset ...
    assert(runtime.initialize() && runtime.state() == LifecycleState::READY);       // ... then an explicit new attempt succeeds
    assert(a.initialize_attempts == 2 && b.initialize_attempts == 2);
    assert(runtime.start() && runtime.state() == LifecycleState::RUNNING);
}

void test_a_successful_check_changes_nothing_outside_the_lifecycle_operation() {
    ProvisionAdapter adapter;
    provide(adapter, {1, 2});
    ConsumerComponent a(make_info(1), adapter, {1, 2});
    RuntimeManager runtime;
    assert(runtime.register_component(a));
    const platform::PlatformRequirements none;
    for (int i = 0; i < 20; ++i) {                                                  // explicit checks between operations
        assert(a.context().has_capability(CapabilityId{1}) && !a.context().has_capability(CapabilityId{3}));
        (void)a.context().evaluate(none);
        assert(a.lifecycle_state() == LifecycleState::UNKNOWN && runtime.state() == LifecycleState::UNKNOWN);
    }
    assert(a.initialize_attempts == 0 && runtime.statistics().sample_count.value() == 0 && runtime.statistics().error_count.value() == 0);
    assert(runtime.initialize() && a.initialize_attempts == 1 && runtime.state() == LifecycleState::READY);
    assert(runtime.statistics().sample_count.value() == 1);                         // the Runtime counted only its own component call
}

void test_health_never_changes_the_lifecycle_and_capabilities_never_change_health() {
    ProvisionAdapter adapter;
    provide(adapter, {1});
    ConsumerComponent a(make_info(1), adapter, {1});
    RuntimeManager runtime;
    assert(runtime.register_component(a) && runtime.initialize() && runtime.start());
    for (const HealthState h : {HealthState::UNHEALTHY, HealthState::DEGRADED, HealthState::HEALTHY, HealthState::UNKNOWN}) {
        a.forced_health = Health(h);
        provide(adapter, h == HealthState::UNHEALTHY ? std::initializer_list<std::uint64_t>{} : std::initializer_list<std::uint64_t>{1});   // availability moves too
        assert(runtime.state() == LifecycleState::RUNNING && a.lifecycle_state() == LifecycleState::RUNNING);                              // nothing follows either
        assert(a.health().state() == h);                                                                                                    // and the capability set did not touch Health
    }
    assert(runtime.statistics().error_count.value() == 0 && runtime.fault_error() == nullptr);
    assert(runtime.stop() && runtime.shutdown());
}

void test_the_runtime_never_takes_a_capability_snapshot_or_evaluates_anything() {
    ProvisionAdapter adapter;
    provide(adapter, {1, 2, 3});
    ConsumerComponent a(make_info(1), adapter, {}), b(make_info(2), adapter, {});   // consumers that require nothing: the provider is never asked
    RuntimeManager runtime;
    assert(runtime.register_component(a) && runtime.register_component(b) && runtime.attach_platform(adapter));
    assert(runtime.configure(Configuration{}) && runtime.initialize() && runtime.start() && runtime.stop() && runtime.shutdown());
    assert(adapter.snapshots == 0);
}

void test_capability_presence_never_changes_the_dependency_order_or_the_initialization_trace() {
    for (unsigned seed = 1; seed <= 60; ++seed) {
        std::mt19937 rng(seed);
        const std::size_t n = 2 + rng() % 5;
        std::vector<std::pair<std::uint64_t, std::uint64_t>> edges;
        for (std::uint64_t a = 2; a <= n; ++a) for (std::uint64_t b = 1; b < a; ++b) if (rng() % 3 == 0) edges.push_back({a, b});
        std::vector<std::vector<std::uint64_t>> traces;
        for (unsigned variant = 0; variant < 3; ++variant) {
            ProvisionAdapter adapter;
            for (std::uint64_t id = 1; id <= 6; ++id) if (rng() % 2 == 0) adapter.provided.add(Capability{CapabilityId{id}, "c", Version{}});
            std::vector<std::unique_ptr<ConsumerComponent>> comps;
            std::vector<std::string> trace;
            RuntimeManager runtime;
            for (std::uint64_t id = 1; id <= n; ++id) {
                comps.push_back(std::make_unique<ConsumerComponent>(make_info(id), adapter, std::vector<std::uint64_t>{}));   // requirements are met by construction
                comps.back()->trace = &trace;
                assert(runtime.register_component(*comps.back()));
            }
            for (const auto& e : edges) assert(runtime.add_dependency(ComponentId{e.first}, ComponentId{e.second}));
            assert(runtime.initialize());
            std::vector<std::uint64_t> order;
            for (const std::string& t : trace) order.push_back(std::stoull(t.substr(0, t.find(':'))));
            traces.push_back(order);
        }
        assert(traces[0] == traces[1] && traces[1] == traces[2]);                   // three different provisions, one and the same order
    }
}

// The model: every outcome is predicted from the dependency order and the provision alone.
void test_a_seeded_model_predicts_every_outcome_and_runs_are_deterministic() {
    int failures = 0, successes = 0, mid_failures = 0;
    for (unsigned seed = 1; seed <= 200; ++seed) {
        auto run = [&](unsigned s, std::string& transcript) {
            std::mt19937 rng(s);
            const std::size_t n = 1 + rng() % 6;
            std::vector<std::pair<std::uint64_t, std::uint64_t>> edges;
            for (std::uint64_t a = 2; a <= n; ++a) for (std::uint64_t b = 1; b < a; ++b) if (rng() % 3 == 0) edges.push_back({a, b});
            ProvisionAdapter adapter;
            for (std::uint64_t id = 1; id <= 5; ++id) if (rng() % 3 != 0) adapter.provided.add(Capability{CapabilityId{id}, "c", Version{static_cast<std::uint32_t>(rng() % 3), 0, 0}});
            std::vector<std::vector<std::uint64_t>> required(n + 1);
            std::vector<std::unique_ptr<ConsumerComponent>> comps;
            std::vector<std::string> trace;
            RuntimeManager runtime;
            for (std::uint64_t id = 1; id <= n; ++id) {
                for (std::uint64_t cap = 1; cap <= 5; ++cap) if (rng() % 5 == 0) required[id].push_back(cap);
                comps.push_back(std::make_unique<ConsumerComponent>(make_info(id), adapter, required[id]));
                comps.back()->trace = &trace;
                assert(runtime.register_component(*comps.back()));
            }
            for (const auto& e : edges) assert(runtime.add_dependency(ComponentId{e.first}, ComponentId{e.second}));
            const auto order = runtime.component_order();
            assert(order.has_value());
            // independent prediction
            ComponentId expected_failing{};
            std::size_t expected_calls = 0;
            for (const ComponentId id : order.value()) {
                ++expected_calls;
                const bool met = std::all_of(required[id.value()].begin(), required[id.value()].end(), [&](std::uint64_t c) { return adapter.provided.contains(CapabilityId{c}); });
                if (!met) { expected_failing = id; break; }
            }
            const auto result = runtime.initialize();
            transcript += std::to_string(static_cast<int>(runtime.state())) + "|" + (result ? "ok" : std::to_string(result.error().source.value()) + "/" + std::to_string(static_cast<int>(result.error().code)));
            for (const std::string& t : trace) transcript += "," + t;
            if (expected_failing.valid()) {
                assert(!result && result.error().code == ErrorCode::UNSUPPORTED && result.error().source == expected_failing);
                assert(runtime.state() == LifecycleState::FAULT && trace.size() == expected_calls);   // fail-fast: exactly the components up to the failing one were invoked
                ++failures;
                if (expected_calls < order.value().size() && expected_calls > 1) ++mid_failures;
                assert(runtime.reset() && runtime.state() == LifecycleState::STOPPED && runtime.fault_error() == nullptr);   // and only an explicit reset recovers
            } else {
                assert(result && runtime.state() == LifecycleState::READY && trace.size() == n);
                ++successes;
            }
            for (const auto& comp : comps) assert(kStates.count(comp->lifecycle_state()) == 1);
            assert(runtime.statistics().retry_count.value() == 0);
        };
        std::string first, second;
        run(seed, first);
        run(seed, second);
        assert(first == second);                                                     // repeated runs are deterministic
    }
    assert(failures > 40 && successes > 40 && mid_failures > 10);                    // the seeds really exercised failures, successes and mid-order failures
}

void test_contexts_stay_unchanged_and_the_runtime_does_not_hold_them() {
    ProvisionAdapter adapter;
    provide(adapter, {1});
    ConsumerComponent a(make_info(1), adapter, {1});
    const ComponentContext copy = a.context();
    RuntimeManager runtime;
    assert(runtime.register_component(a) && runtime.attach_platform(adapter));
    assert(runtime.configure(Configuration{}) && runtime.initialize() && runtime.start() && runtime.stop() && runtime.shutdown());
    assert(a.context().bound() && a.context().id() == ComponentId{1} && a.context().info() == copy.info());      // immutable, never created, passed or probed by the Runtime
    assert(a.context().platform().attached() && copy.platform().attached());
}

} // namespace

int main() {
    test_a_missing_prerequisite_is_surfaced_by_the_component_without_a_new_state();
    test_nothing_retries_or_recovers_because_a_capability_appears_later();
    test_a_successful_check_changes_nothing_outside_the_lifecycle_operation();
    test_health_never_changes_the_lifecycle_and_capabilities_never_change_health();
    test_the_runtime_never_takes_a_capability_snapshot_or_evaluates_anything();
    test_capability_presence_never_changes_the_dependency_order_or_the_initialization_trace();
    test_a_seeded_model_predicts_every_outcome_and_runs_are_deterministic();
    test_contexts_stay_unchanged_and_the_runtime_does_not_hold_them();
    std::puts("capability readiness integration: ok");
    return 0;
}
