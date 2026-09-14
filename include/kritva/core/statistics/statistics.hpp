//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : statistics.hpp
// Description : Common counters and gauges exposed by Core components.
//
// Component   : Kritva Core
// Module      : Statistics
// Layer       : Core Foundation
//
// Requirements: CORE-STAT-003
// API         : CORE-API-STATISTICS
//
// Author      : KritvaOS Core Team
// Created     : 2026-09-14
//==============================================================================

#pragma once

#include "counter.hpp"
#include "gauge.hpp"

namespace kritva::core {

/// @brief Standard statistics for operational observation.
struct Statistics {
    Counter sample_count;
    Counter error_count;
    Counter retry_count;
    Counter drop_count;

    Gauge queue_depth;
    Gauge temperature;
    Gauge utilization;
};

} // namespace kritva::core
