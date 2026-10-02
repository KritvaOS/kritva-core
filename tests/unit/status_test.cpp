//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : status_test.cpp
// Description : Unit tests for Kritva Core Status.
//
// Component   : Kritva Core
// Module      : Status
// Layer       : Core Foundation
//
// Requirements: CORE-STA-001
// API         : CORE-API-STATUS
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================

// Self-containment: each public status header must compile on its own, and
// status.hpp must not depend on being included after anything else.
#include <kritva/core/status/status_code.hpp>
#include <kritva/core/status/status.hpp>

#include <cassert>
#include <string>
#include <type_traits>
#include <utility>

using namespace kritva::core;

int main() {
    // -------------------------------------------------------------------------
    // Default construction
    // -------------------------------------------------------------------------
    {
        Status status;

        assert(status.code() == StatusCode::UNKNOWN);
        assert(status.message().empty());
    }

    // -------------------------------------------------------------------------
    // Construction with OK
    // -------------------------------------------------------------------------
    {
        Status status(StatusCode::OK);

        assert(status.code() == StatusCode::OK);
        assert(status.message().empty());
    }

    // -------------------------------------------------------------------------
    // Code update
    // -------------------------------------------------------------------------
    {
        Status status;

        status.set_code(StatusCode::NOT_READY);

        assert(status.code() == StatusCode::NOT_READY);
    }

    // -------------------------------------------------------------------------
    // Message update
    // -------------------------------------------------------------------------
    {
        Status status;

        status.set_message("component is not ready");

        assert(status.message() == "component is not ready");
    }

    // -------------------------------------------------------------------------
    // Failed operation
    // -------------------------------------------------------------------------
    {
        Status status(StatusCode::FAILED);

        status.set_message("component operation failed");

        assert(status.code() == StatusCode::FAILED);
        assert(status.message() == "component operation failed");
    }

    // -------------------------------------------------------------------------
    // Timeout
    // -------------------------------------------------------------------------
    {
        Status status(StatusCode::TIMEOUT);

        assert(status.code() == StatusCode::TIMEOUT);
    }

    // -------------------------------------------------------------------------
    // Invalid argument
    // -------------------------------------------------------------------------
    {
        Status status(StatusCode::INVALID_ARGUMENT);

        status.set_message("invalid parameter");

        assert(status.code() == StatusCode::INVALID_ARGUMENT);
        assert(status.message() == "invalid parameter");
    }

    // -------------------------------------------------------------------------
    // Message replacement
    // -------------------------------------------------------------------------
    {
        Status status(StatusCode::BUSY);

        status.set_message("resource busy");
        assert(status.message() == "resource busy");

        status.set_message("resource available");
        assert(status.message() == "resource available");
    }

    // -------------------------------------------------------------------------
    // Empty message is valid
    // -------------------------------------------------------------------------
    {
        Status status(StatusCode::OK);

        status.set_message("");

        assert(status.message().empty());
        assert(status.code() == StatusCode::OK);
    }

    // -------------------------------------------------------------------------
    // Every defined StatusCode round-trips through construction and set_code()
    // -------------------------------------------------------------------------
    {
        const StatusCode all[] = {
            StatusCode::UNKNOWN,     StatusCode::OK,          StatusCode::INVALID_ARGUMENT,
            StatusCode::NOT_READY,   StatusCode::BUSY,        StatusCode::TIMEOUT,
            StatusCode::FAILED,      StatusCode::UNAVAILABLE, StatusCode::UNSUPPORTED,
            StatusCode::INTERNAL_ERROR};
        for (StatusCode c : all) {
            Status a(c);
            assert(a.code() == c);
            assert(a.message().empty());
            Status b;
            b.set_code(c);
            assert(b.code() == c);
        }
        // UNKNOWN is the zero value and distinct from OK.
        assert(static_cast<unsigned>(StatusCode::UNKNOWN) == 0u);
        assert(StatusCode::UNKNOWN != StatusCode::OK);
    }

    // -------------------------------------------------------------------------
    // Code and message are independent
    // -------------------------------------------------------------------------
    {
        Status status(StatusCode::FAILED);
        status.set_message("boom");
        status.set_code(StatusCode::OK);
        assert(status.code() == StatusCode::OK);
        assert(status.message() == "boom");  // set_code does not touch message
        status.set_message("");
        assert(status.code() == StatusCode::OK);
    }

    // -------------------------------------------------------------------------
    // Copy and move
    // -------------------------------------------------------------------------
    {
        Status src(StatusCode::TIMEOUT);
        src.set_message("timed out");

        Status copy(src);
        assert(copy.code() == StatusCode::TIMEOUT && copy.message() == "timed out");
        copy.set_message("changed");
        assert(src.message() == "timed out");  // independent copy

        Status assigned;
        assigned = src;
        assert(assigned.code() == StatusCode::TIMEOUT && assigned.message() == "timed out");

        Status moved(std::move(src));
        assert(moved.code() == StatusCode::TIMEOUT && moved.message() == "timed out");
    }

    // -------------------------------------------------------------------------
    // Compile-time contract
    // -------------------------------------------------------------------------
    static_assert(std::is_nothrow_default_constructible_v<Status>);
    static_assert(std::is_nothrow_constructible_v<Status, StatusCode>);
    static_assert(!std::is_convertible_v<StatusCode, Status>);  // explicit
    static_assert(std::is_nothrow_move_constructible_v<Status>);
    static_assert(std::is_copy_constructible_v<Status>);
    static_assert(noexcept(std::declval<const Status&>().code()));
    static_assert(noexcept(std::declval<const Status&>().message()));
    static_assert(noexcept(std::declval<Status&>().set_code(StatusCode::OK)));
    static_assert(!noexcept(std::declval<Status&>().set_message(std::string{})));
    static_assert(std::is_trivially_copyable_v<StatusCode>);
    static_assert(sizeof(StatusCode) == 1);

    return 0;
}
