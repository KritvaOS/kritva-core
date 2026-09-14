//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : core.hpp
// Description : Umbrella header for the public Kritva Core Foundation API.
//
// Component   : Kritva Core
// Module      : Core
// Layer       : Core Foundation
//
// Requirements: CORE-API-001
// API         : CORE-API-UMBRELLA
//
// Author      : KritvaOS Core Team
// Created     : 2026-09-14
//==============================================================================

#pragma once

// Types
#include "types/id.hpp"
#include "types/version.hpp"
#include "types/timestamp.hpp"
#include "types/duration.hpp"
#include "types/metadata.hpp"

// Lifecycle
#include "lifecycle/lifecycle_state.hpp"
#include "lifecycle/lifecycle.hpp"

// Status
#include "status/status_code.hpp"
#include "status/status.hpp"

// Health
#include "health/health_state.hpp"
#include "health/health.hpp"

// Statistics
#include "statistics/counter.hpp"
#include "statistics/gauge.hpp"
#include "statistics/statistics.hpp"

// Error
#include "error/error_code.hpp"
#include "error/error.hpp"
#include "error/result.hpp"

// Event
#include "event/event_type.hpp"
#include "event/event.hpp"

// Capability
#include "capability/capability_id.hpp"
#include "capability/capability.hpp"
#include "capability/capability_set.hpp"

// Configuration
#include "configuration/parameter.hpp"
#include "configuration/configuration.hpp"
#include "configuration/configuration_version.hpp"
