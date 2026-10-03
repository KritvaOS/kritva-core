//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : callback.hpp
// Description : Opaque function/context pair used by platform services.
//
// Component   : Kritva Core
// Module      : Types
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-004
// API         : CORE-API-PLATFORM
//
// Author      : KritvaOS Core Team
// Created     : 04-10-2026
//==============================================================================

#pragma once

namespace kritva::core {

//------------------------------------------------------------------------------
// Callback (CORE-PLAT-004)
//
// A function pointer plus an opaque context pointer: the shared vocabulary for
// "call this with that" across platform services (timer, watchdog notification,
// and so on). It is only a transport of the pair. It says nothing about who
// invokes it, on which execution context, or whether it may block, allocate or
// re-enter a service: each service that accepts a Callback states those rules
// itself.
//
//   - function: the entry point. nullptr is INVALID wherever a service requires
//     a callback (valid() is false). A callback function shall not throw.
//   - context:  an opaque pointer passed unchanged as the single argument. It
//     may be nullptr. It is NON-OWNING: Core and the service never dereference,
//     copy, free or otherwise manage it. The caller owns the pointee and must
//     keep it valid for as long as the service documents that the callback can
//     still be invoked (for example, until a timer's stop() has returned).
//   - The type is a plain value (trivially copyable, no allocation, no
//     ownership); copies are interchangeable and compare nothing.
//------------------------------------------------------------------------------
struct Callback {
    using Function = void (*)(void* context);

    Function function{nullptr};
    void* context{nullptr};

    /// True if a function is set. A null context does not make a callback invalid.
    [[nodiscard]] constexpr bool valid() const noexcept { return function != nullptr; }
};

} // namespace kritva::core
