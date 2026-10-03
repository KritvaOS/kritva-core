//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : platform_requirements_test.cpp
// Description : PlatformRequirements, evaluate() and check_required() contract tests.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-013
// API         : CORE-TEST-PLATFORM-REQUIREMENTS
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#include <cassert>
#include <cstdint>
#include <string>
#include <type_traits>

#include <kritva/core/core.hpp>

#include "../contract/reference_adapter.hpp"

using namespace kritva::core;
using namespace kritva::core::platform;
using kritva::core::platform::contract::ReferenceAdapter;

namespace {

constexpr Requirement REQ = Requirement::REQUIRED;
constexpr Requirement OPT = Requirement::OPTIONAL;

CapabilitySet make_capabilities() {
    CapabilitySet set;
    set.add(Capability{CapabilityId{100}, "gpio", Version{1, 0, 0}});
    set.add(Capability{CapabilityId{101}, "can-bus", Version{2, 1, 0}});
    return set;
}

// Counts every call made to the adapter so evaluation's lack of side effects is observable.
class SpyAdapter final : public ReferenceAdapter {
public:
    explicit SpyAdapter(Provides provides = {}) : ReferenceAdapter(PlatformInfo{"spy", Version{1, 0, 0}}, provides, make_capabilities()) {}
    [[nodiscard]] const PlatformInfo& info() const noexcept override { ++info_calls; return ReferenceAdapter::info(); }
    [[nodiscard]] IScheduler* scheduler() const noexcept override { ++service_calls; return ReferenceAdapter::scheduler(); }
    [[nodiscard]] time::IClock* clock() const noexcept override { ++service_calls; return ReferenceAdapter::clock(); }
    [[nodiscard]] time::ITimer* timer() const noexcept override { ++service_calls; return ReferenceAdapter::timer(); }
    [[nodiscard]] IWatchdog* watchdog() const noexcept override { ++service_calls; return ReferenceAdapter::watchdog(); }
    [[nodiscard]] CapabilitySet capabilities() const override { ++capability_calls; return ReferenceAdapter::capabilities(); }
    mutable int info_calls{0}, service_calls{0}, capability_calls{0};
    time::contract::ReferenceTimer& tim() const { return *static_cast<time::contract::ReferenceTimer*>(ReferenceAdapter::timer()); }
    contract::ReferenceScheduler& sched() const { return *static_cast<contract::ReferenceScheduler*>(ReferenceAdapter::scheduler()); }
    contract::ReferenceWatchdog& dog() const { return *static_cast<contract::ReferenceWatchdog*>(ReferenceAdapter::watchdog()); }
};

void expect_invalid(const Result<void>& result) {
    assert(!result);
    assert(result.error().code == ErrorCode::INVALID_ARGUMENT);
}

void test_shape() {
    static_assert(std::is_same_v<std::underlying_type_t<Requirement>, std::uint8_t>);
    static_assert(Requirement::REQUIRED != Requirement::OPTIONAL);
    static_assert(std::is_copy_constructible_v<PlatformRequirements> && std::is_copy_assignable_v<PlatformRequirements>);
    static_assert(std::is_default_constructible_v<PlatformRequirements>);
    const PlatformRequirements none;
    assert(none.empty() && none.services().empty() && none.capabilities().empty());
    const PlatformRequirementReport report;
    assert(report.satisfied() && report.complete());                         // nothing missing
}

void test_declaring_keeps_order_and_levels() {
    PlatformRequirements requirements;
    assert(requirements.add_service(PlatformService::TIMER, OPT));
    assert(requirements.add_service(PlatformService::SCHEDULER, REQ));
    assert(requirements.add_capability(CapabilityId{102}, OPT));
    assert(requirements.add_capability(CapabilityId{100}, REQ));
    assert(!requirements.empty());
    assert(requirements.services().size() == 2 && requirements.capabilities().size() == 2);
    assert(requirements.services()[0].service == PlatformService::TIMER && requirements.services()[0].level == OPT);
    assert(requirements.services()[1].service == PlatformService::SCHEDULER && requirements.services()[1].level == REQ);
    assert(requirements.capabilities()[0].id == CapabilityId{102} && requirements.capabilities()[0].level == OPT);
    assert(requirements.capabilities()[1].id == CapabilityId{100} && requirements.capabilities()[1].level == REQ);
}

void test_invalid_declarations_are_rejected_atomically() {
    PlatformRequirements requirements;
    assert(requirements.add_service(PlatformService::CLOCK, REQ));
    assert(requirements.add_capability(CapabilityId{100}, OPT));
    expect_invalid(requirements.add_service(static_cast<PlatformService>(4), REQ));      // unknown service
    expect_invalid(requirements.add_service(static_cast<PlatformService>(200), OPT));
    expect_invalid(requirements.add_service(PlatformService::TIMER, static_cast<Requirement>(2)));   // unknown level
    expect_invalid(requirements.add_capability(CapabilityId{}, REQ));                    // the invalid identity
    expect_invalid(requirements.add_capability(CapabilityId{101}, static_cast<Requirement>(9)));
    assert(requirements.services().size() == 1 && requirements.capabilities().size() == 1);   // nothing was added
    assert(requirements.services()[0].service == PlatformService::CLOCK && requirements.capabilities()[0].id == CapabilityId{100});
    assert(requirements.add_service(PlatformService::TIMER, REQ));                       // still usable afterwards
}

void test_duplicates_are_decided_by_identity_not_by_level() {
    PlatformRequirements requirements;
    assert(requirements.add_service(PlatformService::SCHEDULER, REQ));
    expect_invalid(requirements.add_service(PlatformService::SCHEDULER, REQ));           // same item, same level
    expect_invalid(requirements.add_service(PlatformService::SCHEDULER, OPT));           // same item, other level: still rejected
    assert(requirements.services().size() == 1 && requirements.services()[0].level == REQ);   // no silent upgrade or downgrade
    assert(requirements.add_capability(CapabilityId{100}, OPT));
    expect_invalid(requirements.add_capability(CapabilityId{100}, OPT));
    expect_invalid(requirements.add_capability(CapabilityId{100}, REQ));
    assert(requirements.capabilities().size() == 1 && requirements.capabilities()[0].level == OPT);
    assert(requirements.add_service(PlatformService::CLOCK, OPT));                       // a different item is independent
    assert(requirements.add_capability(CapabilityId{101}, REQ));
}

void test_empty_requirements_are_satisfied_by_everything() {
    const PlatformRequirements none;
    ReferenceAdapter full(PlatformInfo{"p", Version{}}, {});
    ReferenceAdapter bare(PlatformInfo{"p", Version{}}, {false, false, false, false});
    for (const PlatformContext& context : {PlatformContext(full), PlatformContext(bare), PlatformContext()}) {
        const PlatformRequirementReport report = evaluate(none, context);
        assert(report.satisfied() && report.complete());
        assert(check_required(none, context));
    }
}

void test_required_service_present_and_absent() {
    PlatformRequirements requirements;
    assert(requirements.add_service(PlatformService::SCHEDULER, REQ));
    assert(requirements.add_service(PlatformService::WATCHDOG, REQ));
    ReferenceAdapter both(PlatformInfo{"p", Version{}}, {true, true, true, true});
    const PlatformRequirementReport met = evaluate(requirements, PlatformContext(both));
    assert(met.satisfied() && met.complete());
    assert(check_required(requirements, PlatformContext(both)));

    ReferenceAdapter missing_watchdog(PlatformInfo{"p", Version{}}, {true, true, true, false});
    const PlatformRequirementReport report = evaluate(requirements, PlatformContext(missing_watchdog));
    assert(!report.satisfied() && !report.complete());
    assert(report.missing_required_services.size() == 1 && report.missing_required_services[0].service == PlatformService::WATCHDOG);
    assert(report.missing_optional_services.empty() && report.missing_required_capabilities.empty());
    const auto checked = check_required(requirements, PlatformContext(missing_watchdog));
    assert(!checked && checked.error().code == ErrorCode::UNSUPPORTED);
    assert(checked.error().severity == ErrorSeverity::ERROR);
    assert(checked.error().message.find("watchdog") != std::string::npos);
}

void test_check_required_names_each_kind_of_missing_service() {
    const struct { PlatformService service; ReferenceAdapter::Provides provides; const char* name; } cases[] = {
        {PlatformService::SCHEDULER, {false, true, true, true}, "scheduler"},
        {PlatformService::CLOCK,     {true, false, true, true}, "clock"},
        {PlatformService::TIMER,     {true, true, false, true}, "timer"},
        {PlatformService::WATCHDOG,  {true, true, true, false}, "watchdog"},
    };
    for (const auto& c : cases) {
        PlatformRequirements requirements;
        assert(requirements.add_service(c.service, REQ));
        ReferenceAdapter adapter(PlatformInfo{"p", Version{}}, c.provides);
        const auto checked = check_required(requirements, PlatformContext(adapter));
        assert(!checked && checked.error().code == ErrorCode::UNSUPPORTED);
        assert(checked.error().message.find(c.name) != std::string::npos);
        for (const char* other : {"scheduler", "clock", "timer", "watchdog"}) {       // and names only that service
            if (std::string(other) != c.name) assert(checked.error().message.find(other) == std::string::npos);
        }
    }
}

void test_optional_service_absent_is_not_an_error() {
    PlatformRequirements requirements;
    assert(requirements.add_service(PlatformService::CLOCK, REQ));
    assert(requirements.add_service(PlatformService::TIMER, OPT));
    ReferenceAdapter no_timer(PlatformInfo{"p", Version{}}, {true, true, false, true});
    const PlatformRequirementReport report = evaluate(requirements, PlatformContext(no_timer));
    assert(report.satisfied());                                                           // only an optional item is missing
    assert(!report.complete());
    assert(report.missing_optional_services.size() == 1 && report.missing_optional_services[0].service == PlatformService::TIMER);
    assert(report.missing_required_services.empty());
    assert(check_required(requirements, PlatformContext(no_timer)));                      // optional items never fail check_required
}

void test_capability_matching_is_by_identity_only() {
    PlatformRequirements requirements;
    assert(requirements.add_capability(CapabilityId{100}, REQ));
    assert(requirements.add_capability(CapabilityId{101}, REQ));
    assert(requirements.add_capability(CapabilityId{102}, OPT));
    ReferenceAdapter adapter(PlatformInfo{"gpio", Version{100, 101, 102}}, {}, make_capabilities());
    const PlatformRequirementReport report = evaluate(requirements, PlatformContext(adapter));
    assert(report.satisfied());
    assert(report.missing_required_capabilities.empty());
    assert(report.missing_optional_capabilities.size() == 1 && report.missing_optional_capabilities[0].id == CapabilityId{102});

    // A platform that merely LOOKS like it has the capability (name, version numbers) provides nothing.
    ReferenceAdapter lookalike(PlatformInfo{"gpio can-bus 100 101", Version{100, 101, 100}}, {});
    const PlatformRequirementReport miss = evaluate(requirements, PlatformContext(lookalike));
    assert(!miss.satisfied() && miss.missing_required_capabilities.size() == 2);
    // A capability's own version and name are irrelevant: only its identity counts.
    CapabilitySet renamed;
    renamed.add(Capability{CapabilityId{100}, "something-else", Version{9, 9, 9}});
    renamed.add(Capability{CapabilityId{101}, "", Version{}});
    ReferenceAdapter other(PlatformInfo{"x", Version{}}, {}, renamed);
    assert(evaluate(requirements, PlatformContext(other)).satisfied());
    const auto checked = check_required(requirements, PlatformContext(lookalike));
    assert(!checked && checked.error().code == ErrorCode::UNSUPPORTED && checked.error().message.find("100") != std::string::npos);
}

void test_unsupported_requirement_reports_every_missing_item_in_declaration_order() {
    PlatformRequirements requirements;
    assert(requirements.add_service(PlatformService::TIMER, REQ));
    assert(requirements.add_service(PlatformService::CLOCK, OPT));
    assert(requirements.add_service(PlatformService::SCHEDULER, REQ));
    assert(requirements.add_capability(CapabilityId{300}, REQ));
    assert(requirements.add_capability(CapabilityId{100}, REQ));
    assert(requirements.add_capability(CapabilityId{301}, OPT));
    ReferenceAdapter none(PlatformInfo{"p", Version{}}, {false, false, false, false}, make_capabilities());
    const PlatformRequirementReport report = evaluate(requirements, PlatformContext(none));
    assert(report.missing_required_services.size() == 2);
    assert(report.missing_required_services[0].service == PlatformService::TIMER);        // declaration order, not enumerator order
    assert(report.missing_required_services[1].service == PlatformService::SCHEDULER);
    assert(report.missing_optional_services.size() == 1 && report.missing_optional_services[0].service == PlatformService::CLOCK);
    assert(report.missing_required_capabilities.size() == 1 && report.missing_required_capabilities[0].id == CapabilityId{300});
    assert(report.missing_optional_capabilities.size() == 1 && report.missing_optional_capabilities[0].id == CapabilityId{301});
    // check_required names the FIRST missing required declaration: services come before capabilities.
    const auto checked = check_required(requirements, PlatformContext(none));
    assert(!checked && checked.error().code == ErrorCode::UNSUPPORTED && checked.error().message.find("timer") != std::string::npos);

    // With every service present the first missing required item is a capability.
    ReferenceAdapter services_only(PlatformInfo{"p", Version{}}, {true, true, true, true}, make_capabilities());
    const auto cap = check_required(requirements, PlatformContext(services_only));
    assert(!cap && cap.error().message.find("300") != std::string::npos);
}

void test_unattached_context_provides_nothing() {
    PlatformRequirements required;
    assert(required.add_service(PlatformService::CLOCK, REQ));
    PlatformRequirements optional_only;
    assert(optional_only.add_service(PlatformService::CLOCK, OPT));
    assert(optional_only.add_capability(CapabilityId{100}, OPT));
    const PlatformContext none;
    assert(!evaluate(required, none).satisfied());
    assert(!check_required(required, none) && check_required(required, none).error().code == ErrorCode::UNSUPPORTED);
    const PlatformRequirementReport report = evaluate(optional_only, none);
    assert(report.satisfied() && !report.complete());                                     // all-OPTIONAL: satisfied, but everything is missing
    assert(report.missing_optional_services.size() == 1 && report.missing_optional_capabilities.size() == 1);
    assert(check_required(optional_only, none));
}

void test_evaluation_has_no_side_effects_and_is_deterministic() {
    SpyAdapter adapter;
    adapter.sched().create_calls = 0;
    PlatformRequirements requirements;
    assert(requirements.add_service(PlatformService::SCHEDULER, REQ));
    assert(requirements.add_service(PlatformService::TIMER, OPT));
    assert(requirements.add_service(PlatformService::WATCHDOG, REQ));
    assert(requirements.add_capability(CapabilityId{100}, REQ));
    assert(requirements.add_capability(CapabilityId{999}, OPT));
    const PlatformContext context(adapter);
    const std::size_t services_before = requirements.services().size(), capabilities_before = requirements.capabilities().size();

    const PlatformRequirementReport first = evaluate(requirements, context);
    assert(adapter.service_calls == 3);                                                   // supports(): once per declared service
    assert(adapter.capability_calls == 1);                                                // one capability snapshot for the whole evaluation
    assert(adapter.info_calls == 0);                                                      // the platform's identity is never consulted
    assert(adapter.activations() == 0);                                                   // nothing was started
    assert(adapter.sched().create_calls == 0 && adapter.sched().start_calls == 0 && adapter.sched().stop_calls == 0);
    assert(adapter.tim().start_calls == 0 && adapter.tim().stop_calls == 0);
    assert(adapter.dog().start_calls == 0 && adapter.dog().kick_calls == 0 && adapter.dog().stop_calls == 0);
    assert(requirements.services().size() == services_before && requirements.capabilities().size() == capabilities_before);   // unchanged

    const PlatformRequirementReport second = evaluate(requirements, context);
    assert(first.satisfied() == second.satisfied() && first.complete() == second.complete());
    assert(first.missing_optional_capabilities.size() == second.missing_optional_capabilities.size());
    assert(first.missing_optional_capabilities.size() == 1 && first.missing_optional_capabilities[0].id == CapabilityId{999});

    assert(check_required(requirements, context));
    assert(adapter.info_calls == 0 && adapter.activations() == 0);

    // Only capabilities declared: no service is queried at all; only services declared: no capability snapshot.
    SpyAdapter other;
    PlatformRequirements caps_only;
    assert(caps_only.add_capability(CapabilityId{100}, REQ));
    (void)evaluate(caps_only, PlatformContext(other));
    assert(other.service_calls == 0 && other.capability_calls == 1);
    PlatformRequirements services_only;
    assert(services_only.add_service(PlatformService::CLOCK, REQ));
    (void)evaluate(services_only, PlatformContext(other));
    assert(other.service_calls == 1 && other.capability_calls == 1);
}

void test_copies_of_requirements_are_independent_values() {
    PlatformRequirements original;
    assert(original.add_service(PlatformService::CLOCK, REQ));
    PlatformRequirements copy = original;
    assert(copy.add_service(PlatformService::TIMER, OPT));
    assert(original.services().size() == 1 && copy.services().size() == 2);
    expect_invalid(copy.add_service(PlatformService::CLOCK, OPT));                        // the copy kept the declaration
}

void test_existing_capability_set_semantics_are_preserved() {
    CapabilitySet set = make_capabilities();
    assert(set.contains(CapabilityId{100}) && !set.contains(CapabilityId{102}));
    assert(set.find(CapabilityId{101}) != nullptr && set.find(CapabilityId{101})->name == "can-bus");
    assert(set.size() == 2 && !set.empty());
}

} // namespace

int main() {
    test_shape();
    test_declaring_keeps_order_and_levels();
    test_invalid_declarations_are_rejected_atomically();
    test_duplicates_are_decided_by_identity_not_by_level();
    test_empty_requirements_are_satisfied_by_everything();
    test_required_service_present_and_absent();
    test_check_required_names_each_kind_of_missing_service();
    test_optional_service_absent_is_not_an_error();
    test_capability_matching_is_by_identity_only();
    test_unsupported_requirement_reports_every_missing_item_in_declaration_order();
    test_unattached_context_provides_nothing();
    test_evaluation_has_no_side_effects_and_is_deterministic();
    test_copies_of_requirements_are_independent_values();
    test_existing_capability_set_semantics_are_preserved();
    return 0;
}
