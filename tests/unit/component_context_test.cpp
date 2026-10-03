//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component_context_test.cpp
// Description : ComponentContext shape, ownership and lifetime contract tests.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-CTX-001
// API         : CORE-TEST-COMPONENT-CONTEXT
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#include <cassert>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include <kritva/core/core.hpp>

#include "../contract/reference_adapter.hpp"
#include "../platform/reference_platform.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;
using namespace kritva::core::platform;
using kritva::core::platform::contract::ReferenceAdapter;
using kritva::core::platform::testing::LifetimeProbe;
using kritva::core::platform::testing::ReferencePlatform;

namespace {


// Member-detection concepts: a missing member makes the concept false instead of a hard error.
template<class T> concept CanBind = requires(T& c, const ComponentInfo& i) { c.bind(i); };
template<class T> concept CanSetPlatform = requires(T& c, PlatformContext p) { c.set_platform(p); };
template<class T> concept CanAttach = requires(T& c, PlatformContext p) { c.attach(p); };
template<class T> concept CanReset = requires(T& c) { c.reset(); };
template<class T> concept CanSetInfo = requires(T& c, const ComponentInfo& i) { c.set_info(i); };
template<class T> concept ExposesAdapter = requires(const T& c) { c.adapter(); };
template<class T> concept ExposesPlatformAdapter = requires(const T& c) { c.platform_adapter(); };
template<class T> concept ExposesRuntime = requires(const T& c) { c.runtime(); };
template<class T> concept ExposesRegistry = requires(const T& c) { c.registry(); };
template<class T> concept ExposesConfiguration = requires(const T& c) { c.configuration(); };
template<class T> concept ExposesStatistics = requires(const T& c) { c.statistics(); };
template<class T> concept ExposesHealth = requires(const T& c) { c.health(); };
template<class T> concept ExposesComponent = requires(const T& c) { c.component(); };
template<class T> concept CanFindById = requires(const T& c, ComponentId id) { c.find(id); };
template<class T> concept CanContextFor = requires(const T& c, ComponentId id) { c.context_for(id); };
template<class T> concept CanLookupByName = requires(const T& c, const char* name) { c.lookup(name); };

ComponentInfo make_info(std::uint64_t id = 7, const char* name = "worker", Version version = Version{1, 2, 3}) {
    auto info = ComponentInfo::create(ComponentId{id}, name, version);
    assert(info.has_value());
    return std::move(info).value();
}

ReferencePlatform::Config config(LifetimeProbe* probe = nullptr) {
    ReferencePlatform::Config c;
    c.probe = probe;
    return c;
}

void test_shape_is_two_pointers_and_trivially_destructible() {
    static_assert(sizeof(ComponentContext) == sizeof(const ComponentInfo*) + sizeof(PlatformContext));
    static_assert(sizeof(ComponentContext) == 2 * sizeof(void*));
    static_assert(std::is_trivially_destructible_v<ComponentContext>);
    static_assert(std::is_nothrow_default_constructible_v<ComponentContext>);
    static_assert(std::is_nothrow_copy_constructible_v<ComponentContext> && std::is_nothrow_move_constructible_v<ComponentContext>);
    static_assert(std::is_copy_constructible_v<ComponentContext>);
    static_assert(std::is_nothrow_constructible_v<ComponentContext, const ComponentInfo&>);
    static_assert(std::is_nothrow_constructible_v<ComponentContext, const ComponentInfo&, PlatformContext>);
    static_assert(noexcept(std::declval<const ComponentContext&>().bound()));
    static_assert(noexcept(std::declval<const ComponentContext&>().id()));
    static_assert(noexcept(std::declval<const ComponentContext&>().info()));
    static_assert(noexcept(std::declval<const ComponentContext&>().platform()));
}

void test_the_context_is_immutable_after_construction() {
    static_assert(!std::is_copy_assignable_v<ComponentContext> && !std::is_move_assignable_v<ComponentContext>);   // no rebinding by assignment
    // No setter, reset or rebinding member of any kind.
    static_assert(!CanBind<ComponentContext> && !CanSetPlatform<ComponentContext> && !CanAttach<ComponentContext>);
    static_assert(!CanReset<ComponentContext> && !CanSetInfo<ComponentContext>);
    // A temporary identity would dangle and is refused at compile time.
    static_assert(!std::is_constructible_v<ComponentContext, ComponentInfo>);
    static_assert(!std::is_constructible_v<ComponentContext, ComponentInfo, PlatformContext>);
    static_assert(std::is_constructible_v<ComponentContext, const ComponentInfo&>);
    static_assert(!std::is_convertible_v<const ComponentInfo&, ComponentContext>);   // binding is always an explicit act
}

void test_unbound_context_has_no_identity_and_no_platform() {
    const ComponentContext context;
    assert(!context.bound());
    assert(context.info() == nullptr);
    assert(!context.id().valid() && context.id() == ComponentId{});
    assert(!context.platform().attached());
    assert(context.platform().scheduler() == nullptr && context.platform().info() == nullptr);
    assert(context.platform().capabilities().empty());
}

void test_bound_context_reports_its_identity() {
    const ComponentInfo info = make_info(7, "worker", Version{1, 2, 3});
    const ComponentContext context(info);
    assert(context.bound());
    assert(context.info() == &info);                                          // the component's own, not a copy
    assert(context.id() == ComponentId{7} && context.id().valid());
    assert(context.info()->name() == "worker" && context.info()->version() == (Version{1, 2, 3}));
    assert(!context.platform().attached());                                   // identity without a platform is valid
    const ComponentInfo other = make_info(8, "other");
    assert(ComponentContext(other).id() == ComponentId{8} && ComponentContext(other).info() == &other);
}

void test_the_platform_view_is_the_r05_view_by_const_reference() {
    static_assert(std::is_same_v<decltype(std::declval<const ComponentContext&>().platform()), const PlatformContext&>);
    static_assert(std::is_same_v<decltype(std::declval<const ComponentContext&>().info()), const ComponentInfo*>);
    ReferencePlatform platform(config());
    const ComponentInfo info = make_info();
    const ComponentContext context(info, PlatformContext(platform));
    assert(context.platform().attached());
    assert(context.platform().info() == &platform.info());                    // forwards to the very adapter
    assert(context.platform().scheduler() == platform.scheduler() && context.platform().timer() == platform.timer());
    assert(&context.platform() == &context.platform());                       // a reference to the member, always the same object
    // A pointer-built R0.5 view (as from runtime.platform()) works too, and a null one is unattached.
    const ComponentContext from_pointer(info, PlatformContext(static_cast<IPlatformAdapter*>(&platform)));
    assert(from_pointer.platform().attached());
    const ComponentContext from_null(info, PlatformContext(static_cast<IPlatformAdapter*>(nullptr)));
    assert(from_null.bound() && !from_null.platform().attached());
}

void test_copies_share_what_the_original_refers_to() {
    ReferencePlatform platform(config());
    const ComponentInfo info = make_info();
    const ComponentContext original(info, PlatformContext(platform));
    const ComponentContext copy(original);
    ComponentContext moved(ComponentContext(info, PlatformContext(platform)));
    assert(copy.info() == original.info() && copy.id() == original.id() && copy.bound());
    assert(copy.platform().info() == original.platform().info() && copy.platform().timer() == original.platform().timer());
    assert(moved.info() == &info && moved.platform().attached());
    const ComponentContext unbound_copy{ComponentContext{}};
    assert(!unbound_copy.bound() && !unbound_copy.platform().attached());
}

void test_the_context_owns_and_destroys_nothing() {
    LifetimeProbe probe;
    auto platform = std::make_unique<ReferencePlatform>(config(&probe));
    auto info = std::make_unique<ComponentInfo>(make_info());
    {
        const ComponentContext context(*info, PlatformContext(*platform));
        const ComponentContext copy = context;
        ComponentContext moved(std::move(ComponentContext(*info, PlatformContext(*platform))));
        assert(copy.bound() && moved.bound());
    }                                                                         // every context is gone
    assert(!probe.platform_destroyed && probe.services_destroyed == 0);       // the adapter was not destroyed by them
    assert(info->id() == ComponentId{7} && info->name() == "worker");         // nor was the identity, which is still usable
    info.reset();                                                             // identity ends with its component
    platform.reset();                                                         // the integrator ends the platform
    assert(probe.platform_destroyed && probe.services_destroyed == 4);
}

// A counting adapter: constructing and querying the context's shape never touches the platform.
class SpyAdapter final : public ReferenceAdapter {
public:
    SpyAdapter() : ReferenceAdapter(PlatformInfo{"spy", Version{}}, {}) {}
    [[nodiscard]] const PlatformInfo& info() const noexcept override { ++calls; return ReferenceAdapter::info(); }
    [[nodiscard]] IScheduler* scheduler() const noexcept override { ++calls; return ReferenceAdapter::scheduler(); }
    [[nodiscard]] time::IClock* clock() const noexcept override { ++calls; return ReferenceAdapter::clock(); }
    [[nodiscard]] time::ITimer* timer() const noexcept override { ++calls; return ReferenceAdapter::timer(); }
    [[nodiscard]] IWatchdog* watchdog() const noexcept override { ++calls; return ReferenceAdapter::watchdog(); }
    [[nodiscard]] CapabilitySet capabilities() const override { ++calls; return ReferenceAdapter::capabilities(); }
    mutable int calls{0};
};

void test_construction_copying_and_shape_queries_have_no_side_effects() {
    SpyAdapter adapter;
    const ComponentInfo info = make_info();
    const ComponentContext context(info, PlatformContext(adapter));
    const ComponentContext copy = context;
    (void)copy.bound(); (void)copy.id(); (void)copy.info(); (void)copy.platform().attached();
    (void)ComponentContext(info, PlatformContext(adapter));
    assert(adapter.calls == 0);                                               // not one call to the adapter or to any service
    assert(adapter.activations() == 0);
}

void test_no_context_amplification() {
    // The context offers no way to reach a broader authority. Every member is listed here: if one is ever added that
    // returns the Runtime, the registry, another component, configuration, statistics, health or the raw adapter,
    // one of these names would have to exist and these assertions would break.
    static_assert(!ExposesAdapter<ComponentContext> && !ExposesPlatformAdapter<ComponentContext>);
    static_assert(!ExposesRuntime<ComponentContext> && !ExposesRegistry<ComponentContext> && !ExposesComponent<ComponentContext>);
    static_assert(!ExposesConfiguration<ComponentContext> && !ExposesStatistics<ComponentContext> && !ExposesHealth<ComponentContext>);
    static_assert(!CanFindById<ComponentContext> && !CanContextFor<ComponentContext> && !CanLookupByName<ComponentContext>);
    // The two pointers it holds are the only state, and the only way out of them is the typed, narrow accessors.
    static_assert(!std::is_convertible_v<ComponentContext, const ComponentInfo*>);
    static_assert(!std::is_convertible_v<ComponentContext, IPlatformAdapter*>);
    static_assert(!std::is_convertible_v<ComponentContext, PlatformContext>);
}

void test_a_component_stores_its_context_built_in_its_constructor() {
    // The supported pattern: the component's own info feeds the context stored as a member (initialized once).
    class Worker final : public Component {
    public:
        Worker(ComponentInfo info, PlatformContext platform) : Component(std::move(info)), context_(this->info(), platform) {}
        Result<void> configure(const Configuration&) override { return Result<void>::success(); }
        Result<void> initialize() override { return Result<void>::success(); }
        Result<void> start() override { return Result<void>::success(); }
        Result<void> stop() override { return Result<void>::success(); }
        Result<void> shutdown() override { return Result<void>::success(); }
        LifecycleState lifecycle_state() const noexcept override { return LifecycleState::UNKNOWN; }
        Status status() const override { return Status{}; }
        Health health() const override { return Health{}; }
        CapabilitySet capabilities() const override { return CapabilitySet{}; }
        const ComponentContext& context() const noexcept { return context_; }
    private:
        const ComponentContext context_;
    };
    ReferencePlatform platform(config());
    Worker worker(make_info(9, "stored"), PlatformContext(platform));
    assert(worker.context().bound() && worker.context().id() == ComponentId{9});
    assert(worker.context().info() == &worker.info());                       // the component's own identity, valid for as long as the component
    assert(worker.context().platform().attached());
}

} // namespace

int main() {
    test_shape_is_two_pointers_and_trivially_destructible();
    test_the_context_is_immutable_after_construction();
    test_unbound_context_has_no_identity_and_no_platform();
    test_bound_context_reports_its_identity();
    test_the_platform_view_is_the_r05_view_by_const_reference();
    test_copies_share_what_the_original_refers_to();
    test_the_context_owns_and_destroys_nothing();
    test_construction_copying_and_shape_queries_have_no_side_effects();
    test_no_context_amplification();
    test_a_component_stores_its_context_built_in_its_constructor();
    return 0;
}
