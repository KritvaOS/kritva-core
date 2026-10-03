//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : requirements.hpp
// Description : Declarative platform service and capability requirements and their evaluation.
//
// Component   : Kritva Core
// Module      : Platform Contract
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-013
// API         : CORE-API-PLATFORM
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#pragma once
#include "context.hpp"
#include "../capability/capability_id.hpp"
#include "../error/result.hpp"
#include <cstdint>
#include <string>
#include <utility>
#include <vector>
namespace kritva::core::platform {

/// How much a declared service or capability matters.
enum class Requirement : std::uint8_t {
    REQUIRED,   ///< The functionality cannot work without it.
    OPTIONAL    ///< The functionality can work, with less, without it.
};

struct ServiceRequirement {
    PlatformService service{PlatformService::SCHEDULER};
    Requirement level{Requirement::REQUIRED};
};

struct CapabilityRequirement {
    CapabilityId id{};
    Requirement level{Requirement::REQUIRED};
};

//------------------------------------------------------------------------------
// PlatformRequirements (CORE-PLAT-013)
//
// A plain, copyable, DECLARATIVE list: which platform services and which
// capabilities some piece of integrator-written functionality needs, and
// whether each is REQUIRED or OPTIONAL. It describes needs; it does not look
// for, start or hold anything. Evaluation (below) compares it with a
// PlatformContext and has no side effect.
//
// DECLARING
//   add_service(service, level) and add_capability(id, level) append one
//   declaration and return Result<void>:
//     INVALID_ARGUMENT  an unknown PlatformService or Requirement enumerator, an
//                       invalid (zero) CapabilityId, or a DUPLICATE: the same
//                       service, or the same CapabilityId, declared twice. A
//                       duplicate is decided by the service or capability
//                       IDENTITY, not by the pair (identity, level): after
//                       add_service(SCHEDULER, REQUIRED), add_service(SCHEDULER,
//                       OPTIONAL) is rejected too. There is no silent upgrade,
//                       downgrade or merge.
//   A rejected declaration is atomic: the object is semantically unchanged.
//   Declaration order is kept and observable (services() and capabilities()).
//
// CAPABILITY IDENTITY IS AUTHORITATIVE
//   A capability requirement is satisfied exactly when the platform reports a
//   capability with that CapabilityId (CapabilitySet::contains). Nothing else
//   is ever consulted: not the platform's name or version, not a capability's
//   name or version, no string matching and no vendor or operating-system
//   inference. A service requirement is satisfied exactly when
//   PlatformContext::supports(service) is true.
//
// EVALUATION (no side effects)
//   evaluate(requirements, context) returns a PlatformRequirementReport, the
//   authoritative structured result. It lists, in declaration order, every
//   declared item that is missing, split into required and optional services
//   and capabilities. It never fails: absence is a normal result, not an error.
//   - It only queries the context: PlatformContext::supports() once per declared
//     service and capabilities() at most once. It never starts, stops,
//     configures or creates a service, never instantiates hardware, and changes
//     neither the requirements, the context, the adapter nor the Runtime.
//   - An unattached context provides nothing: every declared item is missing, so
//     it satisfies only an empty or all-OPTIONAL set.
//   - The same requirements and the same platform give the same report.
//   check_required(requirements, context) turns the report into a Result<void>:
//   success when no REQUIRED item is missing (OPTIONAL items never matter),
//   otherwise a failure with ErrorCode::UNSUPPORTED. Its message names the FIRST
//   missing required declaration (services before capabilities, then declaration
//   order) for humans; consumers decide on the report, never by parsing the
//   message. Optional items are ignored by check_required.
//
// THREADS, ALLOCATION, REAL TIME
//   Control-plane only: declaring and evaluating allocate, and thread safety is
//   that of the context's adapter for evaluation and none for a shared mutable
//   PlatformRequirements. Core makes no real-time claim.
//------------------------------------------------------------------------------
class PlatformRequirements {
public:
    Result<void> add_service(PlatformService service, Requirement level) {
        if (!known(service) || !known(level)) return reject("add_service: unknown service or requirement level");
        for (const ServiceRequirement& existing : services_) {
            if (existing.service == service) return reject("add_service: the service is already declared");
        }
        services_.push_back(ServiceRequirement{service, level});
        return Result<void>::success();
    }

    Result<void> add_capability(CapabilityId id, Requirement level) {
        if (!id.valid()) return reject("add_capability: the capability identity is invalid");
        if (!known(level)) return reject("add_capability: unknown requirement level");
        for (const CapabilityRequirement& existing : capabilities_) {
            if (existing.id == id) return reject("add_capability: the capability is already declared");
        }
        capabilities_.push_back(CapabilityRequirement{id, level});
        return Result<void>::success();
    }

    [[nodiscard]] const std::vector<ServiceRequirement>& services() const noexcept { return services_; }
    [[nodiscard]] const std::vector<CapabilityRequirement>& capabilities() const noexcept { return capabilities_; }
    [[nodiscard]] bool empty() const noexcept { return services_.empty() && capabilities_.empty(); }

private:
    static bool known(PlatformService service) noexcept { return static_cast<std::uint8_t>(service) <= static_cast<std::uint8_t>(PlatformService::WATCHDOG); }
    static bool known(Requirement level) noexcept { return level == Requirement::REQUIRED || level == Requirement::OPTIONAL; }
    static Result<void> reject(const char* message) {
        return Result<void>::failure(Error{ErrorCode::INVALID_ARGUMENT, ErrorSeverity::ERROR, {}, {}, message});
    }

    std::vector<ServiceRequirement> services_;
    std::vector<CapabilityRequirement> capabilities_;
};

/// The authoritative result of evaluating PlatformRequirements against a PlatformContext:
/// every MISSING declared item, in declaration order. Nothing here is a message to parse.
struct PlatformRequirementReport {
    std::vector<ServiceRequirement> missing_required_services;
    std::vector<ServiceRequirement> missing_optional_services;
    std::vector<CapabilityRequirement> missing_required_capabilities;
    std::vector<CapabilityRequirement> missing_optional_capabilities;

    /// True when nothing REQUIRED is missing (optional items never matter).
    [[nodiscard]] bool satisfied() const noexcept { return missing_required_services.empty() && missing_required_capabilities.empty(); }
    /// True when nothing at all, required or optional, is missing.
    [[nodiscard]] bool complete() const noexcept { return satisfied() && missing_optional_services.empty() && missing_optional_capabilities.empty(); }
};

/// Compare the declared requirements with the platform in `context`. No side effects.
[[nodiscard]] inline PlatformRequirementReport evaluate(const PlatformRequirements& requirements, const PlatformContext& context) {
    PlatformRequirementReport report;
    for (const ServiceRequirement& need : requirements.services()) {
        if (context.supports(need.service)) continue;
        (need.level == Requirement::REQUIRED ? report.missing_required_services : report.missing_optional_services).push_back(need);
    }
    if (!requirements.capabilities().empty()) {
        const CapabilitySet provided = context.capabilities();      // one snapshot for the whole evaluation
        for (const CapabilityRequirement& need : requirements.capabilities()) {
            if (provided.contains(need.id)) continue;
            (need.level == Requirement::REQUIRED ? report.missing_required_capabilities : report.missing_optional_capabilities).push_back(need);
        }
    }
    return report;
}

/// Success when every REQUIRED declaration is satisfied; otherwise UNSUPPORTED naming the first missing one.
[[nodiscard]] inline Result<void> check_required(const PlatformRequirements& requirements, const PlatformContext& context) {
    const PlatformRequirementReport report = evaluate(requirements, context);
    if (!report.missing_required_services.empty()) {
        const char* name = "";
        switch (report.missing_required_services.front().service) {
            case PlatformService::SCHEDULER: name = "scheduler"; break;
            case PlatformService::CLOCK:     name = "clock"; break;
            case PlatformService::TIMER:     name = "timer"; break;
            case PlatformService::WATCHDOG:  name = "watchdog"; break;
        }
        return Result<void>::failure(Error{ErrorCode::UNSUPPORTED, ErrorSeverity::ERROR, {}, {},
                                           std::string("required platform service is not available: ") + name});
    }
    if (!report.missing_required_capabilities.empty()) {
        return Result<void>::failure(Error{ErrorCode::UNSUPPORTED, ErrorSeverity::ERROR, {}, {},
                                           "required platform capability is not available: " + std::to_string(report.missing_required_capabilities.front().id.value())});
    }
    return Result<void>::success();
}
} // namespace kritva::core::platform
