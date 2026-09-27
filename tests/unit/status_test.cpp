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
// Requirements: CORE-STATUS-*
// API         : CORE-API-STATUS
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================

#include <cassert>

#include <kritva/core/status/status.hpp>

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

    return 0;
}
