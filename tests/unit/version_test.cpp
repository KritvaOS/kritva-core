//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : version_test.cpp
// Description : Version API contract tests.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-TYP-002
// API         : CORE-TEST-VERSION
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================

#include <cassert>
#include <kritva/core/types/version.hpp>

int main() {
    using namespace kritva::core;

    // Basic version construction and field access.
    Version version{1, 2, 3};

    assert(version.major == 1);
    assert(version.minor == 2);
    assert(version.patch == 3);

    // Canonical string representation.
    assert(version.to_string() == "1.2.3");

    // Zero version.
    Version zero_version{0, 0, 0};
    assert(zero_version.to_string() == "0.0.0");

    // Multi-digit version components.
    Version multi_digit_version{10, 20, 30};
    assert(multi_digit_version.to_string() == "10.20.30");

    return 0;
}
