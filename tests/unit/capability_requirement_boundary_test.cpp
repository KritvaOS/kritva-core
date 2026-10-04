//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : capability_requirement_boundary_test.cpp
// Description : Contract tests for the capability provision / requirement / matching boundary.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-CAP-007, CORE-CAP-008
// API         : CORE-API-CAPABILITY
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

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

#include "../contract/reference_component.hpp"
#include "../platform/reference_platform.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;
using kritva::core::platform::testing::ReferencePlatform;

namespace {

#define KRITVA_REQ_HAS_MEMBER(NAME)                                                           \
    struct Fallback_##NAME { void NAME(); };                                                  \
    template <class T> struct Probe_##NAME : T, Fallback_##NAME {};                           \
    template <class T> constexpr bool has_member_##NAME() {                                   \
        if constexpr (std::is_final_v<T>) return requires { &T::NAME; };                      \
        else return !requires { &Probe_##NAME<T>::NAME; };                                    \
    }                                                                                         \
    template <class T> concept Has_##NAME = has_member_##NAME<T>();
KRITVA_REQ_HAS_MEMBER(require_capability)
KRITVA_REQ_HAS_MEMBER(resolve)
KRITVA_REQ_HAS_MEMBER(bind)
KRITVA_REQ_HAS_MEMBER(inject)
KRITVA_REQ_HAS_MEMBER(satisfy)
KRITVA_REQ_HAS_MEMBER(discover)
KRITVA_REQ_HAS_MEMBER(provider_of)
KRITVA_REQ_HAS_MEMBER(requires_capability)
KRITVA_REQ_HAS_MEMBER(capability_dependency)
template <class T> concept HasResolutionOperation = Has_require_capability<T> || Has_resolve<T> || Has_bind<T> || Has_inject<T> || Has_satisfy<T> ||
                                                     Has_discover<T> || Has_provider_of<T> || Has_requires_capability<T> || Has_capability_dependency<T>;
struct Detector { void resolve(); };
static_assert(HasResolutionOperation<Detector>);

ComponentInfo make_info(std::uint64_t id) {
    auto info = ComponentInfo::create(ComponentId{id}, "c");
    assert(info);
    return std::move(info).value();
}

Capability cap(std::uint64_t id, const char* name, Version v = {}) { return Capability{CapabilityId{id}, name, v}; }

ReferencePlatform::Config config_with(std::initializer_list<Capability> caps) {
    ReferencePlatform::Config c;
    for (const Capability& k : caps) c.capabilities.add(k);
    return c;
}

bool satisfied(const ReferencePlatform& platform, std::uint64_t id) {
    platform::PlatformRequirements r;
    assert(r.add_capability(CapabilityId{id}, platform::Requirement::REQUIRED));
    return static_cast<bool>(platform::check_required(r, platform::PlatformContext(const_cast<ReferencePlatform&>(platform))));
}

void test_the_surface_has_no_resolution_binding_or_capability_dependency_operations() {
    static_assert(!HasResolutionOperation<platform::PlatformRequirements> && !HasResolutionOperation<platform::PlatformContext> &&
                  !HasResolutionOperation<ComponentContext> && !HasResolutionOperation<RuntimeManager> && !HasResolutionOperation<DependencyGraph> &&
                  !HasResolutionOperation<Component> && !HasResolutionOperation<CapabilitySet>);
    static_assert(std::is_aggregate_v<platform::CapabilityRequirement> && std::is_same_v<decltype(platform::CapabilityRequirement::id), CapabilityId> &&
                  std::is_same_v<decltype(platform::CapabilityRequirement::level), platform::Requirement>);
    // The dependency graph orders ComponentIds only: its one edge operation takes two ComponentIds, nothing capability-typed.
    static_assert(std::is_same_v<decltype(&DependencyGraph::add_dependency), Result<void> (DependencyGraph::*)(ComponentId, ComponentId)>);
    static_assert(std::is_same_v<decltype(&RuntimeManager::add_dependency), Result<void> (RuntimeManager::*)(ComponentId, ComponentId)>);
}

void test_matching_is_by_identity_and_ignores_names_versions_and_platform_information() {
    ReferencePlatform::Config config = config_with({cap(10, "gpio", Version{1, 0, 0}), cap(11, "gpio", Version{9, 9, 9}), cap(12, "capability-13", Version{0, 0, 1})});
    config.info = platform::PlatformInfo{"vendor-x-linux-13", Version{13, 13, 13}};          // names and versions that look like identities
    ReferencePlatform platform(config);
    for (const std::uint64_t id : {10ull, 11ull, 12ull}) assert(satisfied(platform, id));
    for (const std::uint64_t absent : {13ull, 1ull, 9ull, 100ull, 0xFFFFFFFFFFFFFFFFull}) assert(!satisfied(platform, absent));   // a lookalike proves nothing
    ReferencePlatform::Config other = config_with({cap(10, "completely-different-name", Version{0, 0, 0})});
    other.info = platform::PlatformInfo{"other", Version{0, 0, 1}};
    ReferencePlatform platform2(other);
    assert(satisfied(platform2, 10));                                                          // satisfied whatever the names, versions and platform info
}

void test_a_requirement_is_declarative_atomic_and_identity_deduplicated() {
    platform::PlatformRequirements r;
    assert(r.add_capability(CapabilityId{5}, platform::Requirement::REQUIRED));
    assert(!r.add_capability(CapabilityId{}, platform::Requirement::REQUIRED));               // an invalid identity cannot be required
    assert(!r.add_capability(CapabilityId{5}, platform::Requirement::OPTIONAL));              // a duplicate is decided by identity, not by level
    assert(!r.add_capability(CapabilityId{5}, platform::Requirement::REQUIRED));
    assert(r.capabilities().size() == 1 && r.capabilities()[0].level == platform::Requirement::REQUIRED);   // each rejection changed nothing
    assert(r.add_capability(CapabilityId{6}, platform::Requirement::OPTIONAL));
    ReferencePlatform platform(config_with({}));                                             // an empty provision
    const platform::PlatformContext context(platform);
    const auto report = platform::evaluate(r, context);
    assert(!report.satisfied() && report.missing_required_capabilities.size() == 1 && report.missing_optional_capabilities.size() == 1);
    assert(!platform::check_required(r, context));                                           // a REQUIRED item decides ...
    platform::PlatformRequirements only_optional;
    assert(only_optional.add_capability(CapabilityId{6}, platform::Requirement::OPTIONAL));
    assert(platform::check_required(only_optional, context));                                // ... an OPTIONAL one never does
}

void test_evaluation_takes_one_snapshot_has_no_side_effect_and_is_deterministic() {
    ReferencePlatform platform(config_with({cap(1, "a"), cap(2, "b")}));
    platform::PlatformRequirements r;
    assert(r.add_capability(CapabilityId{1}, platform::Requirement::REQUIRED) && r.add_capability(CapabilityId{2}, platform::Requirement::REQUIRED) &&
           r.add_capability(CapabilityId{3}, platform::Requirement::OPTIONAL) && r.add_capability(CapabilityId{4}, platform::Requirement::REQUIRED));
    const platform::PlatformContext context(platform);
    const std::size_t before = platform.controls().adapter_queries();
    const auto first = platform::evaluate(r, context);
    assert(platform.controls().adapter_queries() == before + 1);                              // four capability requirements, ONE snapshot
    const auto second = platform::evaluate(r, context);
    assert(platform.controls().adapter_queries() == before + 2);
    assert(first.missing_required_capabilities.size() == 1 && first.missing_required_capabilities[0].id == CapabilityId{4});
    assert(first.missing_optional_capabilities.size() == 1 && first.missing_optional_capabilities[0].id == CapabilityId{3});
    assert(second.missing_required_capabilities.size() == first.missing_required_capabilities.size() &&
           second.missing_optional_capabilities.size() == first.missing_optional_capabilities.size());     // same input, same report
    assert(r.capabilities().size() == 4 && platform.controls().log().empty());                // nothing started, stopped, created or changed
    platform::PlatformRequirements none;
    const std::size_t q = platform.controls().adapter_queries();
    (void)platform::evaluate(none, context);
    assert(platform.controls().adapter_queries() == q);                                       // no capability requirement: the provider is not even asked
}

void test_the_same_model_is_used_through_a_component_context() {
    ReferencePlatform platform(config_with({cap(1, "a")}));
    contract::ReferenceComponent component(make_info(7));
    const ComponentContext context(component, platform::PlatformContext(platform));
    platform::PlatformRequirements r;
    assert(r.add_capability(CapabilityId{1}, platform::Requirement::REQUIRED) && r.add_capability(CapabilityId{2}, platform::Requirement::REQUIRED));
    assert(context.has_capability(CapabilityId{1}));
    for (const std::uint64_t absent : {2ull, 99ull, 100ull, 0xFFFFFFFFFFFFFFFFull}) assert(!context.has_capability(CapabilityId{absent}));
    const auto via_context = context.evaluate(r);
    const auto direct = platform::evaluate(r, platform::PlatformContext(platform));
    assert(via_context.missing_required_capabilities.size() == direct.missing_required_capabilities.size() &&
           via_context.missing_required_capabilities[0].id == direct.missing_required_capabilities[0].id);   // R0.5 report, unchanged
    const auto checked = context.check_required(r);
    assert(!checked && checked.error().code == ErrorCode::UNSUPPORTED && checked.error().source == ComponentId{7});
}

void test_capabilities_never_change_the_dependency_order_or_the_registration() {
    // The same topology with every assignment of provisions to components gives the same order: capabilities are not edges.
    for (unsigned seed = 1; seed <= 100; ++seed) {
        std::mt19937 rng(seed);
        const std::size_t n = 2 + rng() % 5;
        std::vector<std::pair<std::uint64_t, std::uint64_t>> edges;
        for (std::uint64_t a = 2; a <= n; ++a) for (std::uint64_t b = 1; b < a; ++b) if (rng() % 3 == 0) edges.push_back({a, b});   // acyclic
        auto order_with = [&](unsigned capability_seed) {
            std::mt19937 crng(capability_seed);
            struct Providing final : public contract::ReferenceComponent {
                Providing(ComponentInfo info, CapabilitySet set) : ReferenceComponent(std::move(info)), set_(std::move(set)) {}
                CapabilitySet capabilities() const override { return set_; }
                CapabilitySet set_;
            };
            std::vector<std::unique_ptr<Providing>> comps;
            RuntimeManager runtime;
            for (std::uint64_t id = 1; id <= n; ++id) {
                CapabilitySet set;
                for (int k = 0; k < 4; ++k) if (crng() % 2 == 0) set.add(cap(1 + crng() % 8, "x", Version{static_cast<std::uint32_t>(crng() % 3), 0, 0}));
                comps.push_back(std::make_unique<Providing>(make_info(id), set));
                assert(runtime.register_component(*comps.back()));
            }
            for (const auto& e : edges) assert(runtime.add_dependency(ComponentId{e.first}, ComponentId{e.second}));
            auto order = runtime.component_order();
            assert(order.has_value());
            std::vector<std::uint64_t> ids;
            for (const ComponentId id : order.value()) ids.push_back(id.value());
            return ids;
        };
        const auto baseline = order_with(1000 + seed);
        assert(baseline == order_with(2000 + seed) && baseline == order_with(3000 + seed));   // provisions differ, the order does not
    }
}

void test_the_runtime_never_evaluates_resolves_or_asks_for_capabilities() {
    ReferencePlatform platform(config_with({cap(1, "a")}));
    contract::ReferenceComponent a(make_info(1)), b(make_info(2));
    RuntimeManager runtime;
    assert(runtime.register_component(a) && runtime.register_component(b) && runtime.add_dependency(ComponentId{2}, ComponentId{1}));
    assert(runtime.attach_platform(platform));
    assert(runtime.configure(Configuration{}) && runtime.initialize() && runtime.start() && runtime.stop() && runtime.shutdown());
    assert(platform.controls().adapter_queries() == 0 && platform.controls().log().empty());   // no capability snapshot was ever taken or resolved
}

} // namespace

int main() {
    test_the_surface_has_no_resolution_binding_or_capability_dependency_operations();
    test_matching_is_by_identity_and_ignores_names_versions_and_platform_information();
    test_a_requirement_is_declarative_atomic_and_identity_deduplicated();
    test_evaluation_takes_one_snapshot_has_no_side_effect_and_is_deterministic();
    test_the_same_model_is_used_through_a_component_context();
    test_capabilities_never_change_the_dependency_order_or_the_registration();
    test_the_runtime_never_evaluates_resolves_or_asks_for_capabilities();
    return 0;
}
