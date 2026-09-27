//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : health_test.cpp
// Description : Unit tests for Kritva Core Health.
//
// Component   : Kritva Core
// Module      : Health
// Layer       : Core Foundation
//
// Requirements: CORE-HEALTH-*
// API         : CORE-API-HEALTH
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================

#include <cassert>

#include <kritva/core/health/health.hpp>

using namespace kritva::core;

int main() {
    // -------------------------------------------------------------------------
    // Default construction
    // -------------------------------------------------------------------------
    {
        Health health;

        assert(health.state() == HealthState::UNKNOWN);
    }

    // -------------------------------------------------------------------------
    // Construction with health state
    // -------------------------------------------------------------------------
    {
        Health health(HealthState::HEALTHY);

        assert(health.state() == HealthState::HEALTHY);
    }

    // -------------------------------------------------------------------------
    // State update
    // -------------------------------------------------------------------------
    {
        Health health;

        health.set_state(HealthState::HEALTHY);

        assert(health.state() == HealthState::HEALTHY);
    }

    // -------------------------------------------------------------------------
    // HEALTHY → DEGRADED
    // -------------------------------------------------------------------------
    {
        Health health(HealthState::HEALTHY);

        health.set_state(HealthState::DEGRADED);

        assert(health.state() == HealthState::DEGRADED);
    }

    // -------------------------------------------------------------------------
    // DEGRADED → UNHEALTHY
    // -------------------------------------------------------------------------
    {
        Health health(HealthState::DEGRADED);

        health.set_state(HealthState::UNHEALTHY);

        assert(health.state() == HealthState::UNHEALTHY);
    }

    // -------------------------------------------------------------------------
    // UNHEALTHY → HEALTHY
    // -------------------------------------------------------------------------
    {
        Health health(HealthState::UNHEALTHY);

        health.set_state(HealthState::HEALTHY);

        assert(health.state() == HealthState::HEALTHY);
    }

    // -------------------------------------------------------------------------
    // State can return to UNKNOWN
    // -------------------------------------------------------------------------
    {
        Health health(HealthState::HEALTHY);

        health.set_state(HealthState::UNKNOWN);

        assert(health.state() == HealthState::UNKNOWN);
    }

    return 0;
}
