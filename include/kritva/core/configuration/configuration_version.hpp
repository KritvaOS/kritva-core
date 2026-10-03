//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : configuration_version.hpp
// Description : Configuration schema/instance version.
//
// Component   : Kritva Core
// Module      : Configuration
// Layer       : Core Foundation
//
// Requirements: CORE-CFG-003; CORE-CFG-008
// API         : CORE-API-CONFIGURATION
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include "../types/version.hpp"
namespace kritva::core {

//------------------------------------------------------------------------------
// ConfigurationVersion (CORE-CFG-003, CORE-CFG-008)
//
// ConfigurationVersion is an alias of Version. It denotes the SCHEMA / CONTRACT
// COMPATIBILITY VERSION of a configuration: which set and meaning of parameters
// a configuration is written against and a component understands. It answers
// "is this configuration written for a contract I support?", nothing else.
//
// What it is NOT: a configuration instance revision, an update or generation
// counter, a transaction id, a timestamp or a history number. Core never creates,
// increments, stores, compares or interprets one, applying a different or the
// same configuration again never changes it, and R0.8 adds no revision API and
// no version field to Configuration (a Configuration carries parameters only).
//
// Compatibility is policy of the component (or the integrator), not of Core.
// A component that versions its schema declares the ConfigurationVersion(s) it
// understands, learns the version of an incoming configuration by a convention it
// documents (for example an ordinary Parameter, or out of band: Core reserves no
// parameter name) and decides compatibility itself, typically by major version.
// An incompatible schema is reported by the component's configure() as
// ErrorCode::CONFIGURATION_ERROR, source = the component, with nothing applied
// (CORE-CFG-006); no new ErrorCode exists. Core does not know any component's
// schema and therefore knows no parameter range, enumeration or default.
//
// Plain value type: trivially copyable, no allocation (to_string() allocates),
// thread-safe for concurrent const use.
//------------------------------------------------------------------------------
using ConfigurationVersion = Version;
} // namespace kritva::core
