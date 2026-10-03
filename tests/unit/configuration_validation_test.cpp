//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : configuration_validation_test.cpp
// Description : Contract tests for the validation boundary and ConfigurationVersion semantics.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-CFG-007, CORE-CFG-008, CORE-CFG-011
// API         : CORE-API-CONFIGURATION
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>

#include <kritva/core/core.hpp>

using namespace kritva::core;
using namespace kritva::core::runtime;

namespace {

ComponentInfo make_info(std::uint64_t id) {
    auto info = ComponentInfo::create(ComponentId{id}, "cfg");
    assert(info);
    return std::move(info).value();
}

template <class T> concept HasVersionMember = requires(const T t) { t.version(); };
template <class T> concept HasRevisionMember = requires(const T t) { t.revision(); };
template <class T> concept HasGenerationMember = requires(const T t) { t.generation(); };
template <class T> concept HasTransactionMember = requires(const T t) { t.transaction_id(); };
template <class T> concept HasHistoryMember = requires(const T t) { t.history(); };
template <class T> concept HasSetVersion = requires(T t, Version v) { t.set_version(v); };
template <class T> concept HasRangeMember = requires(const T t) { t.range("a"); };
template <class T> concept HasDefaultMember = requires(const T t) { t.default_value("a"); };
template <class T> concept HasSchemaMember = requires(const T t) { t.schema(); };

// A component that owns a schema: it understands configuration schema 2.x, requires "rate" in 1..1000,
// reads the schema version from an ordinary Parameter named "schema" (its OWN convention, not Core's),
// and applies all-or-nothing.
class SchemaComponent final : public Component {
public:
    explicit SchemaComponent(ComponentInfo info) : Component(std::move(info)) {}
    Result<void> configure(const Configuration& c) override {
        if (state_ != LifecycleState::UNKNOWN && state_ != LifecycleState::STOPPED) return fail(ErrorCode::INVALID_STATE, "state");
        if (auto structural = c.validate(); !structural) return fail(structural.error().code, "structurally invalid");   // Core's part
        const Parameter* schema = c.get("schema");
        if (schema == nullptr || !std::holds_alternative<std::int64_t>(schema->value)) return fail(ErrorCode::CONFIGURATION_ERROR, "schema version missing");
        if (std::get<std::int64_t>(schema->value) / 100 != kSupported.major) return fail(ErrorCode::CONFIGURATION_ERROR, "incompatible schema");   // the component's policy
        const Parameter* rate = c.get("rate");
        if (rate == nullptr || !std::holds_alternative<std::int64_t>(rate->value)) return fail(ErrorCode::CONFIGURATION_ERROR, "rate missing");
        const std::int64_t r = std::get<std::int64_t>(rate->value);
        if (r < 1 || r > 1000) return fail(ErrorCode::CONFIGURATION_ERROR, "rate out of range");                       // semantic, the component's part
        rate_ = r;
        ++applied_count;
        return Result<void>::success();
    }
    Result<void> initialize() override { return Result<void>::success(); }
    Result<void> start() override { return Result<void>::success(); }
    Result<void> stop() override { return Result<void>::success(); }
    Result<void> shutdown() override { return Result<void>::success(); }
    LifecycleState lifecycle_state() const noexcept override { return state_; }
    Status status() const override { return Status{}; }
    Health health() const override { return Health{}; }
    CapabilitySet capabilities() const override { return CapabilitySet{}; }
    std::int64_t rate() const { return rate_; }
    static constexpr ConfigurationVersion kSupported{2, 0, 0};
    int applied_count{0};
private:
    Result<void> fail(ErrorCode code, const char* message) const {
        return Result<void>::failure(Error{code, ErrorSeverity::ERROR, info().id(), {}, message});
    }
    LifecycleState state_{LifecycleState::UNKNOWN};
    std::int64_t rate_{0};
};

Configuration schema_config(std::int64_t schema, std::int64_t rate) {
    Configuration c;
    assert(c.set(Parameter{"schema", schema, ""}));
    assert(c.set(Parameter{"rate", rate, ""}));
    return c;
}

// ---- 1. Core structural validation -------------------------------------------------------------
void test_validate_is_the_const_side_effect_free_structural_entry_point() {
    static_assert(std::is_same_v<decltype(&Configuration::validate), Result<void> (Configuration::*)() const>);
    Configuration c;
    assert(c.validate());                                                     // an empty Configuration is structurally valid
    assert(c.set(Parameter{"a", true, ""}));
    assert(c.validate() && c.validate());                                     // repeatable, deterministic
    assert(c.size() == 1 && c.contains("a"));                                 // and it changed nothing
}

void test_an_empty_name_is_rejected_atomically_at_set() {
    Configuration c;
    assert(c.set(Parameter{"keep", std::int64_t{1}, ""}));
    const auto r = c.set(Parameter{"", std::int64_t{2}, "x"});
    assert(!r && r.error().code == ErrorCode::INVALID_ARGUMENT);
    assert(c.size() == 1 && c.contains("keep") && !c.contains(""));           // the container is unchanged
    assert(c.validate());
}

void test_validate_succeeds_for_every_constructible_configuration() {
    // The structural invariant is enforced where a Parameter enters, so any Configuration built through the public API validates.
    const std::string names[] = {"a", " ", "  leading", "trailing  ", "with space", "dots.and.slashes/", std::string(4096, 'n'), "\xCE\xA9\xCE\xBC", "\t", "0"};
    Configuration c;
    for (const std::string& n : names) {
        assert(c.set(Parameter{n, std::int64_t{1}, ""}));
        assert(c.validate());
    }
    Configuration copy = c;
    assert(copy.validate() && c.size() == copy.size());
    Configuration moved = std::move(copy);
    assert(moved.validate());
}

void test_the_container_accepts_any_value_it_knows_no_ranges_or_types() {
    Configuration c;
    assert(c.set(Parameter{"empty_string", std::string(), ""}));
    assert(c.set(Parameter{"min", std::numeric_limits<std::int64_t>::min(), ""}));
    assert(c.set(Parameter{"max", std::numeric_limits<std::int64_t>::max(), ""}));
    assert(c.set(Parameter{"nan", std::numeric_limits<double>::quiet_NaN(), ""}));
    assert(c.set(Parameter{"inf", std::numeric_limits<double>::infinity(), ""}));
    assert(c.set(Parameter{"flag", false, ""}));
    assert(c.set(Parameter{"described", std::int64_t{0}, std::string(1000, 'd')}));
    assert(c.validate() && c.size() == 7);
    assert(std::isnan(std::get<double>(c.get("nan")->value)));                // stored as given: no normalization
    assert(std::get<std::int64_t>(c.get("min")->value) == std::numeric_limits<std::int64_t>::min());
    assert(c.set(Parameter{"flag", std::string("now a string"), ""}) && c.size() == 7);   // replacing by name may change the type: Core has no schema
}

void test_core_exposes_no_semantic_knowledge() {
    static_assert(!HasRangeMember<Configuration> && !HasDefaultMember<Configuration> && !HasSchemaMember<Configuration>);
}

// ---- 2. the structural / semantic split --------------------------------------------------------
void test_a_structurally_valid_configuration_can_be_semantically_rejected_by_the_component() {
    SchemaComponent c(make_info(1));
    const Configuration out_of_range = schema_config(200, 5000);
    assert(out_of_range.validate());                                          // Core: structurally valid
    const auto r = c.configure(out_of_range);
    assert(!r && r.error().code == ErrorCode::CONFIGURATION_ERROR && r.error().source == ComponentId{1});   // the component: semantically invalid
    assert(c.rate() == 0 && c.applied_count == 0);                            // nothing applied
    assert(c.configure(schema_config(200, 50)) && c.rate() == 50 && c.applied_count == 1);
    assert(!c.configure(schema_config(200, 0)) && c.rate() == 50);            // the previous accepted value stays
}

void test_error_codes_follow_the_documented_boundary() {
    SchemaComponent c(make_info(2));
    Configuration missing;
    assert(!c.configure(missing) && c.configure(missing).error().code == ErrorCode::CONFIGURATION_ERROR);   // semantic: component-specific
    assert(c.configure(schema_config(200, 10)));
    assert(c.initialize());
    // lifecycle: the component's own state rule (see CORE-CFG-004) is INVALID_STATE; a malformed container entry is INVALID_ARGUMENT
    Configuration c2;
    assert(c2.set(Parameter{"", std::int64_t{1}, ""}).error().code == ErrorCode::INVALID_ARGUMENT);
    // no per-parameter taxonomy and no new code: the enumeration ends where R0.7 left it
    static_assert(static_cast<unsigned>(ErrorCode::CONFIGURATION_ERROR) == 9 && static_cast<unsigned>(ErrorCode::UNSUPPORTED) == 10 &&
                  static_cast<unsigned>(ErrorCode::INTERNAL_ERROR) == 11);          // the R0.7 enumeration: R0.8 adds no code
}

void test_the_runtime_returns_a_component_configuration_error_unchanged() {
    SchemaComponent a(make_info(1)), b(make_info(2));
    RuntimeManager runtime;
    assert(runtime.register_component(a) && runtime.register_component(b));
    const Configuration bad = schema_config(200, 5000);
    const auto r = runtime.configure(bad);
    assert(!r && r.error().code == ErrorCode::CONFIGURATION_ERROR && r.error().source == ComponentId{1} && r.error().message == "rate out of range");
    assert(runtime.state() == LifecycleState::UNKNOWN && runtime.fault_error() == nullptr);
    assert(a.applied_count == 0 && b.applied_count == 0);                     // first failure stopped the sequence, nothing applied
}

// ---- 3. ConfigurationVersion = schema / contract compatibility version ---------------------------
void test_configuration_version_is_a_plain_alias_of_version() {
    static_assert(std::is_same_v<ConfigurationVersion, Version>);
    static_assert(std::is_trivially_copyable_v<ConfigurationVersion>);
    static_assert(std::is_same_v<decltype(ConfigurationVersion{}.major), std::uint32_t>);
    const ConfigurationVersion v{2, 1, 0};
    assert(v.to_string() == "2.1.0" && v == (Version{2, 1, 0}) && !(v == (Version{2, 1, 1})));
}

void test_a_configuration_carries_no_version_revision_or_history() {
    static_assert(!HasVersionMember<Configuration> && !HasRevisionMember<Configuration> && !HasGenerationMember<Configuration>);
    static_assert(!HasTransactionMember<Configuration> && !HasHistoryMember<Configuration> && !HasSetVersion<Configuration>);
    static_assert(!HasVersionMember<RuntimeManager> && !HasRevisionMember<RuntimeManager> && !HasHistoryMember<RuntimeManager>);
    static_assert(!HasVersionMember<ComponentContext> && !HasRevisionMember<ComponentContext>);
    // Applying the same or a changed configuration repeatedly produces no counter, revision or history in Core:
    // the only observable consequence is the component's own applied state.
    SchemaComponent c(make_info(3));
    const Configuration same = schema_config(200, 10);
    for (int i = 0; i < 5; ++i) assert(c.configure(same));
    assert(c.applied_count == 5 && c.rate() == 10);
    Configuration copy = same;
    assert(copy.size() == same.size());                                       // copying/setting never adds hidden metadata
    assert(copy.set(Parameter{"rate", std::int64_t{11}, ""}) && copy.size() == same.size());
}

void test_schema_compatibility_is_the_components_policy_reported_as_a_configuration_error() {
    SchemaComponent c(make_info(4));
    assert(c.configure(schema_config(200, 10)));                               // schema 2.0: understood
    assert(c.configure(schema_config(250, 20)) && c.rate() == 20);             // 2.5: same major: the component accepts it
    for (const std::int64_t schema : {100, 300, 0, 99, 10000}) {               // other majors: the component rejects them
        const auto r = c.configure(schema_config(schema, 30));
        assert(!r && r.error().code == ErrorCode::CONFIGURATION_ERROR && r.error().source == ComponentId{4});
        assert(c.rate() == 20);                                                // nothing applied, previous state kept
    }
    Configuration no_schema;
    assert(no_schema.set(Parameter{"rate", std::int64_t{40}, ""}));
    assert(!c.configure(no_schema) && c.rate() == 20);                         // Core reserves no parameter name: the component defines its own convention
    assert(SchemaComponent::kSupported == (ConfigurationVersion{2, 0, 0}));
}

} // namespace

int main() {
    test_validate_is_the_const_side_effect_free_structural_entry_point();
    test_an_empty_name_is_rejected_atomically_at_set();
    test_validate_succeeds_for_every_constructible_configuration();
    test_the_container_accepts_any_value_it_knows_no_ranges_or_types();
    test_core_exposes_no_semantic_knowledge();
    test_a_structurally_valid_configuration_can_be_semantically_rejected_by_the_component();
    test_error_codes_follow_the_documented_boundary();
    test_the_runtime_returns_a_component_configuration_error_unchanged();
    test_configuration_version_is_a_plain_alias_of_version();
    test_a_configuration_carries_no_version_revision_or_history();
    test_schema_compatibility_is_the_components_policy_reported_as_a_configuration_error();
    return 0;
}
