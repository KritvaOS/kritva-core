//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component_info.hpp
// Description : Immutable component identity and metadata.
//
// Component   : Kritva Core
// Module      : Runtime
// Layer       : Core Foundation
//
// Requirements: CORE-RT-001
// API         : CORE-API-RUNTIME
//
// Author      : KritvaOS Core Team
// Created     : 03-10-2026
//==============================================================================

#pragma once
#include "component_id.hpp"
#include "../error/result.hpp"
#include "../types/version.hpp"
#include <string>
#include <utility>
namespace kritva::core::runtime {

//------------------------------------------------------------------------------
// ComponentInfo (CORE-RT-001)
//
// The required, platform-independent metadata of a component: identity, name,
// and version. Immutable after creation: there are no setters, and the only way
// to obtain a ComponentInfo is create(), so a ComponentInfo is always valid.
//
//   id       A valid ComponentId (non-zero). The only part that identifies the
//            component.
//   name     Non-empty human-readable label for diagnostics and logs. It is NOT
//            an identity: it need not be unique and is never used for lookup
//            or ordering. Core imposes no character set or length limit.
//   version  The component's own implementation version (Core Version type,
//            major.minor.patch). Informational; the default 0.0.0 is allowed.
//            It is unrelated to the Core library version.
//
// create() fails with ErrorCode::INVALID_ARGUMENT, without side effects, if the
// id is invalid or the name is empty.
//
// Value semantics: copyable and movable. Equality is deliberately not defined
// on the whole record; compare identities with info.id() == other.id().
//
// Real-time / thread-safety: create() and copies allocate (std::string);
// treat them as control-plane operations. Once created, concurrent const access
// to one ComponentInfo is safe because it can never change.
//------------------------------------------------------------------------------
class ComponentInfo {
public:
    [[nodiscard]] static Result<ComponentInfo> create(ComponentId id, std::string name,
                                                      Version version = {}) {
        if (!id.valid()) {
            return Result<ComponentInfo>::failure(Error{
                ErrorCode::INVALID_ARGUMENT, ErrorSeverity::ERROR, {}, {},
                "component id must be valid (non-zero)"});
        }
        if (name.empty()) {
            return Result<ComponentInfo>::failure(Error{
                ErrorCode::INVALID_ARGUMENT, ErrorSeverity::ERROR, {}, {},
                "component name must not be empty"});
        }
        return Result<ComponentInfo>::success(ComponentInfo(id, std::move(name), version));
    }

    [[nodiscard]] ComponentId id() const noexcept { return id_; }
    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] const Version& version() const noexcept { return version_; }

private:
    ComponentInfo(ComponentId id, std::string name, Version version)
        : id_(id), name_(std::move(name)), version_(version) {}

    ComponentId id_;
    std::string name_;
    Version version_;
};

} // namespace kritva::core::runtime
