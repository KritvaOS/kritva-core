//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : capability_contract_test.cpp
// Description : Contract tests for Capability identity and provider semantics.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-CAP-004
// API         : CORE-API-CAPABILITY
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#include <cassert>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>

#include <kritva/core/core.hpp>

#include "../contract/reference_component.hpp"
#include "../platform/reference_platform.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;
using kritva::core::platform::testing::ReferencePlatform;

namespace {

// Name-based member detection (any signature, any overload) for a non-final T.
#define KRITVA_CAP_HAS_MEMBER(NAME)                                                           \
    struct Fallback_##NAME { void NAME(); };                                                  \
    template <class T> struct Probe_##NAME : T, Fallback_##NAME {};                           \
    template <class T> concept Has_##NAME = !requires { &Probe_##NAME<T>::NAME; };
KRITVA_CAP_HAS_MEMBER(token)
KRITVA_CAP_HAS_MEMBER(credential)
KRITVA_CAP_HAS_MEMBER(secret)
KRITVA_CAP_HAS_MEMBER(signature)
KRITVA_CAP_HAS_MEMBER(authorize)
KRITVA_CAP_HAS_MEMBER(authenticate)
KRITVA_CAP_HAS_MEMBER(grant)
KRITVA_CAP_HAS_MEMBER(available)
KRITVA_CAP_HAS_MEMBER(is_available)
KRITVA_CAP_HAS_MEMBER(health)
KRITVA_CAP_HAS_MEMBER(state)
KRITVA_CAP_HAS_MEMBER(ready)
KRITVA_CAP_HAS_MEMBER(vendor)
KRITVA_CAP_HAS_MEMBER(platform)
template <class T> concept HasSecurityOrStateMember = Has_token<T> || Has_credential<T> || Has_secret<T> || Has_signature<T> || Has_authorize<T> ||
                                                      Has_authenticate<T> || Has_grant<T> || Has_available<T> || Has_is_available<T> ||
                                                      Has_health<T> || Has_state<T> || Has_ready<T> || Has_vendor<T> || Has_platform<T>;

ComponentInfo make_info(std::uint64_t id) {
    auto info = ComponentInfo::create(ComponentId{id}, "cap");
    assert(info);
    return std::move(info).value();
}

void test_shape_is_exactly_identity_name_version_and_carries_no_credential_or_state() {
    static_assert(std::is_same_v<CapabilityId, Id>);
    static_assert(std::is_aggregate_v<Capability>);
    static_assert(std::is_same_v<decltype(Capability::id), CapabilityId> && std::is_same_v<decltype(Capability::name), std::string> &&
                  std::is_same_v<decltype(Capability::version), Version>);
    static_assert(std::is_copy_constructible_v<Capability> && std::is_copy_assignable_v<Capability> && std::is_nothrow_move_constructible_v<Capability>);
    static_assert(!HasSecurityOrStateMember<Capability>);                 // not a credential, token, authorization, state or health signal
    const Capability c{CapabilityId{1}, "x", Version{1, 0, 0}};
    const auto& [id, name, version] = c;                                  // exactly three members: a fourth would not decompose
    assert(id == CapabilityId{1} && name == "x" && version == (Version{1, 0, 0}));
}

struct Detector { void token(); void health(); };
static_assert(HasSecurityOrStateMember<Detector>);                        // the detector is sensitive to what it forbids

void test_identity_is_valid_iff_non_zero_and_compares_by_value_only() {
    const CapabilityId invalid{};
    assert(!invalid.valid() && invalid.value() == 0);
    for (const std::uint64_t v : {1ull, 2ull, 100ull, 0xFFFFFFFFFFFFFFFFull}) assert(CapabilityId{v}.valid());
    assert(CapabilityId{5} == CapabilityId{5} && !(CapabilityId{5} == CapabilityId{6}));
    const Capability d;                                                    // a default Capability has no identity, no name, version 0.0.0
    assert(!d.id.valid() && d.name.empty() && d.version == (Version{0, 0, 0}));
}

void test_name_is_metadata_not_identity() {
    const Capability a{CapabilityId{7}, "motion.control", Version{1, 0, 0}};
    const Capability b{CapabilityId{8}, "motion.control", Version{1, 0, 0}};      // same name, different identity: a different capability
    const Capability c{CapabilityId{7}, "something-else", Version{1, 0, 0}};      // same identity, different name: the same capability
    assert(a.id != b.id && a.id == c.id);
    CapabilitySet set;
    set.add(a);
    set.add(b);
    assert(set.size() == 2);                                                      // names repeat across ids
    assert(set.find(CapabilityId{7}) != nullptr && set.find(CapabilityId{8}) != nullptr);
    set.add(c);                                                                   // identity decides: replaced, not added
    assert(set.size() == 2 && set.find(CapabilityId{7})->name == "something-else");
    Capability empty_name{CapabilityId{9}, "", Version{}};                        // an empty name is legal metadata
    set.add(empty_name);
    assert(set.contains(CapabilityId{9}) && set.size() == 3);
    CapabilitySet lookalike;                                                      // a name that looks like an id or another capability proves nothing
    lookalike.add(Capability{CapabilityId{20}, "capability-21", Version{}});
    assert(!lookalike.contains(CapabilityId{21}) && lookalike.contains(CapabilityId{20}));
}

void test_a_capability_is_a_detached_value() {
    Capability original{CapabilityId{3}, std::string(300, 'n'), Version{2, 1, 0}};
    Capability copy = original;
    original.name = "changed";
    original.version = Version{9, 9, 9};
    original.id = CapabilityId{4};
    assert(copy.id == CapabilityId{3} && copy.name == std::string(300, 'n') && copy.version == (Version{2, 1, 0}));
    Capability moved = std::move(copy);
    assert(moved.name == std::string(300, 'n'));
}

void test_providers_publish_value_snapshots_and_querying_has_no_effect() {
    static_assert(std::is_same_v<decltype(std::declval<const Component&>().capabilities()), CapabilitySet>);                       // by value
    static_assert(std::is_same_v<decltype(std::declval<const platform::IPlatformAdapter&>().capabilities()), CapabilitySet>);      // by value
    contract::ReferenceComponent c(make_info(1));
    const LifecycleState before = c.lifecycle_state();
    CapabilitySet first = c.capabilities();
    CapabilitySet second = c.capabilities();
    assert(first.size() == second.size() && c.lifecycle_state() == before);       // a query changes nothing and is repeatable
    first.add(Capability{CapabilityId{50}, "local", Version{}});                  // changing a snapshot never changes the provider
    assert(c.capabilities().size() == second.size() && !c.capabilities().contains(CapabilityId{50}));
    assert(c.initialize() && c.start());
    assert(c.capabilities().size() == second.size());                             // and the declaration does not follow the lifecycle
}

void test_a_declaration_is_not_availability_and_not_proof() {
    ReferencePlatform::Config config;
    config.capabilities.add(Capability{CapabilityId{10}, "claimed", Version{1, 0, 0}});
    ReferencePlatform platform(config);
    const platform::PlatformContext context(platform);
    assert(context.capabilities().contains(CapabilityId{10}));                    // the provider claims it ...
    assert(context.supports(platform::PlatformService::SCHEDULER) == config.provides.scheduler);   // ... Core never probes or implies a service
    platform::PlatformRequirements requirements;
    assert(requirements.add_capability(CapabilityId{10}, platform::Requirement::REQUIRED));
    assert(platform::check_required(requirements, context));                      // an identity-based check is the only relation
    for (const std::uint64_t absent : {11ull, 2'000'000ull, 0xFFFFFFFFFFFFFFFFull}) {   // any identity the provider does not declare is missing
        platform::PlatformRequirements other;
        assert(other.add_capability(CapabilityId{absent}, platform::Requirement::REQUIRED));
        assert(!platform::check_required(other, context));
    }
}

void test_an_invalid_identity_entry_is_storable_but_not_authoritative_and_satisfies_nothing() {
    CapabilitySet set;
    set.add(Capability{CapabilityId{}, "no-identity", Version{1, 0, 0}});         // add() accepts the value unchanged: storable data ...
    assert(set.size() == 1 && set.contains(CapabilityId{}));                      // ... physically present, found by its (invalid) key
    platform::PlatformRequirements requirements;
    const auto rejected = requirements.add_capability(CapabilityId{}, platform::Requirement::REQUIRED);
    assert(!rejected && rejected.error().code == ErrorCode::INVALID_ARGUMENT);    // a requirement cannot name an invalid identity
    assert(requirements.empty());
    ReferencePlatform::Config config;
    config.capabilities = set;
    ReferencePlatform platform(config);
    const platform::PlatformContext context(platform);
    assert(requirements.add_capability(CapabilityId{3}, platform::Requirement::REQUIRED));
    const auto report = platform::evaluate(requirements, context);
    assert(!report.satisfied() && report.missing_required_capabilities.size() == 1);   // the invalid-identity entry satisfies no valid requirement
    set.add(Capability{CapabilityId{3}, "real", Version{}});
    config.capabilities = set;
    ReferencePlatform platform2(config);
    assert(platform::check_required(requirements, platform::PlatformContext(platform2)));   // only a real identity satisfies it
}

} // namespace

int main() {
    test_shape_is_exactly_identity_name_version_and_carries_no_credential_or_state();
    test_identity_is_valid_iff_non_zero_and_compares_by_value_only();
    test_name_is_metadata_not_identity();
    test_a_capability_is_a_detached_value();
    test_providers_publish_value_snapshots_and_querying_has_no_effect();
    test_a_declaration_is_not_availability_and_not_proof();
    test_an_invalid_identity_entry_is_storable_but_not_authoritative_and_satisfies_nothing();
    return 0;
}
