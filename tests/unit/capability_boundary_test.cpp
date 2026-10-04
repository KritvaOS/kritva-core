//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : capability_boundary_test.cpp
// Description : Compile-time snapshot of the frozen capability, requirement and readiness boundary.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-CAP-011
// API         : CORE-API-CAPABILITY
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

// Any drift of the public surface the R0.9 capability, requirement and readiness contracts govern fails to COMPILE here:
// every signature, enumerator value, member type and shape below is the one frozen at the R09 Capability API Review. A
// deliberate change must return to architecture review and update this snapshot with it. Behavior is covered by the
// focused contract, harness and integration tests.

#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

#include <kritva/core/core.hpp>

using namespace kritva::core;
using namespace kritva::core::runtime;

namespace {

// Name-based member detection for any signature and overload on a non-final T (a final T is probed directly).
#define KRITVA_BND_HAS_MEMBER(NAME)                                                           \
    struct Fallback_##NAME { void NAME(); };                                                  \
    template <class T> struct Probe_##NAME : T, Fallback_##NAME {};                           \
    template <class T> constexpr bool has_member_##NAME() {                                   \
        if constexpr (std::is_final_v<T>) return requires { &T::NAME; };                      \
        else return !requires { &Probe_##NAME<T>::NAME; };                                    \
    }                                                                                         \
    template <class T> concept Has_##NAME = has_member_##NAME<T>();
// resolution / injection / registry / discovery
KRITVA_BND_HAS_MEMBER(resolve) KRITVA_BND_HAS_MEMBER(bind) KRITVA_BND_HAS_MEMBER(inject) KRITVA_BND_HAS_MEMBER(satisfy)
KRITVA_BND_HAS_MEMBER(discover) KRITVA_BND_HAS_MEMBER(lookup) KRITVA_BND_HAS_MEMBER(locate) KRITVA_BND_HAS_MEMBER(provider_of)
KRITVA_BND_HAS_MEMBER(require_capability) KRITVA_BND_HAS_MEMBER(register_service) KRITVA_BND_HAS_MEMBER(service_registry)
KRITVA_BND_HAS_MEMBER(instance) KRITVA_BND_HAS_MEMBER(subscribe) KRITVA_BND_HAS_MEMBER(notify)
// readiness
KRITVA_BND_HAS_MEMBER(ready) KRITVA_BND_HAS_MEMBER(is_ready) KRITVA_BND_HAS_MEMBER(readiness) KRITVA_BND_HAS_MEMBER(wait_for)
KRITVA_BND_HAS_MEMBER(wait_until_ready) KRITVA_BND_HAS_MEMBER(prerequisites) KRITVA_BND_HAS_MEMBER(missing_dependencies)
// security evidence
KRITVA_BND_HAS_MEMBER(token) KRITVA_BND_HAS_MEMBER(credential) KRITVA_BND_HAS_MEMBER(authorize) KRITVA_BND_HAS_MEMBER(authenticate)
KRITVA_BND_HAS_MEMBER(grant) KRITVA_BND_HAS_MEMBER(signature)
// state-like members a Capability or CapabilitySet must never carry (a capability is descriptive, not state)
KRITVA_BND_HAS_MEMBER(available) KRITVA_BND_HAS_MEMBER(is_available) KRITVA_BND_HAS_MEMBER(health) KRITVA_BND_HAS_MEMBER(status)
KRITVA_BND_HAS_MEMBER(state) KRITVA_BND_HAS_MEMBER(lifecycle) KRITVA_BND_HAS_MEMBER(priority) KRITVA_BND_HAS_MEMBER(revision)
template <class T> concept HasStateLikeMember = Has_available<T> || Has_is_available<T> || Has_health<T> || Has_status<T> || Has_state<T> ||
                                                 Has_lifecycle<T> || Has_priority<T> || Has_revision<T>;
template <class T> concept HasForbiddenMember =
    Has_resolve<T> || Has_bind<T> || Has_inject<T> || Has_satisfy<T> || Has_discover<T> || Has_lookup<T> || Has_locate<T> || Has_provider_of<T> ||
    Has_require_capability<T> || Has_register_service<T> || Has_service_registry<T> || Has_instance<T> || Has_subscribe<T> || Has_notify<T> ||
    Has_ready<T> || Has_is_ready<T> || Has_readiness<T> || Has_wait_for<T> || Has_wait_until_ready<T> || Has_prerequisites<T> || Has_missing_dependencies<T> ||
    Has_token<T> || Has_credential<T> || Has_authorize<T> || Has_authenticate<T> || Has_grant<T> || Has_signature<T>;
struct DetectorSelfCheck { void resolve(); void is_ready() const; void token(); };
static_assert(HasForbiddenMember<DetectorSelfCheck> && Has_resolve<DetectorSelfCheck> && Has_is_ready<DetectorSelfCheck> && !Has_grant<DetectorSelfCheck>);

// ---- none of the surfaces the capability / requirement / readiness contracts touch has a resolution, readiness or credential member
static_assert(!HasForbiddenMember<Capability> && !HasForbiddenMember<CapabilitySet> && !HasForbiddenMember<platform::PlatformRequirements> &&
              !HasForbiddenMember<platform::PlatformContext> && !HasForbiddenMember<platform::IPlatformAdapter> && !HasForbiddenMember<ComponentContext> &&
              !HasForbiddenMember<Component> && !HasForbiddenMember<RuntimeManager> && !HasForbiddenMember<DependencyGraph> &&
              !HasForbiddenMember<ComponentRegistry> && !HasForbiddenMember<Lifecycle>);

// ---- Capability and CapabilityId
static_assert(!HasStateLikeMember<Capability> && !HasStateLikeMember<CapabilitySet>);   // descriptive metadata, not state
static_assert(std::is_same_v<CapabilityId, Id>);
static_assert(std::is_aggregate_v<Capability> && std::is_same_v<decltype(Capability::id), CapabilityId> && std::is_same_v<decltype(Capability::name), std::string> &&
              std::is_same_v<decltype(Capability::version), Version>);

// ---- CapabilitySet
static_assert(std::is_same_v<decltype(&CapabilitySet::add), void (CapabilitySet::*)(Capability)>);
static_assert(std::is_same_v<decltype(&CapabilitySet::contains), bool (CapabilitySet::*)(CapabilityId) const noexcept>);
static_assert(std::is_same_v<decltype(&CapabilitySet::find), const Capability* (CapabilitySet::*)(CapabilityId) const noexcept>);
static_assert(std::is_same_v<decltype(&CapabilitySet::size), std::size_t (CapabilitySet::*)() const noexcept>);
static_assert(std::is_same_v<decltype(&CapabilitySet::empty), bool (CapabilitySet::*)() const noexcept>);
static_assert(std::is_same_v<decltype(std::declval<const CapabilitySet&>().all()), const std::vector<Capability>&>);

// ---- providers publish by value
static_assert(std::is_same_v<decltype(&Component::capabilities), CapabilitySet (Component::*)() const>);
static_assert(std::is_same_v<decltype(&platform::IPlatformAdapter::capabilities), CapabilitySet (platform::IPlatformAdapter::*)() const>);

// ---- requirements: declarative, atomic, identity-based
static_assert(std::is_aggregate_v<platform::CapabilityRequirement> && std::is_same_v<decltype(platform::CapabilityRequirement::id), CapabilityId> &&
              std::is_same_v<decltype(platform::CapabilityRequirement::level), platform::Requirement>);
static_assert(std::is_aggregate_v<platform::ServiceRequirement> && std::is_same_v<decltype(platform::ServiceRequirement::level), platform::Requirement>);
static_assert(static_cast<int>(platform::Requirement::REQUIRED) == 0 && static_cast<int>(platform::Requirement::OPTIONAL) == 1);
static_assert(std::is_same_v<decltype(&platform::PlatformRequirements::add_capability), Result<void> (platform::PlatformRequirements::*)(CapabilityId, platform::Requirement)>);
static_assert(std::is_same_v<decltype(&platform::PlatformRequirements::add_service), Result<void> (platform::PlatformRequirements::*)(platform::PlatformService, platform::Requirement)>);
static_assert(std::is_same_v<decltype(&platform::PlatformRequirements::capabilities), const std::vector<platform::CapabilityRequirement>& (platform::PlatformRequirements::*)() const noexcept>);
static_assert(std::is_same_v<decltype(platform::evaluate(std::declval<const platform::PlatformRequirements&>(), std::declval<const platform::PlatformContext&>())), platform::PlatformRequirementReport>);
static_assert(std::is_same_v<decltype(platform::check_required(std::declval<const platform::PlatformRequirements&>(), std::declval<const platform::PlatformContext&>())), Result<void>>);
static_assert(std::is_same_v<decltype(&platform::PlatformContext::has_capability), bool (platform::PlatformContext::*)(CapabilityId) const>);
static_assert(std::is_same_v<decltype(&ComponentContext::has_capability), bool (ComponentContext::*)(CapabilityId) const>);

// ---- Component dependency ordering is ComponentId-based and has nothing capability-typed
static_assert(std::is_same_v<decltype(&DependencyGraph::add_dependency), Result<void> (DependencyGraph::*)(ComponentId, ComponentId)>);
static_assert(std::is_same_v<decltype(&RuntimeManager::add_dependency), Result<void> (RuntimeManager::*)(ComponentId, ComponentId)>);
static_assert(std::is_same_v<decltype(&RuntimeManager::component_order), Result<std::vector<ComponentId>> (RuntimeManager::*)() const>);
static_assert(!std::is_constructible_v<ComponentContext, const CapabilitySet&> && !std::is_constructible_v<RuntimeManager, const CapabilitySet&>);

// ---- no readiness state: the lifecycle keeps exactly its eight states and the Runtime reports one of them
static_assert(static_cast<int>(LifecycleState::UNKNOWN) == 0 && static_cast<int>(LifecycleState::INITIALIZING) == 1 && static_cast<int>(LifecycleState::READY) == 2 &&
              static_cast<int>(LifecycleState::RUNNING) == 3 && static_cast<int>(LifecycleState::STOPPING) == 4 && static_cast<int>(LifecycleState::STOPPED) == 5 &&
              static_cast<int>(LifecycleState::FAULT) == 6 && static_cast<int>(LifecycleState::RECOVERING) == 7);
static_assert(std::is_same_v<decltype(&RuntimeManager::state), LifecycleState (RuntimeManager::*)() const noexcept>);
static_assert(std::is_same_v<decltype(&Component::lifecycle_state), LifecycleState (Component::*)() const noexcept>);
// An exhaustive switch with no default: under -Wall -Werror (the strict build) an added enumerator fails to compile.
constexpr int enumerator_count(LifecycleState s) {
    switch (s) {
        case LifecycleState::UNKNOWN: case LifecycleState::INITIALIZING: case LifecycleState::READY: case LifecycleState::RUNNING:
        case LifecycleState::STOPPING: case LifecycleState::STOPPED: case LifecycleState::FAULT: case LifecycleState::RECOVERING: return 8;
    }
    return 0;
}
constexpr int enumerator_count(ErrorCode c) {
    switch (c) {
        case ErrorCode::NONE: case ErrorCode::UNKNOWN: case ErrorCode::INVALID_ARGUMENT: case ErrorCode::INVALID_STATE: case ErrorCode::NOT_INITIALIZED:
        case ErrorCode::NOT_READY: case ErrorCode::ALREADY_RUNNING: case ErrorCode::TIMEOUT: case ErrorCode::RESOURCE_UNAVAILABLE:
        case ErrorCode::CONFIGURATION_ERROR: case ErrorCode::UNSUPPORTED: case ErrorCode::INTERNAL_ERROR: return 12;
    }
    return 0;
}
static_assert(enumerator_count(LifecycleState::UNKNOWN) == 8 && enumerator_count(ErrorCode::NONE) == 12);   // no code or state was added for capabilities or readiness

} // namespace

int main() {
    CapabilitySet s;
    s.add(Capability{CapabilityId{1}, "x", Version{}});
    return s.contains(CapabilityId{1}) && s.size() == 1 ? 0 : 1;
}
