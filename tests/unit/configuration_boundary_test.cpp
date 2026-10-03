//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : configuration_boundary_test.cpp
// Description : Compile-time snapshot of the frozen configuration boundary (signatures, values, shapes).
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-CFG-013
// API         : CORE-API-CONFIGURATION
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

// Any drift of the public surface the R0.8 configuration contract governs fails to COMPILE here: every signature,
// enumerator value, member type and shape below is the one frozen at the R08 Configuration API Review. A deliberate
// change must return to architecture review and update this snapshot with it. Nothing here runs a behavior test;
// the behavior is covered by the focused contract, harness and integration tests.

#include <cstdint>
#include <string>
#include <type_traits>
#include <variant>

#include <kritva/core/core.hpp>

using namespace kritva::core;
using namespace kritva::core::runtime;

namespace {

// ---- what must NOT exist: no dynamic reconfiguration, no accessor, no revision / history, anywhere ------------------
// Name-based member detection that works for any signature and any overload set on a non-final T: a type that derives
// from both T and a class declaring the name makes `&Derived::name` ambiguous exactly when T also declares it. A final T
// cannot be derived from, so it is probed directly (`&T::name`, which sees every non-overloaded member).
#define KRITVA_DECLARE_HAS_MEMBER(NAME)                                                           \
    struct HasMemberFallback_##NAME { void NAME(); };                                             \
    template <class T> struct HasMemberProbe_##NAME : T, HasMemberFallback_##NAME {};             \
    template <class T> constexpr bool has_member_##NAME() {                                       \
        if constexpr (std::is_final_v<T>) return requires { &T::NAME; };                          \
        else return !requires { &HasMemberProbe_##NAME<T>::NAME; };                               \
    }                                                                                             \
    template <class T> concept Has_##NAME = has_member_##NAME<T>();
KRITVA_DECLARE_HAS_MEMBER(reconfigure)
KRITVA_DECLARE_HAS_MEMBER(set_parameter)
KRITVA_DECLARE_HAS_MEMBER(get_parameter)
KRITVA_DECLARE_HAS_MEMBER(configuration)
KRITVA_DECLARE_HAS_MEMBER(apply)
KRITVA_DECLARE_HAS_MEMBER(update_configuration)
KRITVA_DECLARE_HAS_MEMBER(revision)
KRITVA_DECLARE_HAS_MEMBER(version)
KRITVA_DECLARE_HAS_MEMBER(generation)
KRITVA_DECLARE_HAS_MEMBER(history)
KRITVA_DECLARE_HAS_MEMBER(transaction_id)
KRITVA_DECLARE_HAS_MEMBER(schema)
template <class T> concept NoDynamicConfiguration = !Has_reconfigure<T> && !Has_set_parameter<T> && !Has_get_parameter<T> &&
                                                    !Has_configuration<T> && !Has_apply<T> && !Has_update_configuration<T>;
template <class T> concept HasRevisionLike = Has_revision<T> || Has_version<T> || Has_generation<T> || Has_history<T> || Has_transaction_id<T> || Has_schema<T>;
// The detector is itself checked: it finds a member that exists and does not invent one that does not.
struct DetectorSelfCheck { void reconfigure(int); void version() const; };
static_assert(Has_reconfigure<DetectorSelfCheck> && Has_version<DetectorSelfCheck> && !Has_set_parameter<DetectorSelfCheck> && !Has_revision<DetectorSelfCheck>);
static_assert(NoDynamicConfiguration<Component> && NoDynamicConfiguration<RuntimeManager> && NoDynamicConfiguration<ComponentContext>);
static_assert(!HasRevisionLike<Configuration> && !HasRevisionLike<Component> && !HasRevisionLike<RuntimeManager> && !HasRevisionLike<ComponentContext>);

// ---- Configuration and Parameter --------------------------------------------------------------------------
static_assert(std::is_same_v<ParameterValue, std::variant<bool, std::int64_t, double, std::string>>);
static_assert(std::is_aggregate_v<Parameter>);
static_assert(std::is_same_v<decltype(Parameter::name), std::string>);
static_assert(std::is_same_v<decltype(Parameter::value), ParameterValue>);
static_assert(std::is_same_v<decltype(Parameter::description), std::string>);
static_assert(std::is_default_constructible_v<Configuration> && std::is_copy_constructible_v<Configuration> &&
              std::is_copy_assignable_v<Configuration> && std::is_move_constructible_v<Configuration> && std::is_move_assignable_v<Configuration>);
static_assert(std::is_same_v<decltype(&Configuration::validate), Result<void> (Configuration::*)() const>);
static_assert(std::is_same_v<decltype(&Configuration::set), Result<void> (Configuration::*)(Parameter)>);
static_assert(std::is_same_v<decltype(&Configuration::get), const Parameter* (Configuration::*)(const std::string&) const noexcept>);
static_assert(std::is_same_v<decltype(&Configuration::contains), bool (Configuration::*)(const std::string&) const noexcept>);
static_assert(std::is_same_v<decltype(&Configuration::size), std::size_t (Configuration::*)() const noexcept>);

// ---- ConfigurationVersion ---------------------------------------------------------------------------------
static_assert(std::is_same_v<ConfigurationVersion, Version>);
static_assert(std::is_same_v<decltype(Version::major), std::uint32_t> && std::is_same_v<decltype(Version::minor), std::uint32_t> &&
              std::is_same_v<decltype(Version::patch), std::uint32_t>);
static_assert(std::is_trivially_copyable_v<Version>);

// ---- Component: the five operations, the three reports and the identity -------------------------------------
static_assert(std::is_same_v<decltype(&Component::configure), Result<void> (Component::*)(const Configuration&)>);
static_assert(std::is_same_v<decltype(&Component::initialize), Result<void> (Component::*)()>);
static_assert(std::is_same_v<decltype(&Component::start), Result<void> (Component::*)()>);
static_assert(std::is_same_v<decltype(&Component::stop), Result<void> (Component::*)()>);
static_assert(std::is_same_v<decltype(&Component::shutdown), Result<void> (Component::*)()>);
static_assert(std::is_same_v<decltype(&Component::lifecycle_state), LifecycleState (Component::*)() const noexcept>);
static_assert(std::is_same_v<decltype(&Component::status), Status (Component::*)() const>);
static_assert(std::is_same_v<decltype(&Component::health), Health (Component::*)() const>);
static_assert(std::is_same_v<decltype(&Component::capabilities), CapabilitySet (Component::*)() const>);
static_assert(std::is_same_v<decltype(&Component::info), const ComponentInfo& (Component::*)() const noexcept>);
static_assert(std::is_abstract_v<Component> && !std::is_copy_constructible_v<Component> && !std::is_copy_assignable_v<Component>);
static_assert(!std::is_move_constructible_v<Component> && !std::is_move_assignable_v<Component>);

// ---- RuntimeManager: configure forwards, the rest of the lifecycle is unchanged ----------------------------------
static_assert(std::is_same_v<decltype(&RuntimeManager::configure), Result<void> (RuntimeManager::*)(const Configuration&)>);
static_assert(std::is_same_v<decltype(&RuntimeManager::initialize), Result<void> (RuntimeManager::*)()>);
static_assert(std::is_same_v<decltype(&RuntimeManager::start), Result<void> (RuntimeManager::*)()>);
static_assert(std::is_same_v<decltype(&RuntimeManager::stop), Result<void> (RuntimeManager::*)()>);
static_assert(std::is_same_v<decltype(&RuntimeManager::shutdown), Result<void> (RuntimeManager::*)()>);
static_assert(std::is_same_v<decltype(&RuntimeManager::reset), Result<void> (RuntimeManager::*)()>);
static_assert(std::is_same_v<decltype(&RuntimeManager::state), LifecycleState (RuntimeManager::*)() const noexcept>);
static_assert(std::is_same_v<decltype(&RuntimeManager::fault_error), const Error* (RuntimeManager::*)() const noexcept>);
static_assert(std::is_same_v<decltype(&RuntimeManager::statistics), const Statistics& (RuntimeManager::*)() const noexcept>);

// ---- ComponentContext stays configuration-neutral: a two-pointer immutable view ------------------------------
static_assert(sizeof(ComponentContext) == 2 * sizeof(void*) && std::is_trivially_destructible_v<ComponentContext>);
static_assert(!std::is_copy_assignable_v<ComponentContext> && !std::is_move_assignable_v<ComponentContext>);
static_assert(!std::is_constructible_v<ComponentContext, const Configuration&>);
static_assert(!std::is_constructible_v<ComponentContext, const ComponentInfo&, const Configuration&>);

// ---- the enumerations the configuration contract relies on: values are part of the boundary -------------------
static_assert(static_cast<unsigned>(ErrorCode::NONE) == 0 && static_cast<unsigned>(ErrorCode::UNKNOWN) == 1 && static_cast<unsigned>(ErrorCode::INVALID_ARGUMENT) == 2 &&
              static_cast<unsigned>(ErrorCode::INVALID_STATE) == 3 && static_cast<unsigned>(ErrorCode::NOT_INITIALIZED) == 4 && static_cast<unsigned>(ErrorCode::NOT_READY) == 5 &&
              static_cast<unsigned>(ErrorCode::ALREADY_RUNNING) == 6 && static_cast<unsigned>(ErrorCode::TIMEOUT) == 7 && static_cast<unsigned>(ErrorCode::RESOURCE_UNAVAILABLE) == 8 &&
              static_cast<unsigned>(ErrorCode::CONFIGURATION_ERROR) == 9 && static_cast<unsigned>(ErrorCode::UNSUPPORTED) == 10 && static_cast<unsigned>(ErrorCode::INTERNAL_ERROR) == 11);
static_assert(static_cast<unsigned>(ErrorSeverity::INFO) == 0 && static_cast<unsigned>(ErrorSeverity::WARNING) == 1 &&
              static_cast<unsigned>(ErrorSeverity::ERROR) == 2 && static_cast<unsigned>(ErrorSeverity::CRITICAL) == 3);
static_assert(static_cast<int>(LifecycleState::UNKNOWN) == 0 && static_cast<int>(LifecycleState::INITIALIZING) == 1 && static_cast<int>(LifecycleState::READY) == 2 &&
              static_cast<int>(LifecycleState::RUNNING) == 3 && static_cast<int>(LifecycleState::STOPPING) == 4 && static_cast<int>(LifecycleState::STOPPED) == 5 &&
              static_cast<int>(LifecycleState::FAULT) == 6 && static_cast<int>(LifecycleState::RECOVERING) == 7);
static_assert(static_cast<int>(StatusCode::UNKNOWN) == 0 && static_cast<int>(StatusCode::OK) == 1 && static_cast<int>(StatusCode::INTERNAL_ERROR) == 9);
static_assert(static_cast<int>(HealthState::UNKNOWN) == 0 && static_cast<int>(HealthState::HEALTHY) == 1 &&
              static_cast<int>(HealthState::DEGRADED) == 2 && static_cast<int>(HealthState::UNHEALTHY) == 3);

// ---- Error carries the component as a plain Id source ---------------------------------------------------------
static_assert(std::is_same_v<decltype(Error::code), ErrorCode> && std::is_same_v<decltype(Error::severity), ErrorSeverity> &&
              std::is_same_v<decltype(Error::source), Id> && std::is_same_v<decltype(Error::message), std::string>);

// An exhaustive switch with no default: under -Wall -Werror (the strict validation build) adding an enumerator to
// any of these enumerations fails to compile until the boundary snapshot is deliberately updated.
constexpr int enumerator_count(ErrorCode c) {
    switch (c) {
        case ErrorCode::NONE: case ErrorCode::UNKNOWN: case ErrorCode::INVALID_ARGUMENT: case ErrorCode::INVALID_STATE:
        case ErrorCode::NOT_INITIALIZED: case ErrorCode::NOT_READY: case ErrorCode::ALREADY_RUNNING: case ErrorCode::TIMEOUT:
        case ErrorCode::RESOURCE_UNAVAILABLE: case ErrorCode::CONFIGURATION_ERROR: case ErrorCode::UNSUPPORTED: case ErrorCode::INTERNAL_ERROR: return 12;
    }
    return 0;
}
constexpr int enumerator_count(LifecycleState s) {
    switch (s) {
        case LifecycleState::UNKNOWN: case LifecycleState::INITIALIZING: case LifecycleState::READY: case LifecycleState::RUNNING:
        case LifecycleState::STOPPING: case LifecycleState::STOPPED: case LifecycleState::FAULT: case LifecycleState::RECOVERING: return 8;
    }
    return 0;
}
static_assert(enumerator_count(ErrorCode::NONE) == 12 && enumerator_count(LifecycleState::UNKNOWN) == 8);

} // namespace

int main() {
    // The boundary is enforced at compile time; the run only proves the snapshot built and the types are usable.
    Configuration c;
    if (!c.set(Parameter{"a", true, ""}) || !c.validate() || c.size() != 1) return 1;
    return 0;
}
