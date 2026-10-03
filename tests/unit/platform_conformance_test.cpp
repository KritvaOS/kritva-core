//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : platform_conformance_test.cpp
// Description : Runs the platform conformance suite against conforming adapters with
//               different adapter-defined behavior, and against deliberately faulty
//               ones that the suite must reject.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-PLAT-009
// API         : CORE-TEST-PLATFORM-CONFORMANCE-RUN
//
// Author      : KritvaOS Core Team
// Created     : 04-10-2026
//==============================================================================

#include <cassert>
#include <cstdint>
#include <string>
#include <utility>

#include <kritva/core/core.hpp>

#include "../contract/reference_adapter.hpp"
#include "../platform/adapter_conformance.hpp"

using namespace kritva::core;
using namespace kritva::core::platform;
using namespace kritva::core::platform::conformance;
using kritva::core::platform::contract::ReferenceAdapter;
using kritva::core::platform::contract::ReferenceClock;
using kritva::core::platform::contract::ReferenceScheduler;
using kritva::core::platform::contract::ReferenceWatchdog;
using kritva::core::time::contract::ReferenceTimer;
using kritva::core::time::TimerMode;

namespace {

constexpr std::int64_t MS = 1'000'000;

// ---- environments: how this test lets time pass for each reference service ----------

template<class S> Environment scheduler_env(S& scheduler) {
    Environment env;
    env.let_time_pass = [&scheduler](Duration d) {
        for (std::int64_t i = 0; i < d.nanoseconds() / MS; ++i) scheduler.tick();
    };
    return env;
}

template<class T> Environment timer_env(T& timer) {
    Environment env;
    env.let_time_pass = [&timer](Duration d) { timer.advance(d.nanoseconds()); };
    return env;
}

Environment idle_env() {
    Environment env;
    env.let_time_pass = [](Duration) {};
    return env;
}

Environment adapter_env(const ReferenceAdapter& adapter) {
    Environment env;
    env.let_time_pass = [&adapter](Duration d) {
        if (auto* scheduler = static_cast<ReferenceScheduler*>(adapter.scheduler())) {
            for (std::int64_t i = 0; i < d.nanoseconds() / MS; ++i) scheduler->tick();
        }
        if (auto* timer = static_cast<ReferenceTimer*>(adapter.timer())) timer->advance(d.nanoseconds());
    };
    return env;
}

std::string describe(const Report& report) {
    std::string text;
    for (const std::string& failure : report.failures()) text += failure + "\n";
    return text;
}

#define EXPECT_CONFORMS(report) do { if (!(report).ok()) { fputs(describe(report).c_str(), stderr); } assert((report).ok()); } while (0)
#define EXPECT_REJECTED(report) assert(!(report).ok())

// ---- conforming adapters: mandatory behavior holds under every adapter policy -------

void test_suite_is_not_vacuous() {
    ReferenceScheduler scheduler;
    Report report;
    check_scheduler(scheduler, scheduler_env(scheduler), report);
    EXPECT_CONFORMS(report);
    assert(report.checks() > 20);
}

void test_conforming_scheduler_policies() {
    for (const bool dynamic : {false, true}) {
        for (const std::size_t capacity : {std::size_t{2}, std::size_t{4}, std::size_t{64}}) {
            for (const bool affinity : {false, true}) {
                ReferenceScheduler scheduler;
                scheduler.policy.dynamic_creation = dynamic;
                scheduler.policy.capacity = capacity;
                scheduler.policy.affinity_supported = affinity;
                scheduler.policy.max_priority = affinity ? 255 : 10;
                Report report;
                check_scheduler(scheduler, scheduler_env(scheduler), report);
                EXPECT_CONFORMS(report);
                assert(!scheduler.running());                     // documented postcondition
            }
        }
    }
}

void test_conforming_timer_policies() {
    {
        ReferenceTimer timer;
        Report report;
        check_timer(timer, timer_env(timer), report);
        EXPECT_CONFORMS(report);
        assert(report.skips().empty());
    }
    {
        ReferenceTimer timer;
        timer.policy.periodic_supported = false;                  // adapter policy: no periodic timers
        Report report;
        check_timer(timer, timer_env(timer), report);
        EXPECT_CONFORMS(report);
        assert(report.skips().size() == 1);
    }
    {
        ReferenceTimer timer;
        timer.policy.resolution_ns = 20 * MS;                     // coarse resolution, still below the test period
        Report report;
        check_timer(timer, timer_env(timer), report);
        EXPECT_CONFORMS(report);
    }
    {
        ReferenceTimer timer;
        timer.policy.resource_available = false;                  // no timer resource: a skip, not a failure
        Report report;
        check_timer(timer, timer_env(timer), report);
        EXPECT_CONFORMS(report);
        assert(!report.skips().empty());
    }
    {
        ReferenceTimer timer;
        timer.policy.resolution_ns = 100 * MS;                    // the configured period is below resolution: UNSUPPORTED
        Report report;
        check_timer(timer, timer_env(timer), report);
        EXPECT_CONFORMS(report);
        assert(!report.skips().empty());
    }
}

void test_conforming_watchdog_policies() {
    {
        ReferenceWatchdog watchdog;
        Report report;
        check_watchdog(watchdog, idle_env(), report);
        EXPECT_CONFORMS(report);
        assert(!watchdog.running() && !watchdog.expired());       // the suite never lets a watchdog expire
    }
    {
        ReferenceWatchdog watchdog;
        watchdog.policy.can_stop = false;                         // hardware-style: cannot be stopped
        Report report;
        check_watchdog(watchdog, idle_env(), report);
        EXPECT_CONFORMS(report);
        assert(watchdog.running() && !report.skips().empty());
    }
    {
        ReferenceWatchdog watchdog;
        watchdog.policy.max_timeout_ns = 10 * MS;                 // the configured timeout is out of range: UNSUPPORTED
        Report report;
        check_watchdog(watchdog, idle_env(), report);
        EXPECT_CONFORMS(report);
        assert(!report.skips().empty());
    }
    {
        ReferenceWatchdog watchdog;
        watchdog.policy.resource_available = false;
        Report report;
        check_watchdog(watchdog, idle_env(), report);
        EXPECT_CONFORMS(report);
    }
}

void test_conforming_clock() {
    ReferenceClock clock;
    Report report;
    check_clock(clock, idle_env(), report);
    EXPECT_CONFORMS(report);
    assert(report.checks() > 20);
}

void test_conforming_adapters_with_every_service_combination() {
    for (int mask = 0; mask < 16; ++mask) {
        CapabilitySet capabilities;
        capabilities.add(Capability{CapabilityId{100}, "gpio", Version{1, 0, 0}});
        const ReferenceAdapter adapter(PlatformInfo{"reference", Version{1, 0, 0}},
                                       {(mask & 1) != 0, (mask & 2) != 0, (mask & 4) != 0, (mask & 8) != 0}, capabilities);
        Report report;
        check_platform_adapter(adapter, adapter_env(adapter), report);
        EXPECT_CONFORMS(report);
    }
}

// ---- faulty doubles: the suite must reject every one of these -----------------------

template<class R> Result<R> err(ErrorCode code, const char* message) { return Result<R>::failure(make_error(code, message)); }
void noop(void*) {}

// Timer faults
struct ZeroPeriodAccepted : ReferenceTimer {
    Result<void> start(Duration p, TimerMode m, Callback c) override { return ReferenceTimer::start(p.nanoseconds() <= 0 ? Duration::from_milliseconds(50) : p, m, c); }
};
struct NullCallbackAccepted : ReferenceTimer {
    Result<void> start(Duration p, TimerMode m, Callback c) override { return ReferenceTimer::start(p, m, c.valid() ? c : Callback{&noop, nullptr}); }
};
struct StartWhileRunningSucceeds : ReferenceTimer {
    Result<void> start(Duration p, TimerMode m, Callback c) override { return running() ? Result<void>::success() : ReferenceTimer::start(p, m, c); }
};
struct StopFailsWhenStopped : ReferenceTimer {
    Result<void> stop() override { return running() ? ReferenceTimer::stop() : err<void>(ErrorCode::INVALID_STATE, "not running"); }
};
struct OneShotFiresAgain : ReferenceTimer {       // re-arms itself after firing
    Result<void> start(Duration p, TimerMode m, Callback c) override { period_ = p; mode_ = m; cb_ = c; return ReferenceTimer::start(p, m, c); }
    void advance(std::int64_t ns) { const auto before = fired(); ReferenceTimer::advance(ns); if (fired() > before && !running() && mode_ == TimerMode::ONE_SHOT) ReferenceTimer::start(period_, mode_, cb_); }
    Duration period_{}; TimerMode mode_{TimerMode::ONE_SHOT}; Callback cb_{};
};
struct StopDoesNotStop : ReferenceTimer {         // reports success but keeps firing
    Result<void> stop() override { return Result<void>::success(); }
};
struct StopFromCallbackAllowed : ReferenceTimer {
    Result<void> stop() override { const auto r = ReferenceTimer::stop(); return (!r && r.error().code == ErrorCode::INVALID_STATE && running()) ? Result<void>::success() : r; }
};
struct WrongCodeForBadPeriod : ReferenceTimer {
    Result<void> start(Duration p, TimerMode m, Callback c) override { const auto r = ReferenceTimer::start(p, m, c); return (!r && r.error().code == ErrorCode::INVALID_ARGUMENT) ? err<void>(ErrorCode::UNSUPPORTED, "x") : r; }
};

struct OneShotInternalError : ReferenceTimer {     // a failure code the contract does not permit
    Result<void> start(Duration p, TimerMode m, Callback c) override { const auto r = ReferenceTimer::start(p, m, c); return (r && m == TimerMode::ONE_SHOT) ? err<void>(ErrorCode::INTERNAL_ERROR, "x") : r; }
};
struct PeriodicInternalError : ReferenceTimer {
    Result<void> start(Duration p, TimerMode m, Callback c) override { const auto r = ReferenceTimer::start(p, m, c); return (r && m == TimerMode::PERIODIC) ? err<void>(ErrorCode::INTERNAL_ERROR, "x") : r; }
};
struct NegativePeriodAccepted_Timer : ReferenceTimer {
    Result<void> start(Duration p, TimerMode m, Callback c) override { return ReferenceTimer::start(p.nanoseconds() < 0 ? Duration::from_milliseconds(50) : p, m, c); }
};
struct RejectedStartActivates : ReferenceTimer {   // a rejected request leaves a timer running
    Result<void> start(Duration p, TimerMode m, Callback c) override {
        const auto r = ReferenceTimer::start(p, m, c);
        if (!r && r.error().code == ErrorCode::INVALID_ARGUMENT && p.nanoseconds() > 0) ReferenceTimer::start(p, TimerMode::PERIODIC, Callback{&noop, nullptr});
        return r;
    }
};
struct OneShotNotStoppedAfterFiring : ReferenceTimer { // never becomes restartable: start after firing is INVALID_STATE
    Result<void> start(Duration p, TimerMode m, Callback c) override { if (used_) return err<void>(ErrorCode::INVALID_STATE, "x"); const auto r = ReferenceTimer::start(p, m, c); if (r && m == TimerMode::ONE_SHOT) used_ = true; return r; }
    bool used_{false};
};

template<class T> void expect_timer_rejected() {
    T timer;
    Report report;
    check_timer(timer, timer_env(timer), report);
    EXPECT_REJECTED(report);
}

void test_faulty_timers_are_rejected() {
    expect_timer_rejected<ZeroPeriodAccepted>();
    expect_timer_rejected<NullCallbackAccepted>();
    expect_timer_rejected<StartWhileRunningSucceeds>();
    expect_timer_rejected<StopFailsWhenStopped>();
    expect_timer_rejected<OneShotFiresAgain>();
    expect_timer_rejected<StopDoesNotStop>();
    expect_timer_rejected<StopFromCallbackAllowed>();
    expect_timer_rejected<WrongCodeForBadPeriod>();
    expect_timer_rejected<OneShotInternalError>();
    expect_timer_rejected<PeriodicInternalError>();
    expect_timer_rejected<NegativePeriodAccepted_Timer>();
    expect_timer_rejected<RejectedStartActivates>();
    expect_timer_rejected<OneShotNotStoppedAfterFiring>();
}

// Watchdog faults
struct ZeroTimeoutAccepted : ReferenceWatchdog {
    Result<void> start(Duration t) override { return ReferenceWatchdog::start(t.nanoseconds() <= 0 ? Duration::from_milliseconds(1000) : t); }
};
struct KickWhenStoppedSucceeds : ReferenceWatchdog {
    Result<void> kick() override { return running() ? ReferenceWatchdog::kick() : Result<void>::success(); }
};
struct KickStartsWatchdog : ReferenceWatchdog {
    Result<void> kick() override { return running() ? ReferenceWatchdog::kick() : ReferenceWatchdog::start(Duration::from_milliseconds(1000)); }
};
struct WatchdogStartWhileRunningSucceeds : ReferenceWatchdog {
    Result<void> start(Duration t) override { return running() ? Result<void>::success() : ReferenceWatchdog::start(t); }
};
struct WatchdogStopFailsWhenStopped : ReferenceWatchdog {
    Result<void> stop() override { return running() ? ReferenceWatchdog::stop() : err<void>(ErrorCode::INVALID_STATE, "not running"); }
};
struct WatchdogStopDoesNotStop : ReferenceWatchdog {
    Result<void> stop() override { return Result<void>::success(); }
};
struct WatchdogWrongStopCode : ReferenceWatchdog {    // cannot stop, but reports the wrong code
    WatchdogWrongStopCode() { policy.can_stop = false; }
    Result<void> stop() override { const auto r = ReferenceWatchdog::stop(); return r ? r : err<void>(ErrorCode::INVALID_ARGUMENT, "x"); }
};
struct WatchdogNegativeTimeoutAccepted : ReferenceWatchdog {
    Result<void> start(Duration t) override { return ReferenceWatchdog::start(t.nanoseconds() < 0 ? Duration::from_milliseconds(1000) : t); }
};

struct WatchdogStartInternalError : ReferenceWatchdog {
    Result<void> start(Duration t) override { const auto r = ReferenceWatchdog::start(t); return r ? err<void>(ErrorCode::INTERNAL_ERROR, "x") : r; }
};
struct WatchdogRejectedStartRuns : ReferenceWatchdog {   // a rejected start leaves the watchdog running
    Result<void> start(Duration t) override { const auto r = ReferenceWatchdog::start(t); if (!r && r.error().code == ErrorCode::INVALID_ARGUMENT) ReferenceWatchdog::start(Duration::from_milliseconds(1000)); return r; }
};
struct WatchdogKickFails : ReferenceWatchdog {            // a running watchdog cannot be kicked
    Result<void> kick() override { return err<void>(ErrorCode::INVALID_STATE, "x"); }
};
struct WatchdogNoRestartAfterStop : ReferenceWatchdog {
    Result<void> start(Duration t) override { if (stops_ > 0) return err<void>(ErrorCode::INVALID_STATE, "x"); return ReferenceWatchdog::start(t); }
    Result<void> stop() override { const auto r = ReferenceWatchdog::stop(); if (r) ++stops_; return r; }
    int stops_{0};
};
struct WatchdogUnstoppableButSuccess : ReferenceWatchdog { // cannot be stopped but stop() says success
    WatchdogUnstoppableButSuccess() { policy.can_stop = false; }
    Result<void> stop() override { (void)ReferenceWatchdog::stop(); return Result<void>::success(); }
};
template<class W> void expect_watchdog_rejected() {
    W watchdog;
    Report report;
    check_watchdog(watchdog, idle_env(), report);
    EXPECT_REJECTED(report);
}

void test_faulty_watchdogs_are_rejected() {
    expect_watchdog_rejected<ZeroTimeoutAccepted>();
    expect_watchdog_rejected<KickWhenStoppedSucceeds>();
    expect_watchdog_rejected<KickStartsWatchdog>();
    expect_watchdog_rejected<WatchdogStartWhileRunningSucceeds>();
    expect_watchdog_rejected<WatchdogStopFailsWhenStopped>();
    expect_watchdog_rejected<WatchdogStopDoesNotStop>();
    expect_watchdog_rejected<WatchdogWrongStopCode>();
    expect_watchdog_rejected<WatchdogNegativeTimeoutAccepted>();
    expect_watchdog_rejected<WatchdogStartInternalError>();
    expect_watchdog_rejected<WatchdogRejectedStartRuns>();
    expect_watchdog_rejected<WatchdogKickFails>();
    expect_watchdog_rejected<WatchdogNoRestartAfterStop>();
    expect_watchdog_rejected<WatchdogUnstoppableButSuccess>();
}

// Scheduler faults
struct IdZero : ReferenceScheduler {
    Result<TaskId> create_task(const TaskConfig& c, void (*e)(void*), void* x) override { const auto r = ReferenceScheduler::create_task(c, e, x); return r ? Result<TaskId>::success(0) : r; }
};
struct DuplicateIds : ReferenceScheduler {
    Result<TaskId> create_task(const TaskConfig& c, void (*e)(void*), void* x) override { const auto r = ReferenceScheduler::create_task(c, e, x); if (!r) return r; if (first_ == 0) first_ = r.value(); return Result<TaskId>::success(first_); }
    TaskId first_{0};
};
struct NullEntryAccepted : ReferenceScheduler {
    Result<TaskId> create_task(const TaskConfig& c, void (*e)(void*), void* x) override { return ReferenceScheduler::create_task(c, e == nullptr ? &noop : e, x); }
};
struct NegativePeriodAccepted : ReferenceScheduler {
    Result<TaskId> create_task(const TaskConfig& c, void (*e)(void*), void* x) override { TaskConfig fixed = c; if (fixed.period.nanoseconds() < 0) fixed.period = Duration{}; return ReferenceScheduler::create_task(fixed, e, x); }
};
struct NullNameAccepted : ReferenceScheduler {
    Result<TaskId> create_task(const TaskConfig& c, void (*e)(void*), void* x) override { TaskConfig fixed = c; if (fixed.name == nullptr) fixed.name = "x"; return ReferenceScheduler::create_task(fixed, e, x); }
};
struct StartNotIdempotent : ReferenceScheduler {
    Result<void> start() override { if (running()) return err<void>(ErrorCode::INVALID_STATE, "already running"); return ReferenceScheduler::start(); }
};
struct SchedulerStopNotIdempotent : ReferenceScheduler {
    Result<void> stop() override { if (!running()) return err<void>(ErrorCode::INVALID_STATE, "not running"); return ReferenceScheduler::stop(); }
};
struct EntriesAfterStop : ReferenceScheduler {    // stop() reports success but tasks keep running
    Result<void> stop() override { return Result<void>::success(); }
};
struct EntriesBeforeStart : ReferenceScheduler {  // periodic tasks run even while the scheduler is stopped
    Result<TaskId> create_task(const TaskConfig& c, void (*e)(void*), void* x) override { const auto r = ReferenceScheduler::create_task(c, e, x); if (r && c.period.nanoseconds() > 0) { entry_ = e; context_ = x; } return r; }
    void tick() { ReferenceScheduler::tick(); if (!running() && entry_ != nullptr) entry_(context_); }
    void (*entry_)(void*){nullptr}; void* context_{nullptr};
};
struct WrongExhaustionCode : ReferenceScheduler {
    Result<TaskId> create_task(const TaskConfig& c, void (*e)(void*), void* x) override { const auto r = ReferenceScheduler::create_task(c, e, x); return (!r && r.error().code == ErrorCode::RESOURCE_UNAVAILABLE) ? err<TaskId>(ErrorCode::INVALID_ARGUMENT, "x") : r; }
};

// Faults injected at the Nth successful registration of the conformance sequence (1 aperiodic, 2 periodic,
// 3 priority probe, 4 affinity probe, 5 create while running, 6 create after stop).
enum class Fault { ID_ZERO, ID_DUPLICATE, RESOURCE, INTERNAL };
template<int N, Fault F> struct FaultAt : ReferenceScheduler {
    FaultAt() { policy.dynamic_creation = true; policy.capacity = 64; }   // so that registration 5 (while running) can succeed
    Result<TaskId> create_task(const TaskConfig& c, void (*e)(void*), void* x) override {
        const auto r = ReferenceScheduler::create_task(c, e, x);
        if (!r) return r;
        if (++successes_ == 1) first_ = r.value();
        if (successes_ != N) return r;
        switch (F) {
            case Fault::ID_ZERO: return Result<TaskId>::success(0);
            case Fault::ID_DUPLICATE: return Result<TaskId>::success(first_);
            case Fault::RESOURCE: return err<TaskId>(ErrorCode::RESOURCE_UNAVAILABLE, "x");
            case Fault::INTERNAL: return err<TaskId>(ErrorCode::INTERNAL_ERROR, "x");
        }
        return r;
    }
    int successes_{0}; TaskId first_{0};
};
struct EntriesOnlyBeforeFirstStart : ReferenceScheduler {    // runs a periodic entry while stopped, but only before the first start()
    Result<TaskId> create_task(const TaskConfig& c, void (*e)(void*), void* x) override { const auto r = ReferenceScheduler::create_task(c, e, x); if (r && c.period.nanoseconds() > 0) { entry_ = e; context_ = x; } return r; }
    Result<void> start() override { started_ = true; return ReferenceScheduler::start(); }
    void tick() { ReferenceScheduler::tick(); if (!started_ && entry_ != nullptr) entry_(context_); }
    void (*entry_)(void*){nullptr}; void* context_{nullptr}; bool started_{false};
};
struct RestartRefused : ReferenceScheduler {                 // cannot be started a second time
    Result<void> start() override { if (starts_++ > 0) return err<void>(ErrorCode::INVALID_STATE, "x"); return ReferenceScheduler::start(); }
    int starts_{0};
};

template<class S> void expect_scheduler_rejected() {
    S scheduler;
    Report report;
    check_scheduler(scheduler, scheduler_env(scheduler), report);
    EXPECT_REJECTED(report);
}

void test_faulty_schedulers_are_rejected() {
    expect_scheduler_rejected<IdZero>();
    expect_scheduler_rejected<DuplicateIds>();
    expect_scheduler_rejected<NullEntryAccepted>();
    expect_scheduler_rejected<NegativePeriodAccepted>();
    expect_scheduler_rejected<NullNameAccepted>();
    expect_scheduler_rejected<StartNotIdempotent>();
    expect_scheduler_rejected<SchedulerStopNotIdempotent>();
    expect_scheduler_rejected<EntriesAfterStop>();
    expect_scheduler_rejected<EntriesBeforeStart>();
    expect_scheduler_rejected<WrongExhaustionCode>();
    expect_scheduler_rejected<EntriesOnlyBeforeFirstStart>();
    expect_scheduler_rejected<RestartRefused>();
    expect_scheduler_rejected<FaultAt<1, Fault::RESOURCE>>();
    expect_scheduler_rejected<FaultAt<2, Fault::RESOURCE>>();
    expect_scheduler_rejected<FaultAt<1, Fault::ID_ZERO>>();
    expect_scheduler_rejected<FaultAt<2, Fault::ID_ZERO>>();
    expect_scheduler_rejected<FaultAt<2, Fault::ID_DUPLICATE>>();
    expect_scheduler_rejected<FaultAt<3, Fault::INTERNAL>>();
    expect_scheduler_rejected<FaultAt<4, Fault::INTERNAL>>();
    expect_scheduler_rejected<FaultAt<5, Fault::INTERNAL>>();
    expect_scheduler_rejected<FaultAt<3, Fault::ID_ZERO>>();
    expect_scheduler_rejected<FaultAt<4, Fault::ID_ZERO>>();
    expect_scheduler_rejected<FaultAt<5, Fault::ID_ZERO>>();
    expect_scheduler_rejected<FaultAt<3, Fault::ID_DUPLICATE>>();
    expect_scheduler_rejected<FaultAt<4, Fault::ID_DUPLICATE>>();
    expect_scheduler_rejected<FaultAt<5, Fault::ID_DUPLICATE>>();
    expect_scheduler_rejected<FaultAt<6, Fault::INTERNAL>>();
    expect_scheduler_rejected<FaultAt<6, Fault::ID_ZERO>>();
    expect_scheduler_rejected<FaultAt<6, Fault::ID_DUPLICATE>>();
}

// Clock faults
struct DomainFlips final : time::IClock {
    [[nodiscard]] Timestamp now() const noexcept override { flip_ = !flip_; return Timestamp{1, flip_ ? ClockDomain::MONOTONIC : ClockDomain::REALTIME}; }
    mutable bool flip_{false};
};
struct MonotonicDecreases final : time::IClock {
    [[nodiscard]] Timestamp now() const noexcept override { return Timestamp{value_--, ClockDomain::MONOTONIC}; }
    mutable std::int64_t value_{1000};
};

void test_faulty_clocks_are_rejected() {
    DomainFlips flips;
    Report r1;
    check_clock(flips, idle_env(), r1);
    EXPECT_REJECTED(r1);
    MonotonicDecreases decreasing;
    Report r2;
    check_clock(decreasing, idle_env(), r2);
    EXPECT_REJECTED(r2);
    // A REALTIME clock may step backward: that is not a violation.
    struct Realtime final : time::IClock {
        [[nodiscard]] Timestamp now() const noexcept override { return Timestamp{value_--, ClockDomain::REALTIME}; }
        mutable std::int64_t value_{1000};
    } realtime;
    Report r3;
    check_clock(realtime, idle_env(), r3);
    EXPECT_CONFORMS(r3);
}

// Adapter faults
struct EmptyName : ReferenceAdapter {
    EmptyName() : ReferenceAdapter(PlatformInfo{"", Version{1, 0, 0}}, {}) {}
};
struct UnstableInfo : ReferenceAdapter {
    UnstableInfo() : ReferenceAdapter(PlatformInfo{"unstable", Version{1, 0, 0}}, {}) {}
    [[nodiscard]] const PlatformInfo& info() const noexcept override { scratch_ = ReferenceAdapter::info(); scratch_.name += std::to_string(++calls_); return scratch_; }
    mutable PlatformInfo scratch_; mutable int calls_{0};
};
struct UnstableServiceObject : ReferenceAdapter {
    UnstableServiceObject() : ReferenceAdapter(PlatformInfo{"unstable", Version{1, 0, 0}}, {}) {}
    [[nodiscard]] time::IClock* clock() const noexcept override { flip_ = !flip_; return flip_ ? static_cast<time::IClock*>(&a_) : &b_; }
    mutable bool flip_{false}; mutable ReferenceClock a_, b_;
};
struct NondeterministicCapabilities : ReferenceAdapter {
    NondeterministicCapabilities() : ReferenceAdapter(PlatformInfo{"unstable", Version{1, 0, 0}}, {}) {}
    [[nodiscard]] CapabilitySet capabilities() const override { CapabilitySet set; set.add(Capability{CapabilityId{100}, "gpio", Version{1, 0, 0}}); if (++calls_ % 2 == 0) set.add(Capability{CapabilityId{101}, "extra", Version{}}); return set; }
    mutable int calls_{0};
};
struct ReorderedCapabilities : ReferenceAdapter {
    ReorderedCapabilities() : ReferenceAdapter(PlatformInfo{"unstable", Version{1, 0, 0}}, {}) {}
    [[nodiscard]] CapabilitySet capabilities() const override {
        CapabilitySet set;
        if (++calls_ % 2 == 0) { set.add(Capability{CapabilityId{100}, "a", Version{}}); set.add(Capability{CapabilityId{101}, "b", Version{}}); }
        else { set.add(Capability{CapabilityId{101}, "b", Version{}}); set.add(Capability{CapabilityId{100}, "a", Version{}}); }
        return set;
    }
    mutable int calls_{0};
};
struct ChangedVersion : ReferenceAdapter {
    ChangedVersion() : ReferenceAdapter(PlatformInfo{"unstable", Version{1, 0, 0}}, {}) {}
    [[nodiscard]] CapabilitySet capabilities() const override { CapabilitySet set; set.add(Capability{CapabilityId{100}, "gpio", Version{1, static_cast<std::uint32_t>(++calls_), 0}}); return set; }
    mutable int calls_{0};
};

struct InfoAlternatesObjects : ReferenceAdapter {     // equal values, but not the same object on every call
    InfoAlternatesObjects() : ReferenceAdapter(PlatformInfo{"alt", Version{1, 0, 0}}, {}), a_{"alt", Version{1, 0, 0}}, b_{"alt", Version{1, 0, 0}} {}
    [[nodiscard]] const PlatformInfo& info() const noexcept override { flip_ = !flip_; return flip_ ? a_ : b_; }
    PlatformInfo a_, b_; mutable bool flip_{false};
};
template<int Which> struct UnstableAccessor : ReferenceAdapter {   // 0 scheduler, 1 timer, 2 watchdog: a different object every other call
    UnstableAccessor() : ReferenceAdapter(PlatformInfo{"unstable", Version{1, 0, 0}}, {}) {}
    [[nodiscard]] IScheduler* scheduler() const noexcept override { return Which == 0 ? pick(&s_a_, &s_b_) : ReferenceAdapter::scheduler(); }
    [[nodiscard]] time::ITimer* timer() const noexcept override { return Which == 1 ? pick(&t_a_, &t_b_) : ReferenceAdapter::timer(); }
    [[nodiscard]] IWatchdog* watchdog() const noexcept override { return Which == 2 ? pick(&w_a_, &w_b_) : ReferenceAdapter::watchdog(); }
    template<class T> T* pick(T* a, T* b) const noexcept { flip_ = !flip_; return flip_ ? a : b; }
    mutable bool flip_{false};
    mutable ReferenceScheduler s_a_, s_b_; mutable ReferenceTimer t_a_, t_b_; mutable ReferenceWatchdog w_a_, w_b_;
};
struct CapabilityNameChanges : ReferenceAdapter {
    CapabilityNameChanges() : ReferenceAdapter(PlatformInfo{"unstable", Version{1, 0, 0}}, {}) {}
    [[nodiscard]] CapabilitySet capabilities() const override { CapabilitySet set; set.add(Capability{CapabilityId{100}, "gpio" + std::to_string(++calls_), Version{1, 0, 0}}); return set; }
    mutable int calls_{0};
};
struct CapabilityIdChanges : ReferenceAdapter {
    CapabilityIdChanges() : ReferenceAdapter(PlatformInfo{"unstable", Version{1, 0, 0}}, {}) {}
    [[nodiscard]] CapabilitySet capabilities() const override { CapabilitySet set; set.add(Capability{CapabilityId{static_cast<std::uint64_t>(100 + ++calls_)}, "gpio", Version{1, 0, 0}}); return set; }
    mutable int calls_{0};
};

template<class A> void expect_adapter_rejected() {
    const A adapter;
    Report report;
    check_adapter_description(adapter, report);
    EXPECT_REJECTED(report);
}

void test_faulty_adapters_are_rejected() {
    expect_adapter_rejected<EmptyName>();
    expect_adapter_rejected<UnstableInfo>();
    expect_adapter_rejected<UnstableServiceObject>();
    expect_adapter_rejected<NondeterministicCapabilities>();
    expect_adapter_rejected<ReorderedCapabilities>();
    expect_adapter_rejected<ChangedVersion>();
    expect_adapter_rejected<InfoAlternatesObjects>();
    expect_adapter_rejected<UnstableAccessor<0>>();
    expect_adapter_rejected<UnstableAccessor<1>>();
    expect_adapter_rejected<UnstableAccessor<2>>();
    expect_adapter_rejected<CapabilityNameChanges>();
    expect_adapter_rejected<CapabilityIdChanges>();
}

void test_adapter_runs_service_checks_and_reports_service_faults() {
    // An adapter whose watchdog is faulty is rejected by the full adapter check even though its description is fine.
    struct BadWatchdogAdapter : ReferenceAdapter {
        BadWatchdogAdapter() : ReferenceAdapter(PlatformInfo{"bad", Version{1, 0, 0}}, {false, false, false, false}) {}
        [[nodiscard]] IWatchdog* watchdog() const noexcept override { return &dog_; }
        mutable KickWhenStoppedSucceeds dog_;
    };
    const BadWatchdogAdapter adapter;
    Report description;
    check_adapter_description(adapter, description);
    EXPECT_CONFORMS(description);
    Report full;
    check_platform_adapter(adapter, idle_env(), full);
    EXPECT_REJECTED(full);
}

} // namespace

int main() {
    test_suite_is_not_vacuous();
    test_conforming_scheduler_policies();
    test_conforming_timer_policies();
    test_conforming_watchdog_policies();
    test_conforming_clock();
    test_conforming_adapters_with_every_service_combination();
    test_faulty_timers_are_rejected();
    test_faulty_watchdogs_are_rejected();
    test_faulty_schedulers_are_rejected();
    test_faulty_clocks_are_rejected();
    test_faulty_adapters_are_rejected();
    test_adapter_runs_service_checks_and_reports_service_faults();
    return 0;
}
