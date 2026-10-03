//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : platform_adapter_test.cpp
// Description : IPlatformAdapter contract tests against a reference implementation.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-008
// API         : CORE-TEST-ADAPTER-CONTRACT
//
// Author      : KritvaOS Core Team
// Created     : 04-10-2026
//==============================================================================

#include <cassert>
#include <string>
#include <type_traits>

#include <kritva/core/core.hpp>

#include "../contract/reference_adapter.hpp"

using namespace kritva::core;
using namespace kritva::core::platform;
using kritva::core::platform::contract::ReferenceAdapter;

namespace {

constexpr PlatformService ALL[] = {PlatformService::SCHEDULER, PlatformService::CLOCK, PlatformService::TIMER, PlatformService::WATCHDOG};

PlatformInfo make_info() { return PlatformInfo{"reference", Version{1, 2, 3}}; }

CapabilitySet make_capabilities() {
    CapabilitySet set;
    set.add(Capability{CapabilityId{100}, "gpio", Version{1, 0, 0}});
    set.add(Capability{CapabilityId{101}, "can-bus", Version{2, 1, 0}});
    return set;
}

bool provided(const IPlatformAdapter& adapter, PlatformService service) {
    switch (service) {
        case PlatformService::SCHEDULER: return adapter.scheduler() != nullptr;
        case PlatformService::CLOCK:     return adapter.clock() != nullptr;
        case PlatformService::TIMER:     return adapter.timer() != nullptr;
        case PlatformService::WATCHDOG:  return adapter.watchdog() != nullptr;
    }
    return false;
}

void test_shape() {
    static_assert(std::is_abstract_v<IPlatformAdapter>);
    static_assert(std::has_virtual_destructor_v<IPlatformAdapter>);
    static_assert(std::is_same_v<std::underlying_type_t<PlatformService>, std::uint8_t>);
    static_assert(noexcept(std::declval<const IPlatformAdapter&>().info()));
    static_assert(noexcept(std::declval<const IPlatformAdapter&>().supports(PlatformService::CLOCK)));
    static_assert(noexcept(std::declval<const IPlatformAdapter&>().scheduler()));

    const PlatformInfo info;                                  // a default identity is empty, not invented
    assert(info.name.empty() && info.version == Version{});
}

void test_identity_is_stable() {
    const ReferenceAdapter adapter(make_info(), {});
    const PlatformInfo& first = adapter.info();
    assert(first.name == "reference" && first.version == (Version{1, 2, 3}));
    assert(&adapter.info() == &first);                        // same object for the adapter's life
    assert(adapter.info().name == first.name && adapter.info().version == first.version);
    assert(adapter.info().version.to_string() == "1.2.3");
}

void test_adapters_are_independent_objects() {
    // No shared global state: two adapters never alias identity or services (no singleton, no locator).
    const ReferenceAdapter first(PlatformInfo{"first", Version{1, 0, 0}}, {});
    const ReferenceAdapter second(PlatformInfo{"second", Version{2, 0, 0}}, {});
    assert(&first.info() != &second.info());
    assert(first.info().name == "first" && second.info().name == "second");
    assert(first.info().version == (Version{1, 0, 0}) && second.info().version == (Version{2, 0, 0}));
    assert(first.scheduler() != second.scheduler() && first.clock() != second.clock());
    assert(first.timer() != second.timer() && first.watchdog() != second.watchdog());
}

void test_services_present_and_stable() {
    const ReferenceAdapter adapter(make_info(), {});
    for (const PlatformService service : ALL) assert(adapter.supports(service));
    // The same adapter-owned object on every call.
    assert(adapter.scheduler() == adapter.scheduler() && adapter.clock() == adapter.clock());
    assert(adapter.timer() == adapter.timer() && adapter.watchdog() == adapter.watchdog());
    // The services are usable through the contract they implement.
    assert(adapter.clock()->now().domain() == ClockDomain::MONOTONIC);
    assert(adapter.watchdog()->start(Duration::from_milliseconds(10)));
    assert(adapter.watchdog()->stop());
}

void test_supports_agrees_with_accessors_in_both_directions() {
    for (int mask = 0; mask < 16; ++mask) {
        const ReferenceAdapter::Provides provides{(mask & 1) != 0, (mask & 2) != 0, (mask & 4) != 0, (mask & 8) != 0};
        const ReferenceAdapter adapter(make_info(), provides);
        assert(adapter.supports(PlatformService::SCHEDULER) == provides.scheduler);
        assert(adapter.supports(PlatformService::CLOCK) == provides.clock);
        assert(adapter.supports(PlatformService::TIMER) == provides.timer);
        assert(adapter.supports(PlatformService::WATCHDOG) == provides.watchdog);
        for (const PlatformService service : ALL) assert(adapter.supports(service) == provided(adapter, service));
    }
}

void test_unsupported_services_are_null_never_placeholders() {
    const ReferenceAdapter adapter(make_info(), {false, true, false, false});
    assert(adapter.scheduler() == nullptr && adapter.timer() == nullptr && adapter.watchdog() == nullptr);
    assert(adapter.clock() != nullptr);
    assert(!adapter.supports(PlatformService::SCHEDULER));
    const ReferenceAdapter none(make_info(), {false, false, false, false});
    for (const PlatformService service : ALL) assert(!none.supports(service));
    assert(!none.supports(static_cast<PlatformService>(200)));   // unknown enumerator: not supported
}

void test_capabilities_are_a_deterministic_owned_snapshot() {
    const ReferenceAdapter adapter(make_info(), {}, make_capabilities());
    CapabilitySet first = adapter.capabilities();
    const CapabilitySet second = adapter.capabilities();
    assert(first.size() == 2 && second.size() == 2);
    assert(first.all()[0].id == CapabilityId{100} && first.all()[1].id == CapabilityId{101});   // same entries, same order
    for (std::size_t i = 0; i < first.size(); ++i) {
        assert(first.all()[i].id == second.all()[i].id);
        assert(first.all()[i].name == second.all()[i].name && first.all()[i].version == second.all()[i].version);
    }
    // The caller owns the copy: changing it never changes what the adapter reports.
    first.add(Capability{CapabilityId{999}, "caller-extra", Version{}});
    assert(adapter.capabilities().size() == 2 && !adapter.capabilities().contains(CapabilityId{999}));
    assert(first.contains(CapabilityId{100}) && !adapter.capabilities().contains(CapabilityId{102}));   // absence is reported as absence
}

void test_capabilities_do_not_have_to_mirror_services() {
    const ReferenceAdapter adapter(make_info(), {false, false, false, false});
    assert(adapter.capabilities().empty());                   // nothing claimed, nothing invented
}

void test_queries_do_not_activate_anything() {
    ReferenceAdapter adapter(make_info(), {}, make_capabilities());
    for (int i = 0; i < 3; ++i) {
        (void)adapter.info(); (void)adapter.capabilities();
        for (const PlatformService service : ALL) (void)adapter.supports(service);
        (void)adapter.scheduler(); (void)adapter.clock(); (void)adapter.timer(); (void)adapter.watchdog();
    }
    assert(adapter.activations() == 0);
}

void test_ownership_is_the_integrators() {
    // Core holds only the non-owning references handed out; destroying the adapter is the integrator's act.
    IScheduler* scheduler = nullptr;
    {
        const ReferenceAdapter adapter(make_info(), {});
        scheduler = adapter.scheduler();
        assert(scheduler != nullptr);
    }
    (void)scheduler;                                           // dangling by contract after the adapter ends; never dereferenced
    static_assert(!std::is_pointer_v<decltype(PlatformInfo::name)>);
}

} // namespace

int main() {
    test_shape();
    test_identity_is_stable();
    test_adapters_are_independent_objects();
    test_services_present_and_stable();
    test_supports_agrees_with_accessors_in_both_directions();
    test_unsupported_services_are_null_never_placeholders();
    test_capabilities_are_a_deterministic_owned_snapshot();
    test_capabilities_do_not_have_to_mirror_services();
    test_queries_do_not_activate_anything();
    test_ownership_is_the_integrators();
    return 0;
}
