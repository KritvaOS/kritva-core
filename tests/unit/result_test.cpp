//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : result_test.cpp
// Description : Result<T> and Result<void> API contract tests.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-ERR-004
// API         : CORE-TEST-RESULT
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================

#include <cassert>
#include <kritva/core/error/result.hpp>

int main() {
    using namespace kritva::core;

    Error error{};
    error.code = ErrorCode::INVALID_ARGUMENT;
    error.severity = ErrorSeverity::ERROR;
    error.source = Id{7};
    error.timestamp = Timestamp{1000, ClockDomain::MONOTONIC};
    error.message = "bad argument";

    // Result<T>: successful value.
    auto success = Result<int>::success(42);
    assert(success.has_value());
    assert(static_cast<bool>(success));
    assert(success.value() == 42);

    // Result<T>: failed result carries the supplied Error.
    auto failure = Result<int>::failure(error);
    assert(!failure.has_value());
    assert(!static_cast<bool>(failure));
    assert(failure.error().code == ErrorCode::INVALID_ARGUMENT);
    assert(failure.error().severity == ErrorSeverity::ERROR);
    assert(failure.error().source.value() == 7);
    assert(failure.error().timestamp.nanoseconds() == 1000);
    assert(failure.error().message == "bad argument");

    // Result<T>: mutable lvalue access.
    auto mutable_result = Result<int>::success(10);
    mutable_result.value() = 20;
    assert(mutable_result.value() == 20);

    // Result<T>: move access.
    auto move_result = Result<std::string>::success("kritva");
    std::string moved = std::move(move_result).value();
    assert(moved == "kritva");

    // Result<void>: success.
    auto void_success = Result<void>::success();
    assert(void_success.has_value());
    assert(static_cast<bool>(void_success));

    // Result<void>: failure carries the supplied Error.
    auto void_failure = Result<void>::failure(error);
    assert(!void_failure.has_value());
    assert(!static_cast<bool>(void_failure));
    assert(void_failure.error().code == ErrorCode::INVALID_ARGUMENT);
    assert(void_failure.error().message == "bad argument");

    return 0;
}
