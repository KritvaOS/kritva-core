//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : capability_test.cpp
// Description : Capability and CapabilitySet API contract tests.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-CAP-001, CORE-CAP-002, CORE-CAP-003
// API         : CORE-TEST-CAPABILITY
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================

#include <cassert>
#include <kritva/core/capability/capability.hpp>
#include <kritva/core/capability/capability_set.hpp>

int main() {
    using namespace kritva::core;

    // CapabilityId follows the Core Id contract.
    CapabilityId invalid_id{};
    CapabilityId id{100};

    assert(!invalid_id.valid());
    assert(id.valid());
    assert(id.value() == 100);

    // A capability carries identity, name, and semantic version.
    Capability capability{
        id,
        "motion.control",
        Version{1, 2, 3}
    };

    assert(capability.id == id);
    assert(capability.name == "motion.control");
    assert(capability.version.major == 1);
    assert(capability.version.minor == 2);
    assert(capability.version.patch == 3);

    // Empty set contract.
    CapabilitySet set;
    assert(set.empty());
    assert(set.size() == 0);
    assert(!set.contains(id));
    assert(set.find(id) == nullptr);
    assert(set.all().empty());

    // Add and lookup.
    set.add(capability);
    assert(!set.empty());
    assert(set.size() == 1);
    assert(set.contains(id));

    const Capability* found = set.find(id);
    assert(found != nullptr);
    assert(found->name == "motion.control");
    assert(found->version.major == 1);
    assert(found->version.minor == 2);
    assert(found->version.patch == 3);

    // Adding the same identity replaces the existing capability rather
    // than creating a duplicate entry.
    Capability updated{
        id,
        "motion.control",
        Version{2, 0, 0}
    };

    set.add(updated);

    assert(set.size() == 1);
    assert(set.contains(id));

    found = set.find(id);
    assert(found != nullptr);
    assert(found->name == "motion.control");
    assert(found->version.major == 2);
    assert(found->version.minor == 0);
    assert(found->version.patch == 0);

    // A second capability must coexist.
    Capability sense{
        CapabilityId{200},
        "sense.camera",
        Version{1, 0, 0}
    };

    set.add(sense);

    assert(set.size() == 2);
    assert(set.contains(CapabilityId{200}));
    assert(set.find(CapabilityId{200}) != nullptr);
    assert(set.all().size() == 2);

    // Unknown identity must not be reported as present.
    assert(!set.contains(CapabilityId{999}));
    assert(set.find(CapabilityId{999}) == nullptr);

    return 0;
}
