//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : reference_capability.hpp
// Description : Test-only reference capability harness (set, provider and requirement fixtures with broken variants).
//
// Component   : Kritva Core
// Module      : Tests
// Layer       : Core Foundation
//
// Requirements: CORE-CAP-010
// API         : CORE-TEST-CAPABILITY-HARNESS
//
// Author      : KritvaOS Core Team
// Created     : 05-10-2026
//==============================================================================

#pragma once

// TEST SUPPORT ONLY (never compiled into or installed with the production library).
//
// Reusable, deterministic fixtures for the R0.9 capability contracts, built only on public APIs. They add
// nothing to Core, need no platform-specific code and model no real component:
//   - SetFixture        what a conformance check needs to drive ONE capability-set implementation;
//                       RealSetFixture wraps the production CapabilitySet; BrokenSetFixture is a
//                       deliberately defective independent implementation (see SetDefect);
//   - ProviderFixture   what a check needs to drive ONE capability provider (a Component);
//                       ReferenceCapabilityProvider is a conforming reference Component with a controlled,
//                       observable capability snapshot and a menu of ProviderDefects;
//   - RequirementFixture  what a check needs to drive ONE requirement evaluator over a given provision;
//                       RealRequirementFixture wraps the production PlatformRequirements, evaluate() and
//                       check_required() over the test-only ReferencePlatform; BrokenRequirementFixture is a
//                       deliberately defective independent evaluator (see RequirementDefect).

#include <algorithm>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <kritva/core/core.hpp>

#include "../contract/reference_component.hpp"
#include "../platform/reference_platform.hpp"

namespace kritva::core::runtime::contract {

// ---------------------------------------------------------------------------------------------------------------
// capability sets
// ---------------------------------------------------------------------------------------------------------------
class SetFixture {
public:
    virtual ~SetFixture() = default;
    virtual void add(Capability capability) = 0;
    [[nodiscard]] virtual bool contains(CapabilityId id) const = 0;
    [[nodiscard]] virtual std::optional<Capability> find(CapabilityId id) const = 0;
    [[nodiscard]] virtual std::vector<Capability> all() const = 0;                 // in observable order
    [[nodiscard]] virtual std::size_t size() const = 0;
    [[nodiscard]] virtual std::unique_ptr<SetFixture> copy() const = 0;            // an independent snapshot
};

class RealSetFixture final : public SetFixture {
public:
    RealSetFixture() = default;
    explicit RealSetFixture(CapabilitySet set) : set_(std::move(set)) {}
    void add(Capability c) override { set_.add(std::move(c)); }
    bool contains(CapabilityId id) const override { return set_.contains(id); }
    std::optional<Capability> find(CapabilityId id) const override { const Capability* p = set_.find(id); return p != nullptr ? std::optional<Capability>(*p) : std::nullopt; }
    std::vector<Capability> all() const override { return set_.all(); }
    std::size_t size() const override { return set_.size(); }
    std::unique_ptr<SetFixture> copy() const override { return std::make_unique<RealSetFixture>(set_); }
private:
    CapabilitySet set_;
};

enum class SetDefect : std::uint8_t {
    NONE,
    DUPLICATES_ALLOWED,         // re-adding an identity appends a second entry
    REPLACE_MOVES_TO_END,       // a replacement erases the old entry and appends
    REPLACE_KEEPS_OLD_VERSION,  // a replacement updates the name but not the version
    REPLACE_KEEPS_HIGHER_VERSION, // a replacement keeps whichever version is higher (ordering a version)
    ORDER_SORTED,               // entries are kept sorted by id instead of first-insertion order
    ORDER_NEWEST_FIRST,         // new identities are inserted at the front
    FIND_BY_NAME,               // lookup also matches the name
    FIND_IGNORES_EMPTY_NAME,    // an entry with an empty name cannot be found
    REJECTS_INVALID_ID,         // an entry with the invalid identity is silently dropped
    COPY_SHARES_STORAGE,        // a copy aliases the source's storage
    SIZE_COUNTS_NAMES,          // size() counts distinct names, not identities
    ADD_HAS_HIDDEN_LIMIT,       // the set silently stops growing after eight entries
    STARTS_NON_EMPTY,           // a new set already holds an entry
    REPLACE_KEEPS_OLD_NAME,     // a replacement updates the version but not the name
    LOSES_APPEND_AFTER_REPLACE, // the first new identity after a replacement is dropped
    REPLACE_KEEPS_LOWER_VERSION,// a replacement keeps whichever version is lower (ordering a version)
    REPLACE_IGNORED_AFTER_FIRST,// the second replacement of the same identity is ignored
    FINDS_ABSENT_IDS,           // an absent identity is answered with the first entry
    INVALID_ID_KEEPS_FIRST_NAME,// re-adding the invalid identity is ignored
    COPY_WRITES_THROUGH_TO_SOURCE, // a change to a copy also changes its source
    NONDETERMINISTIC_ORDER      // the observable order differs between instances given the same adds
};

class BrokenSetFixture final : public SetFixture {
public:
    explicit BrokenSetFixture(SetDefect defect) : defect_(defect), storage_(std::make_shared<std::vector<Capability>>()) {
        if (defect_ == SetDefect::STARTS_NON_EMPTY) storage_->push_back(Capability{CapabilityId{424242}, "pre-existing", Version{}});
        if (defect_ == SetDefect::NONDETERMINISTIC_ORDER) flip_ = (++instances_) % 2 == 0;
    }
    void add(Capability c) override {
        if (source_ != nullptr) source_->add(c);                                    // a copy that writes through to its source
        auto& v = *storage_;
        if (defect_ == SetDefect::REJECTS_INVALID_ID && !c.id.valid()) return;
        if (defect_ == SetDefect::ADD_HAS_HIDDEN_LIMIT && v.size() >= 8) return;
        for (std::size_t i = 0; i < v.size(); ++i) {
            if (v[i].id != c.id) continue;
            if (defect_ == SetDefect::DUPLICATES_ALLOWED) { v.push_back(std::move(c)); return; }
            if (defect_ == SetDefect::REPLACE_MOVES_TO_END) { v.erase(v.begin() + static_cast<std::ptrdiff_t>(i)); v.push_back(std::move(c)); return; }
            if (defect_ == SetDefect::REPLACE_KEEPS_OLD_VERSION) { v[i].name = std::move(c.name); return; }
            if (defect_ == SetDefect::REPLACE_KEEPS_OLD_NAME) { v[i].version = c.version; replaced_ = true; return; }
            if (defect_ == SetDefect::INVALID_ID_KEEPS_FIRST_NAME && !c.id.valid()) return;
            if (defect_ == SetDefect::REPLACE_KEEPS_LOWER_VERSION) { const Version keep = v[i].version.major < c.version.major ? v[i].version : c.version; v[i] = std::move(c); v[i].version = keep; replaced_ = true; return; }
            if (defect_ == SetDefect::LOSES_APPEND_AFTER_REPLACE) { v[i] = std::move(c); replaced_ = true; return; }
            if (defect_ == SetDefect::REPLACE_IGNORED_AFTER_FIRST) { if (replacements_[c.id.value()]++ >= 1) return; v[i] = std::move(c); return; }
            if (defect_ == SetDefect::REPLACE_KEEPS_HIGHER_VERSION) { const Version keep = v[i].version.major > c.version.major ? v[i].version : c.version; v[i] = std::move(c); v[i].version = keep; return; }
            v[i] = std::move(c);
            return;
        }
        if (defect_ == SetDefect::LOSES_APPEND_AFTER_REPLACE && replaced_) { replaced_ = false; return; }
        if (defect_ == SetDefect::ORDER_NEWEST_FIRST) { v.insert(v.begin(), std::move(c)); return; }
        v.push_back(std::move(c));
        if (defect_ == SetDefect::ORDER_SORTED) std::sort(v.begin(), v.end(), [](const Capability& a, const Capability& b) { return a.id.value() < b.id.value(); });
    }
    bool contains(CapabilityId id) const override { return find(id).has_value(); }
    std::optional<Capability> find(CapabilityId id) const override {
        for (const Capability& c : *storage_) {
            if (c.id == id) { if (defect_ == SetDefect::FIND_IGNORES_EMPTY_NAME && c.name.empty()) continue; return c; }
            if (defect_ == SetDefect::FINDS_ABSENT_IDS && id.value() == 4) return c;
            if (defect_ == SetDefect::FIND_BY_NAME && c.name == "capability-" + std::to_string(id.value())) return c;
        }
        return std::nullopt;
    }
    std::vector<Capability> all() const override {
        std::vector<Capability> out = *storage_;
        if (flip_ && out.size() > 20) std::reverse(out.begin(), out.end());
        return out;
    }
    std::size_t size() const override {
        if (defect_ != SetDefect::SIZE_COUNTS_NAMES) return storage_->size();
        std::vector<std::string> names;
        for (const Capability& c : *storage_) if (std::find(names.begin(), names.end(), c.name) == names.end()) names.push_back(c.name);
        return names.size();
    }
    std::unique_ptr<SetFixture> copy() const override {
        auto c = std::make_unique<BrokenSetFixture>(defect_);
        if (defect_ == SetDefect::COPY_SHARES_STORAGE) c->storage_ = storage_;
        else *c->storage_ = *storage_;
        if (defect_ == SetDefect::COPY_WRITES_THROUGH_TO_SOURCE) c->source_ = const_cast<BrokenSetFixture*>(this);
        return c;
    }
private:
    SetDefect defect_;
    std::shared_ptr<std::vector<Capability>> storage_;
    BrokenSetFixture* source_{nullptr};
    bool replaced_{false};
    std::map<std::uint64_t, int> replacements_;
    bool flip_{false};
    static inline int instances_{0};
};

// ---------------------------------------------------------------------------------------------------------------
// providers
// ---------------------------------------------------------------------------------------------------------------
enum class ProviderDefect : std::uint8_t {
    NONE,
    CHANGES_EVERY_CALL,         // each capabilities() call returns a different declaration
    QUERY_CHANGES_LIFECYCLE,    // capabilities() starts the component
    DECLARATION_FOLLOWS_LIFECYCLE, // the declaration grows while RUNNING
    PUBLISHES_INVALID_ID,       // the declaration contains an entry with the invalid identity
    CHANGES_CONTENT_NOT_SIZE,   // each call returns the same number of entries with a different version
    DROPS_ONE_ENTRY,            // the declaration silently omits an entry it should declare
    SWAPS_VERSION               // the declaration reports the wrong version for an entry
};

/// A conforming Component (lifecycle from the plain ReferenceComponent) whose capabilities() returns a controlled,
/// observable by-value snapshot of `declared`. A test mutates `declared` to model a provider that declares something else.
class ReferenceCapabilityProvider final : public ReferenceComponent {
public:
    ReferenceCapabilityProvider(ComponentInfo info, CapabilitySet declared, ProviderDefect defect = ProviderDefect::NONE)
        : ReferenceComponent(std::move(info)), declared_(std::move(declared)), defect_(defect) {}

    [[nodiscard]] CapabilitySet capabilities() const override {
        ++queries;
        CapabilitySet out = declared_;
        if (defect_ == ProviderDefect::CHANGES_EVERY_CALL) for (int k = 1; k <= queries; ++k) out.add(Capability{CapabilityId{9000 + static_cast<std::uint64_t>(k)}, "noise", Version{}});   // grows with every call
        if (defect_ == ProviderDefect::DECLARATION_FOLLOWS_LIFECYCLE && lifecycle_state() == LifecycleState::RUNNING) out.add(Capability{CapabilityId{8000}, "running-only", Version{}});
        if (defect_ == ProviderDefect::PUBLISHES_INVALID_ID) out.add(Capability{CapabilityId{}, "no-identity", Version{}});
        if (defect_ == ProviderDefect::CHANGES_CONTENT_NOT_SIZE) { CapabilitySet changed; for (const Capability& c : out.all()) changed.add(Capability{c.id, c.name, Version{static_cast<std::uint32_t>(queries), 0, 0}}); out = changed; }
        if (defect_ == ProviderDefect::DROPS_ONE_ENTRY) { CapabilitySet fewer; bool first = true; for (const Capability& c : out.all()) { if (first) { first = false; continue; } fewer.add(c); } out = fewer; }
        if (defect_ == ProviderDefect::SWAPS_VERSION) { CapabilitySet swapped; for (const Capability& c : out.all()) swapped.add(Capability{c.id, c.name, Version{c.version.major + 100, 0, 0}}); out = swapped; }
        if (defect_ == ProviderDefect::QUERY_CHANGES_LIFECYCLE) { auto* self = const_cast<ReferenceCapabilityProvider*>(this); if (lifecycle_state() == LifecycleState::READY) (void)self->start(); }
        return out;
    }

    CapabilitySet declared_;
    mutable int queries{0};
private:
    ProviderDefect defect_;
};

// ---------------------------------------------------------------------------------------------------------------
// requirement evaluation
// ---------------------------------------------------------------------------------------------------------------
struct EvaluationResult {
    std::vector<std::uint64_t> missing_required;      // capability ids, declaration order
    std::vector<std::uint64_t> missing_optional;
    bool check_ok{false};                             // the check_required() outcome
};

class RequirementFixture {
public:
    virtual ~RequirementFixture() = default;
    [[nodiscard]] virtual bool declare(CapabilityId id, bool required) = 0;       // false when rejected
    [[nodiscard]] virtual std::size_t declared_count() const = 0;
    [[nodiscard]] virtual EvaluationResult evaluate() = 0;
    [[nodiscard]] virtual std::size_t snapshots() const = 0;                      // provider snapshots taken so far
    [[nodiscard]] virtual std::size_t side_effects() const = 0;                   // service calls, starts, stops or any change made so far
    /// How many underlying evaluations one evaluate() performs (each is entitled to exactly one provider snapshot).
    [[nodiscard]] virtual std::size_t passes() const { return 1; }
};

/// Builds an evaluator over a provision plus the provider's own descriptive information.
using RequirementFactory = std::function<std::unique_ptr<RequirementFixture>(const CapabilitySet& provision, const std::string& provider_name, Version provider_version)>;

class RealRequirementFixture final : public RequirementFixture {
public:
    RealRequirementFixture(const CapabilitySet& provision, const std::string& provider_name, Version provider_version)
        : platform_(make_config(provision, provider_name, provider_version)) {}
    bool declare(CapabilityId id, bool required) override { return static_cast<bool>(requirements_.add_capability(id, required ? platform::Requirement::REQUIRED : platform::Requirement::OPTIONAL)); }
    std::size_t declared_count() const override { return requirements_.capabilities().size(); }
    EvaluationResult evaluate() override {
        const platform::PlatformContext context(platform_);
        const auto report = platform::evaluate(requirements_, context);
        EvaluationResult out;
        for (const auto& r : report.missing_required_capabilities) out.missing_required.push_back(r.id.value());
        for (const auto& r : report.missing_optional_capabilities) out.missing_optional.push_back(r.id.value());
        out.check_ok = static_cast<bool>(platform::check_required(requirements_, context));
        return out;
    }
    std::size_t snapshots() const override { return platform_.controls().adapter_queries(); }      // only capabilities() queries the adapter here
    std::size_t side_effects() const override { return platform_.controls().log().size(); }
    std::size_t passes() const override { return 2; }                              // platform::evaluate() and platform::check_required()
private:
    static platform::testing::ReferencePlatform::Config make_config(const CapabilitySet& provision, const std::string& name, Version version) {
        platform::testing::ReferencePlatform::Config c;
        c.capabilities = provision;
        c.info = platform::PlatformInfo{name, version};
        return c;
    }
    platform::testing::ReferencePlatform platform_;
    platform::PlatformRequirements requirements_;
};

enum class RequirementDefect : std::uint8_t {
    NONE,
    MATCH_BY_NAME,              // also satisfied by a capability whose name looks like the id
    MATCH_BY_VERSION,           // satisfied only when the provided version is non-zero
    MATCH_BY_PROVIDER_NAME,     // also satisfied when the provider's name mentions the id
    MATCH_BY_PROVIDER_VERSION,  // also satisfied when the provider's version major equals the id
    SNAPSHOT_PER_ITEM,          // one provider snapshot per declared requirement
    SNAPSHOT_EVEN_IF_NONE,      // a snapshot even when no capability is required
    SIDE_EFFECT,                // evaluation starts something
    OPTIONAL_DECIDES,           // a missing optional item makes the check fail
    REQUIRED_IGNORED,           // a missing required item does not make the check fail
    DUPLICATES_BY_PAIR,         // a duplicate is decided by (identity, level)
    ACCEPTS_INVALID_ID,         // a requirement may name the invalid identity
    NON_ATOMIC_REJECTION,       // a rejected duplicate still changes the declaration count
    REPORT_ORDER_REVERSED,      // missing items are reported in reverse declaration order
    NONDETERMINISTIC,           // the second evaluation reports differently
    OPTIONAL_NEVER_REPORTED,    // a missing optional item is not reported
    INVALID_REJECTION_CHANGES_STATE, // rejecting the invalid identity still records a phantom declaration
    ACCEPTS_EXACT_DUPLICATE     // declaring the same identity at the same level twice is accepted
};

class BrokenRequirementFixture final : public RequirementFixture {
public:
    BrokenRequirementFixture(RequirementDefect defect, CapabilitySet provision, std::string provider_name, Version provider_version)
        : defect_(defect), provision_(std::move(provision)), provider_name_(std::move(provider_name)), provider_version_(provider_version) {}
    bool declare(CapabilityId id, bool required) override {
        if (!id.valid() && defect_ != RequirementDefect::ACCEPTS_INVALID_ID) {
            if (defect_ == RequirementDefect::INVALID_REJECTION_CHANGES_STATE) phantom_declarations_++;
            return false;
        }
        for (const Item& it : items_) {
            if (it.id != id) continue;
            if (defect_ == RequirementDefect::DUPLICATES_BY_PAIR && it.required != required) break;
            if (defect_ == RequirementDefect::ACCEPTS_EXACT_DUPLICATE && it.required == required) { items_.push_back(Item{id, required, false}); return true; }
            if (defect_ == RequirementDefect::NON_ATOMIC_REJECTION) items_.push_back(Item{id, required, true});
            return false;
        }
        items_.push_back(Item{id, required, false});
        return true;
    }
    std::size_t declared_count() const override { std::size_t n = phantom_declarations_; for (const Item& it : items_) if (!it.phantom) ++n; return defect_ == RequirementDefect::NON_ATOMIC_REJECTION ? items_.size() : n; }
    EvaluationResult evaluate() override {
        ++evaluations_;
        bool any_capability = !items_.empty();
        if (defect_ == RequirementDefect::SNAPSHOT_EVEN_IF_NONE || any_capability) snapshots_ += defect_ == RequirementDefect::SNAPSHOT_PER_ITEM ? items_.size() : 1;
        if (defect_ == RequirementDefect::SIDE_EFFECT) ++effects_;
        EvaluationResult out;
        for (const Item& it : items_) {
            if (satisfied(it.id)) continue;
            (it.required ? out.missing_required : out.missing_optional).push_back(it.id.value());
        }
        if (defect_ == RequirementDefect::OPTIONAL_NEVER_REPORTED) out.missing_optional.clear();
        if (defect_ == RequirementDefect::REPORT_ORDER_REVERSED) { std::reverse(out.missing_required.begin(), out.missing_required.end()); std::reverse(out.missing_optional.begin(), out.missing_optional.end()); }
        if (defect_ == RequirementDefect::NONDETERMINISTIC && evaluations_ % 2 == 0) std::reverse(out.missing_required.begin(), out.missing_required.end());
        out.check_ok = out.missing_required.empty() && (defect_ != RequirementDefect::OPTIONAL_DECIDES || out.missing_optional.empty());
        if (defect_ == RequirementDefect::REQUIRED_IGNORED) out.check_ok = true;
        return out;
    }
    std::size_t snapshots() const override { return snapshots_; }
    std::size_t side_effects() const override { return effects_; }
private:
    struct Item { CapabilityId id; bool required; bool phantom; };
    bool satisfied(CapabilityId id) const {
        for (const Capability& c : provision_.all()) {
            if (c.id == id) return defect_ != RequirementDefect::MATCH_BY_VERSION || c.version.major + c.version.minor + c.version.patch > 0;
            if (defect_ == RequirementDefect::MATCH_BY_NAME && c.name == "capability-" + std::to_string(id.value())) return true;
        }
        if (defect_ == RequirementDefect::MATCH_BY_PROVIDER_NAME && provider_name_.find(std::to_string(id.value())) != std::string::npos) return true;
        if (defect_ == RequirementDefect::MATCH_BY_PROVIDER_VERSION && provider_version_.major == id.value()) return true;
        return false;
    }
    RequirementDefect defect_;
    CapabilitySet provision_;
    std::string provider_name_;
    Version provider_version_;
    std::vector<Item> items_;
    std::size_t snapshots_{0}, effects_{0}, evaluations_{0}, phantom_declarations_{0};
};
} // namespace kritva::core::runtime::contract
