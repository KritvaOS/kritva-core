//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : error_test.cpp
// Description : Error API contract tests.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-ERR-001, CORE-ERR-002
// API         : CORE-TEST-ERROR
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================

#include <cassert>
#include <kritva/core/error/error.hpp>

int main() {
    using namespace kritva::core;

    // Default error contract.
    Error default_error{};
    assert(default_error.code == ErrorCode::NONE);
    assert(default_error.severity == ErrorSeverity::ERROR);
    assert(!default_error.source.valid());
    assert(default_error.timestamp.nanoseconds() == 0);
    assert(default_error.timestamp.domain() == ClockDomain::MONOTONIC);
    assert(default_error.message.empty());

    // Structured error fields must preserve the supplied values.
    Error error{};
    error.code = ErrorCode::INVALID_ARGUMENT;
    error.severity = ErrorSeverity::WARNING;
    error.source = Id{42};
    error.timestamp = Timestamp{123456789, ClockDomain::MONOTONIC};
    error.message = "invalid parameter";

    assert(error.code == ErrorCode::INVALID_ARGUMENT);
    assert(error.severity == ErrorSeverity::WARNING);
    assert(error.source.valid());
    assert(error.source.value() == 42);
    assert(error.timestamp.nanoseconds() == 123456789);
    assert(error.timestamp.domain() == ClockDomain::MONOTONIC);
    assert(error.message == "invalid parameter");

    // Realtime timestamps must retain their clock domain.
    error.timestamp = Timestamp{987654321, ClockDomain::REALTIME};
    assert(error.timestamp.nanoseconds() == 987654321);
    assert(error.timestamp.domain() == ClockDomain::REALTIME);

    return 0;
}
