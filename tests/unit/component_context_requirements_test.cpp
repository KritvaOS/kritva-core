//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component_context_requirements_test.cpp
// Description : ComponentContext requirement binding: evaluate() and check_required().
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-CTX-004
// API         : CORE-TEST-COMPONENT-CONTEXT-REQUIREMENTS
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#include <cassert>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <kritva/core/core.hpp>

#include "../contract/reference_adapter.hpp"
#include "../platform/reference_platform.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;
using namespace kritva::core::platform;
using kritva::core::platform::contract::ReferenceAdapter;

namespace {

constexpr Requirement REQ = Requirement::REQUIRED;
constexpr Requirement OPT = Requirement::OPTIONAL;

// Member-detection concepts: a missing member makes the concept false instead of a hard error.
template<class T> concept CanAddService = requires(T& c) { c.add_service(PlatformService::CLOCK, Requirement::REQUIRED); };
template<class T> concept CanAddCapability = requires(T& c) { c.add_capability(CapabilityId{1}, Requirement::REQUIRED); };
template<class T> concept CanListRequirements = requires(const T& c) { c.requirements(); };
template<class T> concept CanStoreRequirements = requires(T& c, const PlatformRequirements& r) { c.set_requirements(r); };
template<class T> concept CanDeclare = requires(T& c) { c.declare(PlatformService::CLOCK); };

ComponentInfo make_info(std::uint64_t id = 7) {
    auto info = ComponentInfo::create(ComponentId{id}, "worker", Version{1, 0, 0});
    assert(info.has_value());
    return std::move(info).value();
}

CapabilitySet capabilities_100_101() {
    CapabilitySet set;
    set.add(Capability{CapabilityId{100}, "gpio", Version{1, 0, 0}});
    set.add(Capability{CapabilityId{101}, "can-bus", Version{2, 1, 0}});
    return set;
}

ReferenceAdapter::Provides provides_from(int mask) {
    return ReferenceAdapter::Provides{(mask & 1) != 0, (mask & 2) != 0, (mask & 4) != 0, (mask & 8) != 0};
}

class SpyAdapter final : public ReferenceAdapter {
public:
    explicit SpyAdapter(Provides provides = {}) : ReferenceAdapter(PlatformInfo{"spy", Version{1, 0, 0}}, provides, capabilities_100_101()) {}
    [[nodiscard]] const PlatformInfo& info() const noexcept override { ++info_calls; return ReferenceAdapter::info(); }
    [[nodiscard]] IScheduler* scheduler() const noexcept override { ++service_calls; return ReferenceAdapter::scheduler(); }
    [[nodiscard]] time::IClock* clock() const noexcept override { ++service_calls; return ReferenceAdapter::clock(); }
    [[nodiscard]] time::ITimer* timer() const noexcept override { ++service_calls; return ReferenceAdapter::timer(); }
    [[nodiscard]] IWatchdog* watchdog() const noexcept override { ++service_calls; return ReferenceAdapter::watchdog(); }
    [[nodiscard]] CapabilitySet capabilities() const override { ++capability_calls; return ReferenceAdapter::capabilities(); }
    mutable int info_calls{0}, service_calls{0}, capability_calls{0};
};

PlatformRequirements make_requirements(std::initializer_list<std::pair<PlatformService, Requirement>> services,
                                       std::initializer_list<std::pair<std::uint64_t, Requirement>> capabilities) {
    PlatformRequirements requirements;
    for (const auto& [service, level] : services) assert(requirements.add_service(service, level));
    for (const auto& [id, level] : capabilities) assert(requirements.add_capability(CapabilityId{id}, level));
    return requirements;
}

bool same_report(const PlatformRequirementReport& a, const PlatformRequirementReport& b) {
    auto same_services = [](const std::vector<ServiceRequirement>& x, const std::vector<ServiceRequirement>& y) {
        if (x.size() != y.size()) return false;
        for (std::size_t i = 0; i < x.size(); ++i) if (x[i].service != y[i].service || x[i].level != y[i].level) return false;
        return true;
    };
    auto same_caps = [](const std::vector<CapabilityRequirement>& x, const std::vector<CapabilityRequirement>& y) {
        if (x.size() != y.size()) return false;
        for (std::size_t i = 0; i < x.size(); ++i) if (x[i].id != y[i].id || x[i].level != y[i].level) return false;
        return true;
    };
    return same_services(a.missing_required_services, b.missing_required_services) && same_services(a.missing_optional_services, b.missing_optional_services)
        && same_caps(a.missing_required_capabilities, b.missing_required_capabilities) && same_caps(a.missing_optional_capabilities, b.missing_optional_capabilities);
}

// ---- the surface: bind, do not store -----------------------------------------------------------------

void test_the_binding_surface_and_statelessness() {
    static_assert(std::is_same_v<decltype(std::declval<const ComponentContext&>().evaluate(std::declval<const PlatformRequirements&>())), PlatformRequirementReport>);
    static_assert(std::is_same_v<decltype(std::declval<const ComponentContext&>().check_required(std::declval<const PlatformRequirements&>())), Result<void>>);
    static_assert(sizeof(ComponentContext) == 2 * sizeof(void*));                  // stateless: still two pointers
    static_assert(std::is_trivially_destructible_v<ComponentContext>);
    // The context declares and stores no requirements: that stays with the component and the R0.5 type.
    static_assert(!CanAddService<ComponentContext> && !CanAddCapability<ComponentContext>);
    static_assert(!CanListRequirements<ComponentContext> && !CanStoreRequirements<ComponentContext> && !CanDeclare<ComponentContext>);
}

// ---- evaluate(): the R0.5 report, unchanged -----------------------------------------------------------------

void test_evaluate_is_exactly_the_r05_report_for_every_platform_and_requirement_set() {
    const std::vector<PlatformRequirements> sets = {
        make_requirements({}, {}),
        make_requirements({{PlatformService::SCHEDULER, REQ}}, {}),
        make_requirements({{PlatformService::TIMER, OPT}, {PlatformService::SCHEDULER, REQ}, {PlatformService::WATCHDOG, REQ}, {PlatformService::CLOCK, OPT}}, {}),
        make_requirements({{PlatformService::CLOCK, REQ}}, {{100, REQ}, {102, OPT}}),
        make_requirements({}, {{999, OPT}, {101, REQ}, {100, OPT}, {5, REQ}}),
    };
    const ComponentInfo info = make_info();
    std::size_t compared = 0;
    for (int mask = 0; mask < 16; ++mask) {
        for (const bool with_capabilities : {true, false}) {
            ReferenceAdapter adapter(PlatformInfo{"p", Version{}}, provides_from(mask), with_capabilities ? capabilities_100_101() : CapabilitySet{});
            const PlatformContext platform(adapter);
            const ComponentContext bound(info, platform);
            for (const PlatformRequirements& requirements : sets) {
                const PlatformRequirementReport expected = evaluate(requirements, platform);     // R0.5
                assert(same_report(bound.evaluate(requirements), expected));
                assert(bound.evaluate(requirements).satisfied() == expected.satisfied() && bound.evaluate(requirements).complete() == expected.complete());
                ++compared;
            }
        }
    }
    // An unattached platform provides nothing, bound or not (an unbound context has no platform either).
    const ComponentContext bare(info);
    const ComponentContext unbound;
    for (const PlatformRequirements& requirements : sets) {
        assert(same_report(bare.evaluate(requirements), evaluate(requirements, PlatformContext{})));
        assert(same_report(unbound.evaluate(requirements), evaluate(requirements, PlatformContext{})));
    }
    assert(!bare.evaluate(sets[1]).satisfied() && bare.evaluate(sets[0]).satisfied());           // empty satisfied, a required service missing
    assert(compared == 16u * 2u * sets.size());
}

void test_evaluate_reports_every_missing_item_in_declaration_order() {
    const ComponentInfo info = make_info();
    ReferenceAdapter none(PlatformInfo{"p", Version{}}, {false, false, false, false}, capabilities_100_101());
    const ComponentContext context(info, PlatformContext(none));
    const PlatformRequirements requirements = make_requirements(
        {{PlatformService::TIMER, REQ}, {PlatformService::CLOCK, OPT}, {PlatformService::SCHEDULER, REQ}}, {{300, REQ}, {100, REQ}, {301, OPT}});
    const PlatformRequirementReport report = context.evaluate(requirements);
    assert(report.missing_required_services.size() == 2 && report.missing_required_services[0].service == PlatformService::TIMER && report.missing_required_services[1].service == PlatformService::SCHEDULER);
    assert(report.missing_optional_services.size() == 1 && report.missing_optional_services[0].service == PlatformService::CLOCK);
    assert(report.missing_required_capabilities.size() == 1 && report.missing_required_capabilities[0].id == CapabilityId{300});
    assert(report.missing_optional_capabilities.size() == 1 && report.missing_optional_capabilities[0].id == CapabilityId{301});
}

// ---- check_required(): R05 result, attributed when bound ---------------------------------------------------------

void test_check_required_follows_the_independent_oracle_for_every_combination() {
    struct Need { std::vector<std::pair<PlatformService, Requirement>> services; std::vector<std::pair<std::uint64_t, Requirement>> capabilities; };
    const std::vector<Need> needs = {
        {{{PlatformService::SCHEDULER, REQ}}, {}},
        {{{PlatformService::SCHEDULER, REQ}, {PlatformService::CLOCK, REQ}, {PlatformService::TIMER, REQ}, {PlatformService::WATCHDOG, REQ}}, {}},
        {{{PlatformService::TIMER, OPT}}, {{100, REQ}}},
        {{{PlatformService::WATCHDOG, OPT}, {PlatformService::CLOCK, OPT}}, {{999, OPT}}},
    };
    const ComponentInfo info = make_info(21);
    std::size_t failing = 0, passing = 0;
    for (int mask = 0; mask < 16; ++mask) {
        const ReferenceAdapter::Provides p = provides_from(mask);
        for (const bool has_capability : {true, false}) {
            ReferenceAdapter adapter(PlatformInfo{"p", Version{}}, p, has_capability ? capabilities_100_101() : CapabilitySet{});
            const ComponentContext context(info, PlatformContext(adapter));
            for (const Need& need : needs) {
                PlatformRequirements requirements;
                for (const auto& [service, level] : need.services) assert(requirements.add_service(service, level));
                for (const auto& [id, level] : need.capabilities) assert(requirements.add_capability(CapabilityId{id}, level));
                auto provided = [&](PlatformService s) {
                    switch (s) { case PlatformService::SCHEDULER: return p.scheduler; case PlatformService::CLOCK: return p.clock;
                                 case PlatformService::TIMER: return p.timer; case PlatformService::WATCHDOG: return p.watchdog; }
                    return false;
                };
                bool expected = true;                                                     // the oracle, independent of the implementation
                for (const auto& [service, level] : need.services) if (level == REQ && !provided(service)) expected = false;
                for (const auto& [id, level] : need.capabilities) if (level == REQ && !(id == 100 && has_capability)) expected = false;
                const auto checked = context.check_required(requirements);
                assert(checked.has_value() == expected);
                assert(context.evaluate(requirements).satisfied() == expected);
                if (!expected) {
                    ++failing;
                    assert(checked.error().code == ErrorCode::UNSUPPORTED && checked.error().severity == ErrorSeverity::ERROR);
                    assert(checked.error().source == ComponentId{21});                    // attributed to the component
                } else ++passing;
            }
        }
    }
    assert(failing > 20 && passing > 20);
}

void test_the_check_error_is_the_r05_error_plus_the_source() {
    const ComponentInfo info = make_info(7);
    ReferenceAdapter none(PlatformInfo{"p", Version{}}, {false, true, true, true}, capabilities_100_101());
    const PlatformContext platform(none);
    const PlatformRequirements requirements = make_requirements({{PlatformService::SCHEDULER, REQ}}, {{300, REQ}});
    const Error r05 = check_required(requirements, platform).error();
    const Error r06 = ComponentContext(info, platform).check_required(requirements).error();
    assert(!r05.source.valid() && r06.source == ComponentId{7});
    assert(r06.code == r05.code && r06.severity == r05.severity && r06.message == r05.message && r06.timestamp == r05.timestamp);   // only the source differs
    assert(r06.message.find("scheduler") != std::string::npos);                          // services come before capabilities
    // The first missing required declaration is the one named: with the scheduler present it is the capability.
    ReferenceAdapter with_scheduler(PlatformInfo{"p", Version{}}, {true, true, true, true}, capabilities_100_101());
    const Error cap = ComponentContext(info, PlatformContext(with_scheduler)).check_required(requirements).error();
    assert(cap.message.find("300") != std::string::npos && cap.source == ComponentId{7});
    // An unbound context returns the R0.5 error unchanged.
    const Error from_unbound = ComponentContext{}.check_required(requirements).error();
    assert(!from_unbound.source.valid() && from_unbound.code == ErrorCode::UNSUPPORTED);
    assert(from_unbound.message == check_required(requirements, PlatformContext{}).error().message);
    // A satisfied check returns plain success (nothing to attribute).
    assert(ComponentContext(info, PlatformContext(with_scheduler)).check_required(make_requirements({{PlatformService::SCHEDULER, REQ}}, {{100, REQ}})));
    assert(ComponentContext(info).check_required(make_requirements({}, {})));           // an empty set is satisfied by anything
    assert(ComponentContext(info).check_required(make_requirements({{PlatformService::CLOCK, OPT}}, {{5, OPT}})));   // all-OPTIONAL by nothing
}

// ---- identity only, no side effects, stateless -----------------------------------------------------------------------

void test_matching_is_by_identity_only() {
    const ComponentInfo info = make_info();
    ReferenceAdapter lookalike(PlatformInfo{"gpio can-bus", Version{100, 101, 100}}, {});
    const PlatformRequirements requirements = make_requirements({}, {{100, REQ}, {101, REQ}});
    const ComponentContext context(info, PlatformContext(lookalike));
    assert(!context.check_required(requirements) && context.evaluate(requirements).missing_required_capabilities.size() == 2);   // name and version are not capabilities
    // A capability's own name and version are irrelevant: only its identity counts.
    CapabilitySet renamed;
    renamed.add(Capability{CapabilityId{100}, "something-else", Version{9, 9, 9}});
    renamed.add(Capability{CapabilityId{101}, "", Version{}});
    ReferenceAdapter other(PlatformInfo{"x", Version{}}, {}, renamed);
    assert(ComponentContext(info, PlatformContext(other)).check_required(requirements));
    // Name lengths and versions used as identities are not capabilities either.
    ReferenceAdapter numeric(PlatformInfo{"abcde", Version{7, 3, 1}}, {}, capabilities_100_101());
    const PlatformRequirements by_number = make_requirements({}, {{5, REQ}, {7, REQ}, {3, OPT}, {1, OPT}});
    assert(ComponentContext(info, PlatformContext(numeric)).evaluate(by_number).missing_required_capabilities.size() == 2);
}

void test_binding_is_a_side_effect_free_query_and_stateless() {
    SpyAdapter adapter;
    const ComponentInfo info = make_info();
    const ComponentContext context(info, PlatformContext(adapter));
    PlatformRequirements requirements = make_requirements({{PlatformService::SCHEDULER, REQ}, {PlatformService::TIMER, OPT}, {PlatformService::WATCHDOG, REQ}}, {{100, REQ}, {999, OPT}});
    const std::size_t services = requirements.services().size(), capabilities = requirements.capabilities().size();
    const PlatformRequirementReport first = context.evaluate(requirements);
    assert(adapter.service_calls == 3);                                                       // supports(): once per declared service
    assert(adapter.capability_calls == 1);                                                    // one capability snapshot for the whole evaluation
    assert(adapter.info_calls == 0);                                                          // the platform's identity is never consulted
    assert(adapter.activations() == 0);                                                       // nothing was started
    assert(requirements.services().size() == services && requirements.capabilities().size() == capabilities);   // the requirements are untouched
    const PlatformRequirementReport second = context.evaluate(requirements);
    assert(same_report(first, second));                                                       // deterministic
    assert(context.check_required(requirements));
    assert(adapter.info_calls == 0 && adapter.activations() == 0);
    // Services-only and capabilities-only requirements query only what they need.
    SpyAdapter other;
    const ComponentContext other_context(info, PlatformContext(other));
    (void)other_context.evaluate(make_requirements({}, {{100, REQ}}));
    assert(other.service_calls == 0 && other.capability_calls == 1);
    (void)other_context.evaluate(make_requirements({{PlatformService::CLOCK, REQ}}, {}));
    assert(other.service_calls == 1 && other.capability_calls == 1);
    // The requirement-declaration rules are R0.5's, untouched by the context.
    PlatformRequirements declared;
    assert(declared.add_service(PlatformService::CLOCK, REQ));
    const auto duplicate = declared.add_service(PlatformService::CLOCK, OPT);
    assert(!duplicate && duplicate.error().code == ErrorCode::INVALID_ARGUMENT);
    const auto invalid = declared.add_capability(CapabilityId{}, REQ);
    assert(!invalid && invalid.error().code == ErrorCode::INVALID_ARGUMENT);
}

// ---- a component gates itself with its context and the Runtime sees an ordinary failure ------------------------------

class Gated final : public Component {
public:
    Gated(ComponentInfo info, PlatformContext platform) : Component(std::move(info)), context_(*this, platform) {
        (void)needs_.add_service(PlatformService::SCHEDULER, REQ);
        (void)needs_.add_capability(CapabilityId{100}, REQ);
        (void)needs_.add_service(PlatformService::WATCHDOG, OPT);                          // optional: never a failure
    }
    Result<void> configure(const Configuration&) override { return Result<void>::success(); }
    Result<void> initialize() override { return context_.check_required(needs_); }         // returned directly: the source is already this component
    Result<void> start() override { return Result<void>::success(); }
    Result<void> stop() override { return Result<void>::success(); }
    Result<void> shutdown() override { return Result<void>::success(); }
    LifecycleState lifecycle_state() const noexcept override { return LifecycleState::UNKNOWN; }
    Status status() const override { return Status{}; }
    Health health() const override { return Health{}; }
    CapabilitySet capabilities() const override { return CapabilitySet{}; }
private:
    const ComponentContext context_;
    PlatformRequirements needs_;                                                            // the component keeps its own requirements
};

void test_a_component_gates_itself_with_its_context() {
    {   // satisfied: scheduler and capability 100 (the optional watchdog is missing and does not matter)
        ReferenceAdapter ok(PlatformInfo{"p", Version{}}, {true, true, true, false}, capabilities_100_101());
        RuntimeManager runtime;
        auto info = ComponentInfo::create(ComponentId{4}, "gated");
        Gated component(std::move(info).value(), PlatformContext(ok));
        assert(runtime.register_component(component) && runtime.initialize() && runtime.start());
        assert(runtime.state() == LifecycleState::RUNNING && runtime.statistics().error_count.value() == 0);
        assert(runtime.stop() && runtime.shutdown());
    }
    for (const bool missing_scheduler : {true, false}) {   // not satisfied: the first failing step is initialize()
        ReferenceAdapter bad(PlatformInfo{"p", Version{}}, {!missing_scheduler, true, true, true}, missing_scheduler ? capabilities_100_101() : CapabilitySet{});
        RuntimeManager runtime;
        auto info = ComponentInfo::create(ComponentId{4}, "gated");
        Gated component(std::move(info).value(), PlatformContext(bad));
        assert(runtime.register_component(component));
        const auto initialized = runtime.initialize();
        assert(!initialized && initialized.error().code == ErrorCode::UNSUPPORTED && initialized.error().source == ComponentId{4});
        assert(initialized.error().message.find(missing_scheduler ? "scheduler" : "100") != std::string::npos);
        assert(runtime.state() == LifecycleState::FAULT && runtime.statistics().error_count.value() == 1 && runtime.statistics().retry_count.value() == 0);
        assert(runtime.reset() && runtime.state() == LifecycleState::STOPPED);
    }
}

} // namespace

int main() {
    test_the_binding_surface_and_statelessness();
    test_evaluate_is_exactly_the_r05_report_for_every_platform_and_requirement_set();
    test_evaluate_reports_every_missing_item_in_declaration_order();
    test_check_required_follows_the_independent_oracle_for_every_combination();
    test_the_check_error_is_the_r05_error_plus_the_source();
    test_matching_is_by_identity_only();
    test_binding_is_a_side_effect_free_query_and_stateless();
    test_a_component_gates_itself_with_its_context();
    return 0;
}
