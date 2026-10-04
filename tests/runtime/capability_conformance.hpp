//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : capability_conformance.hpp
// Description : Reusable conformance checks of the capability set, provider and requirement contracts (test only).
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-CAP-010
// API         : CORE-TEST-CAPABILITY-CONFORMANCE
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#pragma once
#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <kritva/core/core.hpp>

#include "reference_capability.hpp"

namespace kritva::core::runtime::conformance {

using contract::SetFixture;
using contract::RequirementFixture;
using contract::EvaluationResult;
using SetFactory = std::function<std::unique_ptr<SetFixture>()>;

namespace detail {
inline Capability cap(std::uint64_t id, const std::string& name, Version v = {}) { return Capability{CapabilityId{id}, name, v}; }
inline std::vector<std::uint64_t> ids(const SetFixture& s) { std::vector<std::uint64_t> out; for (const Capability& c : s.all()) out.push_back(c.id.value()); return out; }
} // namespace detail

/// CORE-CAP-006 / CORE-CAP-005: returns the first violation of the CapabilitySet contract, or an empty string.
inline std::string check_set_contract(const SetFactory& make) {
    using detail::cap; using detail::ids;
    {   // empty
        auto s = make();
        if (s->size() != 0 || !s->all().empty() || s->contains(CapabilityId{1}) || s->find(CapabilityId{1})) return "a new set is not empty";
    }
    {   // at most one entry per identity, replacement in place, first-insertion order, latest wins
        auto s = make();
        s->add(cap(5, "five", Version{1, 0, 0}));
        s->add(cap(3, "three", Version{1, 0, 0}));
        s->add(cap(9, "nine", Version{1, 0, 0}));
        s->add(cap(3, "three-v2", Version{2, 0, 0}));
        s->add(cap(5, "five-v2", Version{2, 1, 0}));
        if (s->size() != 3 || ids(*s).size() != 3) return "an identity is present more than once";
        if (ids(*s) != std::vector<std::uint64_t>{5, 3, 9}) return "the observable order is not first-insertion order (or a replacement moved an entry)";
        if (!s->find(CapabilityId{3}) || s->find(CapabilityId{3})->name != "three-v2" || s->find(CapabilityId{5})->name != "five-v2") return "a replacement did not replace the name";
        if (s->find(CapabilityId{3})->version != Version{2, 0, 0} || s->find(CapabilityId{5})->version != Version{2, 1, 0}) return "a replacement did not replace the version";
        s->add(cap(7, "seven"));
        if (ids(*s) != std::vector<std::uint64_t>{5, 3, 9, 7}) return "a new identity was not appended";
    }
    {   // a version is never ordered: an older version added later still replaces
        auto s = make();
        s->add(cap(1, "x", Version{5, 0, 0}));
        s->add(cap(1, "x", Version{1, 0, 0}));
        if (!s->find(CapabilityId{1}) || s->find(CapabilityId{1})->version != Version{1, 0, 0}) return "a lower version did not replace a higher one (versions must not be ordered)";
        s->add(cap(1, "x", Version{99, 99, 99}));
        if (s->find(CapabilityId{1})->version != Version{99, 99, 99}) return "a replacement version was not stored";
    }
    {   // identity is the only key; names and versions repeat; lookalikes prove nothing; an empty name is findable
        auto s = make();
        s->add(cap(1, "same", Version{1, 0, 0}));
        s->add(cap(2, "same", Version{1, 0, 0}));
        s->add(cap(20, "capability-21", Version{}));
        s->add(cap(30, "", Version{}));
        if (s->size() != 4) return "names or versions that repeat across identities were merged or counted by name";
        if (s->contains(CapabilityId{21}) || s->find(CapabilityId{21})) return "a capability was found by its name";
        if (!s->contains(CapabilityId{30}) || !s->find(CapabilityId{30})) return "an entry with an empty name cannot be found";
        if (s->contains(CapabilityId{4}) || s->find(CapabilityId{4})) return "an absent identity was found";
    }
    {   // the invalid identity is an ordinary key
        auto s = make();
        s->add(cap(0, "no-identity"));
        s->add(cap(1, "one"));
        s->add(cap(0, "no-identity-again"));
        if (s->size() != 2 || ids(*s) != std::vector<std::uint64_t>{0, 1}) return "the invalid identity was dropped or duplicated";
        if (!s->find(CapabilityId{}) || s->find(CapabilityId{})->name != "no-identity-again") return "the invalid identity is not an ordinary key";
    }
    {   // independent snapshots
        auto a = make();
        a->add(cap(1, std::string(300, 'n'), Version{1, 0, 0}));
        a->add(cap(2, "two"));
        auto b = a->copy();
        a->add(cap(1, "changed", Version{9, 9, 9}));
        a->add(cap(3, "three"));
        if (b->size() != 2 || b->contains(CapabilityId{3}) || b->find(CapabilityId{1})->name != std::string(300, 'n')) return "a change to the source showed up in its copy";
        b->add(cap(4, "four"));
        if (a->contains(CapabilityId{4})) return "a change to a copy showed up in its source";
    }
    {   // growth is not limited and the same sequence gives the same state
        auto a = make(); auto b = make();
        for (std::uint64_t i = 1; i <= 30; ++i) { a->add(cap(i, "n")); b->add(cap(i, "n")); }
        if (a->size() != 30 || b->size() != 30) return "the set stopped growing";
        if (ids(*a) != ids(*b)) return "the same sequence of adds gave a different state";
    }
    return {};
}

/// CORE-CAP-004: returns the first violation of the provider-snapshot contract, or an empty string. `component` is the
/// provider under test; `expected` is what it is meant to declare; `prepare_start` brings it to READY.
inline std::string check_provider_contract(Component& component, const CapabilitySet& expected, const std::function<bool()>& drive_to_running) {
    const LifecycleState before = component.lifecycle_state();
    const CapabilitySet first = component.capabilities();
    const CapabilitySet second = component.capabilities();
    if (component.lifecycle_state() != before) return "querying capabilities() changed the lifecycle state";
    if (first.size() != second.size()) return "two consecutive capabilities() calls differ in size";
    for (const Capability& c : first.all()) {
        const Capability* o = second.find(c.id);
        if (o == nullptr || o->name != c.name || !(o->version == c.version)) return "two consecutive capabilities() calls differ in content";
    }
    if (first.size() != expected.size()) return "the declaration has a different number of entries than expected";
    for (const Capability& c : expected.all()) {
        const Capability* o = first.find(c.id);
        if (o == nullptr || o->name != c.name || !(o->version == c.version)) return "the declaration is not the expected one";
    }
    for (const Capability& c : first.all()) if (!c.id.valid()) return "the provider published an invalid capability identity";
    CapabilitySet mutated = first;                               // a returned snapshot is independent of the provider
    mutated.add(Capability{CapabilityId{777}, "local-change", Version{}});
    if (component.capabilities().contains(CapabilityId{777})) return "a change to a returned snapshot showed up in the provider";
    if (!drive_to_running()) return "the provider could not be driven to RUNNING";
    const CapabilitySet running = component.capabilities();
    if (running.size() != expected.size()) return "the declaration follows the lifecycle";
    return {};
}

/// CORE-CAP-007 / CORE-CAP-008: drives an evaluator over scenarios and compares it with an independent identity oracle.
inline std::string check_requirement_contract(const contract::RequirementFactory& make) {
    using detail::cap;
    // oracle: a requirement is missing exactly when its identity is not in the provision
    auto expect = [](const CapabilitySet& provision, const std::vector<std::pair<std::uint64_t, bool>>& items) {
        EvaluationResult e;
        for (const auto& [id, required] : items) if (!provision.contains(CapabilityId{id})) (required ? e.missing_required : e.missing_optional).push_back(id);
        e.check_ok = e.missing_required.empty();
        return e;
    };
    auto run = [&](const CapabilitySet& provision, const std::string& pname, Version pver, const std::vector<std::pair<std::uint64_t, bool>>& items, std::string& error) {
        auto f = make(provision, pname, pver);
        for (const auto& [id, required] : items) if (!f->declare(CapabilityId{id}, required)) { error = "a valid declaration was rejected"; return false; }
        const EvaluationResult got = f->evaluate();
        const EvaluationResult want = expect(provision, items);
        if (got.missing_required != want.missing_required) { error = "the missing required items differ from the identity oracle (or are not in declaration order)"; return false; }
        if (got.missing_optional != want.missing_optional) { error = "the missing optional items differ from the identity oracle (or are not in declaration order)"; return false; }
        if (got.check_ok != want.check_ok) { error = "the overall check differs from the identity oracle (an optional item decided, or a required one was ignored)"; return false; }
        const EvaluationResult again = f->evaluate();
        if (again.missing_required != got.missing_required || again.missing_optional != got.missing_optional || again.check_ok != got.check_ok) { error = "two evaluations of the same input differ"; return false; }
        if (f->side_effects() != 0) { error = "evaluation had a side effect"; return false; }
        if (!items.empty() && f->snapshots() != 2 * f->passes()) { error = "evaluation did not take exactly one provider snapshot per evaluation"; return false; }
        return true;
    };
    std::string error;
    {   // identity-only matching: names, versions, provider information and lookalikes prove nothing
        CapabilitySet provision;
        provision.add(cap(10, "capability-11", Version{0, 0, 0}));       // a name that looks like another identity, a zero version
        provision.add(cap(12, "gpio", Version{3, 0, 0}));
        provision.add(cap(0, "no-identity", Version{1, 0, 0}));          // storable, never satisfies anything
        for (const auto& [pname, pver] : std::vector<std::pair<std::string, Version>>{{"plain", Version{0, 0, 0}}, {"vendor-11-13-linux", Version{11, 13, 13}}, {"", Version{13, 0, 0}}}) {
            if (!run(provision, pname, pver, {{10, true}, {11, true}, {12, false}, {13, true}, {14, false}}, error)) return error;
        }
    }
    {   // a version, however low, still satisfies by identity
        CapabilitySet provision;
        provision.add(cap(1, "x", Version{0, 0, 0}));
        provision.add(cap(2, "x", Version{99, 99, 99}));
        if (!run(provision, "p", Version{}, {{1, true}, {2, true}}, error)) return error;
    }
    {   // missing items are reported in declaration order, required before optional kinds
        CapabilitySet provision;
        provision.add(cap(5, "x"));
        if (!run(provision, "p", Version{}, {{9, true}, {8, true}, {5, true}, {7, false}, {6, false}}, error)) return error;
    }
    {   // only optional items missing: the check still succeeds (an OPTIONAL item never decides); a missing required one fails it
        CapabilitySet provision;
        provision.add(cap(1, "x"));
        if (!run(provision, "p", Version{}, {{1, true}, {2, false}, {3, false}}, error)) return error;
        if (!run(provision, "p", Version{}, {{2, false}}, error)) return error;
        if (!run(provision, "p", Version{}, {{1, false}, {4, true}}, error)) return error;
    }
    {   // no capability requirement: the provider is not asked
        auto f = make(CapabilitySet{}, "p", Version{});
        (void)f->evaluate();
        if (f->snapshots() != 0) return "a snapshot was taken although no capability is required";
    }
    {   // declaration rules: an invalid identity cannot be required, a duplicate is decided by identity, rejection is atomic
        auto f = make(CapabilitySet{}, "p", Version{});
        if (f->declare(CapabilityId{}, true)) return "a requirement was allowed to name the invalid identity";
        if (f->declared_count() != 0) return "a rejected declaration changed the declaration";
        if (!f->declare(CapabilityId{5}, true)) return "a valid declaration was rejected";
        if (f->declare(CapabilityId{5}, true)) return "a duplicate declaration was accepted";
        if (f->declare(CapabilityId{5}, false)) return "a duplicate was decided by (identity, level) instead of identity";
        if (f->declared_count() != 1) return "a rejected duplicate changed the declaration";
    }
    return {};
}
} // namespace kritva::core::runtime::conformance
