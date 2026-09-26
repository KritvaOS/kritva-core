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
#include <optional>
#include <utility>
namespace kritva::core {
template<class T> class Result {
public:
    static Result success(T value) { return Result{std::move(value), std::nullopt}; }
    static Result failure(Error error) { return Result{std::nullopt, std::move(error)}; }
    [[nodiscard]] bool has_value() const noexcept { return value_.has_value(); }
    [[nodiscard]] explicit operator bool() const noexcept { return has_value(); }
    [[nodiscard]] const T& value() const& { return *value_; }
    [[nodiscard]] T& value() & { return *value_; }
    [[nodiscard]] T&& value() && { return std::move(*value_); }
    [[nodiscard]] const Error& error() const& { return *error_; }
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
    [[nodiscard]] const Error& error() const& { return *error_; }
private:
    Result(bool success, std::optional<Error> error) : success_(success), error_(std::move(error)) {}
    bool success_{false};
    std::optional<Error> error_;
};
} // namespace kritva::core
