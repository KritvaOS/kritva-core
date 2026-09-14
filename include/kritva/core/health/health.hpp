//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : health.hpp
// Description : Health value and optional diagnostic detail.
//
// Component   : Kritva Core
// Module      : Health
// Layer       : Core Foundation
//
// Requirements: CORE-HLT-002
// API         : CORE-API-HEALTH
//
// Author      : KritvaOS Core Team
// Created     : 2026-09-14
//==============================================================================

#pragma once

#include "health_state.hpp"

#include <string>
#include <utility>

namespace kritva::core {

/// @brief Represents component health independently of lifecycle status.
class Health {
public:
    constexpr Health() noexcept = default;
    explicit constexpr Health(HealthState state) noexcept : state_(state) {}

    [[nodiscard]] constexpr HealthState state() const noexcept { return state_; }
    [[nodiscard]] const std::string& detail() const noexcept { return detail_; }

    void set_state(HealthState state) noexcept { state_ = state; }
    void set_detail(std::string detail) { detail_ = std::move(detail); }

private:
    HealthState state_{HealthState::UNKNOWN};
    std::string detail_;
};

} // namespace kritva::core
