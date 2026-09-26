//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component.hpp
// Description : Base lifecycle-managed Core component contract.
//
// Component   : Kritva Core
// Module      : Runtime
// Layer       : Core Foundation
//
// Requirements: CORE-RT-001
// API         : CORE-API-RUNTIME
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include "../capability/capability_set.hpp"
#include "../configuration/configuration.hpp"
#include "../health/health.hpp"
#include "../lifecycle/lifecycle.hpp"
#include "../error/result.hpp"
#include "../status/status.hpp"
namespace kritva::core::runtime {
class Component {
public:
    virtual ~Component() = default;
    virtual Result<void> configure(const Configuration&) = 0;
    virtual Result<void> initialize() = 0;
    virtual Result<void> start() = 0;
    virtual Result<void> stop() = 0;
    virtual Result<void> shutdown() = 0;
    [[nodiscard]] virtual LifecycleState lifecycle_state() const noexcept = 0;
    [[nodiscard]] virtual Status status() const = 0;
    [[nodiscard]] virtual Health health() const = 0;
    [[nodiscard]] virtual CapabilitySet capabilities() const = 0;
};
} // namespace kritva::core::runtime
