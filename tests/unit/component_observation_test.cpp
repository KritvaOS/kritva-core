//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component_observation_test.cpp
// Description : Contract tests for ComponentObservation and observe().
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-OPS-001, CORE-OPS-006
// API         : CORE-API-RUNTIME
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#include <cassert>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <kritva/core/core.hpp>

using namespace kritva::core;
using namespace kritva::core::runtime;

namespace {

// A component that logs every accessor call, so the order and count are observable.
class SpyComponent final : public Component {
public:
    explicit SpyComponent(ComponentInfo info) : Component(std::move(info)) {}
    Result<void> configure(const Configuration&) override { ++lifecycle_calls; return Result<void>::success(); }
    Result<void> initialize() override { ++lifecycle_calls; return Result<void>::success(); }
    Result<void> start() override { ++lifecycle_calls; return Result<void>::success(); }
    Result<void> stop() override { ++lifecycle_calls; return Result<void>::success(); }
    Result<void> shutdown() override { ++lifecycle_calls; return Result<void>::success(); }
    LifecycleState lifecycle_state() const noexcept override { log.push_back("lifecycle"); return state; }
    Status status() const override { log.push_back("status"); return status_value; }
    Health health() const override { log.push_back("health"); return health_value; }
    CapabilitySet capabilities() const override { log.push_back("capabilities"); return CapabilitySet{}; }

    LifecycleState state{LifecycleState::UNKNOWN};
    Status status_value{};
    Health health_value{};
    mutable std::vector<std::string> log;
    int lifecycle_calls{0};
};

class SpyStatistics final : public IComponentStatistics {
public:
    explicit SpyStatistics(std::vector<std::string>* shared = nullptr) : shared_(shared) {}
    Statistics statistics() const override {
        ++calls;
        if (shared_ != nullptr) shared_->push_back("statistics");
        return value;
    }
    Statistics value{};
    mutable int calls{0};
private:
    std::vector<std::string>* shared_;
};

ComponentInfo make_info(std::uint64_t id, const char* name = "obs") {
    auto info = ComponentInfo::create(ComponentId{id}, name);
    assert(info);
    return std::move(info).value();
}

void test_shape() {
    static_assert(std::is_copy_constructible_v<ComponentObservation>);
    static_assert(std::is_copy_assignable_v<ComponentObservation>);
    static_assert(std::is_default_constructible_v<ComponentObservation>);
    static_assert(std::is_aggregate_v<ComponentObservation>);
    static_assert(std::is_abstract_v<IComponentStatistics>);
    static_assert(std::has_virtual_destructor_v<IComponentStatistics>);
    // Statistics are optional and separate: they are not part of the Component interface.
    static_assert(!std::is_base_of_v<IComponentStatistics, Component>);
    static_assert(!std::is_base_of_v<Component, IComponentStatistics>);
    // observe() takes a const Component (it cannot call a mutating operation) and an optional provider.
    static_assert(std::is_same_v<decltype(observe(std::declval<const Component&>())), ComponentObservation>);
    static_assert(std::is_same_v<decltype(observe(std::declval<const Component&>(), std::declval<const IComponentStatistics*>())), ComponentObservation>);
}

void test_default_observation() {
    const ComponentObservation o;
    assert(!o.id.valid());
    assert(o.lifecycle == LifecycleState::UNKNOWN);
    assert(o.status.code() == StatusCode::UNKNOWN && o.status.message().empty());
    assert(o.health.state() == HealthState::UNKNOWN && o.health.detail().empty());
    assert(!o.statistics.has_value());
}

void test_observe_reports_the_component_values() {
    SpyComponent c(make_info(7));
    c.state = LifecycleState::RUNNING;
    c.status_value = Status(StatusCode::OK);
    c.status_value.set_message("all good");
    c.health_value = Health(HealthState::DEGRADED);
    c.health_value.set_detail("slow disk");

    const ComponentObservation o = observe(c);
    assert(o.id == ComponentId{7});
    assert(o.lifecycle == LifecycleState::RUNNING);
    assert(o.status.code() == StatusCode::OK && o.status.message() == "all good");
    assert(o.health.state() == HealthState::DEGRADED && o.health.detail() == "slow disk");
}

void test_accessor_order_and_count_are_exactly_as_documented() {
    SpyComponent c(make_info(1));
    std::vector<std::string> expected{"lifecycle", "status", "health"};
    (void)observe(c);
    assert(c.log == expected);                               // once each, in order, nothing else (no capabilities)

    c.log.clear();
    SpyStatistics stats(&c.log);
    (void)observe(c, &stats);
    expected.push_back("statistics");
    assert(c.log == expected);                               // the provider is read last, once
    assert(stats.calls == 1);
}

void test_statistics_semantics() {
    SpyComponent c(make_info(2));
    assert(!observe(c).statistics.has_value());              // null provider -> nullopt
    assert(!observe(c, nullptr).statistics.has_value());

    SpyStatistics stats;                                     // an all-zero Statistics is still engaged
    const ComponentObservation empty = observe(c, &stats);
    assert(empty.statistics.has_value());
    assert(empty.statistics->sample_count.value() == 0 && empty.statistics->queue_depth.value() == 0);

    stats.value.sample_count.increment(5);
    stats.value.error_count.increment(2);
    stats.value.queue_depth.set(-3);
    const ComponentObservation full = observe(c, &stats);
    assert(full.statistics.has_value());
    assert(full.statistics->sample_count.value() == 5);
    assert(full.statistics->error_count.value() == 2);
    assert(full.statistics->queue_depth.value() == -3);
    assert(full.statistics->drop_count.value() == 0);
}

void test_observation_is_detached_from_the_component() {
    SpyComponent c(make_info(3));
    c.status_value = Status(StatusCode::OK);
    c.status_value.set_message("before");
    c.health_value = Health(HealthState::HEALTHY);
    c.state = LifecycleState::READY;
    SpyStatistics stats;
    stats.value.sample_count.increment(1);

    const ComponentObservation before = observe(c, &stats);

    c.status_value = Status(StatusCode::NOT_READY);              // the component moves on ...
    c.status_value.set_message("after");
    c.health_value = Health(HealthState::UNHEALTHY);
    c.state = LifecycleState::FAULT;
    stats.value.sample_count.increment(100);

    assert(before.status.code() == StatusCode::OK && before.status.message() == "before");   // ... the value does not
    assert(before.health.state() == HealthState::HEALTHY);
    assert(before.lifecycle == LifecycleState::READY);
    assert(before.statistics->sample_count.value() == 1);

    const ComponentObservation after = observe(c, &stats);   // a fresh observation sees the new truth: no cache
    assert(after.status.message() == "after" && after.health.state() == HealthState::UNHEALTHY);
    assert(after.lifecycle == LifecycleState::FAULT && after.statistics->sample_count.value() == 101);
}

void test_observation_outlives_the_component() {
    ComponentObservation kept;
    {
        SpyComponent c(make_info(4));
        c.status_value.set_message(std::string(200, 'x'));   // longer than any small-string buffer
        c.health_value.set_detail(std::string(200, 'y'));
        kept = observe(c);
    }                                                        // component destroyed
    assert(kept.id == ComponentId{4});
    assert(kept.status.message() == std::string(200, 'x'));  // owned by the observation (ASan checks the rest)
    assert(kept.health.detail() == std::string(200, 'y'));
}

void test_observation_has_no_effect_on_the_component() {
    SpyComponent c(make_info(5));
    c.state = LifecycleState::RUNNING;
    SpyStatistics stats;
    for (int i = 0; i < 10; ++i) (void)observe(c, &stats);
    assert(c.lifecycle_calls == 0);                          // never a lifecycle operation
    assert(c.state == LifecycleState::RUNNING);
    assert(c.status_value.code() == StatusCode::UNKNOWN && c.health_value.state() == HealthState::UNKNOWN);
    assert(stats.value.sample_count.value() == 0 && stats.calls == 10);
}

void test_deterministic_and_repeatable() {
    SpyComponent c(make_info(6));
    c.state = LifecycleState::READY;
    c.status_value = Status(StatusCode::OK);
    c.health_value = Health(HealthState::HEALTHY);
    SpyStatistics stats;
    stats.value.drop_count.increment(4);
    const ComponentObservation a = observe(c, &stats);
    const ComponentObservation b = observe(c, &stats);
    assert(a.id == b.id && a.lifecycle == b.lifecycle);
    assert(a.status.code() == b.status.code() && a.status.message() == b.status.message());
    assert(a.health.state() == b.health.state() && a.health.detail() == b.health.detail());
    assert(a.statistics->drop_count.value() == b.statistics->drop_count.value());
}

void test_provider_is_not_tied_to_the_component() {
    SpyComponent one(make_info(8)), two(make_info(9));
    SpyStatistics stats;
    stats.value.sample_count.increment(3);
    // The interface carries no identity: the caller pairs them and Core reports what it is given.
    assert(observe(one, &stats).statistics->sample_count.value() == 3);
    assert(observe(two, &stats).statistics->sample_count.value() == 3);
    assert(observe(one, &stats).id == ComponentId{8} && observe(two, &stats).id == ComponentId{9});
}

void test_observation_works_through_the_base_reference_and_a_runtime_registered_component() {
    SpyComponent c(make_info(10));
    RuntimeManager runtime;
    assert(runtime.register_component(c));
    assert(runtime.initialize());
    const LifecycleState before = runtime.state();
    const auto errors = runtime.statistics().error_count.value();
    const auto samples = runtime.statistics().sample_count.value();
    const ComponentObservation o = observe(static_cast<const Component&>(c));
    assert(o.id == ComponentId{10});
    assert(runtime.state() == before);                       // observing never touches the Runtime
    assert(runtime.statistics().error_count.value() == errors && runtime.statistics().sample_count.value() == samples);
}

} // namespace

int main() {
    test_shape();
    test_default_observation();
    test_observe_reports_the_component_values();
    test_accessor_order_and_count_are_exactly_as_documented();
    test_statistics_semantics();
    test_observation_is_detached_from_the_component();
    test_observation_outlives_the_component();
    test_observation_has_no_effect_on_the_component();
    test_deterministic_and_repeatable();
    test_provider_is_not_tied_to_the_component();
    test_observation_works_through_the_base_reference_and_a_runtime_registered_component();
    return 0;
}
