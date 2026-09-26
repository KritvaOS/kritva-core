//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : version.cpp
// Description : Version implementation.
//
// Component   : Kritva Core
// Module      : Types
// Layer       : Core Foundation
//
// Requirements: CORE-TYP-002
// API         : CORE-API-VERSION
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#include <kritva/core/types/version.hpp>
#include <string>
namespace kritva::core {
std::string Version::to_string() const {
    return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
}
} // namespace kritva::core
