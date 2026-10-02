//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : event_test.cpp
// Description : Event and EventType API contract tests.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-EVT-001, CORE-EVT-002, CORE-EVT-003, CORE-EVT-004
// API         : CORE-TEST-EVENT
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================

#include <cassert>

#include <kritva/core/event/event.hpp>
#include <kritva/core/event/event_type.hpp>

int main() {
    using namespace kritva::core;

    //--------------------------------------------------------------------------
    // 1. Default Event
    //--------------------------------------------------------------------------

    Event event{};

    assert(!event.event_id.valid());
    assert(!event.source_id.valid());
    assert(event.type == EventType::UNKNOWN);
    assert(event.timestamp.nanoseconds() == 0);
    assert(event.timestamp.domain() == ClockDomain::MONOTONIC);
    assert(event.severity == ErrorSeverity::INFO);
    assert(!event.correlation_id.valid());

    //--------------------------------------------------------------------------
    // 2. Event ID
    //--------------------------------------------------------------------------

    event.event_id = Id{100};

    assert(event.event_id.valid());
    assert(event.event_id.value() == 100);

    //--------------------------------------------------------------------------
    // 3. Source ID
    //--------------------------------------------------------------------------

    event.source_id = Id{200};

    assert(event.source_id.valid());
    assert(event.source_id.value() == 200);

    //--------------------------------------------------------------------------
    // 4. EventType enumeration
    //--------------------------------------------------------------------------

    event.type = EventType::UNKNOWN;
    assert(event.type == EventType::UNKNOWN);

    event.type = EventType::LIFECYCLE;
    assert(event.type == EventType::LIFECYCLE);

    event.type = EventType::STATUS;
    assert(event.type == EventType::STATUS);

    event.type = EventType::HEALTH;
    assert(event.type == EventType::HEALTH);

    event.type = EventType::ERROR;
    assert(event.type == EventType::ERROR);

    event.type = EventType::CONFIGURATION;
    assert(event.type == EventType::CONFIGURATION);

    event.type = EventType::CAPABILITY;
    assert(event.type == EventType::CAPABILITY);

    //--------------------------------------------------------------------------
    // 5. Timestamp
    //--------------------------------------------------------------------------

    event.timestamp = Timestamp{
        123456789,
        ClockDomain::MONOTONIC
    };

    assert(event.timestamp.nanoseconds() == 123456789);
    assert(event.timestamp.domain() == ClockDomain::MONOTONIC);

    event.timestamp = Timestamp{
        987654321,
        ClockDomain::REALTIME
    };

    assert(event.timestamp.nanoseconds() == 987654321);
    assert(event.timestamp.domain() == ClockDomain::REALTIME);

    //--------------------------------------------------------------------------
    // 6. Severity
    //--------------------------------------------------------------------------

    event.severity = ErrorSeverity::INFO;
    assert(event.severity == ErrorSeverity::INFO);

    event.severity = ErrorSeverity::WARNING;
    assert(event.severity == ErrorSeverity::WARNING);

    event.severity = ErrorSeverity::ERROR;
    assert(event.severity == ErrorSeverity::ERROR);

    //--------------------------------------------------------------------------
    // 7. Correlation ID
    //--------------------------------------------------------------------------

    event.correlation_id = Id{300};

    assert(event.correlation_id.valid());
    assert(event.correlation_id.value() == 300);

    //--------------------------------------------------------------------------
    // 8. Complete Event envelope
    //--------------------------------------------------------------------------

    event.event_id = Id{1001};
    event.source_id = Id{2001};
    event.type = EventType::HEALTH;
    event.timestamp = Timestamp{
        123456789,
        ClockDomain::MONOTONIC
    };
    event.severity = ErrorSeverity::WARNING;
    event.correlation_id = Id{3001};

    assert(event.event_id.value() == 1001);
    assert(event.source_id.value() == 2001);
    assert(event.type == EventType::HEALTH);
    assert(event.timestamp.nanoseconds() == 123456789);
    assert(event.timestamp.domain() == ClockDomain::MONOTONIC);
    assert(event.severity == ErrorSeverity::WARNING);
    assert(event.correlation_id.value() == 3001);

    //--------------------------------------------------------------------------
    // 9. Copy/value semantics
    //--------------------------------------------------------------------------

    Event original{};

    original.event_id = Id{100};
    original.source_id = Id{200};
    original.type = EventType::HEALTH;
    original.timestamp = Timestamp{
        1000,
        ClockDomain::MONOTONIC
    };
    original.severity = ErrorSeverity::WARNING;
    original.correlation_id = Id{300};

    Event copy = original;

    assert(copy.event_id.value() == 100);
    assert(copy.source_id.value() == 200);
    assert(copy.type == EventType::HEALTH);
    assert(copy.timestamp.nanoseconds() == 1000);
    assert(copy.timestamp.domain() == ClockDomain::MONOTONIC);
    assert(copy.severity == ErrorSeverity::WARNING);
    assert(copy.correlation_id.value() == 300);

    // Modifying the copy must not modify the original.
    copy.event_id = Id{999};
    copy.type = EventType::ERROR;

    assert(original.event_id.value() == 100);
    assert(original.type == EventType::HEALTH);

    return 0;
}
