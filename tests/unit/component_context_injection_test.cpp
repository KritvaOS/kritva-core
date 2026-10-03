//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component_context_injection_test.cpp
// Description : Construction-time injection of a ComponentContext without any Runtime or Component change.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-CTX-003
// API         : CORE-TEST-COMPONENT-CONTEXT-INJECTION
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#include <cassert>
#include <cstdint>
#include <memory>
#include <random>
#include <string>
#include <type_traits>
#include <utility>

#include <kritva/core/core.hpp>

#include "../contract/reference_adapter.hpp"
#include "../contract/reference_component.hpp"
#include "../integration/runtime_scenarios.hpp"
#include "../platform/reference_platform.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;
using namespace kritva::core::runtime::scenarios;
using namespace kritva::core::platform;
using kritva::core::platform::contract::ReferenceAdapter;
using kritva::core::platform::testing::Method;
using kritva::core::platform::testing::ReferencePlatform;
using kritva::core::runtime::contract::ReferenceComponent;

namespace {

// Member-detection concepts: a missing member makes the concept false instead of a hard error.
template<class T> concept HasContextMember = requires(const T& t) { t.context(); };
template<class T> concept HasInjectMember = requires(T& t, const ComponentContext& c) { t.inject(c); };
template<class T> concept HasSetContext = requires(T& t, const ComponentContext& c) { t.set_context(c); };
template<class T> concept RuntimeMakesContext = requires(T& t) { t.make_context(); };
template<class T> concept RuntimeContextFor = requires(T& t, ComponentId id) { t.context_for(id); };
template<class T> concept RuntimeInjects = requires(T& t, Component& c) { t.inject_context(c); };
template<class T> concept RuntimeHasComponentContext = requires(const T& t) { t.component_context(); };

// An integrator-written component: its context is built from its own identity in its own constructor.
class Worker final : public Component {
public:
    Worker(ComponentInfo info, PlatformContext platform) : Component(std::move(info)), context_(*this, platform) {}
    Result<void> configure(const Configuration&) override { return Result<void>::success(); }
    Result<void> initialize() override { return Result<void>::success(); }
    Result<void> start() override { return Result<void>::success(); }
    Result<void> stop() override { return Result<void>::success(); }
    Result<void> shutdown() override { return Result<void>::success(); }
    LifecycleState lifecycle_state() const noexcept override { return LifecycleState::UNKNOWN; }
    Status status() const override { return Status{}; }
    Health health() const override { return Health{}; }
    CapabilitySet capabilities() const override { return CapabilitySet{}; }
    const ComponentContext& worker_context() const noexcept { return context_; }
private:
    const ComponentContext context_;
};

ComponentInfo make_info(std::uint64_t id, const char* name = "worker") {
    auto info = ComponentInfo::create(ComponentId{id}, name, Version{2, 0, 0});
    assert(info.has_value());
    return std::move(info).value();
}

// A reference component that also holds a context and USES it during every lifecycle operation (queries only),
// then behaves exactly as the plain reference component.
class ContextedReference final : public ReferenceComponent {
public:
    ContextedReference(ComponentInfo info, PlatformContext platform) : ReferenceComponent(std::move(info)), context_(*this, platform) {}
    Result<void> configure(const Configuration& c) override { use(); return ReferenceComponent::configure(c); }
    Result<void> initialize() override { use(); return ReferenceComponent::initialize(); }
    Result<void> start() override { use(); return ReferenceComponent::start(); }
    Result<void> stop() override { use(); return ReferenceComponent::stop(); }
    Result<void> shutdown() override { use(); return ReferenceComponent::shutdown(); }
    int accessor_calls_made() const noexcept { return calls_; }

private:
    void use() {
        (void)context_.require_timer();          // 1 accessor call
        (void)context_.supports(PlatformService::WATCHDOG);   // 1 accessor call
        (void)context_.require_scheduler();      // 1 accessor call
        (void)context_.has_capability(CapabilityId{100});     // 1 capability snapshot
        (void)context_.id();
        calls_ += 4;
    }
    const ComponentContext context_;
    int calls_{0};
};

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

// ---- injection is by construction, from the component's own identity ----------------------------------

void test_a_component_builds_its_context_from_its_own_identity() {
    ReferencePlatform platform;
    Worker worker(make_info(11, "builder"), PlatformContext(platform));
    const ComponentContext& context = worker.worker_context();
    assert(context.bound() && context.id() == ComponentId{11});
    assert(context.info() == &worker.info());                                   // the component's own identity object, not a copy
    assert(context.info()->name() == "builder" && context.info()->version() == (Version{2, 0, 0}));
    assert(context.platform().attached() && context.platform().info() == &platform.info());

    // The same context can be built outside a component from its info or from the component itself.
    const ComponentContext from_component(worker, PlatformContext(platform));
    const ComponentContext from_info(worker.info(), PlatformContext(platform));
    assert(from_component.info() == context.info() && from_info.info() == context.info());
    assert(from_component.id() == from_info.id());
    // And without a platform.
    assert(ComponentContext(worker).bound() && !ComponentContext(worker).platform().attached());
}

void test_a_temporary_component_is_refused_at_compile_time() {
    static_assert(std::is_constructible_v<ComponentContext, const Worker&>);
    static_assert(std::is_constructible_v<ComponentContext, Worker&, PlatformContext>);
    static_assert(!std::is_constructible_v<ComponentContext, Worker>);          // a temporary component would dangle
    static_assert(!std::is_constructible_v<ComponentContext, Worker, PlatformContext>);
    static_assert(!std::is_convertible_v<const Worker&, ComponentContext>);     // always an explicit act
    static_assert(noexcept(ComponentContext(std::declval<const Worker&>())));
    static_assert(sizeof(ComponentContext) == 2 * sizeof(void*));               // injection added no state
}

void test_a_context_built_from_the_runtimes_platform_pointer() {
    ReferencePlatform platform;
    RuntimeManager runtime;
    Worker worker(make_info(3), PlatformContext{});                             // built before any platform is attached
    assert(runtime.register_component(worker) && runtime.attach_platform(platform));
    // The integrator may build a platform view from runtime.platform() and a context from it.
    const ComponentContext context(worker, PlatformContext(runtime.platform()));
    assert(context.platform().attached() && context.platform().info() == &platform.info());
    const RuntimeManager bare;
    assert(!ComponentContext(worker, PlatformContext(bare.platform())).platform().attached());   // none attached: an unattached view
}

// ---- no Runtime change, no Component change -------------------------------------------------------------------

void test_the_runtime_and_the_component_base_are_unchanged() {
    // The Component lifecycle signatures are exactly the R0.3 ones.
    static_assert(std::is_same_v<decltype(&Component::configure), Result<void> (Component::*)(const Configuration&)>);
    static_assert(std::is_same_v<decltype(&Component::initialize), Result<void> (Component::*)()>);
    static_assert(std::is_same_v<decltype(&Component::start), Result<void> (Component::*)()>);
    static_assert(std::is_same_v<decltype(&Component::stop), Result<void> (Component::*)()>);
    static_assert(std::is_same_v<decltype(&Component::shutdown), Result<void> (Component::*)()>);
    static_assert(std::is_same_v<decltype(&Component::info), const ComponentInfo& (Component::*)() const noexcept>);
    // The Component base knows nothing about contexts.
    static_assert(!HasContextMember<Component> && !HasInjectMember<Component> && !HasSetContext<Component>);
    // The Runtime has no way to create, find, inject, hold or hand out a context.
    static_assert(!RuntimeMakesContext<RuntimeManager> && !RuntimeContextFor<RuntimeManager>);
    static_assert(!RuntimeInjects<RuntimeManager> && !RuntimeHasComponentContext<RuntimeManager>);
    // The RuntimeManager members have exactly their R0.5 signatures.
    static_assert(std::is_same_v<decltype(&RuntimeManager::attach_platform), Result<void> (RuntimeManager::*)(platform::IPlatformAdapter&)>);
    static_assert(std::is_same_v<decltype(&RuntimeManager::platform), platform::IPlatformAdapter* (RuntimeManager::*)() const noexcept>);
    static_assert(std::is_same_v<decltype(&RuntimeManager::register_component), Result<void> (RuntimeManager::*)(Component&)>);
    static_assert(std::is_same_v<decltype(&RuntimeManager::initialize), Result<void> (RuntimeManager::*)()>);
    static_assert(std::is_same_v<decltype(&RuntimeManager::reset), Result<void> (RuntimeManager::*)()>);
    static_assert(std::is_final_v<RuntimeManager> && !std::is_copy_constructible_v<RuntimeManager>);
}

// ---- the Runtime is identical whether or not components hold contexts and whatever they do with them -------------

void test_runtime_differential_with_and_without_component_contexts() {
    std::mt19937 rng(20261010);
    std::size_t operations = 0, non_ok = 0;
    for (int scenario = 0; scenario < 40; ++scenario) {
        const Script script = random_script(rng);
        World baseline(script, nullptr);
        const Trace expected = run(baseline, script);
        for (const std::string& line : expected) if (line.find("-> ok") == std::string::npos && line.find("->") != std::string::npos) ++non_ok;
        operations += script.steps.size();

        for (int mask = 0; mask < 16; ++mask) {                                  // every service combination, every platform method failing
            ReferencePlatform::Config config;
            config.provides = ReferencePlatform::Provides{(mask & 1) != 0, (mask & 2) != 0, (mask & 4) != 0, (mask & 8) != 0};
            ReferencePlatform platform(config);
            for (std::size_t m = 0; m < 8; ++m) platform.controls().fail_all(static_cast<Method>(m), ErrorCode::INTERNAL_ERROR);
            World world(script, &platform, [&](ComponentInfo info) -> std::unique_ptr<ReferenceComponent> {
                return std::make_unique<ContextedReference>(std::move(info), PlatformContext(platform));
            });
            const Trace actual = run(world, script);
            assert(actual == expected);                                          // results, states, faults, statistics, invocation trace
            assert(platform.controls().calls(Method::SCHEDULER_START) == 0 && platform.controls().log().empty());   // contexts started and called nothing
        }
    }
    assert(operations == 40u * 40u && non_ok > 200);
}

void test_the_runtime_never_touches_a_context_or_the_adapter_behind_it() {
    SpyAdapter adapter;
    const PlatformContext platform(adapter);
    const Script script = [] { Script s; s.n = 3; s.registration = {1, 2, 3}; s.edges = {{2, 1}, {3, 2}}; for (int op : {0, 1, 2, 3, 4}) s.steps.push_back(Step{op, 0, 0, ErrorCode::NONE}); return s; }();
    World world(script, &adapter, [&](ComponentInfo info) -> std::unique_ptr<ReferenceComponent> {
        return std::make_unique<ContextedReference>(std::move(info), platform);
    });
    (void)run(world, script);
    int made_by_components = 0;
    for (const auto& c : world.components) made_by_components += static_cast<ContextedReference&>(*c).accessor_calls_made();
    assert(made_by_components == 3 * 5 * 4);                                     // 3 components x 5 operations (configure included) x 4 queries
    // Every adapter call was made by a component's own context use (4 queries each): the Runtime made none, attach included.
    assert(adapter.calls == made_by_components);
    assert(adapter.activations() == 0);
    assert(world.runtime.state() == LifecycleState::STOPPED);
}

void test_a_context_has_no_lifecycle_of_its_own() {
    // Contexts are created and destroyed with their component: neither the Runtime nor Core creates, starts, stops or recovers them.
    ReferencePlatform platform;
    RuntimeManager runtime;
    {
        Worker first(make_info(1), PlatformContext(platform));
        Worker second(make_info(2), PlatformContext(platform));
        assert(runtime.register_component(first) && runtime.register_component(second) && runtime.attach_platform(platform));
        assert(runtime.initialize() && runtime.start());
        assert(first.worker_context().bound() && second.worker_context().id() == ComponentId{2});   // unchanged by the Runtime's operations
        assert(runtime.stop() && runtime.shutdown());
        assert(first.worker_context().info() == &first.info());                  // still the component's own identity afterwards
    }                                                                            // components (and so their contexts) end before the platform
    assert(platform.controls().log().empty() && platform.controls().adapter_queries() == 0);
}

void test_a_failing_component_with_a_context_is_an_ordinary_component_failure() {
    // A component that returns its context's availability error directly: the Runtime sees an ordinary failure.
    class NeedsATimer final : public Component {
    public:
        NeedsATimer(ComponentInfo info, PlatformContext platform) : Component(std::move(info)), context_(*this, platform) {}
        Result<void> configure(const Configuration&) override { return Result<void>::success(); }
        Result<void> initialize() override { return Result<void>::success(); }
        Result<void> start() override {
            const auto timer = context_.require_timer();
            if (!timer) return Result<void>::failure(timer.error());
            return Result<void>::success();
        }
        Result<void> stop() override { return Result<void>::success(); }
        Result<void> shutdown() override { return Result<void>::success(); }
        LifecycleState lifecycle_state() const noexcept override { return LifecycleState::UNKNOWN; }
        Status status() const override { return Status{}; }
        Health health() const override { return Health{}; }
        CapabilitySet capabilities() const override { return CapabilitySet{}; }
    private:
        const ComponentContext context_;
    };
    ReferenceAdapter no_timer(PlatformInfo{"p", Version{}}, {true, true, false, true});
    RuntimeManager runtime;
    NeedsATimer component(make_info(5), PlatformContext(no_timer));
    assert(runtime.register_component(component) && runtime.initialize());
    const auto started = runtime.start();
    assert(!started && started.error().code == ErrorCode::UNSUPPORTED && started.error().source == ComponentId{5});
    assert(runtime.state() == LifecycleState::FAULT && runtime.statistics().error_count.value() == 1 && runtime.statistics().retry_count.value() == 0);
    assert(runtime.reset() && runtime.state() == LifecycleState::STOPPED);
}

} // namespace

int main() {
    test_a_component_builds_its_context_from_its_own_identity();
    test_a_temporary_component_is_refused_at_compile_time();
    test_a_context_built_from_the_runtimes_platform_pointer();
    test_the_runtime_and_the_component_base_are_unchanged();
    test_runtime_differential_with_and_without_component_contexts();
    test_the_runtime_never_touches_a_context_or_the_adapter_behind_it();
    test_a_context_has_no_lifecycle_of_its_own();
    test_a_failing_component_with_a_context_is_an_ordinary_component_failure();
    return 0;
}
