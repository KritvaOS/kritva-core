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

//------------------------------------------------------------------------------
// Statistics (CORE-STS-003)
//
// Plain aggregate of common operational counters and gauges. All fields are
// zero after value-initialization (Statistics{}). Fields are independent:
// updating one never changes another, and Core imposes no relationship between
// them (e.g. no invariant between sample_count and error_count).
//
// Fields (all unitless counts unless stated):
//   sample_count - Counter: items successfully processed/sampled.
//   error_count  - Counter: operations that failed.
//   retry_count  - Counter: retry attempts made (not distinct failures).
//   drop_count   - Counter: items discarded (overflow, staleness, etc.).
//   queue_depth  - Gauge:   number of items currently queued; >= 0 by
//                           convention (not enforced).
//   utilization  - Gauge:   resource utilization as a whole percent, 0..100 by
//                           convention (not enforced; values are stored as-is).
//
// The meaning of "item" and "resource" is defined by the component that owns
// the Statistics. Value type: trivially copyable; copies are independent
// snapshots.
//
// Thread-safety / real-time: NOT thread-safe and NOT a synchronization
// primitive. Copying a Statistics while another thread updates it is a data
// race, and a copy is not an atomic snapshot of all fields. See Counter and
// Gauge for allocation, blocking and complexity (none / none / O(1)).
//
// Out of scope: telemetry transport, export formats, serialization, rates or
// histograms. Those belong outside Core Foundation.
//------------------------------------------------------------------------------
struct Statistics {
    Counter sample_count;
    Counter error_count;
    Counter retry_count;
    Counter drop_count;
    Gauge queue_depth;
    Gauge utilization;
};
} // namespace kritva::core
