//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : reference_configuration.hpp
// Description : Test-only reference configuration harness (component, broken variants, fixture, call log).
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-CFG-012
// API         : CORE-TEST-CONFIGURATION-HARNESS
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#pragma once

// TEST SUPPORT ONLY (never compiled into or installed with the production library).
//
// A reusable, deterministic harness for the R0.8 configuration contract, built only on public
// APIs. It adds nothing to Core and models no real component:
//   - ReferenceConfigurableComponent  a conforming Component (lifecycle from the plain
//                    ReferenceComponent) whose configure() follows the contract exactly: state
//                    check first (valid only from UNKNOWN and STOPPED, INVALID_STATE with no other
//                    effect), then structural validation (Core), then semantic validation against
//                    a small schema (INVALID_ARGUMENT / CONFIGURATION_ERROR), staging of the whole
//                    input as a COPY, and a commit that cannot fail. One-shot failure injection at
//                    a chosen stage. Test-only accessors expose its applied state as a canonical
//                    string, call/evaluation counters and the address it was handed.
//   - Defect          a menu of deliberately broken behaviors of the same component (accepts an
//                    invalid state, changes the lifecycle, retains a reference, applies partially,
//                    retries ...), used to prove the conformance check detects them.
//   - ConfigurationFixture / ReferenceConfigurationFixture  what a generic conformance check needs
//                    to drive one component.
//   - ConfigurationCallLog  an ordered log of every configure() call made on harness components
//                    (component id, address of the Configuration it received, outcome), used to
//                    check Runtime forwarding: order, one call per component, same object,
//                    stop at the first failure, no retry, no rollback.

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <kritva/core/core.hpp>

#include "../contract/reference_component.hpp"

namespace kritva::core::runtime::contract {

struct ConfigurationCall {
    std::uint64_t component{0};
    const Configuration* object{nullptr};     // the address the component was handed
    bool accepted{false};
    ErrorCode code{ErrorCode::NONE};
};
using ConfigurationCallLog = std::vector<ConfigurationCall>;

/// Where a one-shot injected failure strikes, inside configure(), after the state check.
enum class FailAt : std::uint8_t { NONE, STRUCTURAL, SEMANTIC, BEFORE_COMMIT };

enum class Defect : std::uint8_t {
    NONE,
    // lifecycle eligibility (CORE-CFG-004)
    ACCEPTS_ANY_STATE,                  // configure() is not refused in READY / RUNNING / FAULT
    REJECTS_IN_UNKNOWN,                 // configure() is refused in UNKNOWN
    REJECTS_IN_STOPPED,                 // configure() is refused in STOPPED
    WRONG_CODE_IN_INVALID_STATE,        // a refusal uses CONFIGURATION_ERROR instead of INVALID_STATE
    WRONG_SOURCE_IN_INVALID_STATE,      // a refusal carries no component id as source
    INVALID_STATE_HAS_EFFECT,           // refuses with INVALID_STATE but still applies the configuration
    EVALUATES_IN_INVALID_STATE,         // refuses, but only after evaluating the configuration
    REFUSAL_CHANGES_LIFECYCLE,          // a refusal in READY starts the component
    SUCCESS_CHANGES_LIFECYCLE_FROM_UNKNOWN,   // a successful configure() advances UNKNOWN to READY
    SUCCESS_CHANGES_LIFECYCLE_FROM_STOPPED,   // a successful configure() advances STOPPED to READY
    FIRST_FAILURE_CHANGES_LIFECYCLE,    // a rejection of the first-ever configuration moves the component to FAULT
    LATER_FAILURE_CHANGES_LIFECYCLE,    // a rejection after an accepted configuration moves the component to FAULT
    // ownership (CORE-CFG-005)
    RETAINS_REFERENCE,                  // keeps the address of the caller's Configuration and reads through it
    RETAINS_PARAMETER_POINTER,          // keeps the pointer returned by get() beyond the call
    // atomic application (CORE-CFG-006)
    PARTIAL_APPLY,                      // applies parameters one by one and stops at the first bad one
    COMMIT_BEFORE_VALIDATE,             // overwrites the applied state, then validates
    FIRST_FAILURE_HALF_STATE,           // a first-ever failure leaves a half-populated applied state
    NEVER_REPLACES,                     // reports success for a second configuration but keeps the first
    ACCEPTS_BUT_NEVER_APPLIES,          // reports success but applies nothing
    POISONED_AFTER_REJECTION,           // after one rejection every later configuration fails
    RETRIES_ON_FAILURE,                 // evaluates the semantic stage twice after a failure
    WRONG_SOURCE_ON_REJECTION           // a rejection carries no component id as source
};

/// A small semantic schema: "rate" integer 1..1000 (required) and "name" non-empty string (optional).
struct ReferenceSchema {
    static constexpr std::int64_t kMinRate = 1;
    static constexpr std::int64_t kMaxRate = 1000;
};

class ReferenceConfigurableComponent : public ReferenceComponent {
public:
    using Applied = std::map<std::string, ParameterValue>;

    ReferenceConfigurableComponent(ComponentInfo info, Defect defect = Defect::NONE, ConfigurationCallLog* log = nullptr)
        : ReferenceComponent(std::move(info)), defect_(defect), log_(log) {}

    Result<void> configure(const Configuration& c) override {
        ++calls;
        last_object = &c;
        Result<void> result = apply(c);
        if (log_ != nullptr) log_->push_back(ConfigurationCall{info().id().value(), &c, static_cast<bool>(result), result ? ErrorCode::NONE : result.error().code});
        return result;
    }

    // ---- test-only observation ------------------------------------------------------------------
    /// Canonical string of the applied state ("" when nothing was ever accepted). A component that
    /// retains the caller's object reads through it, which is exactly what the checks detect.
    [[nodiscard]] std::string applied() const {
        if (defect_ == Defect::RETAINS_REFERENCE && retained_object_ != nullptr) return fingerprint(from(*retained_object_));
        if (defect_ == Defect::RETAINS_PARAMETER_POINTER && retained_rate_ != nullptr) {
            Applied copy = applied_;
            copy["rate"] = retained_rate_->value;
            return fingerprint(copy);
        }
        return fingerprint(applied_);
    }
    [[nodiscard]] static std::string fingerprint(const Applied& a) {
        std::ostringstream out;
        for (const auto& [name, value] : a) {
            out << name << '=';
            std::visit([&](const auto& v) { out << v; }, value);
            out << ';';
        }
        return out.str();
    }

    int calls{0};                       // every configure() call
    int evaluations{0};                 // semantic-stage evaluations (one per call that got past the state check)
    int accepted_count{0};              // configurations committed
    const Configuration* last_object{nullptr};
    FailAt fail_at{FailAt::NONE};       // one-shot injection, consumed when it strikes
    ErrorCode fail_code{ErrorCode::INTERNAL_ERROR};

private:
    static Applied from(const Configuration& c) {
        Applied a;
        for (const char* n : {"rate", "name"}) if (const Parameter* p = c.get(n)) a[n] = p->value;
        return a;
    }
    Result<void> failure_with(Id source, ErrorCode code, const char* message) {
        return Result<void>::failure(Error{code, ErrorSeverity::ERROR, source, {}, message});
    }
    Result<void> fail(ErrorCode code, const char* message) {                    // a rejection of the configuration itself
        return failure_with(defect_ == Defect::WRONG_SOURCE_ON_REJECTION ? Id{} : info().id(), code, message);
    }
    Result<void> reject(ErrorCode code, const char* message) {
        const bool first = accepted_count == 0;
        if ((defect_ == Defect::FIRST_FAILURE_CHANGES_LIFECYCLE && first) || (defect_ == Defect::LATER_FAILURE_CHANGES_LIFECYCLE && !first)) {
            fail_next_initialize = ErrorCode::INTERNAL_ERROR;                    // a failed configure() must not move the lifecycle: this one does
            (void)ReferenceComponent::initialize();                              // -> FAULT
        }
        poisoned_ = true;
        return fail(code, message);
    }
    bool strikes(FailAt where) {
        if (fail_at != where) return false;
        fail_at = FailAt::NONE;
        return true;
    }

    Result<void> apply(const Configuration& c) {
        // 1. lifecycle eligibility comes first and a refusal has no other effect (CORE-CFG-004)
        const LifecycleState s = lifecycle_state();
        bool eligible = s == LifecycleState::UNKNOWN || s == LifecycleState::STOPPED;
        if (defect_ == Defect::REJECTS_IN_UNKNOWN && s == LifecycleState::UNKNOWN) eligible = false;
        if (defect_ == Defect::REJECTS_IN_STOPPED && s == LifecycleState::STOPPED) eligible = false;
        if (defect_ == Defect::POISONED_AFTER_REJECTION && poisoned_ && eligible) return failure_with(info().id(), ErrorCode::CONFIGURATION_ERROR, "poisoned");
        if (!eligible && defect_ != Defect::ACCEPTS_ANY_STATE) {
            if (defect_ == Defect::INVALID_STATE_HAS_EFFECT) applied_ = from(c);
            if (defect_ == Defect::EVALUATES_IN_INVALID_STATE) ++evaluations;
            if (defect_ == Defect::REFUSAL_CHANGES_LIFECYCLE && s == LifecycleState::READY) (void)ReferenceComponent::start();
            return failure_with(defect_ == Defect::WRONG_SOURCE_IN_INVALID_STATE ? Id{} : info().id(),
                                defect_ == Defect::WRONG_CODE_IN_INVALID_STATE ? ErrorCode::CONFIGURATION_ERROR : ErrorCode::INVALID_STATE,
                                "configure is not valid in this lifecycle state");
        }
        if (defect_ == Defect::COMMIT_BEFORE_VALIDATE) applied_ = from(c);

        // 2. Core structural validation
        if (strikes(FailAt::STRUCTURAL)) return reject(fail_code, "injected structural failure");
        if (auto structural = c.validate(); !structural) return reject(structural.error().code, "structurally invalid");

        // 3. component semantic validation, with the whole input staged as a COPY
        ++evaluations;
        const Parameter* rate = c.get("rate");
        const Parameter* name = c.get("name");
        if (defect_ == Defect::PARTIAL_APPLY && rate != nullptr && std::holds_alternative<std::int64_t>(rate->value)) applied_["rate"] = rate->value;
        if (defect_ == Defect::FIRST_FAILURE_HALF_STATE && applied_.empty() && rate != nullptr) applied_["rate"] = rate->value;
        Result<void> semantic = Result<void>::success();
        if (strikes(FailAt::SEMANTIC)) semantic = fail(fail_code, "injected semantic failure");
        else if (rate == nullptr || !std::holds_alternative<std::int64_t>(rate->value)) semantic = fail(ErrorCode::CONFIGURATION_ERROR, "rate missing or not an integer");
        else if (std::get<std::int64_t>(rate->value) < ReferenceSchema::kMinRate || std::get<std::int64_t>(rate->value) > ReferenceSchema::kMaxRate) semantic = fail(ErrorCode::CONFIGURATION_ERROR, "rate out of range");
        else if (name != nullptr && (!std::holds_alternative<std::string>(name->value) || std::get<std::string>(name->value).empty())) semantic = fail(ErrorCode::CONFIGURATION_ERROR, "name must be a non-empty string");
        if (!semantic) {
            if (defect_ == Defect::RETRIES_ON_FAILURE) ++evaluations;            // a hidden second attempt
            return reject(semantic.error().code, "semantic rejection");
        }
        Applied staged = from(c);                                                // copies: nothing refers to the caller's object
        if (strikes(FailAt::BEFORE_COMMIT)) return reject(fail_code, "injected failure before commit");

        // 4. commit: cannot fail, replaces the applied state as a whole
        ++accepted_count;
        if (defect_ == Defect::NEVER_REPLACES && accepted_count > 1) return Result<void>::success();
        if (defect_ != Defect::ACCEPTS_BUT_NEVER_APPLIES) applied_.swap(staged);
        if (defect_ == Defect::RETAINS_REFERENCE) retained_object_ = &c;         // retained only after a success
        if (defect_ == Defect::RETAINS_PARAMETER_POINTER) retained_rate_ = c.get("rate");
        if (defect_ == Defect::SUCCESS_CHANGES_LIFECYCLE_FROM_UNKNOWN && s == LifecycleState::UNKNOWN) (void)ReferenceComponent::initialize();
        if (defect_ == Defect::SUCCESS_CHANGES_LIFECYCLE_FROM_STOPPED && s == LifecycleState::STOPPED) (void)ReferenceComponent::initialize();
        return Result<void>::success();
    }

    Defect defect_;
    ConfigurationCallLog* log_;
    Applied applied_;
    const Configuration* retained_object_{nullptr};
    const Parameter* retained_rate_{nullptr};
    bool poisoned_{false};
};

/// Builds the configurations a conformance check needs for the reference schema.
inline Configuration reference_valid(int n) {
    Configuration c;
    (void)c.set(Parameter{"rate", std::int64_t{10 + n * 10}, "hz"});
    (void)c.set(Parameter{"name", std::string(n == 0 ? "alpha" : "beta"), ""});
    return c;
}
inline Configuration reference_semantically_invalid() {          // structurally valid, rejected by the component
    Configuration c;
    (void)c.set(Parameter{"rate", std::int64_t{500}, "hz"});     // the first parameter is fine ...
    (void)c.set(Parameter{"name", std::string(), ""});           // ... the second is not
    return c;
}
} // namespace kritva::core::runtime::contract
