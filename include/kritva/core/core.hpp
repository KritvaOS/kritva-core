//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : core.hpp
// Description : Public umbrella header for Kritva Core.
//
// Component   : Kritva Core
// Module      : Core
// Layer       : Core Foundation
//
// Requirements: CORE-API-001
// API         : CORE-API-UMBRELLA
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include "types/id.hpp"
#include "types/version.hpp"
#include "types/timestamp.hpp"
#include "types/duration.hpp"
#include "types/metadata.hpp"
#include "lifecycle/lifecycle_state.hpp"
#include "lifecycle/lifecycle.hpp"
#include "status/status_code.hpp"
#include "status/status.hpp"
#include "health/health_state.hpp"
#include "health/health.hpp"
#include "statistics/counter.hpp"
#include "statistics/gauge.hpp"
#include "statistics/statistics.hpp"
#include "error/error_code.hpp"
#include "error/error.hpp"
#include "error/result.hpp"
#include "event/event_type.hpp"
#include "event/event.hpp"
#include "capability/capability_id.hpp"
#include "capability/capability.hpp"
#include "capability/capability_set.hpp"
#include "configuration/parameter.hpp"
#include "configuration/configuration.hpp"
#include "configuration/configuration_version.hpp"
#include "runtime/component.hpp"
#include "runtime/runtime.hpp"
#include "messaging/message.hpp"
#include "messaging/topic.hpp"
#include "time/clock.hpp"
#include "time/timer.hpp"
#include "platform/scheduler.hpp"
#include "platform/clock.hpp"
#include "platform/watchdog.hpp"
