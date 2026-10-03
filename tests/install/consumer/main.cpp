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

int main() {
    using namespace kritva::core;

    Lifecycle lifecycle;
    if (!lifecycle.transition_to(LifecycleState::INITIALIZING)) return 1;
    if (lifecycle.transition_to(LifecycleState::RUNNING)) return 2;  // invalid transition

    const Result<int> ok = Result<int>::success(7);
    if (!ok || ok.value() != 7) return 3;

    const Status status(StatusCode::OK);
    if (status.code() != StatusCode::OK) return 4;

    if ((Version{0, 2, 0}).to_string() != "0.2.0") return 5;

    // Compiled library code added after R0.2: the component registry.
    using namespace kritva::core::runtime;
    class Stub final : public Component {
    public:
        explicit Stub(ComponentInfo info) : Component(std::move(info)) {}
        Result<void> configure(const Configuration&) override { return Result<void>::success(); }
        Result<void> initialize() override { return Result<void>::success(); }
        Result<void> start() override { return Result<void>::success(); }
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
    return 0;
}
