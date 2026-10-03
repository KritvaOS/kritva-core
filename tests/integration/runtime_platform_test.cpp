//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : runtime_platform_test.cpp
// Description : Runtime-platform integration boundary: optional attachment of a platform
//               adapter changes nothing observable in the frozen R0.3 Runtime.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-010
// API         : CORE-TEST-RUNTIME-PLATFORM
//
// Author      : KritvaOS Core Team
// Created     : 04-10-2026
//==============================================================================

// Public APIs only. The central proof is differential: the same seeded scenarios run on two
// RuntimeManagers, one without and one with an attached platform adapter that records every
// call made to it, and every observable result must be identical while the adapter and its
// services record no call at all.

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <random>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <kritva/core/core.hpp>

#include "../contract/reference_adapter.hpp"
#include "../contract/reference_component.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;
using kritva::core::platform::contract::ReferenceAdapter;
using kritva::core::platform::contract::ReferenceScheduler;
using kritva::core::platform::contract::ReferenceWatchdog;
using kritva::core::runtime::contract::ReferenceComponent;
using kritva::core::time::contract::ReferenceTimer;

namespace {

using Ids = std::vector<std::uint64_t>;
using Edge = std::pair<std::uint64_t, std::uint64_t>;   // (dependent, dependency)
using Trace = std::vector<std::string>;

// An adapter that records every call made to any member, so "the Runtime never touches the
// platform" is observable.
class SpyAdapter final : public ReferenceAdapter {
public:
    SpyAdapter() : ReferenceAdapter(platform::PlatformInfo{"spy", Version{1, 0, 0}}, {}) {}
    ~SpyAdapter() override { if (destroyed != nullptr) *destroyed = true; }
    [[nodiscard]] const platform::PlatformInfo& info() const noexcept override { ++member_calls; return ReferenceAdapter::info(); }
    [[nodiscard]] platform::IScheduler* scheduler() const noexcept override { ++member_calls; return ReferenceAdapter::scheduler(); }
    [[nodiscard]] time::IClock* clock() const noexcept override { ++member_calls; return ReferenceAdapter::clock(); }
    [[nodiscard]] time::ITimer* timer() const noexcept override { ++member_calls; return ReferenceAdapter::timer(); }
    [[nodiscard]] platform::IWatchdog* watchdog() const noexcept override { ++member_calls; return ReferenceAdapter::watchdog(); }
    [[nodiscard]] CapabilitySet capabilities() const override { ++member_calls; return ReferenceAdapter::capabilities(); }

    // Inspection that does not count as a call made by the Runtime.
    int calls() const noexcept { return member_calls; }
    ReferenceScheduler& sched() const { return *static_cast<ReferenceScheduler*>(ReferenceAdapter::scheduler()); }
    ReferenceTimer& tim() const { return *static_cast<ReferenceTimer*>(ReferenceAdapter::timer()); }
    ReferenceWatchdog& dog() const { return *static_cast<ReferenceWatchdog*>(ReferenceAdapter::watchdog()); }
    int service_calls() const {
        return sched().create_calls + sched().start_calls + sched().stop_calls + tim().start_calls + tim().stop_calls
             + dog().start_calls + dog().kick_calls + dog().stop_calls;
    }

    bool* destroyed{nullptr};           // set by the lifetime test to observe destruction
    mutable int member_calls{0};
};

// ---- the compile-time boundary --------------------------------------------------------------

void test_public_api_compatibility_and_boundary() {
    // Frozen R0.3 members keep their exact signatures.
    static_assert(std::is_same_v<decltype(&RuntimeManager::initialize), Result<void> (RuntimeManager::*)()>);
    static_assert(std::is_same_v<decltype(&RuntimeManager::start), Result<void> (RuntimeManager::*)()>);
    static_assert(std::is_same_v<decltype(&RuntimeManager::stop), Result<void> (RuntimeManager::*)()>);
    static_assert(std::is_same_v<decltype(&RuntimeManager::shutdown), Result<void> (RuntimeManager::*)()>);
    static_assert(std::is_same_v<decltype(&RuntimeManager::reset), Result<void> (RuntimeManager::*)()>);
    static_assert(std::is_same_v<decltype(&RuntimeManager::configure), Result<void> (RuntimeManager::*)(const Configuration&)>);
    static_assert(std::is_same_v<decltype(&RuntimeManager::register_component), Result<void> (RuntimeManager::*)(Component&)>);
    static_assert(std::is_same_v<decltype(&RuntimeManager::add_dependency), Result<void> (RuntimeManager::*)(ComponentId, ComponentId)>);
    static_assert(std::is_same_v<decltype(&RuntimeManager::state), LifecycleState (RuntimeManager::*)() const noexcept>);
    static_assert(std::is_same_v<decltype(&RuntimeManager::fault_error), const Error* (RuntimeManager::*)() const noexcept>);
    static_assert(std::is_same_v<decltype(&RuntimeManager::statistics), const Statistics& (RuntimeManager::*)() const noexcept>);
    static_assert(std::is_base_of_v<Runtime, RuntimeManager> && std::is_final_v<RuntimeManager>);
    static_assert(!std::is_copy_constructible_v<RuntimeManager> && !std::is_move_constructible_v<RuntimeManager>);

    // The additive extension: non-owning, no probing, no shared pointers.
    static_assert(std::is_same_v<decltype(&RuntimeManager::attach_platform), Result<void> (RuntimeManager::*)(platform::IPlatformAdapter&)>);
    static_assert(std::is_same_v<decltype(&RuntimeManager::platform), platform::IPlatformAdapter* (RuntimeManager::*)() const noexcept>);
    static_assert(noexcept(std::declval<const RuntimeManager&>().platform()));
}

// ---- attachment semantics ----------------------------------------------------------------------


void test_attach_is_optional_setup_only_and_never_replaces() {
    RuntimeManager runtime;
    assert(runtime.platform() == nullptr);                    // optional: nothing attached by default
    SpyAdapter first, second;
    assert(runtime.attach_platform(first));
    assert(runtime.platform() == &first);
    const auto again = runtime.attach_platform(second);       // never silently replaced
    assert(!again && again.error().code == ErrorCode::INVALID_STATE);
    assert(runtime.platform() == &first);
    const auto same = runtime.attach_platform(first);         // not even the same adapter twice
    assert(!same && same.error().code == ErrorCode::INVALID_STATE);

    auto info = ComponentInfo::create(ComponentId{1}, "c");
    assert(info);
    ReferenceComponent component(std::move(info).value());
    assert(runtime.register_component(component));
    assert(runtime.initialize());                             // fixes the topology
    const auto late = runtime.attach_platform(second);
    assert(!late && late.error().code == ErrorCode::INVALID_STATE);
    assert(runtime.platform() == &first);                     // unchanged
    assert(runtime.stop() && runtime.shutdown());
    const auto after_stop = runtime.attach_platform(second);  // closed for good, like the topology
    assert(!after_stop && after_stop.error().code == ErrorCode::INVALID_STATE);
    assert(first.calls() == 0 && second.calls() == 0);
}

void test_attach_after_closed_topology_with_no_adapter_fails_and_attach_after_failed_initialize_works() {
    {
        RuntimeManager runtime;
        auto info = ComponentInfo::create(ComponentId{1}, "c");
        ReferenceComponent component(std::move(info).value());
        assert(runtime.register_component(component) && runtime.initialize());
        SpyAdapter adapter;
        const auto late = runtime.attach_platform(adapter);
        assert(!late && late.error().code == ErrorCode::INVALID_STATE);
        assert(runtime.platform() == nullptr);                // a failed attach leaves nothing attached
    }
    {
        // An invalid topology makes initialize() fail without fixing it: setup, and so attachment, remains open.
        RuntimeManager runtime;
        auto info = ComponentInfo::create(ComponentId{1}, "c");
        ReferenceComponent component(std::move(info).value());
        assert(runtime.register_component(component));
        assert(runtime.add_dependency(ComponentId{1}, ComponentId{9}));   // 9 is not registered
        assert(!runtime.initialize() && !runtime.topology_fixed());
        SpyAdapter adapter;
        assert(runtime.attach_platform(adapter));
        assert(runtime.platform() == &adapter);
    }
}

void test_attach_does_not_probe_the_adapter() {
    RuntimeManager runtime;
    SpyAdapter adapter;
    assert(runtime.attach_platform(adapter));
    assert(adapter.calls() == 0);                             // no info(), supports(), accessor or capabilities() call
    assert(runtime.platform() == &adapter && adapter.calls() == 0);   // platform() itself returns only the pointer
    assert(adapter.service_calls() == 0);
}

void test_runtime_never_owns_the_adapter() {
    bool destroyed = false;
    auto adapter = std::make_unique<SpyAdapter>();
    adapter->destroyed = &destroyed;
    {
        RuntimeManager runtime;
        auto info = ComponentInfo::create(ComponentId{1}, "c");
        ReferenceComponent component(std::move(info).value());
        assert(runtime.register_component(component));
        assert(runtime.attach_platform(*adapter));
        assert(runtime.initialize() && runtime.start() && runtime.stop() && runtime.shutdown());
    }                                                          // the Runtime is destroyed here
    assert(!destroyed);                                        // it did not destroy the adapter
    assert(adapter->calls() == 0 && adapter->service_calls() == 0);
    adapter.reset();
    assert(destroyed);                                         // the integrator owns it
}

// ---- differential proof ----------------------------------------------------------------------------

struct Step {
    int op{0};                 // 0 configure 1 initialize 2 start 3 stop 4 shutdown 5 reset
    std::uint64_t inject_id{0};
    int inject_op{0};
    ErrorCode code{ErrorCode::NONE};
};

struct Script {
    std::size_t n{0};
    Ids registration;
    std::vector<Edge> edges;
    std::vector<Step> steps;
};

struct World {
    Trace trace;
    std::vector<std::unique_ptr<ReferenceComponent>> components;
    RuntimeManager runtime;
    std::unique_ptr<SpyAdapter> adapter;

    World(const Script& script, bool with_adapter) {
        components.resize(script.n);
        for (std::uint64_t id : script.registration) {
            auto info = ComponentInfo::create(ComponentId{id}, "c");
            components[id - 1] = std::make_unique<ReferenceComponent>(std::move(info).value());
            components[id - 1]->trace = &trace;
            assert(runtime.register_component(*components[id - 1]));
        }
        for (const Edge& e : script.edges) assert(runtime.add_dependency(ComponentId{e.first}, ComponentId{e.second}));
        if (with_adapter) {
            adapter = std::make_unique<SpyAdapter>();
            assert(runtime.attach_platform(*adapter));
        }
    }
};

std::string describe(const Result<void>& r) {
    if (r) return "ok";
    return std::to_string(static_cast<int>(r.error().code)) + "/" + std::to_string(static_cast<int>(r.error().severity)) + "/" +
           std::to_string(r.error().source.value()) + "/" + r.error().message;
}

// Runs the script and returns a transcript of everything observable through the Runtime.
Trace run(World& world, const Script& script) {
    Trace out;
    for (const Step& step : script.steps) {
        for (auto& c : world.components)
            c->fail_next_configure = c->fail_next_initialize = c->fail_next_start = c->fail_next_stop = c->fail_next_shutdown = ErrorCode::NONE;
        if (step.inject_id != 0 && step.code != ErrorCode::NONE) {
            ReferenceComponent& c = *world.components[step.inject_id - 1];
            ErrorCode* slot[] = {&c.fail_next_configure, &c.fail_next_initialize, &c.fail_next_start, &c.fail_next_stop, &c.fail_next_shutdown};
            *slot[step.inject_op % 5] = step.code;
        }
        Result<void> r = Result<void>::success();
        switch (step.op) {
            case 0: r = world.runtime.configure(Configuration{}); break;
            case 1: r = world.runtime.initialize(); break;
            case 2: r = world.runtime.start(); break;
            case 3: r = world.runtime.stop(); break;
            case 4: r = world.runtime.shutdown(); break;
            default: r = world.runtime.reset(); break;
        }
        const Statistics& st = world.runtime.statistics();
        const Error* fault = world.runtime.fault_error();
        out.push_back(std::to_string(step.op) + " -> " + describe(r) + " state=" + std::to_string(static_cast<int>(world.runtime.state())) +
                      " fixed=" + std::to_string(world.runtime.topology_fixed()) +
                      " fault=" + (fault ? std::to_string(static_cast<int>(fault->code)) + "/" + std::to_string(fault->source.value()) + "/" + fault->message : "none") +
                      " samples=" + std::to_string(st.sample_count.value()) + " errors=" + std::to_string(st.error_count.value()) +
                      " retries=" + std::to_string(st.retry_count.value()));
    }
    out.insert(out.end(), world.trace.begin(), world.trace.end());   // the component invocation trace
    return out;
}

Script random_script(std::mt19937& rng) {
    static const ErrorCode codes[] = {ErrorCode::TIMEOUT, ErrorCode::NOT_READY, ErrorCode::RESOURCE_UNAVAILABLE, ErrorCode::INTERNAL_ERROR};
    Script script;
    script.n = 1 + rng() % 7;
    Ids rank(script.n);
    for (std::size_t i = 0; i < script.n; ++i) rank[i] = i + 1;
    std::shuffle(rank.begin(), rank.end(), rng);
    for (std::size_t a = 0; a < script.n; ++a)
        for (std::size_t b = 0; b < script.n; ++b)
            if (rank[a] > rank[b] && rng() % 3 == 0) script.edges.push_back({a + 1, b + 1});
    std::shuffle(script.edges.begin(), script.edges.end(), rng);
    script.registration.resize(script.n);
    for (std::size_t i = 0; i < script.n; ++i) script.registration[i] = i + 1;
    std::shuffle(script.registration.begin(), script.registration.end(), rng);
    for (int i = 0; i < 40; ++i) {
        Step step;
        step.op = static_cast<int>(rng() % 6);
        if (rng() % 3 == 0) {
            step.inject_id = 1 + rng() % script.n;
            step.inject_op = static_cast<int>(rng() % 5);
            step.code = codes[rng() % 4];
        }
        script.steps.push_back(step);
    }
    return script;
}

void test_differential_with_and_without_adapter() {
    std::mt19937 rng(20261005);
    std::size_t operations = 0, failures = 0;
    for (int scenario = 0; scenario < 400; ++scenario) {
        const Script script = random_script(rng);
        World without(script, false);
        World with(script, true);
        const Trace a = run(without, script);
        const Trace b = run(with, script);
        assert(a == b);                                        // every result, state, fault, statistic and invocation identical
        assert(with.adapter->calls() == 0);                    // and the adapter was never called
        assert(with.adapter->service_calls() == 0);            // nor was any service started, stopped or kicked
        operations += script.steps.size();
        for (const std::string& line : a) if (line.find("-> ok") == std::string::npos && line.find("->") != std::string::npos) ++failures;
    }
    assert(operations == 400u * 40u);
    assert(failures > 400);                                    // the scenarios exercise many failure and invalid-state paths
}

void test_full_lifecycle_with_adapter_matches_the_documented_trace() {
    Script script;
    script.n = 3;
    script.registration = {3, 1, 2};
    script.edges = {{1, 2}, {2, 3}};                           // 1 depends on 2 depends on 3
    for (int op : {0, 1, 2, 3, 4}) script.steps.push_back(Step{op, 0, 0, ErrorCode::NONE});
    World with(script, true);
    const Trace t = run(with, script);
    const Trace expected_calls = {"3:configure", "2:configure", "1:configure", "3:initialize", "2:initialize", "1:initialize",
                                  "3:start", "2:start", "1:start", "1:stop", "2:stop", "3:stop", "1:shutdown", "2:shutdown", "3:shutdown"};
    assert(Trace(t.end() - 15, t.end()) == expected_calls);
    assert(with.runtime.state() == LifecycleState::STOPPED);
    assert(with.adapter->calls() == 0);
}

// ---- platform failures arrive only through components ----------------------------------------------

// An integrator-written component that uses the platform through the Runtime's adapter.
class PlatformUser final : public Component {
public:
    PlatformUser(ComponentInfo info, platform::IPlatformAdapter& adapter) : Component(std::move(info)), adapter_(adapter) {}
    Result<void> configure(const Configuration&) override { return Result<void>::success(); }
    Result<void> initialize() override { return Result<void>::success(); }
    Result<void> start() override {
        // The component, not the Runtime, calls the platform and wraps a platform failure as its own Error.
        const Result<void> r = adapter_.timer()->start(Duration::from_milliseconds(50), time::TimerMode::ONE_SHOT, Callback{&noop, nullptr});
        if (r) return r;
        Error e = r.error();
        e.source = info().id();
        return Result<void>::failure(e);
    }
    Result<void> stop() override { (void)adapter_.timer()->stop(); return Result<void>::success(); }
    Result<void> shutdown() override { return Result<void>::success(); }
    LifecycleState lifecycle_state() const noexcept override { return LifecycleState::UNKNOWN; }
    Status status() const override { return Status{}; }
    Health health() const override { return Health{}; }
    CapabilitySet capabilities() const override { return CapabilitySet{}; }
private:
    static void noop(void*) {}
    platform::IPlatformAdapter& adapter_;
};

void test_platform_failure_propagates_through_a_component_unchanged() {
    SpyAdapter adapter;
    adapter.tim().policy.resource_available = false;          // the platform cannot provide a timer
    RuntimeManager runtime;
    auto info = ComponentInfo::create(ComponentId{7}, "user");
    PlatformUser user(std::move(info).value(), adapter);
    assert(runtime.register_component(user));
    assert(runtime.attach_platform(adapter));
    assert(runtime.initialize());
    const auto started = runtime.start();
    assert(!started);
    assert(started.error().code == ErrorCode::RESOURCE_UNAVAILABLE);        // the platform's own code, unchanged
    assert(started.error().message == "no timer resource");               // and its message
    assert(started.error().source == ComponentId{7});                      // attributed to the component that returned it
    assert(runtime.state() == LifecycleState::FAULT);
    assert(runtime.fault_error() != nullptr && runtime.fault_error()->code == ErrorCode::RESOURCE_UNAVAILABLE);
    assert(!runtime.stop());                                               // FAULT accepts only reset()
    assert(runtime.reset() && runtime.state() == LifecycleState::STOPPED);
    // The platform recovers: a new explicit attempt works, and the Runtime never retried by itself.
    adapter.tim().policy.resource_available = true;
    assert(runtime.initialize() && runtime.start() && runtime.state() == LifecycleState::RUNNING);
    assert(runtime.stop() && runtime.shutdown());
    assert(adapter.calls() == 3);                                          // exactly the component's own timer() calls (start, start, stop); the Runtime made none
}

void test_watchdog_expiry_and_runtime_fault_are_independent() {
    SpyAdapter adapter;
    RuntimeManager runtime;
    auto info = ComponentInfo::create(ComponentId{1}, "c");
    ReferenceComponent component(std::move(info).value());
    assert(runtime.register_component(component) && runtime.attach_platform(adapter));
    assert(runtime.initialize() && runtime.start());

    // The integrator starts a watchdog through the adapter and never kicks it: it expires.
    platform::IWatchdog* watchdog = runtime.platform()->watchdog();
    assert(watchdog->start(Duration::from_milliseconds(10)));
    adapter.dog().advance(1000 * 1'000'000);
    assert(adapter.dog().expired());
    assert(runtime.state() == LifecycleState::RUNNING);                    // expiry never enters the Runtime
    assert(runtime.fault_error() == nullptr && runtime.statistics().error_count.value() == 0);
    assert(!runtime.reset());                                              // reset() remains explicit and FAULT-only

    // The reverse: a Runtime FAULT does not touch the watchdog.
    component.fail_next_stop = ErrorCode::TIMEOUT;
    assert(!runtime.stop() && runtime.state() == LifecycleState::FAULT);
    assert(adapter.dog().running() && adapter.dog().kick_calls == 0 && adapter.dog().stop_calls == 0);
    assert(runtime.reset() && adapter.dog().running());                    // reset() does not stop it either
}

void test_services_obtained_through_the_runtime_stay_adapter_owned() {
    SpyAdapter adapter;
    RuntimeManager runtime;
    assert(runtime.attach_platform(adapter));
    platform::IScheduler* scheduler = runtime.platform()->scheduler();
    assert(scheduler == static_cast<platform::IScheduler*>(&adapter.sched()));   // the adapter's own object, not a copy
    assert(runtime.platform()->clock()->now().domain() == ClockDomain::MONOTONIC);
    assert(runtime.platform()->supports(platform::PlatformService::TIMER));
    assert(runtime.state() == LifecycleState::UNKNOWN);                    // using the platform changes no Runtime state
    assert(runtime.statistics().sample_count.value() == 0);
}

} // namespace

int main() {
    test_public_api_compatibility_and_boundary();
    test_attach_is_optional_setup_only_and_never_replaces();
    test_attach_after_closed_topology_with_no_adapter_fails_and_attach_after_failed_initialize_works();
    test_attach_does_not_probe_the_adapter();
    test_runtime_never_owns_the_adapter();
    test_differential_with_and_without_adapter();
    test_full_lifecycle_with_adapter_matches_the_documented_trace();
    test_platform_failure_propagates_through_a_component_unchanged();
    test_watchdog_expiry_and_runtime_fault_are_independent();
    test_services_obtained_through_the_runtime_stay_adapter_owned();
    return 0;
}
