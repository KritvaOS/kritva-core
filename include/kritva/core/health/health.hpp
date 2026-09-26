//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : health.hpp
// Description : Component health representation.
//
// Component   : Kritva Core
// Module      : Health
// Layer       : Core Foundation
//
// Requirements: CORE-HEA-002
// API         : CORE-API-HEALTH
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include "health_state.hpp"
#include <string>
#include <utility>
namespace kritva::core {
class Health {
public:
    Health() noexcept = default;
    explicit Health(HealthState state) noexcept : state_(state) {}
    [[nodiscard]] constexpr HealthState state() const noexcept { return state_; }
    [[nodiscard]] const std::string& detail() const noexcept { return detail_; }
    void set_state(HealthState state) noexcept { state_ = state; }
    void set_detail(std::string detail) { detail_ = std::move(detail); }
private:
    HealthState state_{HealthState::UNKNOWN};
    std::string detail_;
};
} // namespace kritva::core
