//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : result.hpp
// Description : Explicit success/failure propagation.
//
// Component   : Kritva Core
// Module      : Error
// Layer       : Core Foundation
//
// Requirements: CORE-ERR-004
// API         : CORE-API-RESULT
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include "error.hpp"
#include <cassert>
#include <optional>
#include <utility>

namespace kritva::core {

//------------------------------------------------------------------------------
// Result<T> / Result<void>
//
// Contract (CORE-ERR-004):
//   - A Result is exactly one of: success (has_value() == true) or failure
//     (has_value() == false, carrying an Error).
//   - value()  has the precondition has_value() == true.
//   - error()  has the precondition has_value() == false.
//   - Violating a precondition is a programming error: it is undefined
//     behavior in release builds and is trapped by assert() when NDEBUG is not
//     defined. Callers must check has_value() / operator bool first.
//
// Real-time notes:
//   - Allocation: none for Result itself; success(T)/failure(Error) allocate
//     only as T / Error::message do (Error holds a std::string).
//   - Blocking / synchronization: none. Not thread-safe; a Result object must
//     not be accessed concurrently without external synchronization.
//   - Complexity: all operations are O(1) apart from moving/copying T or Error.
//   - Exceptions: accessors never throw.
//------------------------------------------------------------------------------
template<class T> class Result {
public:
    static Result success(T value) { return Result{std::move(value), std::nullopt}; }
    static Result failure(Error error) { return Result{std::nullopt, std::move(error)}; }

    [[nodiscard]] bool has_value() const noexcept { return value_.has_value(); }
    [[nodiscard]] explicit operator bool() const noexcept { return has_value(); }

    /// Precondition: has_value().
    [[nodiscard]] const T& value() const& noexcept { assert(has_value()); return *value_; }
    [[nodiscard]] T& value() & noexcept { assert(has_value()); return *value_; }
    [[nodiscard]] T&& value() && noexcept { assert(has_value()); return std::move(*value_); }

    /// Precondition: !has_value().
    [[nodiscard]] const Error& error() const& noexcept { assert(!has_value()); return *error_; }

private:
    Result(std::optional<T> value, std::optional<Error> error)
        : value_(std::move(value)), error_(std::move(error)) {}
    std::optional<T> value_;
    std::optional<Error> error_;
};

template<> class Result<void> {
public:
    static Result success() { return Result{true, std::nullopt}; }
    static Result failure(Error error) { return Result{false, std::move(error)}; }

    [[nodiscard]] bool has_value() const noexcept { return success_; }
    [[nodiscard]] explicit operator bool() const noexcept { return success_; }

    /// Precondition: !has_value().
    [[nodiscard]] const Error& error() const& noexcept { assert(!success_); return *error_; }

private:
    Result(bool success, std::optional<Error> error) : success_(success), error_(std::move(error)) {}
    bool success_{false};
    std::optional<Error> error_;
};

} // namespace kritva::core
