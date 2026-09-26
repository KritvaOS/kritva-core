//==============================================================================
// Kritva Core — Runtime Contract Tests
// SPDX-License-Identifier: Apache-2.0
//
// Requirements:
//   CORE-RT-001 : Component
//   CORE-RT-002 : Runtime
//
// These tests validate only the contracts exposed by the current Runtime
// headers. They do not impose lifecycle-transition policy or platform
// implementation behavior.
//==============================================================================

#include <cassert>
#include <type_traits>

#include "kritva/core/runtime/component.hpp"
#include "kritva/core/runtime/runtime.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;

namespace {

// -----------------------------------------------------------------------------
// Component test double
// -----------------------------------------------------------------------------

class FakeComponent final : public Component {
public:
    Result<void> configure(const Configuration& configuration) override {
        ++configure_count;
        configured = true;
        last_configuration = configuration;
        return Result<void>::success();
    }

    Result<void> initialize() override {
        ++initialize_count;
        return Result<void>::success();
    }

    Result<void> start() override {
        ++start_count;
        return Result<void>::success();
    }

    Result<void> stop() override {
        ++stop_count;
        return Result<void>::success();
    }

    Result<void> shutdown() override {
        ++shutdown_count;
        return Result<void>::success();
    }

    [[nodiscard]] LifecycleState lifecycle_state() const noexcept override {
        return lifecycle;
    }

    [[nodiscard]] Status status() const override {
        return component_status;
    }

    [[nodiscard]] Health health() const override {
        return component_health;
    }

    [[nodiscard]] CapabilitySet capabilities() const override {
        return component_capabilities;
    }

    Configuration last_configuration{};
    LifecycleState lifecycle{LifecycleState::UNKNOWN};
    Status component_status{};
    Health component_health{};
    CapabilitySet component_capabilities{};

    int configure_count{0};
    int initialize_count{0};
    int start_count{0};
    int stop_count{0};
    int shutdown_count{0};
    bool configured{false};
};

// -----------------------------------------------------------------------------
// Runtime test double
// -----------------------------------------------------------------------------

class FakeRuntime final : public Runtime {
public:
    Result<void> initialize() override {
        ++initialize_count;
        return Result<void>::success();
    }

    Result<void> start() override {
        ++start_count;
        return Result<void>::success();
    }

    Result<void> stop() override {
        ++stop_count;
        return Result<void>::success();
    }

    Result<void> shutdown() override {
        ++shutdown_count;
        return Result<void>::success();
    }

    [[nodiscard]] LifecycleState state() const noexcept override {
        return lifecycle;
    }

    LifecycleState lifecycle{LifecycleState::UNKNOWN};

    int initialize_count{0};
    int start_count{0};
    int stop_count{0};
    int shutdown_count{0};
};

// -----------------------------------------------------------------------------
// CORE-RT-001 — Component
// -----------------------------------------------------------------------------

void test_component_contract_shape() {
    static_assert(std::is_abstract_v<Component>);
    static_assert(std::is_polymorphic_v<Component>);
    static_assert(std::has_virtual_destructor_v<Component>);
}

void test_component_configure() {
    FakeComponent component;

    Configuration configuration;

    const Result<void> result = component.configure(configuration);

    assert(result);
    assert(result.has_value());
    assert(component.configure_count == 1);
    assert(component.configured);
}

void test_component_lifecycle_operations() {
    FakeComponent component;

    assert(component.initialize());
    assert(component.start());
    assert(component.stop());
    assert(component.shutdown());

    assert(component.initialize_count == 1);
    assert(component.start_count == 1);
    assert(component.stop_count == 1);
    assert(component.shutdown_count == 1);
}

void test_component_lifecycle_state() {
    FakeComponent component;

    const LifecycleState states[] = {
        LifecycleState::UNKNOWN,
        LifecycleState::INITIALIZING,
        LifecycleState::READY,
        LifecycleState::RUNNING,
        LifecycleState::STOPPING,
        LifecycleState::STOPPED,
        LifecycleState::FAULT,
        LifecycleState::RECOVERING
    };

    for (const LifecycleState expected : states) {
        component.lifecycle = expected;
        assert(component.lifecycle_state() == expected);
    }
}

void test_component_status() {
    FakeComponent component;

    component.component_status.set_code(StatusCode::OK);
    component.component_status.set_message("component ready");

    const Status status = component.status();

    assert(status.code() == StatusCode::OK);
    assert(status.message() == "component ready");
}

void test_component_health() {
    FakeComponent component;

    component.component_health.set_state(HealthState::HEALTHY);

    const Health health = component.health();

    assert(health.state() == HealthState::HEALTHY);
}

void test_component_capabilities() {
    FakeComponent component;

    const CapabilitySet capabilities = component.capabilities();

    assert(capabilities.all().empty());
}

void test_component_polymorphic_access() {
    FakeComponent implementation;
    Component& component = implementation;

    Configuration configuration;

    assert(component.configure(configuration));
    assert(component.initialize());
    assert(component.start());
    assert(component.stop());
    assert(component.shutdown());

    assert(component.lifecycle_state() == LifecycleState::UNKNOWN);
}

// -----------------------------------------------------------------------------
// CORE-RT-002 — Runtime
// -----------------------------------------------------------------------------

void test_runtime_contract_shape() {
    static_assert(std::is_abstract_v<Runtime>);
    static_assert(std::is_polymorphic_v<Runtime>);
    static_assert(std::has_virtual_destructor_v<Runtime>);
}

void test_runtime_lifecycle_operations() {
    FakeRuntime runtime;

    assert(runtime.initialize());
    assert(runtime.start());
    assert(runtime.stop());
    assert(runtime.shutdown());

    assert(runtime.initialize_count == 1);
    assert(runtime.start_count == 1);
    assert(runtime.stop_count == 1);
    assert(runtime.shutdown_count == 1);
}

void test_runtime_state() {
    FakeRuntime runtime;

    const LifecycleState states[] = {
        LifecycleState::UNKNOWN,
        LifecycleState::INITIALIZING,
        LifecycleState::READY,
        LifecycleState::RUNNING,
        LifecycleState::STOPPING,
        LifecycleState::STOPPED,
        LifecycleState::FAULT,
        LifecycleState::RECOVERING
    };

    for (const LifecycleState expected : states) {
        runtime.lifecycle = expected;
        assert(runtime.state() == expected);
    }
}

void test_runtime_polymorphic_access() {
    FakeRuntime implementation;
    Runtime& runtime = implementation;

    assert(runtime.initialize());
    assert(runtime.start());
    assert(runtime.stop());
    assert(runtime.shutdown());

    assert(runtime.state() == LifecycleState::UNKNOWN);
}

} // namespace

int main() {
    // CORE-RT-001
    test_component_contract_shape();
    test_component_configure();
    test_component_lifecycle_operations();
    test_component_lifecycle_state();
    test_component_status();
    test_component_health();
    test_component_capabilities();
    test_component_polymorphic_access();

    // CORE-RT-002
    test_runtime_contract_shape();
    test_runtime_lifecycle_operations();
    test_runtime_state();
    test_runtime_polymorphic_access();

    return 0;
}
