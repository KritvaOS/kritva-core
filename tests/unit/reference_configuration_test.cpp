//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : reference_configuration_test.cpp
// Description : Tests of the test-only configuration harness and its reusable conformance checks.
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-CFG-012
// API         : CORE-TEST-CONFIGURATION-HARNESS
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <kritva/core/core.hpp>

#include "../runtime/configuration_conformance.hpp"

using namespace kritva::core;
using namespace kritva::core::runtime;
using namespace kritva::core::runtime::contract;
namespace conf = kritva::core::runtime::conformance;

namespace {

ComponentInfo make_info(std::uint64_t id) {
    auto info = ComponentInfo::create(ComponentId{id}, "cfg");
    assert(info);
    return std::move(info).value();
}

class ReferenceConfigurationFixture final : public conf::ConfigurationFixture {
public:
    explicit ReferenceConfigurationFixture(Defect defect) : component_(make_info(1), defect) {}
    Component& component() override { return component_; }
    std::string applied() const override { return component_.applied(); }
    Configuration valid(int n) const override { return reference_valid(n); }
    Configuration invalid() const override { return reference_semantically_invalid(); }
    void clobber(Configuration& c) const override {
        (void)c.set(Parameter{"rate", std::int64_t{7}, ""});
        (void)c.set(Parameter{"name", std::string("clobbered"), ""});
    }
    int evaluations() const override { return component_.evaluations; }
    bool drive_to_fault() override {
        component_.fail_next_initialize = ErrorCode::INTERNAL_ERROR;
        (void)component_.initialize();
        return component_.lifecycle_state() == LifecycleState::FAULT;
    }
    ReferenceConfigurableComponent component_;
};

conf::FixtureFactory factory(Defect defect) {
    return [defect] { return std::unique_ptr<conf::ConfigurationFixture>(new ReferenceConfigurationFixture(defect)); };
}

void test_the_reference_component_conforms() {
    assert(conf::check_configuration_contract(factory(Defect::NONE)).empty());
}

struct DefectCase { Defect defect; const char* expected; };

// Every defect is detected, and by the clause written for it: removing that clause from the check would change the message.
const DefectCase kDefects[] = {
    {Defect::ACCEPTS_ANY_STATE, "accepted in READY"},
    {Defect::REJECTS_IN_UNKNOWN, "rejected in UNKNOWN"},
    {Defect::REJECTS_IN_STOPPED, "rejected in STOPPED"},
    {Defect::WRONG_CODE_IN_INVALID_STATE, "did not fail with INVALID_STATE"},
    {Defect::WRONG_SOURCE_IN_INVALID_STATE, "invalid-state error is not attributed"},
    {Defect::INVALID_STATE_HAS_EFFECT, "invalid-state configure() changed the applied"},
    {Defect::EVALUATES_IN_INVALID_STATE, "went on to evaluate"},
    {Defect::REFUSAL_CHANGES_LIFECYCLE, "invalid-state configure() changed the lifecycle"},
    {Defect::SUCCESS_CHANGES_LIFECYCLE_FROM_UNKNOWN, "lifecycle state in UNKNOWN"},
    {Defect::SUCCESS_CHANGES_LIFECYCLE_FROM_STOPPED, "lifecycle state in STOPPED"},
    {Defect::FIRST_FAILURE_CHANGES_LIFECYCLE, "first rejected configuration changed the lifecycle"},
    {Defect::LATER_FAILURE_CHANGES_LIFECYCLE, "a rejected configuration changed the lifecycle"},
    {Defect::RETAINS_REFERENCE, "reads through the caller"},
    {Defect::RETAINS_PARAMETER_POINTER, "reads through the caller"},
    {Defect::PARTIAL_APPLY, "partially replaced"},
    {Defect::COMMIT_BEFORE_VALIDATE, "partially replaced"},
    {Defect::FIRST_FAILURE_HALF_STATE, "left a partially applied state"},
    {Defect::NEVER_REPLACES, "did not replace the applied state"},
    {Defect::ACCEPTS_BUT_NEVER_APPLIES, "not reflected in the applied state"},
    {Defect::POISONED_AFTER_REJECTION, "after a rejection did not apply"},
    {Defect::RETRIES_ON_FAILURE, "retry"},
    {Defect::WRONG_SOURCE_ON_REJECTION, "rejection is not attributed"},
};

void test_every_defect_is_detected_by_the_clause_written_for_it() {
    for (const DefectCase& d : kDefects) {
        const std::string violation = conf::check_configuration_contract(factory(d.defect));
        assert(!violation.empty());
        assert(violation.find(d.expected) != std::string::npos);
    }
}

void test_the_reference_component_schema_and_staging() {
    ReferenceConfigurableComponent c(make_info(1));
    assert(c.applied().empty());
    assert(c.configure(reference_valid(0)) && c.applied() == "name=alpha;rate=10;");          // canonical, order independent
    Configuration reversed;
    assert(reversed.set(Parameter{"name", std::string("beta"), ""}) && reversed.set(Parameter{"rate", std::int64_t{20}, ""}));
    assert(c.configure(reversed) && c.applied() == "name=beta;rate=20;");
    const std::string before = c.applied();
    Configuration missing_rate;  assert(missing_rate.set(Parameter{"name", std::string("x"), ""}));
    Configuration wrong_type;    assert(wrong_type.set(Parameter{"rate", std::string("fast"), ""}));
    Configuration too_low;       assert(too_low.set(Parameter{"rate", std::int64_t{0}, ""}));
    Configuration too_high;      assert(too_high.set(Parameter{"rate", std::int64_t{1001}, ""}));
    Configuration bad_name;      assert(bad_name.set(Parameter{"rate", std::int64_t{5}, ""}) && bad_name.set(Parameter{"name", std::int64_t{3}, ""}));
    for (const Configuration* bad : {&missing_rate, &wrong_type, &too_low, &too_high, &bad_name}) {
        const auto r = c.configure(*bad);
        assert(!r && r.error().code == ErrorCode::CONFIGURATION_ERROR && r.error().source == ComponentId{1});
        assert(c.applied() == before);
    }
    Configuration edge;          assert(edge.set(Parameter{"rate", std::int64_t{1000}, ""}));
    assert(c.configure(edge) && c.applied() == "rate=1000;");                                    // a configuration replaces the state as a whole
}

void test_injected_failures_strike_after_the_state_check_and_leave_the_state_untouched() {
    for (const FailAt where : {FailAt::STRUCTURAL, FailAt::SEMANTIC, FailAt::BEFORE_COMMIT}) {
        ReferenceConfigurableComponent c(make_info(2));
        assert(c.configure(reference_valid(0)));
        const std::string before = c.applied();
        const int accepted = c.accepted_count;
        c.fail_at = where;
        c.fail_code = ErrorCode::RESOURCE_UNAVAILABLE;
        const auto r = c.configure(reference_valid(1));
        assert(!r && r.error().code == ErrorCode::RESOURCE_UNAVAILABLE && r.error().source == ComponentId{2});
        assert(c.applied() == before && c.accepted_count == accepted && c.fail_at == FailAt::NONE);   // consumed once
        assert(c.configure(reference_valid(1)) && c.applied() != before);                              // and the next attempt works
    }
    ReferenceConfigurableComponent c(make_info(3));
    assert(c.configure(reference_valid(0)) && c.initialize());
    c.fail_at = FailAt::SEMANTIC;
    const auto r = c.configure(reference_valid(1));                                                    // READY: refused before the injection
    assert(!r && r.error().code == ErrorCode::INVALID_STATE && c.fail_at == FailAt::SEMANTIC);          // not consumed: nothing past the state check ran
}

void test_counters_and_the_recorded_address() {
    ConfigurationCallLog log;
    ReferenceConfigurableComponent c(make_info(4), Defect::NONE, &log);
    const Configuration a = reference_valid(0);
    assert(c.configure(a) && c.calls == 1 && c.evaluations == 1 && c.accepted_count == 1 && c.last_object == &a);
    assert(c.initialize());
    assert(!c.configure(a) && c.calls == 2 && c.evaluations == 1 && c.accepted_count == 1);          // refused: no evaluation
    assert(c.stop() && c.shutdown());
    assert(!c.configure(reference_semantically_invalid()) && c.evaluations == 2 && c.accepted_count == 1);
    assert(log.size() == 3 && log[0].object == &a && log[0].accepted && !log[1].accepted && log[1].code == ErrorCode::INVALID_STATE);
    assert(log[2].code == ErrorCode::CONFIGURATION_ERROR && log[2].component == 4);
}

// ---- Runtime forwarding: the harness detects a broken driver ------------------------------------------
struct World {
    ConfigurationCallLog log;
    std::vector<std::unique_ptr<ReferenceConfigurableComponent>> components;     // index = id - 1
    RuntimeManager runtime;
    explicit World(std::size_t n, const std::vector<std::uint64_t>& registration, const std::vector<std::pair<std::uint64_t, std::uint64_t>>& edges) {
        components.resize(n);
        for (const std::uint64_t id : registration) {
            components[id - 1] = std::make_unique<ReferenceConfigurableComponent>(make_info(id), Defect::NONE, &log);
            assert(runtime.register_component(*components[id - 1]));
        }
        for (const auto& e : edges) assert(runtime.add_dependency(ComponentId{e.first}, ComponentId{e.second}));
    }
};

// Drivers: the real Runtime and deliberately broken re-implementations of its forwarding.
using Driver = std::function<bool(World&, const Configuration&)>;
bool real_runtime(World& w, const Configuration& c) { return static_cast<bool>(w.runtime.configure(c)); }
struct DriverFlags { bool retry{false}, rollback{false}, keep_going{false}, copy{false}, stop_early{false}, swallow{false}, invert{false}, duplicate{false}; };
Driver sequence_driver(std::vector<std::uint64_t> order, DriverFlags fl) {
    return [=](World& w, const Configuration& c) {
        bool ok = true;
        std::vector<std::uint64_t> done;
        std::size_t n = 0;
        for (const std::uint64_t id : order) {
            if (fl.stop_early && ++n == order.size()) break;                              // skips the last component
            const Configuration copied = c;
            const Configuration& given = fl.copy ? copied : c;
            auto r = w.components[id - 1]->configure(given);
            if (!r && fl.retry) r = w.components[id - 1]->configure(given);
            if (!r) {
                ok = false;
                if (fl.rollback) for (const std::uint64_t d : done) (void)w.components[d - 1]->configure(Configuration{});   // "rolls back" earlier components
                if (!fl.keep_going) break;
            } else done.push_back(id);
        }
        if (fl.duplicate) (void)w.components[order.front() - 1]->configure(c);               // one extra, trailing call
        if (fl.swallow) return true;                                                      // reports success although a component failed
        if (fl.invert) return !ok;                                                        // reports failure although none failed
        return ok;
    };
}

void test_the_forwarding_check_passes_for_the_real_runtime_and_detects_broken_drivers() {
    const std::vector<std::uint64_t> registration{3, 1, 4, 2};
    const std::vector<std::pair<std::uint64_t, std::uint64_t>> edges{{2, 1}, {3, 2}, {4, 3}};      // dependency order 1, 2, 3, 4
    const std::vector<std::uint64_t> order{1, 2, 3, 4};
    for (const std::uint64_t failing : {std::uint64_t{0}, std::uint64_t{1}, std::uint64_t{3}, std::uint64_t{4}}) {
        const Configuration given = reference_valid(0);
        auto run = [&](const Driver& driver) {
            World w(4, registration, edges);
            if (failing != 0) w.components[failing - 1]->fail_at = FailAt::SEMANTIC;
            const bool ok = driver(w, given);
            return conf::check_runtime_forwarding(w.log, given, conf::ForwardingExpectation{order, failing}, ok);
        };
        auto detects = [&](const Driver& d, const char* fragment) { return run(d).find(fragment) != std::string::npos; };
        assert(run(real_runtime).empty());                                                      // the real Runtime conforms
        assert(run(sequence_driver(order, {})).empty());                                        // and so does a correct hand-written driver
        assert(detects(sequence_driver({4, 3, 2, 1}, {}), "dependency order"));                // wrong order
        assert(detects(sequence_driver(order, DriverFlags{.copy = true}), "caller's own Configuration"));   // a copy instead of the caller's object
        if (failing == 0) {
            assert(detects(sequence_driver(order, DriverFlags{.stop_early = true}), "fewer configure() calls"));
            assert(detects(sequence_driver(order, DriverFlags{.duplicate = true}), "more configure() calls"));
            assert(detects(sequence_driver(order, DriverFlags{.invert = true}), "although no component failed"));
        } else {
            // a retry, a rollback and a continued sequence all show up as calls after a failure
            assert(detects(sequence_driver(order, DriverFlags{.retry = true}), "continued after a failure"));
            assert(detects(sequence_driver(order, DriverFlags{.swallow = true}), "although a component failed"));
            if (failing != 1) assert(detects(sequence_driver(order, DriverFlags{.rollback = true}), "continued after a failure"));
            if (failing != 4) assert(detects(sequence_driver(order, DriverFlags{.keep_going = true}), "continued after a failure"));
        }
    }
    // The same through the real Runtime after the topology is fixed (configure from STOPPED uses the fixed order).
    for (const std::uint64_t failing : {std::uint64_t{0}, std::uint64_t{2}}) {
        World w(4, registration, edges);
        assert(w.runtime.configure(reference_valid(0)) && w.runtime.initialize() && w.runtime.start() && w.runtime.stop());
        w.log.clear();
        if (failing != 0) w.components[failing - 1]->fail_at = FailAt::SEMANTIC;
        const Configuration again = reference_valid(1);
        const bool ok = real_runtime(w, again);
        assert(conf::check_runtime_forwarding(w.log, again, conf::ForwardingExpectation{order, failing}, ok).empty());
        assert(w.runtime.state() == LifecycleState::STOPPED && w.runtime.fault_error() == nullptr);
    }
    // The "sequence continued after a failure" clause: a driver that reaches the expected count by skipping a failed
    // component's successors but calling an earlier one again is caught by the count; the clause guards a failure that is
    // not the last logged call.
    ConfigurationCallLog log;
    const Configuration given = reference_valid(0);
    log.push_back(ConfigurationCall{1, &given, false, ErrorCode::CONFIGURATION_ERROR});
    log.push_back(ConfigurationCall{2, &given, true, ErrorCode::NONE});
    assert(conf::check_runtime_forwarding(log, given, conf::ForwardingExpectation{{1, 2}, 0}, false).find("continued after a failure") != std::string::npos);
}

} // namespace

int main() {
    test_the_reference_component_conforms();
    test_every_defect_is_detected_by_the_clause_written_for_it();
    test_the_reference_component_schema_and_staging();
    test_injected_failures_strike_after_the_state_check_and_leave_the_state_untouched();
    test_counters_and_the_recorded_address();
    test_the_forwarding_check_passes_for_the_real_runtime_and_detects_broken_drivers();
    return 0;
}
