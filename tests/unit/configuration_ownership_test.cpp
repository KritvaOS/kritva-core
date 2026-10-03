//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : configuration_ownership_test.cpp
// Description : Contract tests for configuration input ownership and atomic application.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-CFG-005, CORE-CFG-006
// API         : CORE-API-CONFIGURATION
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <kritva/core/core.hpp>

using namespace kritva::core;
using namespace kritva::core::runtime;

namespace {

ComponentInfo make_info(std::uint64_t id) {
    auto info = ComponentInfo::create(ComponentId{id}, "cfg");
    assert(info);
    return std::move(info).value();
}

Configuration make_config(std::int64_t rate, const std::string& name, bool include_bad = false) {
    Configuration c;
    assert(c.set(Parameter{"rate", rate, "hz"}));
    assert(c.set(Parameter{"name", name, ""}));
    if (include_bad) assert(c.set(Parameter{"mode", std::string("invalid"), ""}));   // fails semantic validation below
    return c;
}

// A component with a small applied state: (rate, name). Semantic rule: rate must be 1..1000, name non-empty,
// and no "mode" parameter other than "auto". The Mode selects a conforming or a deliberately broken implementation.
enum class Defect {
    NONE,
    RETAINS_POINTER,          // keeps the address of the caller's Configuration and reads through it later
    PARTIAL_COMMIT,           // commits `rate` before validating the rest
    COMMIT_BEFORE_VALIDATE,   // overwrites the applied state, then validates (leaves the new state on failure)
    KEEPS_POINTER_TO_PARAM,   // keeps the pointer returned by get() beyond the call
    HALF_STATE_ON_FIRST_FAIL, // a first-ever failing configuration leaves a partly populated state
    CHANGES_STATE_ON_FAILURE  // failure moves the lifecycle state
};

class AppliedComponent final : public Component {
public:
    AppliedComponent(ComponentInfo info, Defect defect) : Component(std::move(info)), defect_(defect) {}

    Result<void> configure(const Configuration& c) override {
        if (state_ != LifecycleState::UNKNOWN && state_ != LifecycleState::STOPPED) {
            return fail(ErrorCode::INVALID_STATE, "configure not valid in this state");
        }
        if (defect_ == Defect::RETAINS_POINTER) retained_ = &c;
        if (defect_ == Defect::KEEPS_POINTER_TO_PARAM) retained_param_ = c.get("rate");
        if (defect_ == Defect::COMMIT_BEFORE_VALIDATE) { if (const Parameter* p = c.get("rate")) rate_ = std::get<std::int64_t>(p->value); }

        const Parameter* rate = c.get("rate");
        const Parameter* name = c.get("name");
        if (defect_ == Defect::PARTIAL_COMMIT && rate != nullptr) rate_ = std::get<std::int64_t>(rate->value);
        if (defect_ == Defect::HALF_STATE_ON_FIRST_FAIL && rate != nullptr) rate_ = std::get<std::int64_t>(rate->value);

        auto reject = [&](const char* m) {
            if (defect_ == Defect::CHANGES_STATE_ON_FAILURE) state_ = LifecycleState::FAULT;
            return fail(ErrorCode::CONFIGURATION_ERROR, m);
        };
        if (rate == nullptr || !std::holds_alternative<std::int64_t>(rate->value)) return reject("rate missing");
        const std::int64_t r = std::get<std::int64_t>(rate->value);
        if (r < 1 || r > 1000) return reject("rate out of range");
        if (name == nullptr || !std::holds_alternative<std::string>(name->value) || std::get<std::string>(name->value).empty()) return reject("name missing");
        if (const Parameter* mode = c.get("mode"); mode != nullptr) {
            if (!std::holds_alternative<std::string>(mode->value) || std::get<std::string>(mode->value) != "auto") return reject("mode rejected");
        }
        // Stage fully, then commit with nothing that can fail.
        const std::string staged_name = std::get<std::string>(name->value);
        rate_ = r;
        name_ = staged_name;
        accepted_ = true;
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

    // Test-only observation of the applied state. A component that retains the caller's object reads through it.
    std::int64_t applied_rate() const {
        if (defect_ == Defect::RETAINS_POINTER && retained_ != nullptr) return std::get<std::int64_t>(retained_->get("rate")->value);
        if (defect_ == Defect::KEEPS_POINTER_TO_PARAM && retained_param_ != nullptr) return std::get<std::int64_t>(retained_param_->value);
        return rate_;
    }
    const std::string& applied_name() const { return name_; }
    bool accepted() const { return accepted_; }

private:
    Result<void> fail(ErrorCode code, const char* message) const {
        return Result<void>::failure(Error{code, ErrorSeverity::ERROR, info().id(), {}, message});
    }
    Defect defect_;
    LifecycleState state_{LifecycleState::UNKNOWN};
    std::int64_t rate_{0};
    std::string name_;
    bool accepted_{false};
    const Configuration* retained_{nullptr};
    const Parameter* retained_param_{nullptr};
};

// The reusable shape of the check: returns the first violation, or an empty string.
std::string check_ownership_and_atomicity(Defect defect) {
    // --- ownership: the caller's object may change or be destroyed after configure() ---------------
    {
        AppliedComponent c(make_info(1), defect);
        auto owned = std::make_unique<Configuration>(make_config(10, "alpha"));
        if (!c.configure(*owned)) return "a valid configuration was rejected";
        if (c.applied_rate() != 10 || c.applied_name() != "alpha") return "the applied state does not match the configuration";
        assert(owned->set(Parameter{"rate", std::int64_t{999}, ""}));            // the caller changes its object ...
        if (c.applied_rate() != 10) return "the component reads through the caller's Configuration after the call";
        owned.reset();                                                           // ... and destroys it
        if (c.applied_rate() != 10 || c.applied_name() != "alpha") return "the applied state changed when the caller's object went away";
    }
    // --- atomicity: a failing configuration leaves the previously accepted one untouched -------------
    {
        AppliedComponent c(make_info(2), defect);
        if (!c.configure(make_config(10, "alpha"))) return "a valid configuration was rejected";
        const Configuration bad = make_config(500, "beta", /*include_bad=*/true);   // rate and name are fine, mode is rejected
        const auto r = c.configure(bad);
        if (r || r.error().code != ErrorCode::CONFIGURATION_ERROR) return "a semantically invalid configuration was accepted";
        if (c.applied_rate() != 10) return "a rejected configuration partially replaced the applied rate";
        if (c.applied_name() != "alpha") return "a rejected configuration partially replaced the applied name";
        if (c.lifecycle_state() != LifecycleState::UNKNOWN) return "a configuration failure changed the lifecycle state";
        const Configuration out_of_range = make_config(5000, "gamma");
        if (c.configure(out_of_range) || c.applied_rate() != 10) return "an out-of-range rate partially applied";
    }
    // --- a first-ever failure leaves the initial state, not a half-populated one ---------------------
    {
        AppliedComponent c(make_info(3), defect);
        const Configuration bad = make_config(20, "", /*include_bad=*/false);       // empty name: rejected after the rate was readable
        if (c.configure(bad)) return "an empty name was accepted";
        if (c.accepted() || c.applied_rate() != 0 || !c.applied_name().empty()) return "a first failing configuration left partial state";
    }
    // --- the new valid configuration after a failure applies completely -------------------------------
    {
        AppliedComponent c(make_info(4), defect);
        (void)c.configure(make_config(5000, "x"));
        if (!c.configure(make_config(30, "delta")) || c.applied_rate() != 30 || c.applied_name() != "delta") return "a valid configuration after a failure did not apply";
    }
    return {};
}

void test_the_conforming_component_passes_the_check() {
    assert(check_ownership_and_atomicity(Defect::NONE).empty());
}

void test_every_broken_component_is_detected() {
    for (const Defect d : {Defect::RETAINS_POINTER, Defect::PARTIAL_COMMIT, Defect::COMMIT_BEFORE_VALIDATE, Defect::KEEPS_POINTER_TO_PARAM,
                           Defect::HALF_STATE_ON_FIRST_FAIL, Defect::CHANGES_STATE_ON_FAILURE}) {
        assert(!check_ownership_and_atomicity(d).empty());
    }
}

void test_configuration_is_a_detached_copyable_value() {
    static_assert(std::is_copy_constructible_v<Configuration> && std::is_copy_assignable_v<Configuration>);
    static_assert(std::is_nothrow_move_constructible_v<Configuration> || std::is_move_constructible_v<Configuration>);
    Configuration original = make_config(10, "alpha");
    Configuration copy = original;
    assert(original.set(Parameter{"rate", std::int64_t{20}, ""}));
    assert(original.set(Parameter{"extra", true, ""}));
    assert(std::get<std::int64_t>(copy.get("rate")->value) == 10 && !copy.contains("extra") && copy.size() == 2);   // independent of its source
    assert(copy.set(Parameter{"name", std::string("beta"), ""}));
    assert(std::get<std::string>(original.get("name")->value) == "alpha");                                         // and the other way round
    Configuration assigned;
    assigned = original;
    assert(original.set(Parameter{"rate", std::int64_t{30}, ""}));
    assert(std::get<std::int64_t>(assigned.get("rate")->value) == 20);
    Configuration moved = std::move(assigned);
    assert(std::get<std::int64_t>(moved.get("rate")->value) == 20);
}

void test_configure_takes_the_input_by_const_reference_and_accepts_a_temporary() {
    static_assert(std::is_same_v<decltype(&Component::configure), Result<void> (Component::*)(const Configuration&)>);
    AppliedComponent c(make_info(5), Defect::NONE);
    assert(c.configure(make_config(7, "temp")));                                   // a temporary dies right after the call
    assert(c.applied_rate() == 7 && c.applied_name() == "temp");
}

void test_the_runtime_forwards_the_callers_object_and_keeps_nothing() {
    class AddressProbe final : public Component {
    public:
        explicit AddressProbe(ComponentInfo info) : Component(std::move(info)) {}
        Result<void> configure(const Configuration& c) override { seen = &c; rate = std::get<std::int64_t>(c.get("rate")->value); return Result<void>::success(); }
        Result<void> initialize() override { return Result<void>::success(); }
        Result<void> start() override { return Result<void>::success(); }
        Result<void> stop() override { return Result<void>::success(); }
        Result<void> shutdown() override { return Result<void>::success(); }
        LifecycleState lifecycle_state() const noexcept override { return LifecycleState::UNKNOWN; }
        Status status() const override { return Status{}; }
        Health health() const override { return Health{}; }
        CapabilitySet capabilities() const override { return CapabilitySet{}; }
        const Configuration* seen{nullptr};
        std::int64_t rate{0};
    };
    AddressProbe a(make_info(1)), b(make_info(2));
    RuntimeManager runtime;
    assert(runtime.register_component(a) && runtime.register_component(b));
    auto owned = std::make_unique<Configuration>(make_config(10, "alpha"));
    assert(runtime.configure(*owned));
    assert(a.seen == owned.get() && b.seen == owned.get());                          // the same object, not a copy and not a Runtime-held one
    owned.reset();                                                                   // the caller destroys it ...
    assert(runtime.initialize() && runtime.start() && runtime.stop());               // ... the Runtime never needs it again
    assert(a.rate == 10 && b.rate == 10);
    auto second = std::make_unique<Configuration>(make_config(20, "beta"));          // reconfiguration from STOPPED (topology now fixed)
    assert(runtime.configure(*second));
    assert(a.seen == second.get() && b.seen == second.get() && a.rate == 20 && b.rate == 20);
    second.reset();
    assert(runtime.shutdown());
}

} // namespace

int main() {
    test_the_conforming_component_passes_the_check();
    test_every_broken_component_is_detected();
    test_configuration_is_a_detached_copyable_value();
    test_configure_takes_the_input_by_const_reference_and_accepts_a_temporary();
    test_the_runtime_forwards_the_callers_object_and_keeps_nothing();
    return 0;
}
