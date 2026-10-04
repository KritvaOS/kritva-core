//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : capability_set_contract_test.cpp
// Description : Contract tests for CapabilitySet invariants, snapshots and capability version semantics.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-CAP-005, CORE-CAP-006
// API         : CORE-API-CAPABILITY
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <kritva/core/core.hpp>

#include "../platform/reference_platform.hpp"

using namespace kritva::core;
using kritva::core::platform::testing::ReferencePlatform;

namespace {

#define KRITVA_SET_HAS_MEMBER(NAME)                                                           \
    struct Fallback_##NAME { void NAME(); };                                                  \
    template <class T> struct Probe_##NAME : T, Fallback_##NAME {};                           \
    template <class T> concept Has_##NAME = !requires { &Probe_##NAME<T>::NAME; };
KRITVA_SET_HAS_MEMBER(remove)
KRITVA_SET_HAS_MEMBER(erase)
KRITVA_SET_HAS_MEMBER(clear)
KRITVA_SET_HAS_MEMBER(merge)
KRITVA_SET_HAS_MEMBER(insert)
KRITVA_SET_HAS_MEMBER(unite)
KRITVA_SET_HAS_MEMBER(priority)
KRITVA_SET_HAS_MEMBER(sort)
KRITVA_SET_HAS_MEMBER(subscribe)
KRITVA_SET_HAS_MEMBER(notify)
KRITVA_SET_HAS_MEMBER(instance)
KRITVA_SET_HAS_MEMBER(resolve)
KRITVA_SET_HAS_MEMBER(discover)
KRITVA_SET_HAS_MEMBER(lookup_by_name)
KRITVA_SET_HAS_MEMBER(find_by_name)
template <class T> concept HasForbiddenOperation = Has_remove<T> || Has_erase<T> || Has_clear<T> || Has_merge<T> || Has_insert<T> || Has_unite<T> ||
                                                    Has_priority<T> || Has_sort<T> || Has_subscribe<T> || Has_notify<T> || Has_instance<T> ||
                                                    Has_resolve<T> || Has_discover<T> || Has_lookup_by_name<T> || Has_find_by_name<T>;
struct Detector { void erase(); void notify(); };
static_assert(HasForbiddenOperation<Detector>);                                  // the detector is sensitive to what it forbids

template <class V> concept VersionLess = requires(V a, V b) { a < b; };
template <class V> concept VersionGreater = requires(V a, V b) { a > b; };
template <class V> concept VersionThreeWay = requires(V a, V b) { a <=> b; };

Capability cap(std::uint64_t id, const char* name, Version v = {}) { return Capability{CapabilityId{id}, name, v}; }

std::vector<std::uint64_t> ids_in_order(const CapabilitySet& s) {
    std::vector<std::uint64_t> out;
    for (const Capability& c : s.all()) out.push_back(c.id.value());
    return out;
}

void test_shape_and_absence_of_registry_style_operations() {
    static_assert(std::is_default_constructible_v<CapabilitySet> && std::is_copy_constructible_v<CapabilitySet> &&
                  std::is_copy_assignable_v<CapabilitySet> && std::is_move_constructible_v<CapabilitySet> && std::is_move_assignable_v<CapabilitySet>);
    static_assert(std::is_same_v<decltype(&CapabilitySet::add), void (CapabilitySet::*)(Capability)>);                   // add never fails and validates nothing
    static_assert(std::is_same_v<decltype(&CapabilitySet::contains), bool (CapabilitySet::*)(CapabilityId) const noexcept>);
    static_assert(std::is_same_v<decltype(&CapabilitySet::find), const Capability* (CapabilitySet::*)(CapabilityId) const noexcept>);
    static_assert(std::is_same_v<decltype(&CapabilitySet::size), std::size_t (CapabilitySet::*)() const noexcept>);
    static_assert(std::is_same_v<decltype(&CapabilitySet::empty), bool (CapabilitySet::*)() const noexcept>);
    static_assert(std::is_same_v<decltype(std::declval<const CapabilitySet&>().all()), const std::vector<Capability>&>);
    static_assert(!HasForbiddenOperation<CapabilitySet>);                        // a set only grows: no registry, locator, merge, removal or notification
}

void test_a_default_set_is_empty_and_looks_up_nothing() {
    const CapabilitySet s;
    assert(s.empty() && s.size() == 0 && s.all().empty());
    assert(!s.contains(CapabilityId{1}) && s.find(CapabilityId{1}) == nullptr);
    assert(!s.contains(CapabilityId{}) && s.find(CapabilityId{}) == nullptr);
}

void test_at_most_one_entry_per_identity_with_replacement_in_place() {
    CapabilitySet s;
    s.add(cap(5, "five", Version{1, 0, 0}));
    s.add(cap(3, "three", Version{1, 0, 0}));
    s.add(cap(9, "nine", Version{1, 0, 0}));
    assert(ids_in_order(s) == (std::vector<std::uint64_t>{5, 3, 9}));
    s.add(cap(3, "three-v2", Version{2, 0, 0}));                                  // replaces in place: position kept, size unchanged
    s.add(cap(5, "five-v2", Version{2, 1, 0}));
    assert(s.size() == 3 && ids_in_order(s) == (std::vector<std::uint64_t>{5, 3, 9}));
    assert(s.find(CapabilityId{3})->name == "three-v2" && s.find(CapabilityId{3})->version == (Version{2, 0, 0}));
    assert(s.find(CapabilityId{5})->name == "five-v2" && s.find(CapabilityId{5})->version == (Version{2, 1, 0}));
    assert(s.find(CapabilityId{9})->name == "nine");                              // untouched
    s.add(cap(7, "seven"));                                                       // a new identity appends
    assert(s.size() == 4 && ids_in_order(s) == (std::vector<std::uint64_t>{5, 3, 9, 7}));
    for (int i = 0; i < 5; ++i) s.add(cap(9, "nine-again", Version{static_cast<std::uint32_t>(i), 0, 0}));
    assert(s.size() == 4 && s.find(CapabilityId{9})->version == (Version{4, 0, 0}));   // latest wins, no history
}

void test_observable_order_is_first_insertion_and_deterministic() {
    const auto build = [](const std::vector<std::uint64_t>& adds) {
        CapabilitySet s;
        for (const std::uint64_t id : adds) s.add(cap(id, "x"));
        return s;
    };
    const std::vector<std::uint64_t> sequence{40, 10, 30, 10, 20, 40, 50};
    const CapabilitySet a = build(sequence), b = build(sequence);
    assert(ids_in_order(a) == ids_in_order(b));                                   // the same sequence gives the same state, every time
    assert(ids_in_order(a) == (std::vector<std::uint64_t>{40, 10, 30, 20, 50}));  // first insertion order, not sorted
    const CapabilitySet reversed = build({50, 20, 30, 10, 40});                   // same contents, another order: another observable order
    assert(reversed.size() == a.size() && ids_in_order(reversed) == (std::vector<std::uint64_t>{50, 20, 30, 10, 40}));
    for (const std::uint64_t id : {10ull, 20ull, 30ull, 40ull, 50ull}) assert(reversed.contains(CapabilityId{id}) && a.contains(CapabilityId{id}));
}

void test_names_and_versions_may_repeat_across_identities_and_only_identity_is_the_key() {
    CapabilitySet s;
    s.add(cap(1, "same", Version{1, 0, 0}));
    s.add(cap(2, "same", Version{1, 0, 0}));
    s.add(cap(3, "same", Version{1, 0, 0}));
    assert(s.size() == 3);
    for (const std::uint64_t id : {1ull, 2ull, 3ull}) assert(s.contains(CapabilityId{id}));
    assert(!s.contains(CapabilityId{4}) && s.find(CapabilityId{4}) == nullptr);   // not found by name or version
    s.add(cap(2, "renamed", Version{3, 3, 3}));
    assert(s.size() == 3 && s.find(CapabilityId{1})->name == "same" && s.find(CapabilityId{2})->name == "renamed");
}

void test_the_invalid_identity_is_just_another_key_and_stays_distinct_from_a_valid_one() {
    CapabilitySet s;
    s.add(cap(0, "no-identity"));
    s.add(cap(1, "one"));
    s.add(cap(0, "no-identity-again"));                                           // the same key: replaced in place, still one entry
    assert(s.size() == 2 && s.find(CapabilityId{})->name == "no-identity-again" && s.find(CapabilityId{1})->name == "one");
    assert(ids_in_order(s) == (std::vector<std::uint64_t>{0, 1}));
}

void test_find_points_at_the_stored_entry_and_all_is_the_same_storage() {
    CapabilitySet s;
    s.add(cap(1, "a"));
    s.add(cap(2, "b"));
    assert(s.find(CapabilityId{1}) == &s.all()[0] && s.find(CapabilityId{2}) == &s.all()[1]);
    s.add(cap(1, "a2"));                                                          // a mutation: re-find (earlier pointers must not be retained)
    assert(s.find(CapabilityId{1}) == &s.all()[0] && s.find(CapabilityId{1})->name == "a2");
}

void test_a_copy_is_an_independent_snapshot_and_nothing_refers_into_its_source() {
    CapabilitySet original;
    original.add(cap(1, std::string(300, 'n').c_str(), Version{1, 0, 0}));
    original.add(cap(2, "two"));
    CapabilitySet copy = original;
    original.add(cap(1, "changed", Version{9, 9, 9}));
    original.add(cap(3, "three"));
    assert(copy.size() == 2 && copy.find(CapabilityId{1})->name == std::string(300, 'n') && copy.find(CapabilityId{1})->version == (Version{1, 0, 0}));
    assert(!copy.contains(CapabilityId{3}));
    copy.add(cap(4, "four"));
    assert(!original.contains(CapabilityId{4}));
    CapabilitySet assigned;
    assigned = copy;
    copy.add(cap(5, "five"));
    assert(!assigned.contains(CapabilityId{5}) && assigned.size() == 3);
    auto heap = std::make_unique<CapabilitySet>(original);
    CapabilitySet survivor = *heap;
    heap.reset();                                                                 // the source is gone: the snapshot is unaffected (ASan checks the rest)
    assert(survivor.size() == original.size() && survivor.find(CapabilityId{1})->name == "changed");
    CapabilitySet moved = std::move(survivor);
    assert(moved.find(CapabilityId{3})->name == "three");
}

void test_version_is_provided_contract_metadata_with_no_ordering_and_no_effect_on_lookup() {
    static_assert(!VersionLess<Version> && !VersionGreater<Version> && !VersionThreeWay<Version>);   // Core gives a capability version no ordering
    CapabilitySet s;
    s.add(cap(1, "x", Version{5, 0, 0}));
    s.add(cap(1, "x", Version{1, 0, 0}));                                         // an older version added later still replaces: no maximum, no merge
    assert(s.find(CapabilityId{1})->version == (Version{1, 0, 0}));
    s.add(cap(1, "x", Version{99, 99, 99}));
    assert(s.find(CapabilityId{1})->version == (Version{99, 99, 99}));
    const Capability defaulted;
    assert(defaulted.version == (Version{0, 0, 0}));                              // no defaulting or interpretation of a missing version
    static_assert(std::is_same_v<decltype(Capability::version), ConfigurationVersion>);   // the TYPE is shared; the documented meanings are not interchangeable
}

void test_matching_ignores_the_version_and_the_name() {
    for (const Version provided : {Version{0, 0, 0}, Version{1, 2, 3}, Version{99, 99, 99}}) {
        ReferencePlatform::Config config;
        config.capabilities.add(cap(10, "label", provided));
        ReferencePlatform platform(config);
        const platform::PlatformContext context(platform);
        platform::PlatformRequirements requirements;
        assert(requirements.add_capability(CapabilityId{10}, platform::Requirement::REQUIRED));
        assert(platform::check_required(requirements, context));                  // satisfied whatever the version: Core does not range-match
    }
}

} // namespace

int main() {
    test_shape_and_absence_of_registry_style_operations();
    test_a_default_set_is_empty_and_looks_up_nothing();
    test_at_most_one_entry_per_identity_with_replacement_in_place();
    test_observable_order_is_first_insertion_and_deterministic();
    test_names_and_versions_may_repeat_across_identities_and_only_identity_is_the_key();
    test_the_invalid_identity_is_just_another_key_and_stays_distinct_from_a_valid_one();
    test_find_points_at_the_stored_entry_and_all_is_the_same_storage();
    test_a_copy_is_an_independent_snapshot_and_nothing_refers_into_its_source();
    test_version_is_provided_contract_metadata_with_no_ordering_and_no_effect_on_lookup();
    test_matching_ignores_the_version_and_the_name();
    return 0;
}
