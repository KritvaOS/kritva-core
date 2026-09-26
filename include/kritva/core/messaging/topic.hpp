//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : topic.hpp
// Description : Logical message topic identifier.
//
// Component   : Kritva Core
// Module      : Messaging
// Layer       : Core Foundation
//
// Requirements: CORE-MSG-002
// API         : CORE-API-MESSAGING
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


#pragma once
#include <string>
#include <utility>
namespace kritva::core::messaging {
class Topic {
public:
    explicit Topic(std::string name) : name_(std::move(name)) {}

    [[nodiscard]] const std::string& name() const noexcept { return name_; }

    /// Topic construction may allocate. Do not construct Topics in hard
    /// real-time control loops.
private:
    std::string name_;
};
} // namespace kritva::core::messaging
