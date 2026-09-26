//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : core_contract_test.cpp
// Description : Public Core API contract smoke test.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-GEN-004
// API         : CORE-TEST-CONTRACT
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#include <kritva/core/core.hpp>
#include <type_traits>
static_assert(std::is_trivially_copy_constructible_v<kritva::core::Id>);
int main() {
    kritva::core::Id id{42};
    return id.valid() && id.value() == 42 ? 0 : 1;
}
