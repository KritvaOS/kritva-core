//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : capability.hpp
// Description : Capability metadata.
//
// Component   : Kritva Core
// Module      : Capability
// Layer       : Core Foundation
//
// Requirements: CORE-CAP-002
// API         : CORE-API-CAPABILITY
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include "capability_id.hpp"
#include "../types/version.hpp"
#include <string>
namespace kritva::core {
struct Capability {
    CapabilityId id{};
    std::string name;
    Version version{};
};
} // namespace kritva::core
