//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : reference_capability_test.cpp
// Description : Tests of the test-only capability harness and its reusable conformance checks.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-CAP-010
// API         : CORE-TEST-CAPABILITY-HARNESS
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#include <cassert>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <kritva/core/core.hpp>

#include "../runtime/capability_conformance.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;
using namespace kritva::core::runtime::contract;
namespace conf = kritva::core::runtime::conformance;

namespace {

ComponentInfo make_info(std::uint64_t id) {
    auto info = ComponentInfo::create(ComponentId{id}, "p");
    assert(info);
    return std::move(info).value();
}

conf::SetFactory real_set() { return [] { return std::unique_ptr<SetFixture>(new RealSetFixture()); }; }
conf::SetFactory broken_set(SetDefect d) { return [d] { return std::unique_ptr<SetFixture>(new BrokenSetFixture(d)); }; }
RequirementFactory real_requirements() {
    return [](const CapabilitySet& p, const std::string& n, Version v) { return std::unique_ptr<RequirementFixture>(new RealRequirementFixture(p, n, v)); };
}
RequirementFactory broken_requirements(RequirementDefect d) {
    return [d](const CapabilitySet& p, const std::string& n, Version v) { return std::unique_ptr<RequirementFixture>(new BrokenRequirementFixture(d, p, n, v)); };
}

CapabilitySet declaration() {
    CapabilitySet s;
    s.add(Capability{CapabilityId{10}, "a", Version{1, 0, 0}});
    s.add(Capability{CapabilityId{20}, "b", Version{2, 0, 0}});
    return s;
}

struct SetCase { SetDefect defect; const char* expected; };
const SetCase kSetDefects[] = {
    {SetDefect::DUPLICATES_ALLOWED, "present more than once"},
    {SetDefect::REPLACE_MOVES_TO_END, "first-insertion order"},
    {SetDefect::REPLACE_KEEPS_OLD_VERSION, "did not replace the version"},
    {SetDefect::REPLACE_KEEPS_HIGHER_VERSION, "versions must not be ordered"},
    {SetDefect::ORDER_SORTED, "first-insertion order"},
    {SetDefect::ORDER_NEWEST_FIRST, "first-insertion order"},
    {SetDefect::FIND_BY_NAME, "found by its name"},
    {SetDefect::FIND_IGNORES_EMPTY_NAME, "empty name cannot be found"},
    {SetDefect::REJECTS_INVALID_ID, "invalid identity was dropped"},
    {SetDefect::COPY_SHARES_STORAGE, "showed up in its copy"},
    {SetDefect::SIZE_COUNTS_NAMES, "repeat across identities"},
    {SetDefect::ADD_HAS_HIDDEN_LIMIT, "stopped growing"},
    {SetDefect::STARTS_NON_EMPTY, "new set is not empty"},
    {SetDefect::REPLACE_KEEPS_OLD_NAME, "did not replace the name"},
    {SetDefect::LOSES_APPEND_AFTER_REPLACE, "new identity was not appended"},
    {SetDefect::REPLACE_KEEPS_LOWER_VERSION, "did not replace the version"},
    {SetDefect::REPLACE_IGNORED_AFTER_FIRST, "replacement version was not stored"},
    {SetDefect::FINDS_ABSENT_IDS, "absent identity was found"},
    {SetDefect::INVALID_ID_KEEPS_FIRST_NAME, "not an ordinary key"},
    {SetDefect::COPY_WRITES_THROUGH_TO_SOURCE, "copy showed up in its source"},
    {SetDefect::NONDETERMINISTIC_ORDER, "different state"},
};

void test_the_real_set_conforms_and_every_set_defect_is_detected_by_its_clause() {
    assert(conf::check_set_contract(real_set()).empty());
    for (const SetCase& c : kSetDefects) {
        const std::string violation = conf::check_set_contract(broken_set(c.defect));
        assert(!violation.empty());
        assert(violation.find(c.expected) != std::string::npos);
    }
    assert(conf::check_set_contract(broken_set(SetDefect::NONE)).empty());           // the independent defect-free model conforms too
}

struct ProviderCase { ProviderDefect defect; const char* expected; };
const ProviderCase kProviderDefects[] = {
    {ProviderDefect::CHANGES_EVERY_CALL, "differ in size"},
    {ProviderDefect::CHANGES_CONTENT_NOT_SIZE, "differ in content"},
    {ProviderDefect::DROPS_ONE_ENTRY, "different number of entries"},
    {ProviderDefect::SWAPS_VERSION, "not the expected one"},
    {ProviderDefect::QUERY_CHANGES_LIFECYCLE, "changed the lifecycle state"},
    {ProviderDefect::DECLARATION_FOLLOWS_LIFECYCLE, "follows the lifecycle"},
    {ProviderDefect::PUBLISHES_INVALID_ID, "published an invalid capability identity"},
};

std::string provider_violation(ProviderDefect defect, bool at_ready) {
    ReferenceCapabilityProvider provider(make_info(1), declaration(), defect);
    if (at_ready) assert(provider.initialize());
    CapabilitySet expected = declaration();
    if (defect == ProviderDefect::PUBLISHES_INVALID_ID) expected.add(Capability{CapabilityId{}, "no-identity", Version{}});   // so the size matches and the identity clause is the one that speaks
    return conf::check_provider_contract(provider, expected, [&] { return (at_ready || provider.initialize()) && provider.start(); });
}

// A returned CapabilitySet is a by-value std::vector-backed snapshot, so "a change to a returned snapshot shows up in the
// provider" cannot be produced by any in-tree provider without undefined behavior; the clause stays in the check for
// out-of-tree providers and is guaranteed here by the return type.
void test_the_reference_provider_conforms_and_every_provider_defect_is_detected() {
    static_assert(std::is_same_v<decltype(std::declval<const Component&>().capabilities()), CapabilitySet>);
    assert(provider_violation(ProviderDefect::NONE, false).empty());
    for (const ProviderCase& c : kProviderDefects) {
        const bool at_ready = c.defect == ProviderDefect::QUERY_CHANGES_LIFECYCLE;     // that defect only shows from READY
        const std::string violation = provider_violation(c.defect, at_ready);
        assert(!violation.empty());
        assert(violation.find(c.expected) != std::string::npos);
    }
}

void test_the_reference_provider_exposes_a_controlled_observable_snapshot() {
    ReferenceCapabilityProvider p(make_info(2), declaration());
    assert(p.queries == 0);
    const CapabilitySet a = p.capabilities();
    assert(p.queries == 1 && a.size() == 2 && a.contains(CapabilityId{10}) && a.contains(CapabilityId{20}));
    p.declared_.add(Capability{CapabilityId{30}, "c", Version{}});                    // the provider declares something else: a later snapshot shows it, an earlier one does not
    assert(p.capabilities().contains(CapabilityId{30}) && !a.contains(CapabilityId{30}) && p.queries == 2);
    assert(p.lifecycle_state() == LifecycleState::UNKNOWN);                           // and the plain reference lifecycle is untouched
    assert(p.initialize() && p.start() && p.capabilities().size() == 3);
}

struct RequirementCase { RequirementDefect defect; const char* expected; };
const RequirementCase kRequirementDefects[] = {
    {RequirementDefect::MATCH_BY_NAME, "differ from the identity oracle"},
    {RequirementDefect::MATCH_BY_VERSION, "differ from the identity oracle"},
    {RequirementDefect::MATCH_BY_PROVIDER_NAME, "differ from the identity oracle"},
    {RequirementDefect::MATCH_BY_PROVIDER_VERSION, "differ from the identity oracle"},
    {RequirementDefect::SNAPSHOT_PER_ITEM, "exactly one provider snapshot"},
    {RequirementDefect::SNAPSHOT_EVEN_IF_NONE, "no capability is required"},
    {RequirementDefect::SIDE_EFFECT, "side effect"},
    {RequirementDefect::OPTIONAL_DECIDES, "overall check differs"},
    {RequirementDefect::REQUIRED_IGNORED, "overall check differs"},
    {RequirementDefect::DUPLICATES_BY_PAIR, "decided by (identity, level)"},
    {RequirementDefect::ACCEPTS_INVALID_ID, "invalid identity"},
    {RequirementDefect::NON_ATOMIC_REJECTION, "rejected duplicate changed"},
    {RequirementDefect::REPORT_ORDER_REVERSED, "declaration order"},
    {RequirementDefect::NONDETERMINISTIC, "two evaluations of the same input differ"},
    {RequirementDefect::OPTIONAL_NEVER_REPORTED, "missing optional items differ"},
    {RequirementDefect::INVALID_REJECTION_CHANGES_STATE, "rejected declaration changed"},
    {RequirementDefect::ACCEPTS_EXACT_DUPLICATE, "duplicate declaration was accepted"},
};

void test_the_real_evaluator_conforms_and_every_requirement_defect_is_detected_by_its_clause() {
    assert(conf::check_requirement_contract(real_requirements()).empty());
    for (const RequirementCase& c : kRequirementDefects) {
        const std::string violation = conf::check_requirement_contract(broken_requirements(c.defect));
        assert(!violation.empty());
        assert(violation.find(c.expected) != std::string::npos);
    }
    assert(conf::check_requirement_contract(broken_requirements(RequirementDefect::NONE)).empty());   // the independent defect-free model conforms too
}

void test_the_harness_is_deterministic_and_needs_no_platform_specific_code() {
    for (int i = 0; i < 3; ++i) {                                                    // repeated runs give identical verdicts
        assert(conf::check_set_contract(real_set()).empty());
        assert(conf::check_requirement_contract(real_requirements()).empty());
        assert(conf::check_set_contract(broken_set(SetDefect::ORDER_SORTED)) == conf::check_set_contract(broken_set(SetDefect::ORDER_SORTED)));
    }
}

} // namespace

int main() {
    test_the_real_set_conforms_and_every_set_defect_is_detected_by_its_clause();
    test_the_reference_provider_conforms_and_every_provider_defect_is_detected();
    test_the_reference_provider_exposes_a_controlled_observable_snapshot();
    test_the_real_evaluator_conforms_and_every_requirement_defect_is_detected_by_its_clause();
    test_the_harness_is_deterministic_and_needs_no_platform_specific_code();
    return 0;
}
