//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : statistics.hpp
// Description : Common operational statistics.
//
// Component   : Kritva Core
// Module      : Statistics
// Layer       : Core Foundation
//
// Requirements: CORE-STS-003
// API         : CORE-API-STATISTICS
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include "counter.hpp"
#include "gauge.hpp"
namespace kritva::core {
// Aggregate of Counter/Gauge members; inherits their contract: not thread-safe
// unless externally synchronized, no allocation, no blocking.
struct Statistics {
    Counter sample_count;
    Counter error_count;
    Counter retry_count;
    Counter drop_count;
    Gauge queue_depth;
    Gauge utilization;
};
} // namespace kritva::core
