//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : platform_context_test.cpp
// Description : PlatformContext contract tests (non-owning view over IPlatformAdapter).
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-012
// API         : CORE-TEST-PLATFORM-CONTEXT
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

#include <kritva/core/core.hpp>

#include "../contract/reference_adapter.hpp"
#include "../contract/reference_component.hpp"

using namespace kritva::core;
using namespace kritva::core::platform;
using kritva::core::platform::contract::ReferenceAdapter;
using kritva::core::runtime::contract::ReferenceComponent;

namespace {

constexpr PlatformService ALL[] = {PlatformService::SCHEDULER, PlatformService::CLOCK, PlatformService::TIMER, PlatformService::WATCHDOG};

CapabilitySet make_capabilities() {
    CapabilitySet set;
    set.add(Capability{CapabilityId{100}, "gpio", Version{1, 0, 0}});
    set.add(Capability{CapabilityId{101}, "can-bus", Version{2, 1, 0}});
    return set;
}

// An adapter that records every call, so "the context caches nothing and has no side effects" is observable.
class CountingAdapter final : public ReferenceAdapter {
public:
    explicit CountingAdapter(Provides provides = {}) : ReferenceAdapter(PlatformInfo{"counting", Version{3, 4, 5}}, provides, make_capabilities()) {}
    [[nodiscard]] const PlatformInfo& info() const noexcept override { ++calls; return ReferenceAdapter::info(); }
    [[nodiscard]] IScheduler* scheduler() const noexcept override { ++calls; return ReferenceAdapter::scheduler(); }
    [[nodiscard]] time::IClock* clock() const noexcept override { ++calls; return ReferenceAdapter::clock(); }
    [[nodiscard]] time::ITimer* timer() const noexcept override { ++calls; return ReferenceAdapter::timer(); }
    [[nodiscard]] IWatchdog* watchdog() const noexcept override { ++calls; return ReferenceAdapter::watchdog(); }
    [[nodiscard]] CapabilitySet capabilities() const override { ++calls; return ReferenceAdapter::capabilities(); }
    mutable int calls{0};
};

void test_shape_is_a_small_copyable_view() {
    static_assert(std::is_copy_constructible_v<PlatformContext> && std::is_copy_assignable_v<PlatformContext>);
    static_assert(std::is_nothrow_default_constructible_v<PlatformContext>);
    static_assert(std::is_nothrow_copy_constructible_v<PlatformContext>);
    static_assert(std::is_trivially_destructible_v<PlatformContext>);          // owns nothing, destroys nothing
    static_assert(sizeof(PlatformContext) == sizeof(void*));                    // one pointer, no registry, no cache
    static_assert(std::is_nothrow_constructible_v<PlatformContext, IPlatformAdapter&>);
    static_assert(std::is_nothrow_constructible_v<PlatformContext, IPlatformAdapter*>);
    static_assert(!std::is_convertible_v<IPlatformAdapter&, PlatformContext>);   // explicit constructors only
    static_assert(!std::is_convertible_v<IPlatformAdapter*, PlatformContext>);
    static_assert(noexcept(std::declval<const PlatformContext&>().scheduler()));
    static_assert(noexcept(std::declval<const PlatformContext&>().supports(PlatformService::CLOCK)));
    static_assert(noexcept(std::declval<const PlatformContext&>().info()));
    static_assert(noexcept(std::declval<const PlatformContext&>().attached()));
}

void test_unattached_context_reports_nothing() {
    const PlatformContext none;
    assert(!none.attached());
    assert(none.info() == nullptr);
    assert(none.scheduler() == nullptr && none.clock() == nullptr && none.timer() == nullptr && none.watchdog() == nullptr);
    for (const PlatformService service : ALL) assert(!none.supports(service));
    assert(!none.supports(static_cast<PlatformService>(200)));
    assert(none.capabilities().empty());
    assert(!none.has_capability(CapabilityId{100}));

    const PlatformContext from_null(static_cast<IPlatformAdapter*>(nullptr));   // RuntimeManager::platform() when none is attached
    assert(!from_null.attached() && from_null.info() == nullptr && from_null.scheduler() == nullptr);
    const PlatformContext from_nullptr(nullptr);
    assert(!from_nullptr.attached());
}

void test_attached_context_forwards_the_adapter() {
    CountingAdapter adapter;
    const PlatformContext context(adapter);
    assert(context.attached());
    assert(context.info() == &adapter.info());                             // the adapter's own identity object
    assert(context.info()->name == "counting" && context.info()->version == (Version{3, 4, 5}));
    assert(context.scheduler() == adapter.scheduler());                    // the same adapter-owned objects, not copies
    assert(context.clock() == adapter.clock());
    assert(context.timer() == adapter.timer());
    assert(context.watchdog() == adapter.watchdog());
    for (const PlatformService service : ALL) assert(context.supports(service));
    assert(!context.supports(static_cast<PlatformService>(4)) && !context.supports(static_cast<PlatformService>(200)));   // unknown: not supported
    assert(context.has_capability(CapabilityId{100}) && context.has_capability(CapabilityId{101}));
    assert(!context.has_capability(CapabilityId{102}));
    assert(context.capabilities().size() == 2);
    assert(context.clock()->now().domain() == ClockDomain::MONOTONIC);     // services are usable through their own contracts
}

void test_service_pointer_consistency_for_every_service_combination() {
    for (int mask = 0; mask < 16; ++mask) {
        const ReferenceAdapter::Provides provides{(mask & 1) != 0, (mask & 2) != 0, (mask & 4) != 0, (mask & 8) != 0};
        ReferenceAdapter adapter(PlatformInfo{"p", Version{}}, provides);
        const PlatformContext context(adapter);
        assert(context.supports(PlatformService::SCHEDULER) == (context.scheduler() != nullptr) && (context.scheduler() != nullptr) == provides.scheduler);
        assert(context.supports(PlatformService::CLOCK) == (context.clock() != nullptr) && (context.clock() != nullptr) == provides.clock);
        assert(context.supports(PlatformService::TIMER) == (context.timer() != nullptr) && (context.timer() != nullptr) == provides.timer);
        assert(context.supports(PlatformService::WATCHDOG) == (context.watchdog() != nullptr) && (context.watchdog() != nullptr) == provides.watchdog);
        for (const PlatformService service : ALL) assert(context.supports(service) == adapter.supports(service));   // identical to the adapter
        assert(context.scheduler() == context.scheduler() && context.timer() == context.timer());                    // stable
    }
}

void test_copies_share_the_adapter_and_owning_nothing() {
    CountingAdapter adapter;
    const PlatformContext original(adapter);
    PlatformContext copy = original;
    PlatformContext assigned;
    assigned = original;
    assert(copy.info() == original.info() && assigned.info() == original.info());
    assert(copy.timer() == original.timer() && assigned.watchdog() == original.watchdog());
    PlatformContext emptied = copy;
    emptied = PlatformContext{};                                           // rebinding a view never touches the adapter or other copies
    assert(!emptied.attached() && copy.attached() && original.attached());
    // Copying and destroying contexts has no effect on the adapter.
    adapter.calls = 0;
    {
        PlatformContext temporary(original);
        PlatformContext another(temporary);
        (void)another;
    }
    assert(adapter.calls == 0);
    assert(adapter.activations() == 0);
}

void test_lifetime_context_never_owns_or_destroys_the_adapter() {
    bool alive = true;
    struct Tracked final : ReferenceAdapter {
        explicit Tracked(bool& flag) : ReferenceAdapter(PlatformInfo{"tracked", Version{}}, {}), flag_(flag) {}
        ~Tracked() override { flag_ = false; }
        bool& flag_;
    };
    auto adapter = std::make_unique<Tracked>(alive);
    {
        const PlatformContext context(*adapter);
        const PlatformContext copy = context;
        assert(copy.attached());
    }                                                                       // every context is gone
    assert(alive);                                                          // the adapter is not destroyed by them
    adapter.reset();                                                        // the integrator owns it
    assert(!alive);
}

void test_the_context_caches_nothing_and_has_no_side_effects() {
    CountingAdapter adapter;
    const PlatformContext context(adapter);
    assert(adapter.calls == 0);                                             // construction does not probe the adapter
    (void)context.attached();
    assert(adapter.calls == 0);                                             // attached() is a pointer test only
    (void)context.scheduler();
    assert(adapter.calls == 1);                                             // one forwarded call per query
    (void)context.scheduler();
    assert(adapter.calls == 2);                                             // nothing is cached
    (void)context.supports(PlatformService::TIMER);
    assert(adapter.calls == 3);
    (void)context.info();
    assert(adapter.calls == 4);
    // The adapter changes its mind about nothing here, but the context follows the adapter, not a snapshot of it.
    ReferenceAdapter changing(PlatformInfo{"x", Version{}}, {});
    const PlatformContext view(changing);
    assert(view.scheduler() != nullptr);
    assert(adapter.activations() == 0 && changing.activations() == 0);      // no query started a task, timer or watchdog
}

void test_capabilities_are_identity_only_and_owned_by_the_caller() {
    ReferenceAdapter adapter(PlatformInfo{"gpio", Version{9, 9, 9}}, {}, make_capabilities());
    const PlatformContext context(adapter);
    CapabilitySet snapshot = context.capabilities();
    snapshot.add(Capability{CapabilityId{999}, "caller-extra", Version{}});
    assert(!context.has_capability(CapabilityId{999}));                     // the caller's copy never leaks back
    assert(context.capabilities().size() == 2);
    // The platform's name and version, and a capability's name or version, decide nothing.
    ReferenceAdapter named_like_a_capability(PlatformInfo{"can-bus", Version{2, 1, 0}}, {});
    assert(!PlatformContext(named_like_a_capability).has_capability(CapabilityId{101}));
    assert(PlatformContext(adapter).has_capability(CapabilityId{101}));
    assert(!context.has_capability(CapabilityId{}));                        // the invalid identity is never reported
    // Nothing about the platform's name or version is ever read as a capability identity.
    ReferenceAdapter five_chars(PlatformInfo{"abcde", Version{7, 3, 1}}, {}, make_capabilities());
    const PlatformContext numeric(five_chars);
    for (std::uint64_t id : {std::uint64_t{1}, std::uint64_t{3}, std::uint64_t{5}, std::uint64_t{7}}) assert(!numeric.has_capability(CapabilityId{id}));
    // A capability's version does not decide either: 100 is reported whatever its version is.
    CapabilitySet other_version;
    other_version.add(Capability{CapabilityId{100}, "renamed", Version{0, 0, 1}});
    ReferenceAdapter versioned(PlatformInfo{"v", Version{}}, {}, other_version);
    assert(PlatformContext(versioned).has_capability(CapabilityId{100}));
}

// A Runtime-attached context does not alter Runtime behavior (lightweight public-API integration check).
void test_runtime_attached_context_does_not_alter_runtime_behavior() {
    CountingAdapter adapter;
    runtime::RuntimeManager runtime;
    auto info = runtime::ComponentInfo::create(runtime::ComponentId{1}, "c");
    ReferenceComponent component(std::move(info).value());
    assert(runtime.register_component(component));
    assert(runtime.attach_platform(adapter));

    const PlatformContext before(runtime.platform());                       // built from the Runtime's nullable pointer
    assert(before.attached() && before.info() == &adapter.info());
    adapter.calls = 0;
    assert(runtime.initialize() && runtime.start());
    assert(runtime.state() == LifecycleState::RUNNING);
    assert(adapter.calls == 0);                                             // the Runtime never calls the adapter, with or without a context
    const PlatformContext during(runtime.platform());
    assert(during.scheduler() != nullptr && runtime.state() == LifecycleState::RUNNING);   // using it changes no Runtime state
    assert(runtime.statistics().sample_count.value() == 2);                 // initialize + start of one component, nothing more
    assert(runtime.stop() && runtime.shutdown());
    assert(adapter.activations() == 0);

    runtime::RuntimeManager bare;                                                    // no adapter: the context is simply unattached
    const PlatformContext none(bare.platform());
    assert(!none.attached());
}

} // namespace

int main() {
    test_shape_is_a_small_copyable_view();
    test_unattached_context_reports_nothing();
    test_attached_context_forwards_the_adapter();
    test_service_pointer_consistency_for_every_service_combination();
    test_copies_share_the_adapter_and_owning_nothing();
    test_lifetime_context_never_owns_or_destroys_the_adapter();
    test_the_context_caches_nothing_and_has_no_side_effects();
    test_capabilities_are_identity_only_and_owned_by_the_caller();
    test_runtime_attached_context_does_not_alter_runtime_behavior();
    return 0;
}
