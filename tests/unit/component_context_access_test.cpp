//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component_context_access_test.cpp
// Description : ComponentContext access policy: require_*, supports, has_capability, attribute.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-CTX-002
// API         : CORE-TEST-COMPONENT-CONTEXT-ACCESS
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

#include "../contract/reference_adapter.hpp"
#include "../platform/reference_platform.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;
using namespace kritva::core::platform;
using kritva::core::platform::contract::ReferenceAdapter;
using kritva::core::platform::testing::Method;
using kritva::core::platform::testing::ReferencePlatform;
using kritva::core::time::TimerMode;

namespace {

// Member-detection concepts: a missing member makes the concept false instead of a hard error.
template<class T> concept CanRequireByEnum = requires(const T& c, PlatformService s) { c.require(s); };
template<class T> concept CanGetByEnum = requires(const T& c, PlatformService s) { c.get(s); };
template<class T> concept CanGetByName = requires(const T& c, const char* n) { c.get(n); };
template<class T> concept CanRequireByName = requires(const T& c, const char* n) { c.require(n); };
template<class T> concept CanTranslate = requires(const T& c, Error e) { c.translate(e); };
template<class T> concept CanWrap = requires(const T& c, Error e) { c.wrap(e); };
template<class T> concept CanListCapabilities = requires(const T& c) { c.capabilities(); };
template<class T> concept CanStartServices = requires(const T& c) { c.start_services(); };

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

ReferencePlatform::Config config(ReferencePlatform::Provides provides = {}) {
    ReferencePlatform::Config c;
    c.provides = provides;
    c.capabilities = capabilities_100_101();
    return c;
}

ReferenceAdapter::Provides provides_from(int mask) {
    return ReferenceAdapter::Provides{(mask & 1) != 0, (mask & 2) != 0, (mask & 4) != 0, (mask & 8) != 0};
}

class SpyAdapter final : public ReferenceAdapter {
public:
    explicit SpyAdapter(Provides provides = {}) : ReferenceAdapter(PlatformInfo{"spy", Version{1, 0, 0}}, provides, capabilities_100_101()) {}
    [[nodiscard]] const PlatformInfo& info() const noexcept override { ++calls; return ReferenceAdapter::info(); }
    [[nodiscard]] IScheduler* scheduler() const noexcept override { ++calls; return ReferenceAdapter::scheduler(); }
    [[nodiscard]] time::IClock* clock() const noexcept override { ++calls; return ReferenceAdapter::clock(); }
    [[nodiscard]] time::ITimer* timer() const noexcept override { ++calls; return ReferenceAdapter::timer(); }
    [[nodiscard]] IWatchdog* watchdog() const noexcept override { ++calls; return ReferenceAdapter::watchdog(); }
    [[nodiscard]] CapabilitySet capabilities() const override { ++calls; return ReferenceAdapter::capabilities(); }
    mutable int calls{0};
};

// ---- the approved surface and nothing more -------------------------------------------------------

void test_the_access_surface_is_exactly_the_approved_one() {
    static_assert(std::is_same_v<decltype(std::declval<const ComponentContext&>().require_scheduler()), Result<IScheduler*>>);
    static_assert(std::is_same_v<decltype(std::declval<const ComponentContext&>().require_clock()), Result<time::IClock*>>);
    static_assert(std::is_same_v<decltype(std::declval<const ComponentContext&>().require_timer()), Result<time::ITimer*>>);
    static_assert(std::is_same_v<decltype(std::declval<const ComponentContext&>().require_watchdog()), Result<IWatchdog*>>);
    static_assert(std::is_same_v<decltype(std::declval<const ComponentContext&>().supports(PlatformService::CLOCK)), bool>);
    static_assert(std::is_same_v<decltype(std::declval<const ComponentContext&>().has_capability(CapabilityId{})), bool>);
    static_assert(std::is_same_v<decltype(std::declval<const ComponentContext&>().attribute(Error{})), Error>);
    static_assert(noexcept(std::declval<const ComponentContext&>().supports(PlatformService::CLOCK)));
    static_assert(noexcept(std::declval<const ComponentContext&>().attribute(Error{})));
    static_assert(sizeof(ComponentContext) == 2 * sizeof(void*));                  // still two pointers: the access policy added no state
    static_assert(std::is_trivially_destructible_v<ComponentContext>);
    // No generic or runtime-keyed access, no translation or wrapping, no capability listing, no service control.
    static_assert(!CanRequireByEnum<ComponentContext> && !CanGetByEnum<ComponentContext> && !CanGetByName<ComponentContext> && !CanRequireByName<ComponentContext>);
    static_assert(!CanTranslate<ComponentContext> && !CanWrap<ComponentContext>);
    static_assert(!CanListCapabilities<ComponentContext> && !CanStartServices<ComponentContext>);
}

// ---- require_*(): the adapter-owned service, or an attributed UNSUPPORTED ---------------------------------

void test_require_forwards_the_adapter_owned_service() {
    ReferencePlatform platform(config());
    const ComponentInfo info = make_info();
    const ComponentContext context(info, PlatformContext(platform));
    const auto scheduler = context.require_scheduler();
    const auto clock = context.require_clock();
    const auto timer = context.require_timer();
    const auto watchdog = context.require_watchdog();
    assert(scheduler.has_value() && scheduler.value() == platform.scheduler() && scheduler.value() != nullptr);
    assert(clock.has_value() && clock.value() == platform.clock());
    assert(timer.has_value() && timer.value() == platform.timer());
    assert(watchdog.has_value() && watchdog.value() == platform.watchdog());
    // The same objects the R0.5 view returns.
    assert(scheduler.value() == context.platform().scheduler() && timer.value() == context.platform().timer());
    // The services are used directly through their own contracts; obtaining them started nothing.
    assert(!platform.fake_scheduler().running() && !platform.fake_timer().running() && !platform.fake_watchdog().running());
    assert(watchdog.value()->start(Duration::from_milliseconds(100)) && watchdog.value()->stop());
}

void expect_attributed_unsupported(const Error& error, std::uint64_t id, const char* service, const char* situation) {
    assert(error.code == ErrorCode::UNSUPPORTED);
    assert(error.severity == ErrorSeverity::ERROR);
    assert(error.source == ComponentId{id});                                        // the component is the source
    assert(error.message.find(service) != std::string::npos && error.message.find(situation) != std::string::npos);
}

void test_an_unavailable_service_is_an_unsupported_error_attributed_to_the_component() {
    const ComponentInfo info = make_info(7);
    {   // attached, but the platform provides nothing
        ReferenceAdapter bare(PlatformInfo{"bare", Version{}}, {false, false, false, false});
        const ComponentContext context(info, PlatformContext(bare));
        expect_attributed_unsupported(context.require_scheduler().error(), 7, "scheduler", "not provided by the platform");
        expect_attributed_unsupported(context.require_clock().error(), 7, "clock", "not provided by the platform");
        expect_attributed_unsupported(context.require_timer().error(), 7, "timer", "not provided by the platform");
        expect_attributed_unsupported(context.require_watchdog().error(), 7, "watchdog", "not provided by the platform");
    }
    {   // bound, but no platform at all
        const ComponentContext context(info);
        expect_attributed_unsupported(context.require_scheduler().error(), 7, "scheduler", "no platform is attached");
        expect_attributed_unsupported(context.require_timer().error(), 7, "timer", "no platform is attached");
    }
    // Every message names only its own service.
    ReferenceAdapter bare(PlatformInfo{"bare", Version{}}, {false, false, false, false});
    const ComponentContext context(info, PlatformContext(bare));
    for (const char* other : {"clock", "timer", "watchdog"}) assert(context.require_scheduler().error().message.find(other) == std::string::npos);
}

void test_the_context_error_is_the_r05_error_plus_the_source_and_r05_is_unchanged() {
    const ComponentInfo info = make_info(7);
    ReferenceAdapter bare(PlatformInfo{"bare", Version{}}, {false, false, false, false});
    const PlatformContext platform(bare);
    const ComponentContext context(info, platform);
    const Error r05 = platform.require_timer().error();
    const Error r06 = context.require_timer().error();
    assert(!r05.source.valid());                                                    // PlatformContext is unchanged: Core is the source, no component
    assert(r06.source == ComponentId{7});
    assert(r06.code == r05.code && r06.severity == r05.severity && r06.message == r05.message && r06.timestamp == r05.timestamp);   // only the source differs
    // And again for every service.
    assert(context.require_scheduler().error().message == platform.require_scheduler().error().message);
    assert(context.require_clock().error().message == platform.require_clock().error().message);
    assert(context.require_watchdog().error().message == platform.require_watchdog().error().message);
}

void test_an_unbound_context_returns_the_r05_error_unchanged() {
    // An unbound context has no identity to bind a platform to, so the unattached case is the only unbound one.
    const ComponentContext unbound;
    const Error from_unbound = unbound.require_clock().error();
    assert(!from_unbound.source.valid());                                           // no identity to attribute
    assert(from_unbound.code == ErrorCode::UNSUPPORTED && from_unbound.message == PlatformContext{}.require_clock().error().message);
    assert(from_unbound.severity == ErrorSeverity::ERROR);
    // A bound context's success is never altered.
    ReferencePlatform platform(config());
    const ComponentInfo info = make_info();
    const ComponentContext bound(info, PlatformContext(platform));
    assert(bound.require_clock().has_value() && bound.require_clock().value() == platform.clock());
}

void test_every_service_combination_matches_the_adapter() {
    const ComponentInfo info = make_info(9);
    for (int mask = 0; mask < 16; ++mask) {
        const ReferenceAdapter::Provides provides = provides_from(mask);
        ReferenceAdapter adapter(PlatformInfo{"p", Version{}}, provides);
        const ComponentContext context(info, PlatformContext(adapter));
        assert(context.require_scheduler().has_value() == provides.scheduler && context.supports(PlatformService::SCHEDULER) == provides.scheduler);
        assert(context.require_clock().has_value() == provides.clock && context.supports(PlatformService::CLOCK) == provides.clock);
        assert(context.require_timer().has_value() == provides.timer && context.supports(PlatformService::TIMER) == provides.timer);
        assert(context.require_watchdog().has_value() == provides.watchdog && context.supports(PlatformService::WATCHDOG) == provides.watchdog);
        if (!provides.watchdog) assert(context.require_watchdog().error().source == ComponentId{9});
        if (provides.timer) assert(context.require_timer().value() == adapter.timer());   // success is never nullptr
    }
}

void test_results_are_deterministic() {
    const ComponentInfo info = make_info(7);
    ReferenceAdapter bare(PlatformInfo{"bare", Version{}}, {false, true, true, true});
    const ComponentContext context(info, PlatformContext(bare));
    const Error first = context.require_scheduler().error(), second = context.require_scheduler().error();
    assert(first.code == second.code && first.message == second.message && first.source == second.source);
}

// ---- queries have no side effects -----------------------------------------------------------------------

void test_every_access_path_is_a_side_effect_free_query() {
    SpyAdapter adapter;
    const ComponentInfo info = make_info();
    const ComponentContext context(info, PlatformContext(adapter));
    assert(adapter.calls == 0);
    (void)context.require_scheduler();
    assert(adapter.calls == 1);                                                     // exactly one accessor call per require
    (void)context.require_clock(); (void)context.require_timer(); (void)context.require_watchdog();
    assert(adapter.calls == 4);
    (void)context.supports(PlatformService::TIMER);
    assert(adapter.calls == 5);
    (void)context.has_capability(CapabilityId{100});
    assert(adapter.calls == 6);                                                     // one capability snapshot
    (void)context.attribute(Error{ErrorCode::TIMEOUT, ErrorSeverity::ERROR, {}, {}, "x"});
    assert(adapter.calls == 6);                                                     // attribution never touches the platform
    assert(adapter.activations() == 0);                                             // nothing was started
}

// ---- supports() and has_capability(): identity only --------------------------------------------------------

void test_supports_and_capabilities_follow_the_platform_by_identity() {
    const ComponentInfo info = make_info();
    ReferenceAdapter adapter(PlatformInfo{"can-bus", Version{101, 100, 0}}, {true, false, true, false}, capabilities_100_101());
    const ComponentContext context(info, PlatformContext(adapter));
    assert(context.supports(PlatformService::SCHEDULER) && !context.supports(PlatformService::CLOCK));
    assert(context.supports(PlatformService::TIMER) && !context.supports(PlatformService::WATCHDOG));
    assert(!context.supports(static_cast<PlatformService>(200)));                   // an unknown enumerator is not supported
    assert(context.has_capability(CapabilityId{100}) && context.has_capability(CapabilityId{101}));
    assert(!context.has_capability(CapabilityId{102}));
    assert(!context.has_capability(CapabilityId{}));
    // A platform that merely looks like it has a capability (its name, its version numbers) has none.
    ReferenceAdapter lookalike(PlatformInfo{"can-bus", Version{101, 100, 0}}, {});
    assert(!ComponentContext(info, PlatformContext(lookalike)).has_capability(CapabilityId{101}));
    // Name lengths and version numbers used as identities are not capabilities either.
    ReferenceAdapter numeric(PlatformInfo{"abcde", Version{7, 3, 1}}, {}, capabilities_100_101());
    const ComponentContext numeric_context(info, PlatformContext(numeric));
    for (std::uint64_t id : {std::uint64_t{1}, std::uint64_t{3}, std::uint64_t{5}, std::uint64_t{7}}) assert(!numeric_context.has_capability(CapabilityId{id}));
    // Without a platform nothing is supported and no capability is reported.
    const ComponentContext bare(info);
    assert(!bare.supports(PlatformService::SCHEDULER) && !bare.has_capability(CapabilityId{100}));
}

// ---- attribute(): only the source changes ----------------------------------------------------------------------

Error rich_error() {
    Error e;
    e.code = ErrorCode::RESOURCE_UNAVAILABLE;
    e.severity = ErrorSeverity::WARNING;
    e.source = Id{99};
    e.timestamp = Timestamp(123456, ClockDomain::REALTIME);
    e.message = "the original message";
    return e;
}

void test_attribute_changes_only_the_source() {
    const ComponentInfo info = make_info(7);
    const ComponentContext context(info);
    const Error in = rich_error();
    const Error out = context.attribute(in);
    assert(out.source == ComponentId{7});                                           // replaced, even though another source was present
    assert(out.code == in.code && out.severity == in.severity && out.message == in.message);
    assert(out.timestamp == in.timestamp);                                          // including the domain
    assert(out.timestamp.domain() == ClockDomain::REALTIME && out.timestamp.nanoseconds() == 123456);
    // Mutate every field of the input in turn: only the source of the output is ever the component's.
    for (const ErrorCode code : {ErrorCode::TIMEOUT, ErrorCode::UNSUPPORTED, ErrorCode::INVALID_STATE, ErrorCode::INTERNAL_ERROR}) {
        Error e = rich_error(); e.code = code;
        const Error r = context.attribute(e);
        assert(r.code == code && r.source == ComponentId{7} && r.severity == e.severity && r.message == e.message && r.timestamp == e.timestamp);
    }
    for (const ErrorSeverity severity : {ErrorSeverity::INFO, ErrorSeverity::WARNING, ErrorSeverity::ERROR}) {
        Error e = rich_error(); e.severity = severity;
        const Error r = context.attribute(e);
        assert(r.severity == severity && r.source == ComponentId{7} && r.code == e.code && r.message == e.message && r.timestamp == e.timestamp);
    }
    for (const std::string& message : {std::string{}, std::string("a"), std::string("a much longer message with spaces and 123")}) {
        Error e = rich_error(); e.message = message;
        const Error r = context.attribute(e);
        assert(r.message == message && r.source == ComponentId{7} && r.code == e.code && r.severity == e.severity && r.timestamp == e.timestamp);
    }
    for (const Id source : {Id{}, Id{1}, Id{7}, Id{12345}}) {
        Error e = rich_error(); e.source = source;
        assert(context.attribute(e).source == ComponentId{7});                      // whatever the source was
    }
    for (const Timestamp timestamp : {Timestamp(), Timestamp(1, ClockDomain::MONOTONIC), Timestamp(-5, ClockDomain::REALTIME)}) {
        Error e = rich_error(); e.timestamp = timestamp;
        const Error r = context.attribute(e);
        assert(r.timestamp == timestamp && r.source == ComponentId{7} && r.code == e.code && r.message == e.message);
    }
    // Attributing twice, or to the same component, is idempotent.
    assert(context.attribute(context.attribute(in)).source == ComponentId{7});
}

void test_attribute_of_an_unbound_context_returns_the_error_unchanged() {
    const ComponentContext unbound;
    const Error in = rich_error();
    const Error out = unbound.attribute(in);
    assert(out.source == in.source && out.code == in.code && out.severity == in.severity && out.message == in.message && out.timestamp == in.timestamp);
    assert(out.source == Id{99});                                                   // even the source is unchanged: nothing to attribute to
}

void test_a_real_service_error_keeps_its_own_fields_until_the_component_attributes_it() {
    ReferencePlatform platform(config());
    platform.controls().fail_nth(Method::TIMER_START, 1, ErrorCode::RESOURCE_UNAVAILABLE);
    const ComponentInfo info = make_info(7);
    const ComponentContext context(info, PlatformContext(platform));
    const auto timer = context.require_timer();
    assert(timer.has_value());
    const auto started = timer.value()->start(Duration::from_milliseconds(10), TimerMode::ONE_SHOT, Callback{[](void*) {}, nullptr});
    assert(!started);
    assert(started.error().code == ErrorCode::RESOURCE_UNAVAILABLE && started.error().message == "injected failure");
    assert(!started.error().source.valid());                                        // the context never touched the service's error
    const Error attributed = context.attribute(started.error());                   // the component's explicit step
    assert(attributed.source == ComponentId{7} && attributed.code == ErrorCode::RESOURCE_UNAVAILABLE && attributed.message == "injected failure");
}

// ---- a Component can return a context error directly ----------------------------------------------------------------

class NeedsATimer final : public Component {
public:
    NeedsATimer(ComponentInfo info, PlatformContext platform) : Component(std::move(info)), context_(this->info(), platform) {}
    Result<void> configure(const Configuration&) override { return Result<void>::success(); }
    Result<void> initialize() override { return Result<void>::success(); }
    Result<void> start() override {
        const auto timer = context_.require_timer();
        if (!timer) return Result<void>::failure(timer.error());                    // returned as is: the source is already this component
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

void test_a_component_returns_the_context_error_directly_and_the_runtime_propagates_it() {
    ReferenceAdapter no_timer(PlatformInfo{"p", Version{}}, {true, true, false, true});
    RuntimeManager runtime;
    auto info = ComponentInfo::create(ComponentId{42}, "needs-a-timer");
    NeedsATimer component(std::move(info).value(), PlatformContext(no_timer));
    assert(runtime.register_component(component) && runtime.initialize());
    const auto started = runtime.start();
    assert(!started && started.error().code == ErrorCode::UNSUPPORTED && started.error().source == ComponentId{42});   // the Component contract is met
    assert(started.error().message.find("timer") != std::string::npos);
    assert(runtime.state() == LifecycleState::FAULT && runtime.fault_error()->source == ComponentId{42});
    assert(runtime.reset() && runtime.state() == LifecycleState::STOPPED);
}

} // namespace

int main() {
    test_the_access_surface_is_exactly_the_approved_one();
    test_require_forwards_the_adapter_owned_service();
    test_an_unavailable_service_is_an_unsupported_error_attributed_to_the_component();
    test_the_context_error_is_the_r05_error_plus_the_source_and_r05_is_unchanged();
    test_an_unbound_context_returns_the_r05_error_unchanged();
    test_every_service_combination_matches_the_adapter();
    test_results_are_deterministic();
    test_every_access_path_is_a_side_effect_free_query();
    test_supports_and_capabilities_follow_the_platform_by_identity();
    test_attribute_changes_only_the_source();
    test_attribute_of_an_unbound_context_returns_the_error_unchanged();
    test_a_real_service_error_keeps_its_own_fields_until_the_component_attributes_it();
    test_a_component_returns_the_context_error_directly_and_the_runtime_propagates_it();
    return 0;
}
