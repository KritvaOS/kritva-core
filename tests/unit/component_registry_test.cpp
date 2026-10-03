//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component_registry_test.cpp
// Description : ComponentRegistry contract tests.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-RT-003
// API         : CORE-TEST-COMPONENT-REGISTRY
//
// Author      : KritvaOS Core Team
// Created     : 03-10-2026
//==============================================================================

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <numeric>
#include <random>
#include <string>
#include <type_traits>
#include <vector>

#include <kritva/core/core.hpp>

#include "../contract/component_contract.hpp"
#include "../contract/reference_component.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;
using kritva::core::runtime::contract::ReferenceComponent;

namespace {

std::unique_ptr<ReferenceComponent> make_component(std::uint64_t id, const char* name = "c") {
    auto info = ComponentInfo::create(ComponentId{id}, name);
    assert(info.has_value());
    return std::make_unique<ReferenceComponent>(std::move(info).value());
}

std::vector<std::uint64_t> ids_of(const std::vector<Component*>& components) {
    std::vector<std::uint64_t> ids;
    for (const Component* c : components) ids.push_back(c->info().id().value());
    return ids;
}

int lifecycle_calls(const ReferenceComponent& c) {
    return c.configure_calls + c.initialize_calls + c.start_calls + c.stop_calls + c.shutdown_calls;
}

// -----------------------------------------------------------------------------
// Registration (AC-002-01)
// -----------------------------------------------------------------------------

void test_register_valid_component() {
    ComponentRegistry registry;
    assert(registry.empty() && registry.size() == 0);

    auto component = make_component(7, "sensor");
    const Result<void> r = registry.register_component(*component);
    assert(r.has_value());
    assert(registry.size() == 1 && !registry.empty());

    // Discoverable, and the registry refers to the very object (not a copy).
    assert(registry.contains(ComponentId{7}));
    assert(registry.find(ComponentId{7}) == component.get());
    assert(registry.find(ComponentId{7})->info().name() == "sensor");
}

void test_invalid_identity_cannot_reach_the_registry() {
    // An invalid id cannot be turned into a Component (ComponentInfo::create
    // rejects it) and a reference cannot be null, so register_component() has no
    // invalid-identity or null input to reject.
    assert(!ComponentInfo::create(ComponentId{}, "x").has_value());
    static_assert(std::is_same_v<decltype(&ComponentRegistry::register_component),
                                 Result<void> (ComponentRegistry::*)(Component&)>);
}

void test_registration_does_not_touch_the_component() {
    ComponentRegistry registry;
    auto component = make_component(1);
    assert(registry.register_component(*component));
    // The registry never calls a lifecycle operation, and the state is unchanged.
    assert(lifecycle_calls(*component) == 0);
    assert(component->lifecycle_state() == LifecycleState::UNKNOWN);

    // Registration is allowed in any lifecycle state and does not change it.
    auto running = make_component(2);
    assert(running->initialize() && running->start());
    assert(registry.register_component(*running));
    assert(running->lifecycle_state() == LifecycleState::RUNNING);

    auto faulted = make_component(3);
    faulted->fail_next_initialize = ErrorCode::INTERNAL_ERROR;
    assert(!faulted->initialize());
    assert(registry.register_component(*faulted));
    assert(faulted->lifecycle_state() == LifecycleState::FAULT);
    assert(registry.size() == 3);
}

// -----------------------------------------------------------------------------
// Duplicate identity (AC-002-02)
// -----------------------------------------------------------------------------

void test_duplicate_identity_fails_without_replacement() {
    ComponentRegistry registry;
    auto first = make_component(5, "first");
    auto second = make_component(5, "second");        // different object, same id
    assert(registry.register_component(*first));

    const Result<void> dup = registry.register_component(*second);
    assert(!dup.has_value());
    assert(dup.error().code == ErrorCode::INVALID_ARGUMENT);
    assert(dup.error().severity == ErrorSeverity::ERROR);
    assert(dup.error().source == ComponentId{5});                 // attributable
    assert(dup.error().message.find("5") != std::string::npos);   // names the id

    // The existing registration is not replaced, and the registry is unchanged.
    assert(registry.size() == 1);
    assert(registry.find(ComponentId{5}) == first.get());
    assert(registry.find(ComponentId{5})->info().name() == "first");

    // Registering the same object again is a duplicate too.
    const Result<void> again = registry.register_component(*first);
    assert(!again && again.error().code == ErrorCode::INVALID_ARGUMENT);
    assert(registry.size() == 1);

    // The failure is deterministic: it repeats identically.
    const Result<void> dup2 = registry.register_component(*second);
    assert(!dup2 && dup2.error().code == dup.error().code && dup2.error().message == dup.error().message);
}

void test_failed_registration_leaves_registry_unchanged() {
    ComponentRegistry registry;
    auto a = make_component(10);
    auto b = make_component(20);
    auto c = make_component(30);
    auto dup = make_component(20);
    assert(registry.register_component(*a) && registry.register_component(*b) && registry.register_component(*c));

    const std::vector<std::uint64_t> before = ids_of(registry.components());
    assert(!registry.register_component(*dup));
    assert(ids_of(registry.components()) == before);
    assert(registry.size() == 3);
    assert(registry.find(ComponentId{20}) == b.get());
    assert(lifecycle_calls(*dup) == 0);

    // A later, valid registration still works after a failure.
    auto d = make_component(40);
    assert(registry.register_component(*d));
    assert(registry.size() == 4);
}

// -----------------------------------------------------------------------------
// Lookup and contains (AC-002-03, AC-002-04)
// -----------------------------------------------------------------------------

void test_lookup_and_contains() {
    ComponentRegistry registry;
    auto a = make_component(1);
    auto b = make_component(2);
    assert(registry.register_component(*a) && registry.register_component(*b));

    assert(registry.find(ComponentId{1}) == a.get());
    assert(registry.find(ComponentId{2}) == b.get());
    assert(registry.contains(ComponentId{1}) && registry.contains(ComponentId{2}));

    // Missing: deterministic not-found, including the invalid identity.
    assert(registry.find(ComponentId{3}) == nullptr);
    assert(!registry.contains(ComponentId{3}));
    assert(registry.find(ComponentId{}) == nullptr);
    assert(!registry.contains(ComponentId{}));

    // No side effects: nothing was registered or changed by looking.
    assert(registry.size() == 2);
    assert(!registry.contains(ComponentId{3}));
    assert(lifecycle_calls(*a) == 0 && lifecycle_calls(*b) == 0);

    // Works through a const registry (shallow-const non-owning handle).
    const ComponentRegistry& view = registry;
    assert(view.find(ComponentId{1}) == a.get());
    assert(view.contains(ComponentId{2}) && view.size() == 2);

    static_assert(noexcept(std::declval<const ComponentRegistry&>().find(ComponentId{})));
    static_assert(noexcept(std::declval<const ComponentRegistry&>().contains(ComponentId{})));

    // Lookup on an empty registry.
    ComponentRegistry empty;
    assert(empty.find(ComponentId{1}) == nullptr && !empty.contains(ComponentId{1}));
}

// -----------------------------------------------------------------------------
// Enumeration (AC-002-05)
// -----------------------------------------------------------------------------

void test_enumeration_is_ascending_id_order_regardless_of_registration_order() {
    const std::vector<std::uint64_t> sorted = {1, 2, 3, 4, 5};
    std::vector<std::uint64_t> order = {5, 1, 3, 2, 4};
    do {
        ComponentRegistry registry;
        std::vector<std::unique_ptr<ReferenceComponent>> owned;
        for (std::uint64_t id : order) {
            owned.push_back(make_component(id));
            assert(registry.register_component(*owned.back()));
        }
        assert(ids_of(registry.components()) == sorted);
    } while (std::next_permutation(order.begin(), order.end()));          // all 120 orders
}

void test_enumeration_exactly_once_and_large_sets() {
    constexpr std::uint64_t kCount = 1000;
    std::vector<std::uint64_t> ids(kCount);
    std::iota(ids.begin(), ids.end(), std::uint64_t{1});
    std::mt19937 rng(12345);                                   // fixed seed: deterministic
    std::shuffle(ids.begin(), ids.end(), rng);

    ComponentRegistry registry;
    std::vector<std::unique_ptr<ReferenceComponent>> owned;
    for (std::uint64_t id : ids) {
        owned.push_back(make_component(id));
        assert(registry.register_component(*owned.back()));
    }
    assert(registry.size() == kCount);

    const auto listed = ids_of(registry.components());
    assert(listed.size() == kCount);
    assert(std::is_sorted(listed.begin(), listed.end()));
    assert(std::adjacent_find(listed.begin(), listed.end()) == listed.end());   // no repeats
    assert(listed.front() == 1 && listed.back() == kCount);

    // Extreme ids order by unsigned value.
    ComponentRegistry extremes;
    auto big = make_component(UINT64_MAX);
    auto one = make_component(1);
    auto mid = make_component(UINT64_MAX / 2);
    assert(extremes.register_component(*big) && extremes.register_component(*one) && extremes.register_component(*mid));
    assert((ids_of(extremes.components()) == std::vector<std::uint64_t>{1, UINT64_MAX / 2, UINT64_MAX}));
}

void test_enumeration_is_a_snapshot_of_non_owning_pointers() {
    ComponentRegistry registry;
    auto a = make_component(1);
    auto b = make_component(2);
    assert(registry.register_component(*a));

    std::vector<Component*> snapshot = registry.components();
    assert(snapshot.size() == 1 && snapshot[0] == a.get());

    // Later registration does not change an earlier snapshot.
    assert(registry.register_component(*b));
    assert(snapshot.size() == 1);
    assert(registry.components().size() == 2);

    // Changing the snapshot does not change the registry.
    snapshot.clear();
    assert(registry.size() == 2);

    // The pointers are the registered objects themselves.
    const auto all = registry.components();
    assert(all[0] == a.get() && all[1] == b.get());

    ComponentRegistry empty;
    assert(empty.components().empty());
}

// -----------------------------------------------------------------------------
// Ownership / lifetime (AC-002-06) and runtime separation (AC-002-08)
// -----------------------------------------------------------------------------

void test_registry_is_non_owning() {
    static_assert(!std::is_copy_constructible_v<ComponentRegistry>);
    static_assert(!std::is_copy_assignable_v<ComponentRegistry>);
    static_assert(!std::is_move_constructible_v<ComponentRegistry>);
    static_assert(!std::is_move_assignable_v<ComponentRegistry>);
    static_assert(std::is_same_v<decltype(std::declval<ComponentRegistry&>().find(ComponentId{})), Component*>);

    // The component is owned by the caller and outlives the registry; destroying
    // the registry never touches it.
    auto component = make_component(9);
    {
        ComponentRegistry registry;
        assert(registry.register_component(*component));
        assert(component->initialize());           // the owner still drives it
    }                                              // registry destroyed here
    assert(lifecycle_calls(*component) == 1);      // only the owner's call
    assert(component->lifecycle_state() == LifecycleState::READY);
    assert(component->info().id() == ComponentId{9});

    // The reverse order is also safe: destroy the component after the registry
    // is no longer used. (Destroying the registry first never dereferences it.)
    auto shortlived = make_component(10);
    auto registry = std::make_unique<ComponentRegistry>();
    assert(registry->register_component(*shortlived));
    registry.reset();                              // registry gone first
    shortlived.reset();                            // then the component: no double free
}

void test_registry_never_drives_components() {
    ComponentRegistry registry;
    std::vector<std::unique_ptr<ReferenceComponent>> owned;
    for (std::uint64_t id = 1; id <= 5; ++id) {
        owned.push_back(make_component(id));
        assert(registry.register_component(*owned.back()));
    }
    for (Component* c : registry.components()) (void)registry.find(c->info().id());
    for (const auto& c : owned) {
        assert(lifecycle_calls(*c) == 0);
        assert(c->lifecycle_state() == LifecycleState::UNKNOWN);
    }
}

// -----------------------------------------------------------------------------
// Integration through the public API (reference components, multiple coexisting)
// -----------------------------------------------------------------------------

void test_reference_components_coexist_and_are_driven_by_their_owner() {
    ComponentRegistry registry;
    auto sensor = make_component(100, "sensor");
    auto planner = make_component(200, "planner");
    auto actuator = make_component(300, "actuator");
    assert(registry.register_component(*planner));
    assert(registry.register_component(*actuator));
    assert(registry.register_component(*sensor));

    // The owner (not the registry) runs lifecycle operations; the registry only
    // hands back the same objects.
    for (Component* c : registry.components()) {
        assert(c->initialize());
        assert(c->start());
    }
    assert(sensor->lifecycle_state() == LifecycleState::RUNNING);
    assert(planner->lifecycle_state() == LifecycleState::RUNNING);
    assert(actuator->lifecycle_state() == LifecycleState::RUNNING);

    // Component contract (R03-001) still holds for a registered component.
    auto fresh = make_component(400);
    assert(registry.register_component(*fresh));
    contract::check_component_contract(*registry.find(ComponentId{400}), Configuration{});

    // Lookup by id returns the component whose identity matches.
    for (std::uint64_t id : {100u, 200u, 300u, 400u}) {
        assert(registry.find(ComponentId{id})->info().id() == ComponentId{id});
    }
    assert(registry.size() == 4);
}

} // namespace

int main() {
    test_register_valid_component();
    test_invalid_identity_cannot_reach_the_registry();
    test_registration_does_not_touch_the_component();
    test_duplicate_identity_fails_without_replacement();
    test_failed_registration_leaves_registry_unchanged();
    test_lookup_and_contains();
    test_enumeration_is_ascending_id_order_regardless_of_registration_order();
    test_enumeration_exactly_once_and_large_sets();
    test_enumeration_is_a_snapshot_of_non_owning_pointers();
    test_registry_is_non_owning();
    test_registry_never_drives_components();
    test_reference_components_coexist_and_are_driven_by_their_owner();
    return 0;
}
