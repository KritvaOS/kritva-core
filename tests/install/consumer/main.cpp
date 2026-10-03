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

    if ((Version{0, 4, 0}).to_string() != "0.4.0") return 5;

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
    return 0;
}
