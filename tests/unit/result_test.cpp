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
#include <string>
#include <type_traits>
#include <cstdlib>
#include <kritva/core/error/result.hpp>

#if defined(__unix__) || defined(__APPLE__)
#define KRITVA_TEST_FORK 1
#include <sys/wait.h>
#include <unistd.h>
#endif

#ifdef KRITVA_TEST_FORK
// Returns true if fn() terminates the process abnormally (assert -> SIGABRT).
template<class F> static bool traps(F fn) {
    pid_t pid = fork();
    if (pid == 0) {
        // Silence the assert diagnostic in the child.
        if (!freopen("/dev/null", "w", stderr)) _exit(2);
        fn();
        _exit(0);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    return WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT;
}
#endif

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

    // Contract: accessors are noexcept (preconditions, never exceptions).
    static_assert(noexcept(std::declval<const Result<int>&>().value()));
    static_assert(noexcept(std::declval<const Result<int>&>().error()));
    static_assert(noexcept(std::declval<const Result<void>&>().error()));

    // Contract: value access through a const result.
    const auto const_success = Result<std::string>::success("const");
    assert(const_success.has_value());
    assert(const_success.value() == "const");

    // Contract: a failure never reports a value, a success never reports failure.
    assert(!Result<int>::failure(error).has_value());
    assert(Result<int>::success(0).has_value());
    assert(Result<int>::success(0).value() == 0);  // falsy payload is still success

    // Contract: Error survives copy of the Result.
    auto copy = failure;
    assert(!copy.has_value());
    assert(copy.error().message == "bad argument");

    // Move construction (Result<T>): destination takes outcome and payload.
    {
        auto src = Result<std::string>::success("moved-in");
        Result<std::string> dst{std::move(src)};
        assert(dst.has_value());
        assert(dst.value() == "moved-in");
        assert(src.has_value());  // outcome of a moved-from Result is unchanged

        auto fsrc = Result<std::string>::failure(error);
        Result<std::string> fdst{std::move(fsrc)};
        assert(!fdst.has_value());
        assert(fdst.error().message == "bad argument");
        assert(!fsrc.has_value());
    }

    // Move assignment: replaces both outcome and payload (all four transitions).
    {
        auto dst = Result<std::string>::failure(error);
        dst = Result<std::string>::success("now ok");
        assert(dst.has_value() && dst.value() == "now ok");

        dst = Result<std::string>::failure(error);
        assert(!dst.has_value() && dst.error().code == ErrorCode::INVALID_ARGUMENT);

        auto s2 = Result<std::string>::success("a");
        s2 = Result<std::string>::success("b");
        assert(s2.has_value() && s2.value() == "b");

        auto f2 = Result<std::string>::failure(error);
        Error other = error;
        other.code = ErrorCode::TIMEOUT;
        f2 = Result<std::string>::failure(other);
        assert(!f2.has_value() && f2.error().code == ErrorCode::TIMEOUT);
    }

    // Copy assignment preserves the source.
    {
        auto src = Result<int>::success(5);
        auto dst = Result<int>::failure(error);
        dst = src;
        assert(dst.has_value() && dst.value() == 5);
        assert(src.has_value() && src.value() == 5);
    }

    // Result<void>: copy, move construction, and assignment.
    {
        auto vs = Result<void>::success();
        auto vf = Result<void>::failure(error);
        Result<void> vc{vf};
        assert(!vc.has_value() && vc.error().message == "bad argument");
        Result<void> vm{std::move(vf)};
        assert(!vm.has_value() && vm.error().code == ErrorCode::INVALID_ARGUMENT);
        vm = vs;
        assert(vm.has_value());
        vm = Result<void>::failure(error);
        assert(!vm.has_value());
    }

    // Contract: Result has no default constructor (no ambiguous empty state).
    static_assert(!std::is_default_constructible_v<Result<int>>);
    static_assert(!std::is_default_constructible_v<Result<void>>);
    static_assert(std::is_copy_constructible_v<Result<int>>);
    static_assert(std::is_nothrow_move_constructible_v<Result<int>>);

#ifdef KRITVA_TEST_FORK
    // Contract: invalid access / invalid failure construction is trapped.
    assert(traps([&] { (void)Result<int>::failure(error).value(); }));
    assert(traps([&] { (void)Result<int>::success(1).error(); }));
    assert(traps([&] { (void)Result<void>::success().error(); }));
    Error none{};  // code == NONE
    assert(traps([&] { (void)Result<int>::failure(none); }));
    assert(traps([&] { (void)Result<void>::failure(none); }));
    // Valid access does not trap.
    assert(!traps([&] { (void)Result<int>::success(1).value(); }));
#endif

    return 0;
}
