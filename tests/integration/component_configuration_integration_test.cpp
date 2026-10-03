//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component_configuration_integration_test.cpp
// Description : Runtime and Component configuration stay within the frozen contract (public APIs only).
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-CFG-009, CORE-CFG-010
// API         : CORE-API-CONFIGURATION
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

// Public APIs only: RuntimeManager, Component, Configuration, ComponentContext, the test-only reference platform and
// the test-only configuration harness. No private detail of the Runtime is touched.
//
// The central proof is that RuntimeManager::configure() only FORWARDS: seeded scenarios with random topologies,
// registration orders, injected configuration failures and explicit resets are replayed with plain reference components
// and with configuration-aware components, and must give identical results, states, faults, statistics and invocation
// traces; and for every configure step the shared call log shows the caller's own object reaching the components in
// dependency order, exactly once, stopping at the first failure, with no retry and no rollback.

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include <kritva/core/core.hpp>

#include "../platform/reference_platform.hpp"
#include "../runtime/configuration_conformance.hpp"
#include "runtime_scenarios.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;
using namespace kritva::core::runtime::contract;
using kritva::core::platform::testing::ReferencePlatform;
namespace sc = kritva::core::runtime::scenarios;
namespace conf = kritva::core::runtime::conformance;

namespace {

ComponentInfo make_info(std::uint64_t id) {
    auto info = ComponentInfo::create(ComponentId{id}, "c");
    assert(info.has_value());
    return std::move(info).value();
}

// A configuration-aware component that also counts every Status/Health read, so the Runtime's behavior can be observed.
class WatchedComponent final : public ReferenceConfigurableComponent {
public:
    using ReferenceConfigurableComponent::ReferenceConfigurableComponent;
    [[nodiscard]] Status status() const override { ++status_reads; return ReferenceConfigurableComponent::status(); }
    [[nodiscard]] Health health() const override { ++health_reads; return ReferenceConfigurableComponent::health(); }
    mutable int status_reads{0}, health_reads{0};
};

// The same scenario runner as the Runtime scenarios, except that a configure step passes a real configuration.
std::string apply_and_observe(sc::World& world, const sc::Step& step, const Configuration& configuration, Result<void>& out) {
    switch (step.op) {
        case 0: out = world.runtime.configure(configuration); break;
        case 1: out = world.runtime.initialize(); break;
        case 2: out = world.runtime.start(); break;
        case 3: out = world.runtime.stop(); break;
        case 4: out = world.runtime.shutdown(); break;
        default: out = world.runtime.reset(); break;
    }
    return sc::observe(world, step.op, out);
}

struct Coverage { int configure_steps{0}, refused{0}, forwarded{0}, failed_forwarding{0}, failing_mid_sequence{0}, succeeded{0}; };
Coverage g_coverage;

struct Replay {
    sc::Trace transcript;
    ConfigurationCallLog log;
    std::vector<WatchedComponent*> watched;
};

// Replays `script`; with `operational` the components are configuration-aware and per-step contract facts are asserted.
Replay replay(const sc::Script& script, bool configuration_aware) {
    Replay out;
    sc::World::Factory factory;
    if (configuration_aware) {
        factory = [&](ComponentInfo info) -> std::unique_ptr<ReferenceComponent> {
            auto c = std::make_unique<WatchedComponent>(std::move(info), Defect::NONE, &out.log);
            out.watched.push_back(c.get());
            return c;
        };
    }
    sc::World world(script, nullptr, factory);
    int configure_round = 0;
    for (std::size_t i = 0; i < script.steps.size(); ++i) {
        const sc::Step& step = script.steps[i];
        sc::inject(world, step);
        const Configuration given = reference_valid(configure_round++ % 2);
        const LifecycleState before = world.runtime.state();
        const bool had_fault = world.runtime.fault_error() != nullptr;
        out.log.clear();
        std::vector<std::string> applied_before;
        if (configuration_aware) for (const auto& c : world.components) applied_before.push_back(static_cast<ReferenceConfigurableComponent&>(*c).applied());
        std::optional<std::vector<ComponentId>> order;
        if (configuration_aware && step.op == 0) { if (auto o = world.runtime.component_order()) order = o.value(); }
        Result<void> result = Result<void>::success();
        out.transcript.push_back(apply_and_observe(world, step, given, result));
        assert(world.runtime.statistics().retry_count.value() == 0);                                 // the Runtime never retries anything

        if (configuration_aware && step.op == 0) {
            const bool eligible = before == LifecycleState::UNKNOWN || before == LifecycleState::STOPPED;
            assert(world.runtime.state() == before);                                              // configure never changes the Runtime state ...
            assert((world.runtime.fault_error() != nullptr) == had_fault);                        // ... and never enters or leaves FAULT
            ++g_coverage.configure_steps;
            if (!eligible) {
                ++g_coverage.refused;
                assert(!result && result.error().code == ErrorCode::INVALID_STATE && out.log.empty());   // refused before any component is invoked
            } else {
                ++g_coverage.forwarded;
                assert(order.has_value());
                std::vector<std::uint64_t> expected;
                for (const ComponentId id : *order) expected.push_back(id.value());
                const bool injected = step.inject_op % 5 == 0 && step.inject_id != 0 && step.code != ErrorCode::NONE;
                const std::uint64_t failing = injected ? step.inject_id : 0;
                if (failing != 0) { ++g_coverage.failed_forwarding; if (failing != expected.back() && failing != expected.front()) ++g_coverage.failing_mid_sequence; } else ++g_coverage.succeeded;
                assert(conf::check_runtime_forwarding(out.log, given, conf::ForwardingExpectation{expected, failing}, static_cast<bool>(result)).empty());
                // no rollback: components before the failing one accepted the new configuration, the others kept what they had
                bool reached_failure = false;
                for (std::size_t k = 0; k < expected.size(); ++k) {
                    const auto& comp = static_cast<ReferenceConfigurableComponent&>(*world.components[expected[k] - 1]);
                    if (expected[k] == failing) reached_failure = true;                                // the failing component and those after it
                    if (!reached_failure) assert(comp.applied() == ReferenceConfigurableComponent::fingerprint({{"rate", given.get("rate")->value}, {"name", given.get("name")->value}}));
                    else assert(comp.applied() == applied_before[expected[k] - 1]);
                }
                if (failing != 0) {                                                               // the failing component kept its previous state
                    assert(static_cast<ReferenceConfigurableComponent&>(*world.components[failing - 1]).applied() == applied_before[failing - 1]);
                    assert(!result && result.error().source == ComponentId{failing});
                }
            }
        }
    }
    out.transcript.insert(out.transcript.end(), world.trace.begin(), world.trace.end());
    for (WatchedComponent* c : out.watched) assert(c->status_reads == 0 && c->health_reads == 0);   // the Runtime never read Status or Health
    return out;
}

sc::Script random_configuration_script(std::mt19937& rng) {
    sc::Script script = sc::random_script(rng, 40);
    for (sc::Step& step : script.steps) {
        if (rng() % 3 == 0) step.op = 0;                           // plenty of configure steps
        if (step.op == 0 && rng() % 2 == 0) { step.inject_id = 1 + rng() % script.n; step.inject_op = 0; step.code = rng() % 2 == 0 ? ErrorCode::CONFIGURATION_ERROR : ErrorCode::RESOURCE_UNAVAILABLE; }
    }
    return script;
}

void test_seeded_differential_and_per_step_forwarding_facts() {
    for (unsigned seed = 1; seed <= 200; ++seed) {
        std::mt19937 rng(seed);
        const sc::Script script = random_configuration_script(rng);
        const Replay plain = replay(script, /*configuration_aware=*/false);
        const Replay aware = replay(script, /*configuration_aware=*/true);
        assert(plain.transcript == aware.transcript);                // identical results, states, topology, faults, statistics and invocation traces
    }
    // the seeds really exercised every case (this guards against a vacuous differential)
    assert(g_coverage.configure_steps > 1000 && g_coverage.refused > 100 && g_coverage.forwarded > 500);
    assert(g_coverage.failed_forwarding > 150 && g_coverage.failing_mid_sequence > 30 && g_coverage.succeeded > 150);
}

// ---- targeted scenarios ------------------------------------------------------------------------------------
struct Chain {
    ConfigurationCallLog log;
    std::unique_ptr<WatchedComponent> c[4];
    RuntimeManager runtime;
    Chain() {
        for (std::uint64_t id : {3u, 1u, 4u, 2u}) { c[id - 1] = std::make_unique<WatchedComponent>(make_info(id), Defect::NONE, &log); assert(runtime.register_component(*c[id - 1])); }
        assert(runtime.add_dependency(ComponentId{2}, ComponentId{1}) && runtime.add_dependency(ComponentId{3}, ComponentId{2}) && runtime.add_dependency(ComponentId{4}, ComponentId{3}));
    }
};

void test_the_first_failure_stops_the_sequence_and_nothing_is_rolled_back_or_retried() {
    Chain w;
    w.c[2 - 1]->fail_next_configure = ErrorCode::CONFIGURATION_ERROR;
    const Configuration given = reference_valid(0);
    const auto r = w.runtime.configure(given);
    assert(!r && r.error().code == ErrorCode::CONFIGURATION_ERROR && r.error().source == ComponentId{2} && r.error().message == "configure failed");   // the component's own Error, unchanged
    assert(w.log.size() == 2 && w.log[0].component == 1 && w.log[0].accepted && w.log[1].component == 2 && !w.log[1].accepted);   // 3 and 4 never called; 2 called once
    assert(w.c[0]->applied() == "name=alpha;rate=10;");                                // component 1 accepted and is NOT rolled back
    assert(w.c[1]->applied().empty() && w.c[2]->applied().empty() && w.c[3]->applied().empty());
    assert(w.runtime.state() == LifecycleState::UNKNOWN && w.runtime.fault_error() == nullptr);   // not a Runtime FAULT
    assert(w.runtime.statistics().error_count.value() == 1 && w.runtime.statistics().sample_count.value() == 1);   // the Runtime counts its calls only
    w.log.clear();
    assert(w.runtime.configure(given));                                                // an explicit new attempt; the Runtime never retried by itself
    assert(w.log.size() == 4 && w.c[3]->applied() == "name=alpha;rate=10;");
}

void test_the_same_logical_configuration_reaches_every_component_in_order_after_the_topology_is_fixed() {
    Chain w;
    const Configuration first = reference_valid(0);
    assert(w.runtime.configure(first) && w.runtime.initialize() && w.runtime.start());
    const Configuration refused = reference_valid(1);
    w.log.clear();
    const auto r = w.runtime.configure(refused);
    assert(!r && r.error().code == ErrorCode::INVALID_STATE && w.log.empty());         // RUNNING: refused by the Runtime, no component invoked
    assert(w.runtime.stop());
    const Configuration second = reference_valid(1);
    assert(w.runtime.configure(second));
    std::vector<std::uint64_t> seen;
    for (const ConfigurationCall& call : w.log) if (call.object == &second) seen.push_back(call.component);
    assert((seen == std::vector<std::uint64_t>{1, 2, 3, 4}));
    for (const auto& comp : w.c) assert(comp->applied() == "name=beta;rate=20;");
    assert(w.runtime.state() == LifecycleState::STOPPED);
}

void test_configuration_is_independent_of_fault_status_and_health() {
    Chain w;
    w.c[3 - 1]->fail_next_start = ErrorCode::INTERNAL_ERROR;
    assert(w.runtime.configure(reference_valid(0)) && w.runtime.initialize());
    assert(!w.runtime.start() && w.runtime.state() == LifecycleState::FAULT);
    const auto in_fault = w.runtime.configure(reference_valid(1));                     // FAULT: configure is not an exit from it
    assert(!in_fault && in_fault.error().code == ErrorCode::INVALID_STATE && w.runtime.state() == LifecycleState::FAULT);
    assert(w.runtime.reset() && w.runtime.state() == LifecycleState::STOPPED);        // only the explicit reset
    w.c[2 - 1]->fail_next_configure = ErrorCode::CONFIGURATION_ERROR;                  // a configuration failure after a fault and a reset
    assert(!w.runtime.configure(reference_valid(1)) && w.runtime.state() == LifecycleState::STOPPED && w.runtime.fault_error() == nullptr);
    for (const auto& comp : w.c) assert(comp->status_reads == 0 && comp->health_reads == 0);   // nothing consulted Status or Health at any point
}

void test_component_context_and_the_platform_are_not_touched_by_configuration() {
    ReferencePlatform platform;
    Chain w;
    assert(w.runtime.attach_platform(platform));
    const ComponentContext before(w.c[0]->info(), platform::PlatformContext(platform));     // an integrator-built context, built beforehand
    const ComponentContext copy = before;
    assert(w.runtime.configure(reference_valid(0)) && w.runtime.initialize() && w.runtime.start() && w.runtime.stop() && w.runtime.configure(reference_valid(1)));
    assert(before.bound() && before.id() == ComponentId{1} && before.platform().attached());   // unchanged: immutable, and the Runtime never held it
    assert(copy.id() == before.id() && copy.info() == before.info());
    assert(platform.controls().adapter_queries() == 0 && platform.controls().log().empty());   // no adapter query or service call: no platform lifecycle
}

void test_the_runtime_forwards_the_callers_own_object_and_keeps_nothing() {
    Chain w;
    auto owned = std::make_unique<Configuration>(reference_valid(0));
    assert(w.runtime.configure(*owned));
    for (const ConfigurationCall& call : w.log) assert(call.object == owned.get());
    owned.reset();                                                                      // the caller destroys it: the Runtime held nothing
    assert(w.runtime.initialize() && w.runtime.start() && w.runtime.stop() && w.runtime.shutdown());
    for (const auto& comp : w.c) assert(comp->applied() == "name=alpha;rate=10;");    // each component copied what it needed
}

} // namespace

int main() {
    test_seeded_differential_and_per_step_forwarding_facts();
    test_the_first_failure_stops_the_sequence_and_nothing_is_rolled_back_or_retried();
    test_the_same_logical_configuration_reaches_every_component_in_order_after_the_topology_is_fixed();
    test_configuration_is_independent_of_fault_status_and_health();
    test_component_context_and_the_platform_are_not_touched_by_configuration();
    test_the_runtime_forwards_the_callers_own_object_and_keeps_nothing();
    std::puts("component configuration integration: ok");
    return 0;
}
