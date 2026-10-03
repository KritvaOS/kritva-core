//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : component_context.hpp
// Description : ComponentContext: a Component's explicit, non-owning execution context.
//
// Component   : Kritva Core
// Module      : Runtime
// Layer       : Core Foundation
//
// Requirements: CORE-CTX-001
// API         : CORE-API-COMPONENT-CONTEXT
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#pragma once
#include "component_id.hpp"
#include "component_info.hpp"
#include "../platform/context.hpp"
namespace kritva::core::runtime {

//------------------------------------------------------------------------------
// ComponentContext (CORE-CTX-001)
//
// The explicit, deterministic, NON-OWNING context an integrator-written
// Component holds: who the component is (its immutable ComponentInfo) and which
// platform it was given (the R0.5 platform::PlatformContext view). "Context
// carries access; Runtime retains control."
//
// SHAPE
//   A small copyable value of exactly two non-owning pointers: a
//   const ComponentInfo* and a platform::PlatformContext (itself one pointer).
//   It is trivially destructible, nothrow default-constructible, and has no
//   cache, no registry, no static, global or thread-local state, and no
//   inference from any name or version.
//     ComponentContext()                       UNBOUND: no identity, no platform
//     explicit ComponentContext(const ComponentInfo&, platform::PlatformContext = {})
//                                              BOUND to that identity and platform
//   bound() is true exactly for the second form. info() returns the identity
//   or nullptr when unbound; id() returns the component's ComponentId or the
//   invalid id (ComponentId{}) when unbound. platform() returns the R0.5 view BY
//   CONST REFERENCE ONLY: a default-constructed (unattached) PlatformContext when
//   none was given.
//
// IMMUTABLE AFTER CONSTRUCTION
//   A context is fixed when it is constructed. It has no setter, no reset and no
//   rebinding of any kind, and it is not assignable (copy and move assignment are
//   deleted; copy and move construction are available): a component stores its
//   context, built once, in its constructor, so a context can never become
//   mutable shared state.
//
// OWNERSHIP AND LIFETIME
//   The context owns nothing, creates nothing and destroys nothing. The
//   ComponentInfo it refers to must outlive the context (it lives as long as its
//   Component, and Component::info() is stable until destruction); the adapter
//   behind the platform view must outlive every context, exactly as for
//   PlatformContext. Copying a ComponentContext copies the two pointers: it
//   neither extends the lifetime of the identity or the adapter nor transfers or
//   duplicates ownership. Using a context after what it refers to is destroyed
//   is undefined behavior; it cannot detect that. Destroying a context never
//   touches the identity or the adapter.
//
// NO CONTEXT AMPLIFICATION
//   The context never returns, directly or indirectly, a broader authority than
//   it was given: no RuntimeManager, ComponentRegistry, other Component,
//   Configuration, Statistics, Health, another component's context, or a raw
//   IPlatformAdapter. It cannot be used to look anything up by name.
//
// SIDE EFFECTS, THREADS, REAL TIME
//   Constructing, copying and querying identity or the platform view are plain
//   pointer operations: no allocation, no blocking, no call to the adapter or to
//   any service, and no effect on the Runtime. Core makes no thread-safety or
//   real-time claim beyond that of the adapter and services reached through it.
//------------------------------------------------------------------------------
class ComponentContext {
public:
    /// Unbound: no identity and an unattached platform view.
    ComponentContext() noexcept = default;

    /// Bound to `info` (which must outlive the context) and to the platform view `platform`.
    explicit ComponentContext(const ComponentInfo& info, platform::PlatformContext platform = {}) noexcept
        : info_(&info), platform_(platform) {}

    /// A temporary identity would dangle: refused at compile time.
    ComponentContext(const ComponentInfo&&, platform::PlatformContext = {}) = delete;

    ComponentContext(const ComponentContext&) noexcept = default;
    ComponentContext(ComponentContext&&) noexcept = default;
    ComponentContext& operator=(const ComponentContext&) = delete;      // immutable after construction
    ComponentContext& operator=(ComponentContext&&) = delete;
    ~ComponentContext() = default;

    /// True when the context was constructed with a ComponentInfo.
    [[nodiscard]] bool bound() const noexcept { return info_ != nullptr; }

    /// The component's identity, or nullptr when unbound.
    [[nodiscard]] const ComponentInfo* info() const noexcept { return info_; }

    /// The component's ComponentId, or the invalid id when unbound.
    [[nodiscard]] ComponentId id() const noexcept { return info_ != nullptr ? info_->id() : ComponentId{}; }

    /// The R0.5 platform view (unattached when none was given). Const reference only.
    [[nodiscard]] const platform::PlatformContext& platform() const noexcept { return platform_; }

private:
    const ComponentInfo* info_{nullptr};      // non-owning
    platform::PlatformContext platform_{};    // non-owning view
};
} // namespace kritva::core::runtime
