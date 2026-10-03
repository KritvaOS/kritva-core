//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component_statistics_test.cpp
// Description : Contract tests for the optional Component statistics provider.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-OPS-004
// API         : CORE-API-RUNTIME
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#include <cassert>
#include <cstdint>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>

#include <kritva/core/core.hpp>

#include "../runtime/statistics_conformance.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;
using kritva::core::runtime::conformance::check_statistics_provider;

namespace {

ComponentInfo make_info(std::uint64_t id) {
    auto info = ComponentInfo::create(ComponentId{id}, "st");
    assert(info);
    return std::move(info).value();
}

template <class T> concept HasStatistics = requires(const T t) { t.statistics(); };

// A component with no statistics at all: the common case.
class PlainComponent final : public Component {
public:
    explicit PlainComponent(ComponentInfo info) : Component(std::move(info)) {}
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

// A component that opts in by also implementing IComponentStatistics. Its own numbers, its own meaning.
class CountingComponent final : public Component, public IComponentStatistics {
public:
    explicit CountingComponent(ComponentInfo info) : Component(std::move(info)) {}
    Result<void> configure(const Configuration&) override { return Result<void>::success(); }
    Result<void> initialize() override { return Result<void>::success(); }
    Result<void> start() override { return Result<void>::success(); }
    Result<void> stop() override { return Result<void>::success(); }
    Result<void> shutdown() override { return Result<void>::success(); }
    LifecycleState lifecycle_state() const noexcept override { return LifecycleState::UNKNOWN; }
    Status status() const override { return Status{}; }
    Health health() const override { return Health{}; }
    CapabilitySet capabilities() const override { return CapabilitySet{}; }
    Statistics statistics() const override { ++reads; return own; }
    Statistics own{};
    mutable int reads{0};
};

struct GoodFixture {
    CountingComponent c{make_info(1)};
    const IComponentStatistics& provider() { return c; }
    void add_samples(std::uint64_t n) { c.own.sample_count.increment(n); }
    void add_errors(std::uint64_t n) { c.own.error_count.increment(n); }
    void set_queue_depth(std::int64_t g) { c.own.queue_depth.set(g); }
};

// Non-conforming providers, to prove the check detects each violation.
class ResetOnRead final : public IComponentStatistics {
public:
    Statistics statistics() const override { Statistics s = value; value.sample_count.reset(); return s; }
    mutable Statistics value{};
};
class Frozen final : public IComponentStatistics {          // always returns the first snapshot it ever took
public:
    Statistics statistics() const override { if (!taken) { snapshot = value; taken = true; } return snapshot; }
    Statistics value{};
    mutable Statistics snapshot{};
    mutable bool taken{false};
};
class Coupled final : public IComponentStatistics {         // an error update also moves the samples
public:
    Statistics statistics() const override { Statistics s = value; s.sample_count.increment(errors); return s; }
    Statistics value{};
    std::uint64_t errors{0};
};
struct BadFixture {
    explicit BadFixture(const IComponentStatistics& p, std::uint64_t* err = nullptr, Statistics* v = nullptr) : p_(p), err_(err), v_(v) {}
    const IComponentStatistics& provider() { return p_; }
    void add_samples(std::uint64_t n) { if (v_ != nullptr) v_->sample_count.increment(n); }
    void add_errors(std::uint64_t n) { if (v_ != nullptr) v_->error_count.increment(n); if (err_ != nullptr) *err_ += n; }
    void set_queue_depth(std::int64_t g) { if (v_ != nullptr) v_->queue_depth.set(g); }
    const IComponentStatistics& p_;
    std::uint64_t* err_;
    Statistics* v_;
};

void test_shape_and_optionality() {
    static_assert(std::is_abstract_v<IComponentStatistics> && std::has_virtual_destructor_v<IComponentStatistics>);
    static_assert(!HasStatistics<Component>);                             // nothing is forced onto the base interface
    static_assert(!HasStatistics<PlainComponent>);                        // a plain component simply has none
    static_assert(HasStatistics<CountingComponent>);
    static_assert(!std::is_base_of_v<IComponentStatistics, Component> && !std::is_base_of_v<Component, IComponentStatistics>);
    static_assert(std::is_base_of_v<IComponentStatistics, CountingComponent> && std::is_base_of_v<Component, CountingComponent>);
    static_assert(std::is_trivially_copyable_v<Statistics>);              // a snapshot is a plain value
    static_assert(std::is_same_v<decltype(std::declval<const IComponentStatistics&>().statistics()), Statistics>);   // by value
}

void test_a_component_without_statistics_is_fully_usable() {
    PlainComponent c(make_info(2));
    RuntimeManager runtime;
    assert(runtime.register_component(c));
    assert(runtime.initialize() && runtime.start() && runtime.stop() && runtime.shutdown());
    const ComponentObservation o = observe(c);
    assert(!o.statistics.has_value());                                    // optional: nothing meaningless is invented
    assert(!observe(c, nullptr).statistics.has_value());
}

void test_conforming_provider_passes_and_is_observed() {
    GoodFixture f;
    assert(check_statistics_provider(f).empty());
    f.c.own = Statistics{};
    f.c.own.sample_count.increment(9);
    f.c.own.drop_count.increment(3);
    f.c.own.queue_depth.set(4);
    f.c.own.utilization.set(55);
    f.c.own.retry_count.increment(1);
    const ComponentObservation o = observe(f.c, &f.c);
    assert(o.statistics.has_value());
    assert(o.statistics->sample_count.value() == 9 && o.statistics->drop_count.value() == 3);
    assert(o.statistics->queue_depth.value() == 4 && o.statistics->utilization.value() == 55 && o.statistics->retry_count.value() == 1);
    assert(o.statistics->error_count.value() == 0);
}

void test_check_detects_non_conforming_providers() {
    { ResetOnRead p; Statistics* v = &p.value; BadFixture f(p, nullptr, v); assert(!check_statistics_provider(f).empty()); }
    { Frozen p; BadFixture f(p, nullptr, &p.value); assert(!check_statistics_provider(f).empty()); }
    { Coupled p; BadFixture f(p, &p.errors, &p.value); assert(!check_statistics_provider(f).empty()); }
}

void test_values_pass_through_unchanged_without_clamping_or_derivation() {
    CountingComponent c(make_info(3));
    c.own.sample_count.increment(std::numeric_limits<std::uint64_t>::max());
    c.own.queue_depth.set(std::numeric_limits<std::int64_t>::min());
    c.own.utilization.set(1000);                                          // outside 0..100: stored and reported as-is
    const Statistics s = observe(c, &c).statistics.value();
    assert(s.sample_count.value() == std::numeric_limits<std::uint64_t>::max());
    assert(s.queue_depth.value() == std::numeric_limits<std::int64_t>::min());
    assert(s.utilization.value() == 1000);
    assert(s.error_count.value() == 0 && s.retry_count.value() == 0 && s.drop_count.value() == 0);   // nothing added by Core
}

void test_snapshot_is_independent_of_the_provider() {
    CountingComponent c(make_info(4));
    c.own.sample_count.increment(1);
    Statistics copy = c.statistics();
    copy.sample_count.increment(1000);                                    // changing the copy never changes the owner
    copy.queue_depth.set(99);
    assert(c.own.sample_count.value() == 1 && c.own.queue_depth.value() == 0);
    const ComponentObservation o = observe(c, &c);
    c.own.sample_count.increment(1);                                      // nor does the owner change a held snapshot
    assert(o.statistics->sample_count.value() == 1);
}

void test_component_statistics_are_separate_from_runtime_statistics() {
    CountingComponent c(make_info(5));
    c.own.sample_count.increment(1'000'000);                              // the component's own, unrelated numbers
    c.own.error_count.increment(777);
    RuntimeManager runtime;
    assert(runtime.register_component(c));
    assert(runtime.initialize() && runtime.start() && runtime.stop() && runtime.shutdown());
    assert(runtime.statistics().sample_count.value() == 4 && runtime.statistics().error_count.value() == 0);   // Runtime counts its calls only
    const ComponentObservation o = observe(c, &c);
    assert(o.statistics->sample_count.value() == 1'000'000 && o.statistics->error_count.value() == 777);      // untouched by the Runtime
    assert(c.reads == 1);                                                 // the Runtime never read the provider; only observe() did
    c.own.sample_count.increment(5);
    assert(runtime.statistics().sample_count.value() == 4);               // and the component's updates never reach the Runtime
}

void test_runtime_never_reads_or_resets_component_statistics_through_fault_and_reset() {
    class FailingStart final : public Component, public IComponentStatistics {
    public:
        explicit FailingStart(ComponentInfo info) : Component(std::move(info)) {}
        Result<void> configure(const Configuration&) override { return Result<void>::success(); }
        Result<void> initialize() override { return Result<void>::success(); }
        Result<void> start() override { return Result<void>::failure(Error{ErrorCode::INTERNAL_ERROR, ErrorSeverity::ERROR, info().id(), {}, "x"}); }
        Result<void> stop() override { return Result<void>::success(); }
        Result<void> shutdown() override { return Result<void>::success(); }
        LifecycleState lifecycle_state() const noexcept override { return LifecycleState::FAULT; }
        Status status() const override { return Status{}; }
        Health health() const override { return Health{}; }
        CapabilitySet capabilities() const override { return CapabilitySet{}; }
        Statistics statistics() const override { ++reads; return own; }
        Statistics own{};
        mutable int reads{0};
    };
    FailingStart c(make_info(6));
    c.own.error_count.increment(41);
    RuntimeManager runtime;
    assert(runtime.register_component(c));
    assert(runtime.initialize() && !runtime.start());
    assert(runtime.state() == LifecycleState::FAULT && runtime.reset());
    assert(c.reads == 0);                                                 // never consulted
    assert(c.own.error_count.value() == 41);                              // never reset or adjusted by the Runtime
    assert(runtime.statistics().error_count.value() == 1);                // the Runtime's own count is separate
}

void test_one_provider_for_each_component_and_pairing_is_the_callers() {
    CountingComponent a(make_info(7)), b(make_info(8));
    a.own.sample_count.increment(1);
    b.own.sample_count.increment(2);
    assert(observe(a, &a).statistics->sample_count.value() == 1);
    assert(observe(b, &b).statistics->sample_count.value() == 2);
    assert(observe(a, &b).statistics->sample_count.value() == 2);         // Core reports what the caller paired; no identity check
}

} // namespace

int main() {
    test_shape_and_optionality();
    test_a_component_without_statistics_is_fully_usable();
    test_conforming_provider_passes_and_is_observed();
    test_check_detects_non_conforming_providers();
    test_values_pass_through_unchanged_without_clamping_or_derivation();
    test_snapshot_is_independent_of_the_provider();
    test_component_statistics_are_separate_from_runtime_statistics();
    test_runtime_never_reads_or_resets_component_statistics_through_fault_and_reset();
    test_one_provider_for_each_component_and_pairing_is_the_callers();
    return 0;
}
