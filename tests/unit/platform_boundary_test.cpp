//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : platform_boundary_test.cpp
// Description : Platform adapter boundary contract tests (Callback, errors, ownership).
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-004
// API         : CORE-TEST-PLATFORM-BOUNDARY
//
// Author      : KritvaOS Core Team
// Created     : 04-10-2026
//==============================================================================

#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <kritva/core/core.hpp>

using namespace kritva::core;
using namespace kritva::core::platform;

namespace {

// -----------------------------------------------------------------------------
// Callback: a plain, non-owning function/context pair
// -----------------------------------------------------------------------------

void bump(void* context) {
    if (context != nullptr) ++*static_cast<int*>(context);
}

void test_callback_is_a_plain_value() {
    static_assert(std::is_trivially_copyable_v<Callback>);
    static_assert(std::is_standard_layout_v<Callback>);
    static_assert(sizeof(Callback) == sizeof(void (*)(void*)) + sizeof(void*));
    static_assert(std::is_nothrow_default_constructible_v<Callback>);
    static_assert(std::is_same_v<Callback::Function, void (*)(void*)>);

    constexpr Callback none{};
    static_assert(!none.valid());                         // no function: invalid
    static_assert(none.context == nullptr);

    constexpr Callback with_null_context{&bump, nullptr};
    static_assert(with_null_context.valid());             // a null context does not invalidate it

    Callback only_context{nullptr, &only_context};
    assert(!only_context.valid());                        // a context without a function is invalid
}

void test_callback_passes_the_context_through_unchanged() {
    int counter = 0;
    const Callback cb{&bump, &counter};
    assert(cb.valid() && cb.context == &counter);
    cb.function(cb.context);
    cb.function(cb.context);
    assert(counter == 2);

    // A copy is interchangeable with the original (the type compares nothing).
    const Callback copy = cb;
    copy.function(copy.context);
    assert(counter == 3 && copy.context == cb.context && copy.function == cb.function);

    // A null context is passed through as null and the function copes with it.
    const Callback null_context{&bump, nullptr};
    null_context.function(null_context.context);
    assert(counter == 3);
}

// -----------------------------------------------------------------------------
// Error helper and propagation through Result (existing Result/Error semantics)
// -----------------------------------------------------------------------------

void test_make_error_builds_a_plain_error() {
    const Error e = make_error(ErrorCode::UNSUPPORTED, "no affinity");
    assert(e.code == ErrorCode::UNSUPPORTED && e.severity == ErrorSeverity::ERROR);
    assert(e.message == "no affinity");
    assert(!e.source.valid() && e.timestamp == Timestamp{});

    const Error w = make_error(ErrorCode::TIMEOUT, "slow", ErrorSeverity::WARNING);
    assert(w.severity == ErrorSeverity::WARNING && w.code == ErrorCode::TIMEOUT);
}

// A fake adapter service that reports each boundary error and never touches a context.
class FakeScheduler final : public IScheduler {
public:
    Result<TaskId> create_task(const TaskConfig& config, void (*entry)(void*), void* context) override {
        last_context = context;                           // stored, never dereferenced
        if (entry == nullptr) return Result<TaskId>::failure(make_error(ErrorCode::INVALID_ARGUMENT, "entry is null"));
        if (config.period.nanoseconds() < 0) return Result<TaskId>::failure(make_error(ErrorCode::INVALID_ARGUMENT, "negative period"));
        if (config.cpu_affinity != 0) return Result<TaskId>::failure(make_error(ErrorCode::UNSUPPORTED, "affinity unsupported"));
        if (created >= capacity) return Result<TaskId>::failure(make_error(ErrorCode::RESOURCE_UNAVAILABLE, "no task slots"));
        return Result<TaskId>::success(++created);
    }
    Result<void> start() override {
        if (running) return Result<void>::failure(make_error(ErrorCode::INVALID_STATE, "already running"));
        running = true;
        return Result<void>::success();
    }
    Result<void> stop() override { running = false; return Result<void>::success(); }

    void* last_context{nullptr};
    TaskId created{0};
    TaskId capacity{2};
    bool running{false};
};

void entry(void*) {}

void test_adapter_errors_propagate_unchanged_through_result() {
    FakeScheduler adapter;
    TaskConfig config;

    const auto bad_entry = adapter.create_task(config, nullptr, nullptr);
    assert(!bad_entry && bad_entry.error().code == ErrorCode::INVALID_ARGUMENT && bad_entry.error().message == "entry is null");

    config.period = Duration::from_nanoseconds(-1);
    assert(adapter.create_task(config, entry, nullptr).error().code == ErrorCode::INVALID_ARGUMENT);
    config.period = Duration{};
    config.cpu_affinity = 1;
    assert(adapter.create_task(config, entry, nullptr).error().code == ErrorCode::UNSUPPORTED);
    config.cpu_affinity = 0;

    assert(adapter.create_task(config, entry, nullptr) && adapter.create_task(config, entry, nullptr));
    const auto full = adapter.create_task(config, entry, nullptr);
    assert(!full && full.error().code == ErrorCode::RESOURCE_UNAVAILABLE);
    assert(adapter.created == 2);                          // a failed request had no effect

    assert(adapter.start());
    const auto again = adapter.start();
    assert(!again && again.error().code == ErrorCode::INVALID_STATE);

    // The code travels through generic Result code without any translation.
    auto forward = [](const Result<void>& r) -> Result<void> { return r.has_value() ? r : Result<void>::failure(r.error()); };
    const auto relayed = forward(again);
    assert(!relayed && relayed.error().code == again.error().code && relayed.error().message == again.error().message);
}

// -----------------------------------------------------------------------------
// Ownership and lifetime: the integrator owns everything, Core-facing code only
// refers to it
// -----------------------------------------------------------------------------

struct Probe {
    explicit Probe(bool* destroyed) : destroyed_(destroyed) {}
    ~Probe() { *destroyed_ = true; }
    int value{41};
private:
    bool* destroyed_;
};

// A Core-side holder: keeps a NON-OWNING pointer to a service, as RuntimeManager will.
struct Holder {
    IScheduler* scheduler{nullptr};
};

void test_services_and_contexts_are_never_owned_by_the_contract() {
    bool context_gone = false;
    auto context = std::make_unique<Probe>(&context_gone);
    {
        FakeScheduler adapter;
        const auto id = adapter.create_task(TaskConfig{}, entry, context.get());
        assert(id);
        assert(adapter.last_context == context.get());
        assert(context->value == 41);                      // the service never touched the context
    }                                                       // adapter destroyed
    assert(!context_gone && context->value == 41);          // the context is the caller's; not freed
    context.reset();
    assert(context_gone);

    // A holder refers to a service without owning it: destroying the holder leaves the service intact.
    FakeScheduler adapter;
    {
        Holder holder;
        holder.scheduler = &adapter;
        assert(holder.scheduler->start());
    }
    assert(adapter.running);
    static_assert(std::is_pointer_v<decltype(Holder::scheduler)>);
}

void test_no_global_platform_state() {
    // Two adapters coexist and are fully independent: there is no singleton or shared registry.
    FakeScheduler first, second;
    assert(first.start() && !second.running);
    assert(first.create_task(TaskConfig{}, entry, nullptr).value() == 1);
    assert(second.create_task(TaskConfig{}, entry, nullptr).value() == 1);   // ids are per instance
    assert(first.created == 1 && second.created == 1);
    static_assert(std::has_virtual_destructor_v<IScheduler>);
    static_assert(std::has_virtual_destructor_v<IWatchdog>);
    static_assert(std::has_virtual_destructor_v<time::IClock>);
}

// The boundary needs nothing from an operating system: this file, the Core headers and the
// test registration use only the C++ standard library and Core.

} // namespace

int main() {
    test_callback_is_a_plain_value();
    test_callback_passes_the_context_through_unchanged();
    test_make_error_builds_a_plain_error();
    test_adapter_errors_propagate_unchanged_through_result();
    test_services_and_contexts_are_never_owned_by_the_contract();
    test_no_global_platform_state();
    return 0;
}
