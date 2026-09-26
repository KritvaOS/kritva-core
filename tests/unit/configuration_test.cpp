//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : configuration_test.cpp
// Description : Configuration and Parameter API contract tests.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-CFG-001, CORE-CFG-002, CORE-CFG-003
// API         : CORE-TEST-CONFIGURATION
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================

#include <cassert>
#include <cstdint>
#include <string>

#include <kritva/core/configuration/configuration.hpp>
#include <kritva/core/configuration/configuration_version.hpp>
#include <kritva/core/configuration/parameter.hpp>

int main() {
    using namespace kritva::core;

    Configuration configuration;

    // Empty configuration is structurally valid.
    assert(configuration.size() == 0);
    assert(!configuration.contains("robot.rate"));
    assert(configuration.get("robot.rate") == nullptr);

    auto empty_validation = configuration.validate();
    assert(empty_validation.has_value());

    // Parameter names are mandatory.
    Parameter invalid_parameter{
        "",
        std::int64_t{10},
        "Invalid parameter with no name"
    };

    auto invalid_result = configuration.set(std::move(invalid_parameter));
    assert(!invalid_result.has_value());
    assert(invalid_result.error().code == ErrorCode::INVALID_ARGUMENT);

    // Boolean parameter.
    Parameter enabled{
        "robot.enabled",
        true,
        "Enable robot runtime"
    };

    auto enabled_result = configuration.set(enabled);
    assert(enabled_result.has_value());
    assert(configuration.size() == 1);
    assert(configuration.contains("robot.enabled"));

    const Parameter* found = configuration.get("robot.enabled");
    assert(found != nullptr);
    assert(found->name == "robot.enabled");
    assert(std::get<bool>(found->value));
    assert(found->description == "Enable robot runtime");

    // Integer parameter.
    Parameter cycle_time{
        "control.cycle_us",
        std::int64_t{1000},
        "Control cycle period in microseconds"
    };

    auto cycle_result = configuration.set(cycle_time);
    assert(cycle_result.has_value());
    assert(configuration.size() == 2);

    found = configuration.get("control.cycle_us");
    assert(found != nullptr);
    assert(std::get<std::int64_t>(found->value) == 1000);

    // Floating-point parameter.
    Parameter gain{
        "control.gain",
        0.75,
        "Controller gain"
    };

    assert(configuration.set(gain).has_value());
    found = configuration.get("control.gain");
    assert(found != nullptr);
    assert(std::get<double>(found->value) == 0.75);

    // String parameter.
    Parameter frame{
        "robot.frame",
        std::string{"base_link"},
        "Root robot frame"
    };

    assert(configuration.set(frame).has_value());
    found = configuration.get("robot.frame");
    assert(found != nullptr);
    assert(std::get<std::string>(found->value) == "base_link");

    assert(configuration.size() == 4);

    // Setting an existing name updates the parameter rather than increasing
    // the number of entries.
    Parameter updated_cycle{
        "control.cycle_us",
        std::int64_t{500},
        "Updated control cycle period"
    };

    assert(configuration.set(updated_cycle).has_value());
    assert(configuration.size() == 4);

    found = configuration.get("control.cycle_us");
    assert(found != nullptr);
    assert(std::get<std::int64_t>(found->value) == 500);
    assert(found->description == "Updated control cycle period");

    // Configuration remains structurally valid.
    auto validation = configuration.validate();
    assert(validation.has_value());

    // Unknown parameters return nullptr.
    assert(!configuration.contains("does.not.exist"));
    assert(configuration.get("does.not.exist") == nullptr);

    // ConfigurationVersion is the Core semantic Version type.
    ConfigurationVersion version{1, 2, 3};
    assert(version.major == 1);
    assert(version.minor == 2);
    assert(version.patch == 3);

    return 0;
}
