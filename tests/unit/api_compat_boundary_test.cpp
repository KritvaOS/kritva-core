//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : api_compat_boundary_test.cpp
// Description : Compile-time snapshot of the compatibility-sensitive boundary of the stable Core 1.x public API.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-COMPAT-009
// API         : CORE-API-COMPATIBILITY-BOUNDARY
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

// The properties docs/compatibility/COMPATIBILITY_POLICY.md protects that a declaration snapshot alone does not state
// plainly: the numeric value and underlying type of every public enumeration, the type properties of the stable value
// types (default construction, copy, move assignment, triviality, aggregate shape) and the shape of every virtual
// interface clients implement (the exact set of pure virtuals and their signatures). Drift fails to COMPILE: a changed
// value or property fails a static_assert, an added enumerator fails the exhaustive switches under the strict warning
// build, and an added pure virtual makes the conforming implementer below abstract. A deliberate change must pass the
// evolution review (docs/compatibility/VERSIONING_POLICY.md) and update this snapshot with it. The declaration surface
// itself is guarded by scripts/audit/check_api_surface.py.

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <type_traits>

#include <kritva/core/core.hpp>
#include <kritva/core/platform/context.hpp>
#include <kritva/core/platform/requirements.hpp>
#include <kritva/core/runtime/component_context.hpp>
#include <kritva/core/runtime/component_events.hpp>
#include <kritva/core/runtime/component_observation.hpp>
#include <kritva/core/runtime/component_registry.hpp>
#include <kritva/core/runtime/component_statistics.hpp>
#include <kritva/core/runtime/dependency_graph.hpp>
#include <kritva/core/runtime/runtime_manager.hpp>
#include <kritva/core/types/callback.hpp>

using namespace kritva::core;

namespace {

int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::cerr << __FILE__ << ":" << __LINE__ << ": CHECK failed: " #cond "\n"; ++failures; } } while (0)

template <class E> constexpr auto v(E e) { return static_cast<std::underlying_type_t<E>>(e); }

// --- 1. Enumerations: underlying type, every enumerator value, and an exhaustive switch (an added enumerator fails the strict build) ---
#define KRITVA_COMPAT_ENUM_TYPE(E, U) static_assert(std::is_same_v<std::underlying_type_t<E>, U>, #E " underlying type changed")

KRITVA_COMPAT_ENUM_TYPE(ErrorSeverity, std::uint8_t);
KRITVA_COMPAT_ENUM_TYPE(ErrorCode, std::uint32_t);
KRITVA_COMPAT_ENUM_TYPE(StatusCode, std::uint8_t);
KRITVA_COMPAT_ENUM_TYPE(HealthState, std::uint8_t);
KRITVA_COMPAT_ENUM_TYPE(LifecycleState, std::uint8_t);
KRITVA_COMPAT_ENUM_TYPE(EventType, std::uint32_t);
KRITVA_COMPAT_ENUM_TYPE(platform::PlatformService, std::uint8_t);
KRITVA_COMPAT_ENUM_TYPE(platform::Requirement, std::uint8_t);
KRITVA_COMPAT_ENUM_TYPE(time::TimerMode, std::uint8_t);
KRITVA_COMPAT_ENUM_TYPE(ClockDomain, std::uint8_t);

int severity(ErrorSeverity e) {
    switch (e) { case ErrorSeverity::INFO: return 0; case ErrorSeverity::WARNING: return 1; case ErrorSeverity::ERROR: return 2; case ErrorSeverity::CRITICAL: return 3; }
    return -1;
}
int error_code(ErrorCode e) {
    switch (e) {
        case ErrorCode::NONE: return 0; case ErrorCode::UNKNOWN: return 1; case ErrorCode::INVALID_ARGUMENT: return 2;
        case ErrorCode::INVALID_STATE: return 3; case ErrorCode::NOT_INITIALIZED: return 4; case ErrorCode::NOT_READY: return 5;
        case ErrorCode::ALREADY_RUNNING: return 6; case ErrorCode::TIMEOUT: return 7; case ErrorCode::RESOURCE_UNAVAILABLE: return 8;
        case ErrorCode::CONFIGURATION_ERROR: return 9; case ErrorCode::UNSUPPORTED: return 10; case ErrorCode::INTERNAL_ERROR: return 11;
    }
    return -1;
}
int status_code(StatusCode e) {
    switch (e) {
        case StatusCode::UNKNOWN: return 0; case StatusCode::OK: return 1; case StatusCode::INVALID_ARGUMENT: return 2;
        case StatusCode::NOT_READY: return 3; case StatusCode::BUSY: return 4; case StatusCode::TIMEOUT: return 5;
        case StatusCode::FAILED: return 6; case StatusCode::UNAVAILABLE: return 7; case StatusCode::UNSUPPORTED: return 8;
        case StatusCode::INTERNAL_ERROR: return 9;
    }
    return -1;
}
int health_state(HealthState e) {
    switch (e) { case HealthState::UNKNOWN: return 0; case HealthState::HEALTHY: return 1; case HealthState::DEGRADED: return 2; case HealthState::UNHEALTHY: return 3; }
    return -1;
}
int lifecycle_state(LifecycleState e) {
    switch (e) {
        case LifecycleState::UNKNOWN: return 0; case LifecycleState::INITIALIZING: return 1; case LifecycleState::READY: return 2;
        case LifecycleState::RUNNING: return 3; case LifecycleState::STOPPING: return 4; case LifecycleState::STOPPED: return 5;
        case LifecycleState::FAULT: return 6; case LifecycleState::RECOVERING: return 7;
    }
    return -1;
}
int event_type(EventType e) {
    switch (e) {
        case EventType::UNKNOWN: return 0; case EventType::LIFECYCLE: return 1; case EventType::STATUS: return 2; case EventType::HEALTH: return 3;
        case EventType::ERROR: return 4; case EventType::CONFIGURATION: return 5; case EventType::CAPABILITY: return 6;
    }
    return -1;
}
int platform_service(platform::PlatformService e) {
    switch (e) {
        case platform::PlatformService::SCHEDULER: return 0; case platform::PlatformService::CLOCK: return 1;
        case platform::PlatformService::TIMER: return 2; case platform::PlatformService::WATCHDOG: return 3;
    }
    return -1;
}
int requirement(platform::Requirement e) {
    switch (e) { case platform::Requirement::REQUIRED: return 0; case platform::Requirement::OPTIONAL: return 1; }
    return -1;
}
int timer_mode(time::TimerMode e) {
    switch (e) { case time::TimerMode::ONE_SHOT: return 0; case time::TimerMode::PERIODIC: return 1; }
    return -1;
}
int clock_domain(ClockDomain e) {
    switch (e) { case ClockDomain::MONOTONIC: return 0; case ClockDomain::REALTIME: return 1; }
    return -1;
}

void test_enumeration_values() {
    // The numeric value of each enumerator is the contract (stored, compared and switched on by clients).
    CHECK(v(ErrorSeverity::INFO) == 0 && v(ErrorSeverity::WARNING) == 1 && v(ErrorSeverity::ERROR) == 2 && v(ErrorSeverity::CRITICAL) == 3);
    for (auto e : {ErrorSeverity::INFO, ErrorSeverity::WARNING, ErrorSeverity::ERROR, ErrorSeverity::CRITICAL}) CHECK(severity(e) == v(e));
    const ErrorCode codes[] = {ErrorCode::NONE, ErrorCode::UNKNOWN, ErrorCode::INVALID_ARGUMENT, ErrorCode::INVALID_STATE, ErrorCode::NOT_INITIALIZED,
                               ErrorCode::NOT_READY, ErrorCode::ALREADY_RUNNING, ErrorCode::TIMEOUT, ErrorCode::RESOURCE_UNAVAILABLE,
                               ErrorCode::CONFIGURATION_ERROR, ErrorCode::UNSUPPORTED, ErrorCode::INTERNAL_ERROR};
    for (std::uint32_t i = 0; i < 12; ++i) { CHECK(v(codes[i]) == i); CHECK(error_code(codes[i]) == static_cast<int>(i)); }
    const StatusCode statuses[] = {StatusCode::UNKNOWN, StatusCode::OK, StatusCode::INVALID_ARGUMENT, StatusCode::NOT_READY, StatusCode::BUSY,
                                   StatusCode::TIMEOUT, StatusCode::FAILED, StatusCode::UNAVAILABLE, StatusCode::UNSUPPORTED, StatusCode::INTERNAL_ERROR};
    for (std::uint32_t i = 0; i < 10; ++i) { CHECK(v(statuses[i]) == i); CHECK(status_code(statuses[i]) == static_cast<int>(i)); }
    const HealthState healths[] = {HealthState::UNKNOWN, HealthState::HEALTHY, HealthState::DEGRADED, HealthState::UNHEALTHY};
    for (std::uint32_t i = 0; i < 4; ++i) { CHECK(v(healths[i]) == i); CHECK(health_state(healths[i]) == static_cast<int>(i)); }
    const LifecycleState lifecycles[] = {LifecycleState::UNKNOWN, LifecycleState::INITIALIZING, LifecycleState::READY, LifecycleState::RUNNING,
                                         LifecycleState::STOPPING, LifecycleState::STOPPED, LifecycleState::FAULT, LifecycleState::RECOVERING};
    for (std::uint32_t i = 0; i < 8; ++i) { CHECK(v(lifecycles[i]) == i); CHECK(lifecycle_state(lifecycles[i]) == static_cast<int>(i)); }
    const EventType events[] = {EventType::UNKNOWN, EventType::LIFECYCLE, EventType::STATUS, EventType::HEALTH, EventType::ERROR,
                                EventType::CONFIGURATION, EventType::CAPABILITY};
    for (std::uint32_t i = 0; i < 7; ++i) { CHECK(v(events[i]) == i); CHECK(event_type(events[i]) == static_cast<int>(i)); }
    const platform::PlatformService services[] = {platform::PlatformService::SCHEDULER, platform::PlatformService::CLOCK,
                                                  platform::PlatformService::TIMER, platform::PlatformService::WATCHDOG};
    for (std::uint32_t i = 0; i < 4; ++i) { CHECK(v(services[i]) == i); CHECK(platform_service(services[i]) == static_cast<int>(i)); }
    CHECK(v(platform::Requirement::REQUIRED) == 0 && v(platform::Requirement::OPTIONAL) == 1);
    CHECK(requirement(platform::Requirement::REQUIRED) == 0 && requirement(platform::Requirement::OPTIONAL) == 1);
    CHECK(v(time::TimerMode::ONE_SHOT) == 0 && v(time::TimerMode::PERIODIC) == 1);
    CHECK(timer_mode(time::TimerMode::ONE_SHOT) == 0 && timer_mode(time::TimerMode::PERIODIC) == 1);
    CHECK(v(ClockDomain::MONOTONIC) == 0 && v(ClockDomain::REALTIME) == 1);
    CHECK(clock_domain(ClockDomain::MONOTONIC) == 0 && clock_domain(ClockDomain::REALTIME) == 1);
}

// --- 2. Type properties of the stable value types (the properties COMPATIBILITY_POLICY.md names as contract) ---
// Columns: default-constructible, copy-constructible, copy-assignable, move-assignable, trivially copyable, aggregate.
#define KRITVA_COMPAT_TRAITS(T, DEF, CC, CA, MA, TC, AG)                                                                   \
    static_assert(std::is_default_constructible_v<T> == bool(DEF), #T ": default construction changed");                  \
    static_assert(std::is_copy_constructible_v<T> == bool(CC), #T ": copy construction changed");                         \
    static_assert(std::is_copy_assignable_v<T> == bool(CA), #T ": copy assignment changed");                              \
    static_assert(std::is_move_assignable_v<T> == bool(MA), #T ": move assignment changed");                              \
    static_assert(std::is_trivially_copyable_v<T> == bool(TC), #T ": triviality changed");                                \
    static_assert(std::is_aggregate_v<T> == bool(AG), #T ": aggregate shape changed")

namespace type_properties {
    using namespace runtime;
    KRITVA_COMPAT_TRAITS(Id, 1, 1, 1, 1, 1, 0);
    KRITVA_COMPAT_TRAITS(Version, 1, 1, 1, 1, 1, 1);
    KRITVA_COMPAT_TRAITS(Duration, 1, 1, 1, 1, 1, 0);
    KRITVA_COMPAT_TRAITS(Timestamp, 1, 1, 1, 1, 1, 0);
    KRITVA_COMPAT_TRAITS(Metadata, 1, 1, 1, 1, 0, 0);
    KRITVA_COMPAT_TRAITS(Callback, 1, 1, 1, 1, 1, 1);
    KRITVA_COMPAT_TRAITS(Error, 1, 1, 1, 1, 0, 1);
    KRITVA_COMPAT_TRAITS(Status, 1, 1, 1, 1, 0, 0);
    KRITVA_COMPAT_TRAITS(Health, 1, 1, 1, 1, 0, 0);
    KRITVA_COMPAT_TRAITS(Event, 1, 1, 1, 1, 1, 1);
    KRITVA_COMPAT_TRAITS(messaging::MessageHeader, 1, 1, 1, 1, 1, 1);
    KRITVA_COMPAT_TRAITS(messaging::Topic, 0, 1, 1, 1, 0, 0);
    KRITVA_COMPAT_TRAITS(Parameter, 1, 1, 1, 1, 0, 1);
    KRITVA_COMPAT_TRAITS(Configuration, 1, 1, 1, 1, 0, 0);
    KRITVA_COMPAT_TRAITS(Counter, 1, 1, 1, 1, 1, 0);
    KRITVA_COMPAT_TRAITS(Gauge, 1, 1, 1, 1, 1, 0);
    KRITVA_COMPAT_TRAITS(Statistics, 1, 1, 1, 1, 1, 1);
    KRITVA_COMPAT_TRAITS(Capability, 1, 1, 1, 1, 0, 1);
    KRITVA_COMPAT_TRAITS(CapabilitySet, 1, 1, 1, 1, 0, 0);
    KRITVA_COMPAT_TRAITS(Lifecycle, 1, 1, 1, 1, 1, 0);
    KRITVA_COMPAT_TRAITS(runtime::ComponentId, 1, 1, 1, 1, 1, 0);
    KRITVA_COMPAT_TRAITS(runtime::ComponentInfo, 0, 1, 1, 1, 0, 0);
    KRITVA_COMPAT_TRAITS(runtime::ComponentContext, 1, 1, 0, 0, 1, 0);
    KRITVA_COMPAT_TRAITS(runtime::ComponentRegistry, 1, 0, 0, 0, 0, 0);
    KRITVA_COMPAT_TRAITS(runtime::DependencyGraph, 1, 1, 1, 1, 0, 0);
    KRITVA_COMPAT_TRAITS(runtime::RuntimeManager, 1, 0, 0, 0, 0, 0);
    KRITVA_COMPAT_TRAITS(platform::PlatformContext, 1, 1, 1, 1, 1, 0);
    KRITVA_COMPAT_TRAITS(platform::PlatformRequirements, 1, 1, 1, 1, 0, 0);
    KRITVA_COMPAT_TRAITS(platform::ServiceRequirement, 1, 1, 1, 1, 1, 1);
    KRITVA_COMPAT_TRAITS(platform::CapabilityRequirement, 1, 1, 1, 1, 1, 1);
    KRITVA_COMPAT_TRAITS(platform::PlatformInfo, 1, 1, 1, 1, 0, 1);
    KRITVA_COMPAT_TRAITS(platform::PlatformRequirementReport, 1, 1, 1, 1, 0, 1);
    KRITVA_COMPAT_TRAITS(platform::TaskConfig, 1, 1, 1, 1, 1, 1);
    KRITVA_COMPAT_TRAITS(Result<int>, 0, 1, 1, 1, 0, 0);
    KRITVA_COMPAT_TRAITS(Result<void>, 0, 1, 1, 1, 0, 0);
} // namespace type_properties


// --- 3. Virtual interfaces clients implement: the exact set of pure virtuals and their signatures ---
// Each fixture overrides exactly the documented members with `override`; an added pure virtual makes the fixture abstract and a
// changed signature makes an `override` ill-formed, so either fails to compile. The interfaces themselves stay abstract with a
// virtual destructor.

struct FixtureComponent final : runtime::Component {
    explicit FixtureComponent(runtime::ComponentInfo info) : Component(std::move(info)) {}
    Result<void> configure(const Configuration&) override { return Result<void>::success(); }
    Result<void> initialize() override { return Result<void>::success(); }
    Result<void> start() override { return Result<void>::success(); }
    Result<void> stop() override { return Result<void>::success(); }
    Result<void> shutdown() override { return Result<void>::success(); }
    LifecycleState lifecycle_state() const noexcept override { return LifecycleState::UNKNOWN; }
    Status status() const override { return {}; }
    Health health() const override { return {}; }
    CapabilitySet capabilities() const override { return {}; }
};
struct FixtureRuntime final : runtime::Runtime {
    Result<void> initialize() override { return Result<void>::success(); }
    Result<void> start() override { return Result<void>::success(); }
    Result<void> stop() override { return Result<void>::success(); }
    Result<void> shutdown() override { return Result<void>::success(); }
    LifecycleState state() const noexcept override { return LifecycleState::UNKNOWN; }
};
struct FixtureScheduler final : platform::IScheduler {
    Result<platform::TaskId> create_task(const platform::TaskConfig&, void (*)(void*), void*) override { return Result<platform::TaskId>::success(platform::TaskId{}); }
    Result<void> start() override { return Result<void>::success(); }
    Result<void> stop() override { return Result<void>::success(); }
};
struct FixtureWatchdog final : platform::IWatchdog {
    Result<void> start(Duration) override { return Result<void>::success(); }
    Result<void> kick() override { return Result<void>::success(); }
    Result<void> stop() override { return Result<void>::success(); }
};
struct FixtureClock final : time::IClock {
    Timestamp now() const noexcept override { return {}; }
};
struct FixtureTimer final : time::ITimer {
    Result<void> start(Duration, time::TimerMode, Callback) override { return Result<void>::success(); }
    Result<void> stop() override { return Result<void>::success(); }
};
struct FixtureAdapter final : platform::IPlatformAdapter {
    platform::PlatformInfo info_{};
    const platform::PlatformInfo& info() const noexcept override { return info_; }
    platform::IScheduler* scheduler() const noexcept override { return nullptr; }
    time::IClock* clock() const noexcept override { return nullptr; }
    time::ITimer* timer() const noexcept override { return nullptr; }
    platform::IWatchdog* watchdog() const noexcept override { return nullptr; }
    CapabilitySet capabilities() const override { return {}; }
};
struct FixtureSink final : runtime::IEventSink {
    Result<void> report(const Event&) override { return Result<void>::success(); }
};
struct FixtureStatistics final : runtime::IComponentStatistics {
    Statistics statistics() const override { return {}; }
};

// Every interface is abstract with a virtual destructor, and each fixture above is a complete (non-abstract) implementation.
#define KRITVA_COMPAT_INTERFACE(I, F)                                                                              \
    static_assert(std::is_abstract_v<I> && std::has_virtual_destructor_v<I>, #I " must stay an abstract interface");  \
    static_assert(!std::is_abstract_v<F>, "an added pure virtual member makes " #F " abstract");                      \
    static_assert(std::is_base_of_v<I, F>)
KRITVA_COMPAT_INTERFACE(runtime::Component, FixtureComponent);
KRITVA_COMPAT_INTERFACE(runtime::Runtime, FixtureRuntime);
KRITVA_COMPAT_INTERFACE(platform::IScheduler, FixtureScheduler);
KRITVA_COMPAT_INTERFACE(platform::IWatchdog, FixtureWatchdog);
KRITVA_COMPAT_INTERFACE(time::IClock, FixtureClock);
KRITVA_COMPAT_INTERFACE(time::ITimer, FixtureTimer);
KRITVA_COMPAT_INTERFACE(platform::IPlatformAdapter, FixtureAdapter);
KRITVA_COMPAT_INTERFACE(runtime::IEventSink, FixtureSink);
KRITVA_COMPAT_INTERFACE(runtime::IComponentStatistics, FixtureStatistics);

// The Runtime implementation is final and a Runtime; the registry and manager are neither copyable nor movable.
static_assert(std::is_final_v<runtime::RuntimeManager> && std::is_base_of_v<runtime::Runtime, runtime::RuntimeManager>);

void test_interfaces_are_usable() {
    // The fixtures are real implementations: they construct and are usable through the interface (a compile-and-run check of
    // the call contract's entry points, not of behavior, which the focused tests cover).
    auto info = runtime::ComponentInfo::create(runtime::ComponentId{1}, "fixture");
    CHECK(static_cast<bool>(info));
    if (!info) return;
    FixtureComponent component(info.value());
    runtime::Component& c = component;
    CHECK(c.initialize().has_value() && c.start().has_value() && c.stop().has_value() && c.shutdown().has_value());
    FixtureRuntime rt;
    runtime::Runtime& r = rt;
    CHECK(r.initialize().has_value() && r.state() == LifecycleState::UNKNOWN);
    FixtureAdapter adapter;
    platform::IPlatformAdapter& a = adapter;
    CHECK(a.scheduler() == nullptr && a.clock() == nullptr && a.timer() == nullptr && a.watchdog() == nullptr && a.capabilities().empty());
}

} // namespace

int main() {
    test_enumeration_values();
    test_interfaces_are_usable();
    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "api_compat_boundary_test passed\n";
    return EXIT_SUCCESS;
}
