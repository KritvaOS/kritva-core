//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : configuration_version.hpp
// Description : Version identifier for a configuration schema or instance.
//
// Component   : Kritva Core
// Module      : Configuration
// Layer       : Core Foundation
//
// Requirements: CORE-CFG-003
// API         : CORE-API-CONFIGURATION
//
// Author      : KritvaOS Core Team
// Created     : 2026-09-14
//==============================================================================

#pragma once

#include "../types/version.hpp"

namespace kritva::core {

/// @brief Explicit configuration schema/instance version.
using ConfigurationVersion = Version;

} // namespace kritva::core
