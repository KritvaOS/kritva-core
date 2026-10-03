//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : reference_platform.hpp
// Description : Test-only reference platform and service fakes with deterministic controls.
//
// Component   : Kritva Core
// Module      : Platform Test Support
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-016
// API         : CORE-TEST-REFERENCE-PLATFORM
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#pragma once

// A deterministic, hardware-free platform for tests. TEST SUPPORT ONLY: it lives under
// tests/, is compiled only by test targets and is never part of the production library
// or its install (a CTest and the repository audit enforce this).
//
// It implements only the PUBLIC contracts (IPlatformAdapter, IScheduler, time::IClock,
// time::ITimer, IWatchdog) by composing the reference doubles of tests/contract/, and adds
// controls a test needs and a real platform would never offer:
//   - selectable services and capabilities, fixed at construction (the contract requires
//     stable service objects for the adapter's life, so availability is never changed later);
//   - deterministic fault injection per service method: fail the Nth call, or every call,
//     with a chosen ErrorCode. An injected failure is atomic (no state changes), is counted
//     as a call, and is logged;
//   - an ordered log of every service call (method, outcome, code) and of every timer
//     callback and scheduler task entry, plus a count of queries made to the adapter itself,
//     so "who called what, in which order" is observable;
//   - a controllable clock and explicit time advancement (nothing ever advances by itself:
//     there is no thread, no timer loop and no real time anywhere);
//   - lifetime observation through a LifetimeProbe the test owns.
// Nothing here is a model of any real platform.

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include <kritva/core/platform/adapter.hpp>

#include "../contract/reference_scheduler.hpp"
#include "../contract/reference_timer.hpp"
#include "../contract/reference_watchdog.hpp"
#include "conformance.hpp"

namespace kritva::core::platform::testing {

/// Everything a test can observe or make fail.
enum class Method : std::uint8_t {
    SCHEDULER_CREATE_TASK, SCHEDULER_START, SCHEDULER_STOP,
    TIMER_START, TIMER_STOP,
    WATCHDOG_START, WATCHDOG_KICK, WATCHDOG_STOP,
    TIMER_CALLBACK,   // a timer callback was invoked (log only; cannot be made to fail)
    TASK_ENTRY        // a scheduler task entry was invoked (log only; cannot be made to fail)
};
constexpr std::size_t METHOD_COUNT = 10;

struct Record {
    Method method;
    bool ok;
    ErrorCode code;   // ErrorCode::NONE when ok
};

/// Observes the lifetimes of a ReferencePlatform and of its services.
struct LifetimeProbe {
    bool platform_destroyed{false};
    int services_destroyed{0};
};

/// Deterministic fault injection and the call log, shared by a platform and its services.
class Controls {
public:
    /// Fail the `nth` call (1-based, counted since construction or the last reset_counts()) of `method`.
    void fail_nth(Method method, std::size_t nth, ErrorCode code) { rules_.push_back(Rule{method, nth, code, false}); }
    /// Fail every call of `method`.
    void fail_all(Method method, ErrorCode code) { rules_.push_back(Rule{method, 0, code, true}); }
    /// Remove every injected failure (counts and log are kept).
    void clear_faults() { rules_.clear(); }
    void reset_counts() { for (std::size_t& c : counts_) c = 0; }
    void clear_log() { log_.clear(); }

    [[nodiscard]] std::size_t calls(Method method) const noexcept { return counts_[static_cast<std::size_t>(method)]; }
    [[nodiscard]] const std::vector<Record>& log() const noexcept { return log_; }
    [[nodiscard]] std::size_t log_size(Method method) const noexcept {
        std::size_t n = 0;
        for (const Record& r : log_) if (r.method == method) ++n;
        return n;
    }
    /// Queries made to the adapter itself (info, accessors, capabilities): a test uses this to show a caller never touched it.
    [[nodiscard]] std::size_t adapter_queries() const noexcept { return adapter_queries_; }

    // Used by the fakes.
    void count_adapter_query() const noexcept { ++adapter_queries_; }
    /// Counts the call and returns the injected ErrorCode, or NONE.
    ErrorCode begin(Method method) {
        const std::size_t n = ++counts_[static_cast<std::size_t>(method)];
        for (const Rule& rule : rules_) {
            if (rule.method == method && (rule.all || rule.nth == n)) return rule.code;
        }
        return ErrorCode::NONE;
    }
    void record(Method method, ErrorCode code) { log_.push_back(Record{method, code == ErrorCode::NONE, code}); }

private:
    struct Rule { Method method; std::size_t nth; ErrorCode code; bool all; };
    std::vector<Rule> rules_;
    std::size_t counts_[METHOD_COUNT]{};
    std::vector<Record> log_;
    mutable std::size_t adapter_queries_{0};
};

inline Error injected(ErrorCode code) { return make_error(code, "injected failure"); }

class FakeScheduler final : public contract::ReferenceScheduler {
public:
    FakeScheduler(Controls& controls, LifetimeProbe* probe) : controls_(controls), probe_(probe) {}
    ~FakeScheduler() override { if (probe_ != nullptr) ++probe_->services_destroyed; }

    Result<TaskId> create_task(const TaskConfig& config, void (*entry)(void*), void* context) override {
        if (const ErrorCode code = controls_.begin(Method::SCHEDULER_CREATE_TASK); code != ErrorCode::NONE) {
            controls_.record(Method::SCHEDULER_CREATE_TASK, code);
            return Result<TaskId>::failure(injected(code));              // atomic: no slot, no id
        }
        Trampoline* trampoline = nullptr;
        if (entry != nullptr) {
            wrapped_.push_back(std::make_unique<Trampoline>(Trampoline{this, entry, context}));
            trampoline = wrapped_.back().get();
        }
        const Result<TaskId> result = ReferenceScheduler::create_task(config, entry == nullptr ? nullptr : &FakeScheduler::run, trampoline);
        if (!result && trampoline != nullptr) wrapped_.pop_back();          // a rejected creation keeps nothing
        controls_.record(Method::SCHEDULER_CREATE_TASK, result ? ErrorCode::NONE : result.error().code);
        return result;
    }
    Result<void> start() override {
        if (const ErrorCode code = controls_.begin(Method::SCHEDULER_START); code != ErrorCode::NONE) {
            controls_.record(Method::SCHEDULER_START, code);
            return Result<void>::failure(injected(code));
        }
        const Result<void> result = ReferenceScheduler::start();
        controls_.record(Method::SCHEDULER_START, result ? ErrorCode::NONE : result.error().code);
        return result;
    }
    Result<void> stop() override {
        if (const ErrorCode code = controls_.begin(Method::SCHEDULER_STOP); code != ErrorCode::NONE) {
            controls_.record(Method::SCHEDULER_STOP, code);
            return Result<void>::failure(injected(code));
        }
        const Result<void> result = ReferenceScheduler::stop();
        controls_.record(Method::SCHEDULER_STOP, result ? ErrorCode::NONE : result.error().code);
        return result;
    }

private:
    struct Trampoline { FakeScheduler* owner; void (*entry)(void*); void* context; };
    static void run(void* raw) {
        auto* t = static_cast<Trampoline*>(raw);
        t->owner->controls_.record(Method::TASK_ENTRY, ErrorCode::NONE);
        t->entry(t->context);
    }
    Controls& controls_;
    LifetimeProbe* probe_;
    std::vector<std::unique_ptr<Trampoline>> wrapped_;
};

/// A clock a test controls. A MONOTONIC clock never goes backward; a REALTIME clock may be set to any value.
class FakeClock final : public time::IClock {
public:
    FakeClock(ClockDomain domain, LifetimeProbe* probe) : domain_(domain), probe_(probe) {}
    ~FakeClock() override { if (probe_ != nullptr) ++probe_->services_destroyed; }
    [[nodiscard]] Timestamp now() const noexcept override { return Timestamp{nanoseconds_, domain_}; }
    void advance(std::int64_t nanoseconds) noexcept { if (nanoseconds > 0) nanoseconds_ += nanoseconds; }
    /// Returns false and changes nothing when a MONOTONIC clock would go backward.
    bool set(std::int64_t nanoseconds) noexcept {
        if (domain_ == ClockDomain::MONOTONIC && nanoseconds < nanoseconds_) return false;
        nanoseconds_ = nanoseconds;
        return true;
    }
private:
    ClockDomain domain_;
    LifetimeProbe* probe_;
    std::int64_t nanoseconds_{0};
};

class FakeTimer final : public time::contract::ReferenceTimer {
public:
    FakeTimer(Controls& controls, LifetimeProbe* probe) : controls_(controls), probe_(probe) {}
    ~FakeTimer() override { if (probe_ != nullptr) ++probe_->services_destroyed; }

    Result<void> start(Duration period, time::TimerMode mode, Callback callback) override {
        if (const ErrorCode code = controls_.begin(Method::TIMER_START); code != ErrorCode::NONE) {
            controls_.record(Method::TIMER_START, code);
            return Result<void>::failure(injected(code));               // atomic: the timer stays as it was
        }
        const Result<void> result = ReferenceTimer::start(period, mode, callback.valid() ? Callback{&FakeTimer::fire, this} : Callback{});
        if (result) user_ = callback;                                  // a rejected start never disturbs a running activation
        controls_.record(Method::TIMER_START, result ? ErrorCode::NONE : result.error().code);
        return result;
    }
    Result<void> stop() override {
        if (const ErrorCode code = controls_.begin(Method::TIMER_STOP); code != ErrorCode::NONE) {
            controls_.record(Method::TIMER_STOP, code);
            return Result<void>::failure(injected(code));
        }
        const Result<void> result = ReferenceTimer::stop();
        controls_.record(Method::TIMER_STOP, result ? ErrorCode::NONE : result.error().code);
        return result;
    }

private:
    static void fire(void* raw) {
        auto* timer = static_cast<FakeTimer*>(raw);
        timer->controls_.record(Method::TIMER_CALLBACK, ErrorCode::NONE);
        timer->user_.function(timer->user_.context);
    }
    Controls& controls_;
    LifetimeProbe* probe_;
    Callback user_{};
};

class FakeWatchdog final : public contract::ReferenceWatchdog {
public:
    FakeWatchdog(Controls& controls, LifetimeProbe* probe) : controls_(controls), probe_(probe) {}
    ~FakeWatchdog() override { if (probe_ != nullptr) ++probe_->services_destroyed; }

    Result<void> start(Duration timeout) override { return guarded(Method::WATCHDOG_START, [&] { return ReferenceWatchdog::start(timeout); }); }
    Result<void> kick() override { return guarded(Method::WATCHDOG_KICK, [&] { return ReferenceWatchdog::kick(); }); }
    Result<void> stop() override { return guarded(Method::WATCHDOG_STOP, [&] { return ReferenceWatchdog::stop(); }); }

private:
    template<class F> Result<void> guarded(Method method, F&& real) {
        if (const ErrorCode code = controls_.begin(method); code != ErrorCode::NONE) {
            controls_.record(method, code);
            return Result<void>::failure(injected(code));                // atomic: state unchanged
        }
        const Result<void> result = real();
        controls_.record(method, result ? ErrorCode::NONE : result.error().code);
        return result;
    }
    Controls& controls_;
    LifetimeProbe* probe_;
};

/// The test-only reference platform: an IPlatformAdapter whose services are the fakes above.
class ReferencePlatform final : public IPlatformAdapter {
public:
    struct Provides { bool scheduler{true}, clock{true}, timer{true}, watchdog{true}; };
    struct Config {
        PlatformInfo info{"reference-platform", Version{1, 0, 0}};
        Provides provides{};
        CapabilitySet capabilities{};
        ClockDomain clock_domain{ClockDomain::MONOTONIC};
        LifetimeProbe* probe{nullptr};
    };

    ReferencePlatform() : ReferencePlatform(Config{}) {}
    explicit ReferencePlatform(Config config)
        : config_(std::move(config)), scheduler_(controls_, config_.probe), clock_(config_.clock_domain, config_.probe),
          timer_(controls_, config_.probe), watchdog_(controls_, config_.probe) {}
    ~ReferencePlatform() override { if (config_.probe != nullptr) config_.probe->platform_destroyed = true; }

    ReferencePlatform(const ReferencePlatform&) = delete;
    ReferencePlatform& operator=(const ReferencePlatform&) = delete;

    // --- IPlatformAdapter (each call is counted so a caller's silence is observable) ---
    [[nodiscard]] const PlatformInfo& info() const noexcept override { controls_.count_adapter_query(); return config_.info; }
    [[nodiscard]] IScheduler* scheduler() const noexcept override { controls_.count_adapter_query(); return config_.provides.scheduler ? &scheduler_ : nullptr; }
    [[nodiscard]] time::IClock* clock() const noexcept override { controls_.count_adapter_query(); return config_.provides.clock ? &clock_ : nullptr; }
    [[nodiscard]] time::ITimer* timer() const noexcept override { controls_.count_adapter_query(); return config_.provides.timer ? &timer_ : nullptr; }
    [[nodiscard]] IWatchdog* watchdog() const noexcept override { controls_.count_adapter_query(); return config_.provides.watchdog ? &watchdog_ : nullptr; }
    [[nodiscard]] CapabilitySet capabilities() const override { controls_.count_adapter_query(); return config_.capabilities; }

    // --- test controls (never reached through the contract) ---
    [[nodiscard]] Controls& controls() noexcept { return controls_; }
    [[nodiscard]] const Controls& controls() const noexcept { return controls_; }
    [[nodiscard]] FakeScheduler& fake_scheduler() noexcept { return scheduler_; }
    [[nodiscard]] FakeClock& fake_clock() noexcept { return clock_; }
    [[nodiscard]] FakeTimer& fake_timer() noexcept { return timer_; }
    [[nodiscard]] FakeWatchdog& fake_watchdog() noexcept { return watchdog_; }

    /// Explicit time advancement: ticks the scheduler once per millisecond and advances the timer and the clock.
    /// The watchdog is NOT advanced (expiry is a separate, explicit step: fake_watchdog().advance()).
    void let_time_pass(Duration d) {
        for (std::int64_t i = 0; i < d.nanoseconds() / 1'000'000; ++i) scheduler_.tick();
        timer_.advance(d.nanoseconds());
        clock_.advance(d.nanoseconds());
    }

    /// A conformance Environment wired to this platform.
    [[nodiscard]] conformance::Environment environment() {
        conformance::Environment env;
        env.let_time_pass = [this](Duration d) { let_time_pass(d); };
        return env;
    }

private:
    Config config_;
    mutable Controls controls_;
    mutable FakeScheduler scheduler_;
    mutable FakeClock clock_;
    mutable FakeTimer timer_;
    mutable FakeWatchdog watchdog_;
};

} // namespace kritva::core::platform::testing
