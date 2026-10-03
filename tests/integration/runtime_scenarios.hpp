//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : runtime_scenarios.hpp
// Description : Seeded Runtime scenarios that can be replayed with and without a platform.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-015
// API         : CORE-TEST-RUNTIME-SCENARIOS
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <functional>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include <kritva/core/core.hpp>

#include "../contract/reference_component.hpp"

namespace kritva::core::runtime::scenarios {

using Ids = std::vector<std::uint64_t>;
using Edge = std::pair<std::uint64_t, std::uint64_t>;   // (dependent, dependency)
using Trace = std::vector<std::string>;

/// One Runtime operation, with an optional one-shot failure injected into one component's operation.
struct Step {
    int op{0};                 // 0 configure 1 initialize 2 start 3 stop 4 shutdown 5 reset
    std::uint64_t inject_id{0};
    int inject_op{0};
    ErrorCode code{ErrorCode::NONE};
};

struct Script {
    std::size_t n{0};
    Ids registration;
    std::vector<Edge> edges;
    std::vector<Step> steps;
};

/// Reference components plus a RuntimeManager, optionally with an attached platform adapter.
struct World {
    Trace trace;
    std::vector<std::unique_ptr<contract::ReferenceComponent>> components;
    RuntimeManager runtime;

    /// Builds one component from its info; the default builds a plain ReferenceComponent.
    using Factory = std::function<std::unique_ptr<contract::ReferenceComponent>(ComponentInfo)>;

    World(const Script& script, platform::IPlatformAdapter* adapter, const Factory& make = {}) {
        components.resize(script.n);
        for (std::uint64_t id : script.registration) {
            auto info = ComponentInfo::create(ComponentId{id}, "c");
            components[id - 1] = make ? make(std::move(info).value())
                                      : std::make_unique<contract::ReferenceComponent>(std::move(info).value());
            components[id - 1]->trace = &trace;
            assert(runtime.register_component(*components[id - 1]));
        }
        for (const Edge& e : script.edges) assert(runtime.add_dependency(ComponentId{e.first}, ComponentId{e.second}));
        if (adapter != nullptr) assert(runtime.attach_platform(*adapter));
    }
};

inline std::string describe(const Result<void>& r) {
    if (r) return "ok";
    return std::to_string(static_cast<int>(r.error().code)) + "/" + std::to_string(static_cast<int>(r.error().severity)) + "/" +
           std::to_string(r.error().source.value()) + "/" + r.error().message;
}

/// What a Runtime step looks like from outside: result, state, topology, fault and statistics.
inline std::string observe(const World& world, int op, const Result<void>& r) {
    const Statistics& st = world.runtime.statistics();
    const Error* fault = world.runtime.fault_error();
    return std::to_string(op) + " -> " + describe(r) + " state=" + std::to_string(static_cast<int>(world.runtime.state())) +
           " fixed=" + std::to_string(world.runtime.topology_fixed()) +
           " fault=" + (fault ? std::to_string(static_cast<int>(fault->code)) + "/" + std::to_string(fault->source.value()) + "/" + fault->message : "none") +
           " samples=" + std::to_string(st.sample_count.value()) + " errors=" + std::to_string(st.error_count.value()) +
           " retries=" + std::to_string(st.retry_count.value());
}

inline void inject(World& world, const Step& step) {
    for (auto& c : world.components)
        c->fail_next_configure = c->fail_next_initialize = c->fail_next_start = c->fail_next_stop = c->fail_next_shutdown = ErrorCode::NONE;
    if (step.inject_id != 0 && step.code != ErrorCode::NONE) {
        contract::ReferenceComponent& c = *world.components[step.inject_id - 1];
        ErrorCode* slot[] = {&c.fail_next_configure, &c.fail_next_initialize, &c.fail_next_start, &c.fail_next_stop, &c.fail_next_shutdown};
        *slot[step.inject_op % 5] = step.code;
    }
}

inline Result<void> apply(World& world, int op) {
    switch (op) {
        case 0: return world.runtime.configure(Configuration{});
        case 1: return world.runtime.initialize();
        case 2: return world.runtime.start();
        case 3: return world.runtime.stop();
        case 4: return world.runtime.shutdown();
        default: return world.runtime.reset();
    }
}

/// Hooks around each Runtime step: the integrator's side of the world (using the platform, starting services...).
struct Hooks {
    std::function<void(World&, std::size_t)> before;   // before the Runtime operation of step i
    std::function<void(World&, std::size_t)> after;    // after it
};

/// Runs the script and returns a transcript of everything observable through the Runtime.
inline Trace run(World& world, const Script& script, const Hooks& hooks = {}) {
    Trace out;
    for (std::size_t i = 0; i < script.steps.size(); ++i) {
        const Step& step = script.steps[i];
        inject(world, step);
        if (hooks.before) hooks.before(world, i);
        const Result<void> r = apply(world, step.op);
        if (hooks.after) hooks.after(world, i);
        out.push_back(observe(world, step.op, r));
    }
    out.insert(out.end(), world.trace.begin(), world.trace.end());   // the component invocation trace
    return out;
}

inline Script random_script(std::mt19937& rng, int steps = 40) {
    static const ErrorCode codes[] = {ErrorCode::TIMEOUT, ErrorCode::NOT_READY, ErrorCode::RESOURCE_UNAVAILABLE, ErrorCode::INTERNAL_ERROR, ErrorCode::UNSUPPORTED};
    Script script;
    script.n = 1 + rng() % 7;
    Ids rank(script.n);
    for (std::size_t i = 0; i < script.n; ++i) rank[i] = i + 1;
    std::shuffle(rank.begin(), rank.end(), rng);
    for (std::size_t a = 0; a < script.n; ++a)
        for (std::size_t b = 0; b < script.n; ++b)
            if (rank[a] > rank[b] && rng() % 3 == 0) script.edges.push_back({a + 1, b + 1});
    std::shuffle(script.edges.begin(), script.edges.end(), rng);
    script.registration.resize(script.n);
    for (std::size_t i = 0; i < script.n; ++i) script.registration[i] = i + 1;
    std::shuffle(script.registration.begin(), script.registration.end(), rng);
    for (int i = 0; i < steps; ++i) {
        Step step;
        step.op = static_cast<int>(rng() % 6);
        if (rng() % 3 == 0) {
            step.inject_id = 1 + rng() % script.n;
            step.inject_op = static_cast<int>(rng() % 5);
            step.code = codes[rng() % 5];
        }
        script.steps.push_back(step);
    }
    return script;
}

} // namespace kritva::core::runtime::scenarios
