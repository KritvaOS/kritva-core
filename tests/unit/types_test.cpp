//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : types_test.cpp
// Description : Core Types API contract tests.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-TYP-001, CORE-TYP-002, CORE-TYP-003, CORE-TYP-004
// API         : CORE-TEST-TYPES
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================

#include <cassert>
#include <cstdint>
#include <string>
#include <unordered_map>

#include <kritva/core/types/id.hpp>
#include <kritva/core/types/version.hpp>
#include <kritva/core/types/timestamp.hpp>
#include <kritva/core/types/duration.hpp>
#include <kritva/core/types/metadata.hpp>

int main() {
    using namespace kritva::core;

    //--------------------------------------------------------------------------
    // 1. Id default and validity semantics
    //--------------------------------------------------------------------------

    Id invalid_id{};

    assert(invalid_id.value() == 0);
    assert(!invalid_id.valid());

    Id id{42};

    assert(id.value() == 42);
    assert(id.valid());

    //--------------------------------------------------------------------------
    // 2. Id equality, ordering, and hash support
    //--------------------------------------------------------------------------

    Id id_same{42};
    Id id_other{100};

    assert(id == id_same);
    assert(id != id_other);
    assert(id < id_other);
    assert(id_other > id);

    std::unordered_map<Id, std::string> id_map;
    id_map.emplace(id, "robot");

    assert(id_map.size() == 1);
    assert(id_map.at(Id{42}) == "robot");

    //--------------------------------------------------------------------------
    // 3. Version default and field semantics
    //--------------------------------------------------------------------------

    Version default_version{};

    assert(default_version.major == 0);
    assert(default_version.minor == 0);
    assert(default_version.patch == 0);

    Version version{
        1,
        2,
        3
    };

    assert(version.major == 1);
    assert(version.minor == 2);
    assert(version.patch == 3);

    Version same_version{
        1,
        2,
        3
    };

    Version different_version{
        1,
        2,
        4
    };

    assert(version == same_version);
    assert(version != different_version);

    // Version::to_string() is intentionally not checked for a specific
    // textual format here because the header declares the API but does not
    // specify its required formatting contract.

    //--------------------------------------------------------------------------
    // 4. Timestamp default semantics
    //--------------------------------------------------------------------------

    Timestamp default_timestamp{};

    assert(default_timestamp.nanoseconds() == 0);
    assert(default_timestamp.domain() == ClockDomain::MONOTONIC);

    //--------------------------------------------------------------------------
    // 5. Timestamp value and clock-domain semantics
    //--------------------------------------------------------------------------

    Timestamp monotonic_timestamp{
        123456789,
        ClockDomain::MONOTONIC
    };

    assert(monotonic_timestamp.nanoseconds() == 123456789);
    assert(monotonic_timestamp.domain() == ClockDomain::MONOTONIC);

    Timestamp realtime_timestamp{
        987654321,
        ClockDomain::REALTIME
    };

    assert(realtime_timestamp.nanoseconds() == 987654321);
    assert(realtime_timestamp.domain() == ClockDomain::REALTIME);

    assert(monotonic_timestamp != realtime_timestamp);

    Timestamp same_monotonic_timestamp{
        123456789,
        ClockDomain::MONOTONIC
    };
    
    assert(same_monotonic_timestamp == monotonic_timestamp);
    
    Timestamp different_domain_timestamp{
        123456789,
        ClockDomain::REALTIME
    };
    
    assert(same_monotonic_timestamp != different_domain_timestamp);

    //--------------------------------------------------------------------------
    // 6. Duration default and direct construction
    //--------------------------------------------------------------------------

    Duration default_duration{};

    assert(default_duration.nanoseconds() == 0);

    Duration duration{
        123456789
    };

    assert(duration.nanoseconds() == 123456789);

    Duration negative_duration{
        -500
    };

    assert(negative_duration.nanoseconds() == -500);

    //--------------------------------------------------------------------------
    // 7. Duration factory conversions
    //--------------------------------------------------------------------------

    assert(
        Duration::from_nanoseconds(1234).nanoseconds()
        == 1234
    );

    assert(
        Duration::from_microseconds(12).nanoseconds()
        == 12000
    );

    assert(
        Duration::from_milliseconds(3).nanoseconds()
        == 3000000
    );

    //--------------------------------------------------------------------------
    // 8. Duration equality and ordering
    //--------------------------------------------------------------------------

    Duration duration_same{
        123456789
    };

    Duration duration_other{
        123456790
    };

    assert(duration == duration_same);
    assert(duration != duration_other);
    assert(duration < duration_other);
    assert(duration_other > duration);

    //--------------------------------------------------------------------------
    // 9. Metadata default state
    //--------------------------------------------------------------------------

    Metadata metadata{};

    assert(metadata.empty());
    assert(metadata.size() == 0);
    assert(!metadata.contains("robot.name"));
    assert(metadata.get("robot.name") == nullptr);

    //--------------------------------------------------------------------------
    // 10. Metadata insertion and lookup
    //--------------------------------------------------------------------------

    metadata.set("robot.name", "Kritva-R1");
    metadata.set("robot.model", "reference");

    assert(!metadata.empty());
    assert(metadata.size() == 2);

    assert(metadata.contains("robot.name"));
    assert(metadata.contains("robot.model"));

    const std::string* name = metadata.get("robot.name");
    const std::string* model = metadata.get("robot.model");

    assert(name != nullptr);
    assert(model != nullptr);
    assert(*name == "Kritva-R1");
    assert(*model == "reference");

    // Missing keys must return false/nullptr.
    assert(!metadata.contains("does.not.exist"));
    assert(metadata.get("does.not.exist") == nullptr);

    //--------------------------------------------------------------------------
    // 11. Metadata update semantics
    //--------------------------------------------------------------------------

    metadata.set("robot.name", "Kritva-R2");

    assert(metadata.size() == 2);

    name = metadata.get("robot.name");

    assert(name != nullptr);
    assert(*name == "Kritva-R2");

    //--------------------------------------------------------------------------
    // 12. Metadata key/value boundary cases
    //--------------------------------------------------------------------------

    metadata.set("", "empty-key-value");

    assert(metadata.contains(""));
    assert(metadata.get("") != nullptr);
    assert(*metadata.get("") == "empty-key-value");

    metadata.set("empty-value", "");

    assert(metadata.contains("empty-value"));
    assert(metadata.get("empty-value") != nullptr);
    assert(metadata.get("empty-value")->empty());

    assert(metadata.size() == 4);

    //--------------------------------------------------------------------------
    // 13. Metadata value semantics
    //--------------------------------------------------------------------------

    Metadata copy{};

    copy.set("component", "runtime");
    copy.set("version", "0.2");

    Metadata copy2 = copy;

    assert(copy2.size() == 2);
    assert(copy2.contains("component"));
    assert(copy2.contains("version"));
    assert(*copy2.get("component") == "runtime");
    assert(*copy2.get("version") == "0.2");

    // Updating the copy must not modify the original.
    copy2.set("component", "messaging");

    assert(*copy.get("component") == "runtime");
    assert(*copy2.get("component") == "messaging");

    return 0;
}
