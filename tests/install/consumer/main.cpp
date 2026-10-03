//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : main.cpp
// Description : Minimal consumer that uses an installed Kritva Core.
//
// Component   : Kritva Core
// Module      : Install Test
// Layer       : Development Infrastructure
//
// Requirements: CORE-BUILD-002
// API         : CORE-TEST-INSTALL-CONSUMER
//
// Author      : KritvaOS Core Team
// Created     : 02-10-2026
//==============================================================================

// Uses only the installed public headers and the installed library: header-only
// types (Result, Status), and library code (Lifecycle, Version) that requires
// linking libkritva_core.

#include <kritva/core/core.hpp>

#include <string>
#include <utility>

// A minimal platform adapter (no services, no capabilities) implemented against the installed contract.
class BareAdapter final : public kritva::core::platform::IPlatformAdapter {
public:
    [[nodiscard]] const kritva::core::platform::PlatformInfo& info() const noexcept override { return info_; }
    [[nodiscard]] kritva::core::platform::IScheduler* scheduler() const noexcept override { return nullptr; }
    [[nodiscard]] kritva::core::time::IClock* clock() const noexcept override { return nullptr; }
    [[nodiscard]] kritva::core::time::ITimer* timer() const noexcept override { return nullptr; }
    [[nodiscard]] kritva::core::platform::IWatchdog* watchdog() const noexcept override { return nullptr; }
    [[nodiscard]] kritva::core::CapabilitySet capabilities() const override { return {}; }
private:
    kritva::core::platform::PlatformInfo info_{"bare", kritva::core::Version{1, 0, 0}};
};

int main() {
    using namespace kritva::core;

    Lifecycle lifecycle;
    if (!lifecycle.transition_to(LifecycleState::INITIALIZING)) return 1;
    if (lifecycle.transition_to(LifecycleState::RUNNING)) return 2;  // invalid transition

    const Result<int> ok = Result<int>::success(7);
    if (!ok || ok.value() != 7) return 3;

    const Status status(StatusCode::OK);
    if (status.code() != StatusCode::OK) return 4;

    if ((Version{0, 7, 0}).to_string() != "0.7.0") return 5;

    // Compiled runtime library code: the component registry.
    using namespace kritva::core::runtime;
    class Stub final : public Component {
    public:
        explicit Stub(ComponentInfo info) : Component(std::move(info)) {}
        bool fail_start{false};    // one-shot failure injection for the failure/recovery check below
        Result<void> configure(const Configuration&) override { return Result<void>::success(); }
        Result<void> initialize() override { return Result<void>::success(); }
        Result<void> start() override {
            if (fail_start) {
                fail_start = false;
                return Result<void>::failure(Error{ErrorCode::TIMEOUT, ErrorSeverity::ERROR, info().id(), {}, "stub start failed"});
            }
            return Result<void>::success();
        }
        Result<void> stop() override { return Result<void>::success(); }
        Result<void> shutdown() override { return Result<void>::success(); }
        LifecycleState lifecycle_state() const noexcept override { return LifecycleState::UNKNOWN; }
        Status status() const override { return Status{}; }
        Health health() const override { return Health{}; }
        CapabilitySet capabilities() const override { return CapabilitySet{}; }
    };
    auto info = ComponentInfo::create(ComponentId{1}, "stub");
    if (!info) return 6;
    Stub stub(std::move(info).value());
    ComponentRegistry registry;
    if (!registry.register_component(stub)) return 7;
    if (registry.register_component(stub)) return 8;           // duplicate id
    if (registry.find(ComponentId{1}) != &stub) return 9;

    // Dependency graph (compiled library code): one edge and a registered order.
    DependencyGraph graph;
    if (!graph.add_dependency(ComponentId{2}, ComponentId{1})) return 10;
    if (graph.add_dependency(ComponentId{1}, ComponentId{2})) return 11;   // would be a cycle
    const auto order = graph.order(registry);
    if (order) return 12;                                                    // id 2 is not registered

    // Runtime manager (compiled library code): topology validation then fixing.
    RuntimeManager manager;
    if (!manager.register_component(stub)) return 13;
    if (!manager.initialize()) return 14;
    if (manager.state() != LifecycleState::READY || !manager.topology_fixed()) return 15;
    if (manager.register_component(stub)) return 16;                         // topology is fixed
    if (!manager.start() || manager.state() != LifecycleState::RUNNING) return 17;
    if (!manager.stop() || !manager.shutdown()) return 18;                   // orchestrates the stub component
    if (manager.reset()) return 19;                                          // reset is only valid in FAULT
    if (manager.statistics().sample_count.value() == 0) return 20;           // runtime-owned statistics

    // Failure propagation and explicit recovery through the installed library.
    RuntimeManager faulty;
    if (!faulty.register_component(stub) || !faulty.initialize()) return 21;
    stub.fail_start = true;
    const auto failed = faulty.start();
    if (failed || failed.error().code != ErrorCode::TIMEOUT || failed.error().source != ComponentId{1}) return 22;   // original error
    if (faulty.state() != LifecycleState::FAULT || faulty.fault_error() == nullptr) return 23;
    if (faulty.stop()) return 24;                                            // FAULT rejects everything but reset()
    if (!faulty.reset() || faulty.state() != LifecycleState::STOPPED || faulty.fault_error() != nullptr) return 25;
    if (faulty.statistics().error_count.value() != 1) return 26;
    if (!faulty.initialize() || !faulty.start()) return 27;                  // a new explicit attempt

    // Platform adapter boundary (R0.4) through the installed headers and library.
    BareAdapter adapter;
    RuntimeManager platform_manager;
    if (platform_manager.platform() != nullptr) return 28;                   // optional: nothing attached by default
    if (!platform_manager.attach_platform(adapter)) return 29;
    if (platform_manager.platform() != &adapter) return 30;
    if (platform_manager.attach_platform(adapter)) return 31;                // never replaced, not even by itself
    for (const auto service : {platform::PlatformService::SCHEDULER, platform::PlatformService::CLOCK,
                               platform::PlatformService::TIMER, platform::PlatformService::WATCHDOG}) {
        if (adapter.supports(service)) return 32;                            // nullptr means unsupported
    }
    if (platform_manager.platform()->info().name != "bare") return 33;
    if (!platform_manager.register_component(stub) || !platform_manager.initialize()) return 34;
    const auto late = platform_manager.attach_platform(adapter);
    if (late || late.error().code != ErrorCode::INVALID_STATE) return 35;    // closed once the topology is fixed
    if (platform_manager.platform() != &adapter) return 36;

    // Platform context, requirements and explicit service consumption (R0.5) through the installed headers and library.
    const platform::PlatformContext context(platform_manager.platform());    // built from the Runtime's nullable pointer
    if (!context.attached() || context.info() == nullptr || context.info()->name != "bare") return 37;
    for (const auto service : {platform::PlatformService::SCHEDULER, platform::PlatformService::CLOCK,
                               platform::PlatformService::TIMER, platform::PlatformService::WATCHDOG}) {
        if (context.supports(service)) return 38;
    }
    const auto scheduler = context.require_scheduler();
    if (scheduler || scheduler.error().code != ErrorCode::UNSUPPORTED) return 39;     // an unavailable service is UNSUPPORTED
    if (context.require_watchdog().has_value()) return 40;
    if (context.has_capability(CapabilityId{100})) return 41;
    const platform::PlatformContext unattached(static_cast<platform::IPlatformAdapter*>(nullptr));
    if (unattached.attached() || unattached.require_timer().has_value()) return 42;
    platform::PlatformRequirements requirements;
    if (!requirements.add_service(platform::PlatformService::CLOCK, platform::Requirement::OPTIONAL)) return 43;
    if (!requirements.add_capability(CapabilityId{100}, platform::Requirement::OPTIONAL)) return 44;
    if (requirements.add_service(platform::PlatformService::CLOCK, platform::Requirement::REQUIRED)) return 45;   // a duplicate, by identity
    if (!platform::check_required(requirements, context)) return 46;                  // only optional items are missing
    if (!platform::evaluate(requirements, context).satisfied() || platform::evaluate(requirements, context).complete()) return 47;
    if (!requirements.add_service(platform::PlatformService::SCHEDULER, platform::Requirement::REQUIRED)) return 48;
    const auto needed = platform::check_required(requirements, context);
    if (needed || needed.error().code != ErrorCode::UNSUPPORTED) return 49;           // a required service is missing

    // Component execution context (R0.6) through the installed headers and library.
    const auto component_info = ComponentInfo::create(ComponentId{77}, "consumer");
    if (!component_info) return 50;
    const ComponentContext unbound;
    if (unbound.bound() || unbound.info() != nullptr || unbound.id().valid() || unbound.platform().attached()) return 51;
    const ComponentContext component_context(component_info.value(), context);
    if (!component_context.bound() || component_context.id() != ComponentId{77} || component_context.info() != &component_info.value()) return 52;
    if (!component_context.platform().attached() || component_context.platform().info() == nullptr) return 53;
    const auto no_scheduler = component_context.require_scheduler();
    if (no_scheduler || no_scheduler.error().code != ErrorCode::UNSUPPORTED) return 54;
    if (no_scheduler.error().source != ComponentId{77}) return 55;                       // the availability error is attributed to the component
    if (unbound.require_timer().error().source.valid()) return 56;                      // an unbound context has no identity to attribute
    if (component_context.supports(platform::PlatformService::TIMER) || component_context.has_capability(CapabilityId{100})) return 57;
    Error original;
    original.code = ErrorCode::TIMEOUT;
    original.severity = ErrorSeverity::WARNING;
    original.source = Id{5};
    original.message = "original";
    const Error attributed = component_context.attribute(original);                     // attribution replaces only the source
    if (attributed.source != ComponentId{77} || attributed.code != ErrorCode::TIMEOUT || attributed.severity != ErrorSeverity::WARNING || attributed.message != "original") return 58;
    if (unbound.attribute(original).source != Id{5}) return 59;
    platform::PlatformRequirements component_needs;
    if (!component_needs.add_service(platform::PlatformService::CLOCK, platform::Requirement::REQUIRED)) return 60;
    const auto component_check = component_context.check_required(component_needs);
    if (component_check || component_check.error().code != ErrorCode::UNSUPPORTED || component_check.error().source != ComponentId{77}) return 61;
    if (component_context.evaluate(component_needs).satisfied()) return 62;

    // Component operational information (R0.7) through the installed headers and library.
    class OpsComponent final : public Component, public IComponentStatistics {
    public:
        explicit OpsComponent(ComponentInfo info) : Component(std::move(info)) {}
        Result<void> configure(const Configuration&) override { return Result<void>::success(); }
        Result<void> initialize() override { return Result<void>::success(); }
        Result<void> start() override { return Result<void>::success(); }
        Result<void> stop() override { return Result<void>::success(); }
        Result<void> shutdown() override { return Result<void>::success(); }
        LifecycleState lifecycle_state() const noexcept override { return LifecycleState::RUNNING; }
        Status status() const override { Status s(StatusCode::OK); s.set_message("running"); return s; }
        Health health() const override { Health h(HealthState::DEGRADED); h.set_detail("slow"); return h; }
        CapabilitySet capabilities() const override { return CapabilitySet{}; }
        Statistics statistics() const override { Statistics s; s.sample_count.increment(3); s.queue_depth.set(-2); return s; }
    };
    class CountingSink final : public IEventSink {
    public:
        Result<void> report(const Event& event) override {
            ++calls;
            last = event;
            if (reject) return Result<void>::failure(Error{ErrorCode::RESOURCE_UNAVAILABLE, ErrorSeverity::WARNING, Id{900}, {}, "full"});
            return Result<void>::success();
        }
        int calls{0};
        bool reject{false};
        Event last{};
    };
    auto ops_info = ComponentInfo::create(ComponentId{88}, "ops");
    if (!ops_info) return 63;
    OpsComponent ops(std::move(ops_info).value());
    const ComponentObservation plain = observe(ops);
    if (plain.id != ComponentId{88} || plain.lifecycle != LifecycleState::RUNNING || plain.statistics.has_value()) return 64;   // no provider: nullopt
    if (plain.status.code() != StatusCode::OK || plain.status.message() != "running") return 65;
    if (plain.health.state() != HealthState::DEGRADED || plain.health.detail() != "slow") return 66;                            // independent of the lifecycle
    const ComponentObservation full = observe(ops, &ops);
    if (!full.statistics || full.statistics->sample_count.value() != 3 || full.statistics->queue_depth.value() != -2) return 67;
    CountingSink sink;
    const ComponentEventReporter reporter(ops, sink);
    if (!reporter.bound() || reporter.id() != ComponentId{88}) return 68;
    Event event{Id{1}, Id{}, EventType::HEALTH, Timestamp{5}, ErrorSeverity::WARNING, Id{2}};
    if (!reporter.report(event) || sink.calls != 1 || sink.last.source_id != ComponentId{88}) return 69;          // zero source is stamped
    event.source_id = Id{89};
    const auto foreign = reporter.report(event);
    if (foreign || foreign.error().code != ErrorCode::INVALID_ARGUMENT || foreign.error().source != ComponentId{88} || sink.calls != 1) return 70;
    sink.reject = true;
    event.source_id = ComponentId{88};
    const auto refused = reporter.report(event);
    if (refused || refused.error().code != ErrorCode::RESOURCE_UNAVAILABLE || refused.error().source != Id{900} || sink.calls != 2) return 71;   // the sink's Result, unchanged
    if (ComponentEventReporter{}.report(event).error().code != ErrorCode::INVALID_STATE) return 72;               // unbound

    // Component configuration contract (R0.8) through the installed headers and library.
    class ConfigComponent final : public Component {
    public:
        explicit ConfigComponent(ComponentInfo info) : Component(std::move(info)) {}
        Result<void> configure(const Configuration& c) override {
            if (state_ != LifecycleState::UNKNOWN && state_ != LifecycleState::STOPPED) return fail(ErrorCode::INVALID_STATE);
            if (auto structural = c.validate(); !structural) return fail(structural.error().code);
            const Parameter* rate = c.get("rate");
            if (rate == nullptr || !std::holds_alternative<std::int64_t>(rate->value)) return fail(ErrorCode::CONFIGURATION_ERROR);
            const std::int64_t r = std::get<std::int64_t>(rate->value);
            if (r < 1 || r > 100) return fail(ErrorCode::CONFIGURATION_ERROR);                  // semantic validation is the component's
            rate_ = r;                                                                          // staged, then committed: all or nothing
            return Result<void>::success();
        }
        Result<void> initialize() override { state_ = LifecycleState::READY; return Result<void>::success(); }
        Result<void> start() override { state_ = LifecycleState::RUNNING; return Result<void>::success(); }
        Result<void> stop() override { state_ = LifecycleState::STOPPED; return Result<void>::success(); }
        Result<void> shutdown() override { return Result<void>::success(); }
        LifecycleState lifecycle_state() const noexcept override { return state_; }
        Status status() const override { return Status{}; }
        Health health() const override { return Health{}; }
        CapabilitySet capabilities() const override { return CapabilitySet{}; }
        std::int64_t rate() const { return rate_; }
    private:
        Result<void> fail(ErrorCode code) const { return Result<void>::failure(Error{code, ErrorSeverity::ERROR, info().id(), {}, "configure"}); }
        LifecycleState state_{LifecycleState::UNKNOWN};
        std::int64_t rate_{0};
    };
    static_assert(std::is_same_v<ConfigurationVersion, Version>);                              // the schema/contract compatibility version, an alias
    Configuration good;
    if (!good.set(Parameter{"rate", std::int64_t{10}, "hz"}) || !good.validate()) return 73;
    if (good.set(Parameter{"", std::int64_t{1}, ""}).error().code != ErrorCode::INVALID_ARGUMENT || good.size() != 1) return 74;   // structural: atomic rejection
    Configuration out_of_range;
    if (!out_of_range.set(Parameter{"rate", std::int64_t{500}, ""}) || !out_of_range.validate()) return 75;     // structurally valid ...
    auto cfg_a = ComponentInfo::create(ComponentId{91}, "a"), cfg_b = ComponentInfo::create(ComponentId{92}, "b");
    if (!cfg_a || !cfg_b) return 76;
    ConfigComponent ca(std::move(cfg_a).value()), cb(std::move(cfg_b).value());
    const auto semantic = ca.configure(out_of_range);                                                          // ... but semantically rejected by the component
    if (semantic || semantic.error().code != ErrorCode::CONFIGURATION_ERROR || semantic.error().source != ComponentId{91} || ca.rate() != 0) return 77;
    RuntimeManager config_runtime;
    if (!config_runtime.register_component(ca) || !config_runtime.register_component(cb) || !config_runtime.add_dependency(ComponentId{92}, ComponentId{91})) return 78;
    if (!config_runtime.configure(good) || ca.rate() != 10 || cb.rate() != 10 || config_runtime.state() != LifecycleState::UNKNOWN) return 79;   // forwarded, state unchanged
    if (!config_runtime.initialize()) return 80;
    const auto not_eligible = config_runtime.configure(good);                                                       // valid only from UNKNOWN and STOPPED
    if (not_eligible || not_eligible.error().code != ErrorCode::INVALID_STATE || config_runtime.state() != LifecycleState::READY) return 81;
    if (!config_runtime.stop()) return 82;
    const auto rejected = config_runtime.configure(out_of_range);                                                // the first component rejects: stop, no rollback
    if (rejected || rejected.error().source != ComponentId{91} || config_runtime.state() != LifecycleState::STOPPED || config_runtime.fault_error() != nullptr || ca.rate() != 10) return 83;
    return 0;
}
